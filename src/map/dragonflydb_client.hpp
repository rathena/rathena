// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// DragonflyDB distributed state client interface

#ifndef DRAGONFLYDB_CLIENT_HPP
#define DRAGONFLYDB_CLIENT_HPP

#include <cstdint>

class DragonflyDBClient {
public:
    virtual ~DragonflyDBClient() = default;

    // Update entity assignment in distributed state
    virtual void update_entity_assignment(uint64_t entity_id, int worker_id) = 0;

    // Update entity migration in distributed state
    virtual void update_entity_migration(uint64_t entity_id, int from_worker, int to_worker) = 0;
};

#endif // DRAGONFLYDB_CLIENT_HPP