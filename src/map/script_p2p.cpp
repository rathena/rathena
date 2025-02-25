#include "script.hpp"
#include "pc.hpp"
#include "map.hpp"
#include "p2p_host.hpp"
#include "p2p_map.hpp"

using namespace rathena;

/*==========================================
 * Get current map's P2P host ID
 * getp2phostid();
 *------------------------------------------*/
static BUILDIN_FUNC(getp2phostid) {
    TBL_PC* sd;
    
    if (!script_rid2sd(sd))
        return SCRIPT_CMD_FAILURE;
    
    const char* mapname = map_getmapdata(sd->bl.m)->name;
    auto& map_server = P2PMapServer::getInstance();
    auto state = map_server.getMapState(mapname);
    
    script_pushint(st, state ? state->current_host_id : 0);
    return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Get P2P host statistics
 * getp2phoststats(<host_id>);
 *------------------------------------------*/
static BUILDIN_FUNC(getp2phoststats) {
    uint32 host_id = script_getnum(st, 2);
    
    auto& host_mgr = P2PHostManager::getInstance();
    auto host = host_mgr.getHost(host_id);
    
    if (!host) {
        script_pusharray_nil(st);
        return SCRIPT_CMD_FAILURE;
    }
    
    const auto& host_stats = host->getStats();
    struct script_array* arr = script_array_tmp_initialize(st);
    
    script_array_push_num(arr, host_stats.cpu_ghz);                  // CPU speed (GHz)
    script_array_push_num(arr, host_stats.cpu_cores);                // CPU cores
    script_array_push_num(arr, host_stats.free_ram_mb);             // Free RAM (MB)
    script_array_push_num(arr, host_stats.network_speed_mbps);      // Network speed (Mbps)
    script_array_push_num(arr, host_stats.latency_ms);              // Latency (ms)
    script_array_push_num(arr, host_stats.disconnects_per_hour);    // Disconnects/hour
    
    script_pusharray(st, arr);
    return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Request to become P2P host for current map
 * requestp2phost();
 *------------------------------------------*/
static BUILDIN_FUNC(requestp2phost) {
    TBL_PC* sd;
    
    if (!script_rid2sd(sd))
        return SCRIPT_CMD_FAILURE;
    
    const char* mapname = map_getmapdata(sd->bl.m)->name;
    auto& map_server = P2PMapServer::getInstance();
    
    bool success = map_server.registerPlayerAsHost(sd) &&
                  map_server.onPlayerEnterMap(sd, mapname);
    
    script_pushint(st, success ? 1 : 0);
    return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Get list of maps currently being hosted by player
 * getp2phostingmaps();
 *------------------------------------------*/
static BUILDIN_FUNC(getp2phostingmaps) {
    TBL_PC* sd;
    
    if (!script_rid2sd(sd))
        return SCRIPT_CMD_FAILURE;
    
    auto& host_mgr = P2PHostManager::getInstance();
    auto host = host_mgr.getHost(sd->status.account_id);
    
    struct script_array* arr = script_array_tmp_initialize(st);
    
    if (host) {
        const auto& maps = host->getAssignedMaps();
        for (const auto& map_name : maps) {
            script_array_push_str(arr, map_name.c_str());
        }
    }
    
    script_pusharray(st, arr);
    return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Register script commands
 *------------------------------------------*/
void script_p2p_init(void) {
    BUILDIN_DEF(getp2phostid, ""),
    BUILDIN_DEF(getp2phoststats, "i"),
    BUILDIN_DEF(requestp2phost, ""),
    BUILDIN_DEF(getp2phostingmaps, "")
}