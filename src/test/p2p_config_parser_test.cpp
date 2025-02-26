#include <gtest/gtest.h>
#include "common/p2p_config_parser.hpp"
#include <fstream>
#include <sstream>

class P2PConfigParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary test config file
        std::ofstream config_file("test_p2p_security.conf");
        config_file << createTestConfig();
        config_file.close();
    }

    void TearDown() override {
        // Clean up temporary file
        std::remove("test_p2p_security.conf");
    }

    std::string createTestConfig() {
        std::stringstream ss;
        ss << "network_monitor: {\n"
           << "    enabled: true\n"
           << "    update_interval: 60\n"
           << "    cleanup_interval: 300\n"
           << "    metrics: {\n"
           << "        store_duration: 86400\n"
           << "        alert_timeout: 3600\n"
           << "        max_samples: 1000\n"
           << "    }\n"
           << "}\n"
           << "security: {\n"
           << "    ddos_protection: {\n"
           << "        enabled: true\n"
           << "        packet_rate: 1000\n"
           << "        connection_rate: 100\n"
           << "        bandwidth_limit: 1048576\n"
           << "        burst_threshold: 2000\n"
           << "        blacklist_duration: 3600\n"
           << "    }\n"
           << "}\n"
           << "host_requirements: {\n"
           << "    minimum: {\n"
           << "        cpu_cores: 2\n"
           << "        ram_gb: 4\n"
           << "        bandwidth_mbps: 10\n"
           << "        storage_gb: 20\n"
           << "        uptime_percent: 95\n"
           << "    }\n"
           << "    limits: {\n"
           << "        max_maps: 5\n"
           << "        max_players: 100\n"
           << "        max_instances: 3\n"
           << "        bandwidth_limit: 100\n"
           << "        storage_limit: 50\n"
           << "    }\n"
           << "}\n";
        return ss.str();
    }
};

TEST_F(P2PConfigParserTest, LoadConfig) {
    auto& parser = P2PConfigParser::getInstance();
    ASSERT_TRUE(parser.load("test_p2p_security.conf"));
}

TEST_F(P2PConfigParserTest, NetworkMonitorConfig) {
    auto& parser = P2PConfigParser::getInstance();
    ASSERT_TRUE(parser.load("test_p2p_security.conf"));

    const auto& config = parser.getNetworkMonitorConfig();
    EXPECT_TRUE(config.enabled);
    EXPECT_EQ(config.update_interval, 60u);
    EXPECT_EQ(config.cleanup_interval, 300u);
    EXPECT_EQ(config.metrics.store_duration, 86400u);
    EXPECT_EQ(config.metrics.alert_timeout, 3600u);
    EXPECT_EQ(config.metrics.max_samples, 1000u);
}

TEST_F(P2PConfigParserTest, SecurityConfig) {
    auto& parser = P2PConfigParser::getInstance();
    ASSERT_TRUE(parser.load("test_p2p_security.conf"));

    const auto& config = parser.getSecurityConfig();
    EXPECT_TRUE(config.ddos_protection.enabled);
    EXPECT_EQ(config.ddos_protection.packet_rate, 1000u);
    EXPECT_EQ(config.ddos_protection.connection_rate, 100u);
    EXPECT_EQ(config.ddos_protection.bandwidth_limit, 1048576u);
    EXPECT_EQ(config.ddos_protection.burst_threshold, 2000u);
    EXPECT_EQ(config.ddos_protection.blacklist_duration, 3600u);
}

TEST_F(P2PConfigParserTest, HostRequirements) {
    auto& parser = P2PConfigParser::getInstance();
    ASSERT_TRUE(parser.load("test_p2p_security.conf"));

    const auto& config = parser.getHostRequirements();
    
    // Check minimum requirements
    EXPECT_EQ(config.minimum.cpu_cores, 2u);
    EXPECT_EQ(config.minimum.ram_gb, 4u);
    EXPECT_EQ(config.minimum.bandwidth_mbps, 10u);
    EXPECT_EQ(config.minimum.storage_gb, 20u);
    EXPECT_EQ(config.minimum.uptime_percent, 95u);

    // Check limits
    EXPECT_EQ(config.limits.max_maps, 5u);
    EXPECT_EQ(config.limits.max_players, 100u);
    EXPECT_EQ(config.limits.max_instances, 3u);
    EXPECT_EQ(config.limits.bandwidth_limit, 100u);
    EXPECT_EQ(config.limits.storage_limit, 50u);
}

TEST_F(P2PConfigParserTest, InvalidConfig) {
    auto& parser = P2PConfigParser::getInstance();
    
    // Try to load non-existent file
    EXPECT_FALSE(parser.load("nonexistent.conf"));

    // Create invalid config
    std::ofstream bad_config("bad_config.conf");
    bad_config << "invalid_json{";
    bad_config.close();

    // Try to load invalid config
    EXPECT_FALSE(parser.load("bad_config.conf"));
    std::remove("bad_config.conf");
}

TEST_F(P2PConfigParserTest, ReloadConfig) {
    auto& parser = P2PConfigParser::getInstance();
    ASSERT_TRUE(parser.load("test_p2p_security.conf"));

    // Modify config file
    std::ofstream config_file("test_p2p_security.conf");
    config_file << "network_monitor: { enabled: false }\n";
    config_file.close();

    // Reload and check changes
    parser.reload();
    EXPECT_FALSE(parser.getNetworkMonitorConfig().enabled);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}