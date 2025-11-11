// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Redis Pub/Sub Subscriber for AI NPC Actions
 * Phase 8B: Async communication for autonomous NPC behavior
 * Subscribes to Redis channels and executes NPC actions in-game
 */

#ifndef REDIS_SUBSCRIBER_HPP
#define REDIS_SUBSCRIBER_HPP

#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <map>

// Forward declarations
namespace sw {
namespace redis {
	class Redis;
	class Subscriber;
}
}

/**
 * Redis Pub/Sub Subscriber
 * Listens for NPC action messages from AI service
 */
class RedisSubscriber {
public:
	/**
	 * Constructor
	 * @param redis_host Redis server host
	 * @param redis_port Redis server port
	 * @param redis_password Redis password (optional)
	 */
	RedisSubscriber(
		const std::string& redis_host = "127.0.0.1",
		int redis_port = 6379,
		const std::string& redis_password = ""
	);
	
	/**
	 * Destructor
	 */
	~RedisSubscriber();
	
	/**
	 * Start subscriber thread
	 * @return true if started successfully
	 */
	bool start();
	
	/**
	 * Stop subscriber thread
	 */
	void stop();
	
	/**
	 * Check if subscriber is running
	 * @return true if running
	 */
	bool is_running() const { return running_; }
	
	/**
	 * Subscribe to NPC action channel
	 * @param npc_id NPC identifier
	 * @return true if subscribed successfully
	 */
	bool subscribe_npc(const std::string& npc_id);
	
	/**
	 * Unsubscribe from NPC action channel
	 * @param npc_id NPC identifier
	 * @return true if unsubscribed successfully
	 */
	bool unsubscribe_npc(const std::string& npc_id);
	
	/**
	 * Subscribe to all NPC actions (pattern subscription)
	 * @return true if subscribed successfully
	 */
	bool subscribe_all_npcs();
	
	/**
	 * Set callback for NPC action messages
	 * @param callback Function to call when message received
	 */
	void set_action_callback(std::function<void(const std::string&, const std::string&)> callback);

private:
	/**
	 * Subscriber thread function
	 */
	void subscriber_thread();
	
	/**
	 * Handle incoming message
	 * @param channel Channel name
	 * @param message Message content (JSON)
	 */
	void handle_message(const std::string& channel, const std::string& message);
	
	/**
	 * Execute NPC action from message
	 * @param npc_id NPC identifier
	 * @param action_type Action type (move, emote, state_change)
	 * @param action_data Action data (JSON)
	 */
	void execute_npc_action(
		const std::string& npc_id,
		const std::string& action_type,
		const std::string& action_data
	);
	
	std::string redis_host_;
	int redis_port_;
	std::string redis_password_;
	
	std::atomic<bool> running_;
	std::thread subscriber_thread_;
	
	std::function<void(const std::string&, const std::string&)> action_callback_;
	
	// Redis connection (will be initialized in .cpp)
	void* redis_connection_;  // Opaque pointer to avoid header dependency
	void* redis_subscriber_;  // Opaque pointer to sw::redis::Subscriber
};

#endif // REDIS_SUBSCRIBER_HPP

