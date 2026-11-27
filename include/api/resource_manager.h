#ifndef API_RESOURCE_MANAGER
#define API_RESOURCE_MANAGER

#include "log.h"
#include <atomic>
#include <memory>
#include <string>
#include <thread>

#ifdef __cpp_lib_atomic_shared_ptr
#define atom_shared_ptr std::atomic<std::shared_ptr<const data_type>>
#define atom_load(data) data.load()
#define atom_store(data, val) data.store(val)
#else
#define atom_shared_ptr std::shared_ptr<const data_type>
#define atom_load(data) std::atomic_load(&data)
#define atom_store(data, val) std::atomic_store(&data, atom_shared_ptr(val))
#endif

// Thread-safe wrapper to asynchronously fetch resources from the api, caching the result
template <auto ApiCall> class cached_resource {
    using result_type = decltype(ApiCall());
    using data_type = typename result_type::first_type;

    const std::string name;

    std::atomic<bool> thread_exists{false};
    std::thread worker;

    // nullptr if data is unavailable (not yet fetched or network error)
    atom_shared_ptr data;

  private:
    void fetch_thread() {
        struct thread_guard {
            cached_resource<ApiCall>* resource;

            ~thread_guard() {
                resource->thread_exists = false;
                resource->thread_exists.notify_all();
            }
        } guard{this};

        auto result = ApiCall();
        if (!result.second.empty()) {
            LOG_INFO("Failed to fetch resource {}: {}", name, result.second);
            atom_store(data, nullptr);
            return;
        }
        LOG_DEBUG("Fetched {}", name);
        atom_store(data, std::make_shared<const data_type>(std::move(result.first)));
    }

  public:
    cached_resource(std::string name)
        : name(std::move(name)) {}

    ~cached_resource() {
        if (worker.joinable()) {
            worker.join();
        }
    }

    // Returns true if the data has already been fetched
    // Returns false and asynchronously fetches data if unavailable
    bool fetch() {
        if (atom_load(data)) {
            // Resource is cached
            return true;
        }

        bool expected = false;
        if (!thread_exists.compare_exchange_strong(expected, true)) {
            // Resource is already being fetched
            return false;
        }

        // Fetch resource
        if (worker.joinable()) {
            worker.join();
        }
        worker = std::thread(&cached_resource::fetch_thread, this);
        return false;
    }

    // Attempts to fetch data and blocks
    // Returns true if data is fetched
    // Returns false if failed to retrieve data (network error)
    bool await_fetch() {
        if (fetch()) {
            return true;
        }

        thread_exists.wait(true);
        return atom_load(data) != nullptr;
    }

    // Returns nullptr if data is unavailable (not yet fetched or network error)
    std::shared_ptr<const data_type> get_data() const { return atom_load(data); };

    // Clears the cached data
    // Won't cancel an existing fetch thread
    void clear_cache() { atom_store(data, nullptr); }
};

#endif
