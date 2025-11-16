// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// P2P Coordinator interface for distributed map server

#ifndef P2P_COORDINATOR_HPP
#define P2P_COORDINATOR_HPP

#include <cstdint>
#include <string>

class P2PCoordinator {
public:
    virtual ~P2PCoordinator() = default;

    // Notify coordinator of entity assignment to a worker
    virtual void notify_assignment(uint64_t entity_id, int worker_id) = 0;

    // Notify coordinator of entity migration between workers
    virtual void notify_migration(uint64_t entity_id, int from_worker, int to_worker) = 0;

    // Validate authority for a critical action (returns true if allowed)
    virtual bool validate_authority(uint64_t entity_id, const std::string& action, int worker_id) = 0;
};

#endif // P2P_COORDINATOR_HPP