#ifndef P2P_MAP_HPP
#define P2P_MAP_HPP

#include <string>
#include "../common/cbasetypes.hpp"
#include "map.hpp"
#include "p2p_host.hpp"

namespace rathena {

class P2PMapServer {
public:
    static P2PMapServer& getInstance();

    bool init();
    void final();

    // Called when a player enters a map
    bool onPlayerEnterMap(struct map_session_data* sd, const std::string& map_name);
    
    // Called when a player leaves a map
    void onPlayerLeaveMap(struct map_session_data* sd, const std::string& map_name);
    
    // Called to check if a player can become a P2P host
    bool canPlayerHost(struct map_session_data* sd);
    
    // Register a player as potential P2P host
    bool registerPlayerAsHost(struct map_session_data* sd);
    
    // Unregister a player as P2P host
    void unregisterPlayerHost(struct map_session_data* sd);
    
    // Handle map server operations during VPS fallback
    bool handleVPSFallback(const std::string& map_name);
    
    // Process P2P task distribution for VPS-hosted maps
    bool processP2PAssistance(const std::string& map_name);

private:
    P2PMapServer();
    ~P2PMapServer();

    // Find or assign a P2P host for a map
    std::shared_ptr<P2PHost> findOrAssignHost(const std::string& map_name);
    
    // Collect system performance metrics from a player's client
    bool collectPlayerMetrics(struct map_session_data* sd, P2PHostStats& stats);
    
    // Handle migration when a host becomes unavailable
    bool handleHostMigration(const std::string& map_name, uint32 old_host_id);
    
    // Validate map data from P2P hosts
    bool validateMapData(const std::string& map_name, uint32 host_id);
    
    // Internal map state tracking
    struct MapState {
        uint32 current_host_id;
        std::vector<uint32> backup_host_ids;
        bool is_vps_hosted;
        std::vector<uint32> assistance_host_ids;
    };
    
    std::map<std::string, MapState> map_states;
    static P2PMapServer* instance;
};

} // namespace rathena

#endif // P2P_MAP_HPP