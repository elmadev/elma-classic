#include "menu/replay_cache.h"
#include "debug/profiler.h"
#include "game/recorder.h"
#include "main.h"
#include "util/file_iter.h"
#include <filesystem>
#include <unordered_set>

replay_cache::~replay_cache() {
    stop_requested.store(true, std::memory_order_relaxed);
    if (worker.joinable()) {
        worker.join();
    }
}

void replay_cache::start() {
    if (worker.joinable()) {
        internal_error("replay_cache::start called more than once");
    }

    // Read metadata of all replay files on different thread
    worker = std::thread([this] {
        START_TIME(cache_timer);
        std::unordered_map<int, std::vector<std::string>> level_id_to_filenames_;
        std::unordered_map<std::string, int> filename_to_level_id_;

        std::filesystem::directory_iterator it;
        try {
            it = std::filesystem::directory_iterator("rec");
        } catch (const std::filesystem::filesystem_error&) {
            // rec/ directory doesn't exist - nothing to scan
            ready.store(true, std::memory_order_release);
            return;
        }

        for (const auto& entry : it) {
            if (stop_requested.load(std::memory_order_relaxed)) {
                return;
            }
            if (!entry.is_regular_file()) {
                continue;
            }
            const std::filesystem::path& path = entry.path();
            if (path.extension() != ".rec") {
                continue;
            }
            std::string stem = path.stem().string();
            if (stem.size() > MAX_REPLAY_NAME_LEN) {
                continue;
            }

            std::string filename = path.filename().string();
            auto header = recorder::read_header(filename);
            if (!header) {
                continue;
            }

            level_id_to_filenames_[header->level_id].push_back(filename);
            filename_to_level_id_[filename] = header->level_id;
        }

        {
            std::scoped_lock lock(mutex);
            // Initialize replay data from initial scan
            level_id_to_filenames = std::move(level_id_to_filenames_);
            filename_to_level_id = std::move(filename_to_level_id_);
            // Update data for any replays that were saved during the initial scan
            flush_pending_upserts();
            ready.store(true, std::memory_order_release);
        }
        END_TIME(cache_timer, "Replay cache")
    });
}

bool replay_cache::is_ready() const { return ready.load(std::memory_order_acquire); }

std::vector<std::string> replay_cache::filenames_for_level(int level_id) const {
    if (!is_ready()) {
        internal_error("replay_cache::filenames_for_level called before cache is ready");
    }

    std::scoped_lock lock(mutex);
    auto it = level_id_to_filenames.find(level_id);
    if (it != level_id_to_filenames.end()) {
        return it->second;
    }
    return {};
}

void replay_cache::upsert(const std::string& filename, int level_id) {
    std::scoped_lock lock(mutex);
    if (!ready.load(std::memory_order_acquire)) {
        // Initial start-up scan is in progress
        // We need to wait for it to complete before processing data
        pending_upserts.emplace_back(filename, level_id);
        return;
    }
    apply_upsert(filename, level_id);
}

void replay_cache::apply_upsert(const std::string& filename, int level_id) {
    // Check if we are overwriting an existing file
    auto it = filename_to_level_id.find(filename);
    if (it != filename_to_level_id.end()) {
        if (it->second == level_id) {
            return;
        }
        // If we are overwriting an existing file, we need to delete the old reference
        auto& old_bucket = level_id_to_filenames[it->second];
        std::erase(old_bucket, filename);
    }
    // Apply the new references
    level_id_to_filenames[level_id].push_back(filename);
    filename_to_level_id[filename] = level_id;
}

void replay_cache::flush_pending_upserts() {
    for (const auto& [filename, level_id] : pending_upserts) {
        apply_upsert(filename, level_id);
    }
    pending_upserts.clear();
}

void replay_cache::sync(const std::vector<std::string>& current_filenames) {
    if (!is_ready()) {
        internal_error("replay_cache::sync called before cache is ready");
    }

    std::scoped_lock lock(mutex);

    std::unordered_set<std::string> on_disk(current_filenames.begin(), current_filenames.end());

    // Remove deleted files
    std::vector<std::string> stale;
    for (const auto& [filename, _] : filename_to_level_id) {
        if (!on_disk.contains(filename)) {
            stale.push_back(filename);
        }
    }
    for (const std::string& filename : stale) {
        int level_id = filename_to_level_id[filename];
        auto& bucket = level_id_to_filenames[level_id];
        std::erase(bucket, filename);
        filename_to_level_id.erase(filename);
    }

    // Add new files
    for (const std::string& filename : current_filenames) {
        if (!filename_to_level_id.contains(filename)) {
            auto header = recorder::read_header(filename);
            if (header) {
                apply_upsert(filename, header->level_id);
            }
        }
    }
}
