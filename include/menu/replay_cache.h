#ifndef REPLAY_CACHE_H
#define REPLAY_CACHE_H

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class replay_cache {
    std::unordered_map<int, std::vector<std::string>> level_id_to_filenames_;
    std::unordered_map<std::string, int> filename_to_level_id_;
    std::vector<std::pair<std::string, int>> pending_upserts_;
    mutable std::mutex mutex_;
    std::atomic<bool> ready_{false};
    std::atomic<bool> stop_requested_{false};
    std::thread worker_;

    void apply_upsert(const std::string& filename, int level_id);
    // Apply pending upserts that were added while cache was still building
    // We can't apply them immediately because the cache is still being built
    void flush_pending_upserts();

  public:
    ~replay_cache();

    // Start background scan of rec/ directory
    void start();

    bool is_ready() const;

    // Thread-safe accessors - return copies
    std::vector<std::string> filenames_for_level(int level_id) const;

    // Called after saving a replay - updates existing entry or inserts new one
    void upsert(const std::string& filename, int level_id);

    // Sync cache with current filesystem state: add new files, remove stale entries
    void sync(const std::vector<std::string>& current_filenames);
};

#endif
