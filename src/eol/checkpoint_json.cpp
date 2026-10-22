#include "eol/checkpoint.h"
#include "eol/status_messages.h"
#include "main.h"
#include <format>
#include <fstream>

#define JSON_DIAGNOSTICS 1
#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;

constexpr int CHECKPOINT_JSON_VERSION = 1;
constexpr char FOLDER[] = "checkpoints";

void from_json(const json& j, checkpoint_state& v) {
    if (j.is_null()) {
        return;
    }
    double best_seconds = j.at("best_time").get<double>();
    v.best_centiseconds = (int)(best_seconds * 100.0);
    v.best_speed = j.at("best_speed").get<double>();
}

void to_json(json& j, const checkpoint_state& v) {
    if (!v.best_centiseconds) {
        return;
    }
    j = {
        {"best_time", (*v.best_centiseconds) / 100.0},
        {"best_speed", v.best_speed.value()},
    };
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(vect2, x, y);

NLOHMANN_JSON_SERIALIZE_ENUM(checkpoint::Polarity, {
                                                       {checkpoint::Polarity::Both, "both"},
                                                       {checkpoint::Polarity::Positive, "positive"},
                                                       {checkpoint::Polarity::Negative, "negative"},
                                                   });

void from_json(const json& j, checkpoint& v) {
    v.start.click_anchor = j.at("start").get<vect2>();
    v.end.click_anchor = j.at("end").get<vect2>();
    j.at("polarity").get_to(v.polarity);
    j.at("records").get_to(v.state);
}

void to_json(json& j, const checkpoint& v) {
    j = {
        {"start", v.start.click_anchor},
        {"end", v.end.click_anchor},
        {"polarity", v.polarity},
        {"records", v.state},
    };
}

void from_json(const json& j, checkpoints& v) {
    int version = j.at("json_version").get<int>();
    if (version > CHECKPOINT_JSON_VERSION) {
        throw json::other_error::create(
            -1, "File version too new! Please update EOL to the newest version!", &j);
    }
    j.at("crc").get_to(v.crc);
    j.at("object_offset").get_to(v.food_index_offset);
    j.at("linear_checkpoints").get_to(v.linear);
    j.at("object_checkpoints").get_to(v.objects);
}

void to_json(json& j, const checkpoints& v) {
    j = {
        {"json_version", CHECKPOINT_JSON_VERSION}, {"crc", v.crc},
        {"object_offset", v.food_index_offset},    {"linear_checkpoints", v.linear},
        {"object_checkpoints", v.objects},
    };
}

static std::filesystem::path level_to_json_path(const std::string& level_filename) {
    return std::filesystem::path(FOLDER) /
           std::filesystem::path(level_filename).replace_extension(".json");
}

void checkpoints::save_json() {
    std::filesystem::create_directory(FOLDER);
    std::filesystem::path json_path = level_to_json_path(level_filename);

    // No checkpoint data to save?
    if (linear.empty() && std::ranges::all_of(objects, [](const checkpoint_state& state) {
            return !state.best_centiseconds;
        })) {
        if (std::filesystem::exists(json_path)) {
            // Try to delete the file, but ignore any errors
            std::filesystem::remove(json_path);
        }
        return;
    }

    // Save checkpoint data
    std::ofstream h(json_path);
    if (!h.is_open()) {
        StatusMessages->add(
            std::format("Failed to save checkpoint data to file: {}", json_path.string()));
        return;
    }

    json j = *this;
    h << std::setw(4) << j;
    h.close();
}

std::unique_ptr<checkpoints> checkpoints::load_json(const std::string& level_filename) {
    std::filesystem::path json_path =
        std::filesystem::path(FOLDER) /
        std::filesystem::path(level_filename).replace_extension(".json");

    if (!std::filesystem::exists(json_path)) {
        return nullptr;
    }

    std::ifstream h(json_path);
    if (!h.is_open()) {
        internal_error(std::format("Failed to open checkpoint data file: {}", json_path.string()));
    }

    try {
        json j = json::parse(h);
        return std::make_unique<checkpoints>(j.get<checkpoints>());
    } catch (const json::exception& e) {
        internal_error(std::format("Failed to load checkpoint data from file: {}\nPlease delete or "
                                   "rename the corrupted file.\n{}",
                                   json_path.string(), e.what()));
        return nullptr;
    }
}
