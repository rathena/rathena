#include <gtest/gtest.h>
#include "common/network_monitor.hpp"
#include "common/showmsg.hpp"
#include <chrono>
#include <thread>

class NetworkMonitorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize with test database
        NetworkMonitor::getInstance().initialize(
            "localhost",      // hostname
            "test_user",     // username
            "test_pass",     // password
            "test_rathena",  // database
            3306            // port
        );
    }

    void TearDown() override {
        NetworkMonitor::getInstance().shutdown();
    }
};

TEST_F(NetworkMonitorTest, RecordPacket) {
    uint32 account_id = 1000;
    size_t packet_size = 256;
    
    // Record incoming packet
    NetworkMonitor::getInstance().recordPacket(account_id, packet_size, true);
    
    // Record outgoing packet
    NetworkMonitor::getInstance().recordPacket(account_id, packet_size, false);
    
    // Get stats
    auto stats = NetworkMonitor::getInstance().getNetworkStats(account_id);
    
    EXPECT_EQ(stats.packets_processed, 2);
    EXPECT_EQ(stats.bytes_transferred, packet_size * 2);
    EXPECT_EQ(stats.packet_loss_rate, 0.0);
}

TEST_F(NetworkMonitorTest, RecordLatency) {
    uint32 account_id = 1001;
    double latency = 50.0;
    
    // Record multiple latency samples
    NetworkMonitor::getInstance().recordLatency(account_id, latency);
    NetworkMonitor::getInstance().recordLatency(account_id, latency * 2);
    
    // Get stats
    auto stats = NetworkMonitor::getInstance().getNetworkStats(account_id);
    
    EXPECT_EQ(stats.average_latency, (latency + latency * 2) / 2);
}

TEST_F(NetworkMonitorTest, RecordPacketLoss) {
    uint32 account_id = 1002;
    
    // Record some packets and losses
    NetworkMonitor::getInstance().recordPacket(account_id, 128, false);  // sent
    NetworkMonitor::getInstance().recordPacket(account_id, 128, false);  // sent
    NetworkMonitor::getInstance().recordPacketLoss(account_id);         // lost
    
    // Get stats
    auto stats = NetworkMonitor::getInstance().getNetworkStats(account_id);
    
    // Expected loss rate: 1 lost / (2 sent + 1 lost) = 33.33%
    EXPECT_NEAR(stats.packet_loss_rate, 33.33, 0.01);
}

TEST_F(NetworkMonitorTest, HostMetrics) {
    uint32 account_id = 1003;
    NetworkMonitor::HostMetrics metrics;
    metrics.account_id = account_id;
    metrics.ip_address = "192.168.1.100";
    metrics.connected_players = 10;
    metrics.cpu_usage = 45.5;
    metrics.memory_usage = 60.0;
    metrics.network_usage = 75.2;
    metrics.is_active = true;
    metrics.last_update = std::chrono::system_clock::now();
    
    // Update metrics
    NetworkMonitor::getInstance().updateHostMetrics(account_id, metrics);
    
    // Check active status
    EXPECT_TRUE(NetworkMonitor::getInstance().isHostActive(account_id));
    
    // Wait 6 minutes (host should become inactive)
    metrics.last_update = std::chrono::system_clock::now() - std::chrono::minutes(6);
    NetworkMonitor::getInstance().updateHostMetrics(account_id, metrics);
    
    EXPECT_FALSE(NetworkMonitor::getInstance().isHostActive(account_id));
}

TEST_F(NetworkMonitorTest, SecurityEvents) {
    uint32 account_id = 1004;
    std::string event_type = "suspicious_activity";
    std::string details = "Multiple failed login attempts";
    
    // Record security event
    NetworkMonitor::getInstance().recordSecurityEvent(event_type, account_id, details);
    
    // Get stats
    auto stats = NetworkMonitor::getInstance().getNetworkStats(account_id);
    
    EXPECT_GT(stats.security_events, 0);
}

TEST_F(NetworkMonitorTest, SystemMetrics) {
    // Add some active hosts
    for (uint32 i = 2000; i < 2003; i++) {
        NetworkMonitor::HostMetrics metrics;
        metrics.account_id = i;
        metrics.ip_address = "192.168.1." + std::to_string(i - 1999);
        metrics.connected_players = 5;
        metrics.cpu_usage = 50.0;
        metrics.memory_usage = 60.0;
        metrics.network_usage = 70.0;
        metrics.is_active = true;
        metrics.last_update = std::chrono::system_clock::now();
        
        NetworkMonitor::getInstance().updateHostMetrics(i, metrics);
    }
    
    // Update system metrics
    NetworkMonitor::getInstance().updateSystemMetrics();
    
    // Unfortunately we can't easily verify the metrics in the database
    // But we can verify that the hosts are active
    EXPECT_TRUE(NetworkMonitor::getInstance().isHostActive(2000));
    EXPECT_TRUE(NetworkMonitor::getInstance().isHostActive(2001));
    EXPECT_TRUE(NetworkMonitor::getInstance().isHostActive(2002));
}

TEST_F(NetworkMonitorTest, DatabaseCleanup) {
    // Add some old records that should be cleaned up
    uint32 account_id = 3000;
    NetworkMonitor::getInstance().recordPacket(account_id, 128, true);
    NetworkMonitor::getInstance().recordSecurityEvent("test_event", account_id, "test");
    
    // Trigger cleanup
    NetworkMonitor::cleanupOldData(0, 0, 0, 0);
    
    // Unfortunately we can't easily verify the cleanup in the database
    // Real implementation would need integration tests with actual database
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}