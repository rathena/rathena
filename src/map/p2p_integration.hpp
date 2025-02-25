#ifndef P2P_INTEGRATION_HPP
#define P2P_INTEGRATION_HPP

#include <string>
#include <vector>
#include <chrono>
#include "../common/cbasetypes.hpp"
#include "p2p_host.hpp"

namespace rathena {

class P2PIntegration {
public:
    static P2PIntegration& getInstance();

    bool init();
    void final();
    
    // VPS Health Check
    bool checkVPSConnection();
    void setVPSStatus(bool connected);
    
    // Regional Latency
    bool measureRegionalLatency(uint32 account_id);
    bool updateRegionalLatency(uint32 account_id, const std::string& region, uint16 latency);
    bool setupRegionalRelay(const std::string& region, const std::string& relay_address);

    // Load Balancing
    void redistributeLoad();
    void updateHostMetrics();
    double calculateHostScore(const std::shared_ptr<P2PHost>& host) const;
    
    // System Metrics
    void collectSystemMetrics(struct map_session_data* sd);
    void reportMetricsToHost(uint32 host_id);

    // Map Migration
    bool initiateMigration(const std::string& map_name, uint32 from_host, uint32 to_host);
    bool completeMigration(const std::string& map_name, uint32 new_host);

private:
    P2PIntegration();
    ~P2PIntegration();

    // Internal Methods
    bool validateConnection(const std::string& endpoint);
    void logMetrics(const std::string& metric_name, double value);
    bool syncMapState(const std::string& map_name, uint32 source_host, uint32 target_host);

    // Health Check Implementation
    void performHealthCheck();
    bool validateHostHealth(const std::shared_ptr<P2PHost>& host);
    void handleHostFailure(uint32 account_id);

    // Regional Latency Implementation
    uint16 pingEndpoint(const std::string& endpoint);
    void updateRegionalRoutingTable();
    bool selectOptimalRelay(const std::string& region, std::string& relay_address);

    // Load Balancing Implementation
    double calculateMapLoad(const std::string& map_name);
    bool isLoadBalanceNeeded();
    std::vector<std::string> findOverloadedMaps();
    bool findOptimalHostForMap(const std::string& map_name, uint32& host_id);

    // Member Variables
    bool vps_connected;
    std::chrono::system_clock::time_point last_vps_check;
    std::map<std::string, std::string> regional_relays;
    std::map<uint32, std::map<std::string, uint16>> latency_cache;

    static P2PIntegration* instance;
    static const int VPS_CHECK_INTERVAL = 30; // seconds
    static const int MAX_RETRY_COUNT = 3;
    static const int LOAD_CHECK_INTERVAL = 60; // seconds
};

} // namespace rathena

// Timer callback function (must be in global namespace)
TIMER_FUNC(p2p_health_check);

#endif // P2P_INTEGRATION_HPP