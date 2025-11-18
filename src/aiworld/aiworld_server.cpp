/**
 * aiworld_server.cpp
 * Standalone AIWorld server process for rAthena (ZeroMQ IPC)
 * Runs independently, like char, login, and map servers.
 * Handles all AI logic, mission orchestration, and entity management.
 */

#include "aiworld_ipc.hpp"
#include "aiworld_utils.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <zmq.h>

#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>

// --- Multi-threaded AI Worker Pool for Autonomous NPCs ---
class AIWorkerPool {
public:
    AIWorkerPool(size_t num_workers = std::thread::hardware_concurrency()) : stop_flag(false) {
        for (size_t i = 0; i < num_workers; ++i) {
            workers.emplace_back([this, i] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] { return this->stop_flag || !this->tasks.empty(); });
                        if (this->stop_flag && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    ~AIWorkerPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop_flag = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
            worker.join();
    }

    template<class F>
    auto enqueue(F&& f) -> std::future<typename std::result_of<F()>::type> {
        using return_type = typename std::result_of<F()>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::forward<F>(f));
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop_flag)
                throw std::runtime_error("enqueue on stopped AIWorkerPool");
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop_flag;
};
using namespace aiworld;

class AIWorldServer {
public:
    AIWorldServer(const std::string& endpoint)
        : zmq_context(nullptr), zmq_socket(nullptr), running(false), server_endpoint(endpoint) {}

    ~AIWorldServer() {
        stop();
    }

    bool start() {
        zmq_context = zmq_ctx_new();
        if (!zmq_context) {
            log_error("AIWorldServer: ZeroMQ context creation failed.");
            return false;
        }
        zmq_socket = zmq_socket(zmq_context, ZMQ_REP);
        if (!zmq_socket) {
            log_error("AIWorldServer: ZeroMQ socket creation failed.");
            zmq_ctx_term(zmq_context);
            zmq_context = nullptr;
            return false;
        }
        if (zmq_bind(zmq_socket, server_endpoint.c_str()) != 0) {
            log_error("AIWorldServer: ZeroMQ bind failed: " + std::string(zmq_strerror(zmq_errno())));
            zmq_close(zmq_socket);
            zmq_socket = nullptr;
            zmq_ctx_term(zmq_context);
            zmq_context = nullptr;
            return false;
        }
        running = true;
        log_info("AIWorldServer started and listening on " + server_endpoint);
        server_thread = std::thread(&AIWorldServer::main_loop, this);
        return true;
    }

    void stop() {
        running = false;
        if (server_thread.joinable()) server_thread.join();
        if (zmq_socket) {
            int rc = zmq_close(zmq_socket);
            if (rc != 0) {
                log_error("AIWorldServer: Error closing ZeroMQ socket: " + std::string(zmq_strerror(zmq_errno())));
            }
            zmq_socket = nullptr;
        }
        if (zmq_context) {
            int rc = zmq_ctx_term(zmq_context);
            if (rc != 0) {
                log_error("AIWorldServer: Error terminating ZeroMQ context: " + std::string(zmq_strerror(zmq_errno())));
            }
            zmq_context = nullptr;
        }
        log_info("AIWorldServer stopped.");
    }

private:
    void* zmq_context;
    void* zmq_socket;
    std::atomic<bool> running;
    std::thread server_thread;
    std::string server_endpoint;

    void main_loop() {
        // Multi-threaded AI worker pool for autonomous NPCs
        static AIWorkerPool ai_pool(std::thread::hardware_concurrency());

        while (running) {
            zmq_msg_t zmq_msg;
            zmq_msg_init(&zmq_msg);
            int rc = zmq_msg_recv(&zmq_msg, zmq_socket, 0);
            if (rc < 0) {
                log_error("AIWorldServer: Error receiving ZeroMQ message: " + std::string(zmq_strerror(zmq_errno())));
                zmq_msg_close(&zmq_msg);
                continue;
            }
            std::string msg_str(static_cast<char*>(zmq_msg_data(&zmq_msg)), zmq_msg_size(&zmq_msg));
            zmq_msg_close(&zmq_msg);

            log_info("AIWorldServer received message: " + msg_str);

            // Parse full message (type, correlation_id, payload)
            ai_pool.enqueue([msg_str, this]() {
                nlohmann::json response_json;
                try {
                    nlohmann::json req = nlohmann::json::parse(msg_str);
                    int msg_type = req.value("message_type", 0);
                    std::string corr_id = req.value("correlation_id", "");
                    nlohmann::json payload = req.value("payload", nlohmann::json::object());
                    log_info("AIWorldServer: Parsed message_type=" + std::to_string(msg_type) + ", correlation_id=" + corr_id);

                    // --- World Concept Design: Handle expanded entity state sync ---
                    if (msg_type == static_cast<int>(IPCMessageType::ENTITY_STATE_SYNC)) {
                        std::string entity_id = payload.value("entity_id", "");
                        if (payload.contains("consciousness")) {
                            log_info("AIWorldServer: [Threaded] Received consciousness update for " + entity_id);
                        }
                        if (payload.contains("memory")) {
                            log_info("AIWorldServer: [Threaded] Received memory update for " + entity_id);
                        }
                        if (payload.contains("goals")) {
                            log_info("AIWorldServer: [Threaded] Received goals update for " + entity_id);
                        }
                        if (payload.contains("emotional_state")) {
                            log_info("AIWorldServer: [Threaded] Received emotion update for " + entity_id);
                        }
/**
 * --- World Concept Design Integration ---
 * Persistent world model storage for expanded entity state sync
 * Now uses file-backed storage for production reliability.
 */
static std::mutex world_model_mutex;
static std::unordered_map<std::string, nlohmann::json> persistent_world_model;
static const std::string world_model_file = "aiworld_world_model.json";

// Load world model from file
static void load_world_model() {
    std::lock_guard<std::mutex> lock(world_model_mutex);
    std::ifstream infile(world_model_file);
    if (infile) {
        try {
            nlohmann::json j;
            infile >> j;
            for (auto& el : j.items()) {
                persistent_world_model[el.key()] = el.value();
            }
            log_info("AIWorldServer: Loaded world model from file.");
        } catch (const std::exception& e) {
            log_error(std::string("AIWorldServer: Error loading world model file: ") + e.what());
        }
    } else {
        log_info("AIWorldServer: No existing world model file found, starting fresh.");
    }
}

// Save world model to file
static void save_world_model() {
    std::lock_guard<std::mutex> lock(world_model_mutex);
    try {
        nlohmann::json j;
        for (const auto& pair : persistent_world_model) {
            j[pair.first] = pair.second;
        }
        std::ofstream outfile(world_model_file);
        outfile << j.dump(2);
        log_info("AIWorldServer: Saved world model to file.");
    } catch (const std::exception& e) {
        log_error(std::string("AIWorldServer: Error saving world model file: ") + e.what());
    }
}

// Update world model and persist
{
    std::lock_guard<std::mutex> lock(world_model_mutex);

    // --- Episodic, Semantic, Procedural Memory Integration ---
    if (!payload.contains("episodic_memory")) {
        payload["episodic_memory"] = nlohmann::json::array();
    }
    if (!payload.contains("semantic_memory")) {
        payload["semantic_memory"] = nlohmann::json::object();
    }
    if (!payload.contains("procedural_memory")) {
        payload["procedural_memory"] = nlohmann::json::object();
    }

    // --- Moral Alignment System ---
    if (!payload.contains("moral_alignment")) {
        payload["moral_alignment"] = {
            {"good_evil", 0.0},   // -1.0 evil, 0.0 neutral, 1.0 good
            {"law_chaos", 0.0}    // -1.0 chaotic, 0.0 neutral, 1.0 lawful
        };
    }

    // --- Maslow's Pyramid Goal Hierarchy ---
    if (!payload.contains("goals")) {
        payload["goals"] = {
            {"physiological", nlohmann::json::array()},
            {"safety", nlohmann::json::array()},
            {"love_belonging", nlohmann::json::array()},
            {"esteem", nlohmann::json::array()},
            {"self_actualization", nlohmann::json::array()}
        };
    }

    persistent_world_model[entity_id] = payload;
    log_info("AIWorldServer: [Persistent] Updated world model for " + entity_id);
    save_world_model();
}
                        // TODO: Store in persistent world model (future)
                        response_json["message_type"] = msg_type;
                        response_json["correlation_id"] = corr_id;
                        response_json["payload"] = {
                            {"status", "ok"},
                            {"entity_id", entity_id}
                        };
                    } else {
                        response_json["message_type"] = msg_type;
                        response_json["correlation_id"] = corr_id;
                        response_json["payload"] = {
                            {"status", "ok"},
                            {"echo", payload}
                        };
                    }
// --- World Concept Design Integration ---
// World bootstrap: initial NPC, faction, economy, and relationship seeding
// This is a minimal demonstration. In production, load from config/db.

static void bootstrap_world() {
    std::lock_guard<std::mutex> lock(world_model_mutex);

    // Seed NPCs
    for (int i = 0; i < 100; ++i) {
        std::string npc_id = "npc_" + std::to_string(i);
        nlohmann::json npc = {
            {"entity_id", npc_id},
            {"entity_type", "npc"},
            {"personality", {/* TODO: random Big Five + custom traits */}},
            {"background_story", "Generated background for " + npc_id},
            {"skills", {/* TODO: random skills */}},
            {"physical", {/* TODO: random appearance, age, health */}},
            {"moral_alignment", {/* TODO: random alignment */}},
            {"episodic_memory", nlohmann::json::array()},
            {"semantic_memory", nlohmann::json::object()},
            {"procedural_memory", nlohmann::json::object()},
            {"goals", {/* TODO: random goals */}},
            {"emotional_state", {/* TODO: random emotion */}},
            {"relationships", nlohmann::json::array()},
            {"environment_state", nlohmann::json::object()},
            {"extra", nlohmann::json::object()},
            {"state", nlohmann::json::object()}
        };
        persistent_world_model[npc_id] = npc;
    }

    // Seed factions
    persistent_world_model["faction_kingdom"] = {
        {"entity_id", "faction_kingdom"},
        {"entity_type", "faction"},
        {"name", "Kingdom"},
        {"members", nlohmann::json::array()},
        {"resources", {/* TODO: initial resources */}},
        {"relationships", nlohmann::json::array()}
    };
    persistent_world_model["faction_merchant_guild"] = {
        {"entity_id", "faction_merchant_guild"},
        {"entity_type", "faction"},
        {"name", "Merchant Guild"},
        {"members", nlohmann::json::array()},
        {"resources", {/* TODO: initial resources */}},
        {"relationships", nlohmann::json::array()}
    };
    persistent_world_model["faction_thieves_guild"] = {
        {"entity_id", "faction_thieves_guild"},
        {"entity_type", "faction"},
        {"name", "Thieves Guild"},
        {"members", nlohmann::json::array()},
        {"resources", {/* TODO: initial resources */}},
        {"relationships", nlohmann::json::array()}
    };

    // TODO: Seed initial economy, prices, trade routes, relationships, etc.

    log_info("AIWorldServer: World bootstrap complete (seeded NPCs and factions)");
}

// Call bootstrap on server start
struct WorldBootstrapper {
    WorldBootstrapper() { bootstrap_world(); }
};
static WorldBootstrapper world_bootstrapper_instance;
                } catch (const std::exception& e) {
                    log_error(std::string("AIWorldServer: Exception in message handler: ") + e.what());
                    response_json["message_type"] = 99;
                    response_json["payload"] = {{"error", e.what()}};
                }

                std::string resp_str = response_json.dump();
                zmq_msg_t resp_msg;
                zmq_msg_init_size(&resp_msg, resp_str.size());
                std::memcpy(zmq_msg_data(&resp_msg), resp_str.data(), resp_str.size());
                zmq_msg_send(&resp_msg, zmq_socket, 0);
                zmq_msg_close(&resp_msg);
            });
        }
    }
};

int main(int argc, char* argv[]) {
    std::string endpoint = "tcp://*:5555";
    if (argc > 1) {
        endpoint = argv[1];
    }
    AIWorldServer server(endpoint);
    if (!server.start()) {
        log_error("Failed to start AIWorldServer.");
        return 1;
    }
    // Wait for server to finish (Ctrl+C to exit)
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}