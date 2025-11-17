// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// Stub implementation of DragonflyDBClient for distributed state management

#include "dragonflydb_client.hpp"
#include <iostream>
#include <memory>

class DragonflyDBClientStub : public DragonflyDBClient {
public:
    void update_entity_assignment(uint64_t entity_id, int worker_id) override {
        std::cout << "[dragonflydb] update_entity_assignment: entity=" << entity_id << " worker=" << worker_id << std::endl;
    }
    void update_entity_migration(uint64_t entity_id, int from_worker, int to_worker) override {
        std::cout << "[dragonflydb] update_entity_migration: entity=" << entity_id << " from=" << from_worker << " to=" << to_worker << std::endl;
    }
};

// Factory for use in map server initialization
std::shared_ptr<DragonflyDBClient> create_dragonflydb_client_stub() {
    return std::make_shared<DragonflyDBClientStub>();
}