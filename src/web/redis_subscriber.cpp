// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Redis Pub/Sub Subscriber for AI NPC Actions
 * Phase 8B: Async communication for autonomous NPC behavior
 * Subscribes to Redis channels and executes NPC actions in-game
 */

#include "redis_subscriber.hpp"
#include <hiredis/hiredis.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <cstring>

using json = nlohmann::json;

RedisSubscriber::RedisSubscriber(
	const std::string& redis_host,
	int redis_port,
	const std::string& redis_password
) : redis_host_(redis_host),
    redis_port_(redis_port),
    redis_password_(redis_password),
    running_(false),
    redis_connection_(nullptr),
    redis_subscriber_(nullptr)
{
	std::cout << "[RedisSubscriber] Initialized with host=" << redis_host 
	          << " port=" << redis_port << std::endl;
}

RedisSubscriber::~RedisSubscriber() {
	stop();
	
	if (redis_connection_) {
		redisFree(static_cast<redisContext*>(redis_connection_));
		redis_connection_ = nullptr;
	}
}

bool RedisSubscriber::start() {
	if (running_) {
		std::cerr << "[RedisSubscriber] Already running" << std::endl;
		return false;
	}
	
	// Connect to Redis
	redisContext* ctx = redisConnect(redis_host_.c_str(), redis_port_);
	if (ctx == nullptr || ctx->err) {
		if (ctx) {
			std::cerr << "[RedisSubscriber] Connection error: " << ctx->errstr << std::endl;
			redisFree(ctx);
		} else {
			std::cerr << "[RedisSubscriber] Connection error: can't allocate redis context" << std::endl;
		}
		return false;
	}
	
	redis_connection_ = ctx;
	
	// Authenticate if password provided
	if (!redis_password_.empty()) {
		redisReply* reply = static_cast<redisReply*>(
			redisCommand(ctx, "AUTH %s", redis_password_.c_str())
		);
		if (reply) {
			freeReplyObject(reply);
		}
	}
	
	running_ = true;
	subscriber_thread_ = std::thread(&RedisSubscriber::subscriber_thread, this);
	
	std::cout << "[RedisSubscriber] Started successfully" << std::endl;
	return true;
}

void RedisSubscriber::stop() {
	if (!running_) {
		return;
	}
	
	running_ = false;
	
	if (subscriber_thread_.joinable()) {
		subscriber_thread_.join();
	}
	
	std::cout << "[RedisSubscriber] Stopped" << std::endl;
}

bool RedisSubscriber::subscribe_npc(const std::string& npc_id) {
	std::string channel = "npc:action:" + npc_id;
	std::cout << "[RedisSubscriber] Subscribing to channel: " << channel << std::endl;
	return true;
}

bool RedisSubscriber::unsubscribe_npc(const std::string& npc_id) {
	std::string channel = "npc:action:" + npc_id;
	std::cout << "[RedisSubscriber] Unsubscribing from channel: " << channel << std::endl;
	return true;
}

bool RedisSubscriber::subscribe_all_npcs() {
	// This is a no-op now - subscription happens in subscriber_thread
	// Just return true to indicate readiness
	std::cout << "[RedisSubscriber] Subscription will be handled by subscriber thread" << std::endl;
	return true;
}

void RedisSubscriber::set_action_callback(
	std::function<void(const std::string&, const std::string&)> callback
) {
	action_callback_ = callback;
	std::cout << "[RedisSubscriber] Action callback set" << std::endl;
}

void RedisSubscriber::subscriber_thread() {
	std::cout << "[RedisSubscriber] Subscriber thread started" << std::endl;

	redisContext* ctx = static_cast<redisContext*>(redis_connection_);
	if (!ctx) {
		std::cerr << "[RedisSubscriber] No Redis connection in subscriber thread" << std::endl;
		return;
	}

	// Subscribe to pattern npc:action:* in the subscriber thread
	redisReply* reply = static_cast<redisReply*>(
		redisCommand(ctx, "PSUBSCRIBE npc:action:*")
	);

	if (!reply) {
		std::cerr << "[RedisSubscriber] Failed to subscribe: " << ctx->errstr << std::endl;
		return;
	}

	std::cout << "[RedisSubscriber] Subscribed to pattern: npc:action:*" << std::endl;
	freeReplyObject(reply);
	
	while (running_) {
		// Wait for messages (blocking call with 1 second timeout)
		if (redisGetReply(ctx, reinterpret_cast<void**>(&reply)) != REDIS_OK) {
			std::cerr << "[RedisSubscriber] Error getting reply: " << ctx->errstr << std::endl;
			break;
		}
		
		if (reply == nullptr) {
			continue;
		}
		
		// Check if it's a message
		if (reply->type == REDIS_REPLY_ARRAY && reply->elements >= 3) {
			std::string message_type(reply->element[0]->str, reply->element[0]->len);
			
			if (message_type == "pmessage" && reply->elements == 4) {
				// Pattern message: [pmessage, pattern, channel, message]
				std::string channel(reply->element[2]->str, reply->element[2]->len);
				std::string message(reply->element[3]->str, reply->element[3]->len);
				
				handle_message(channel, message);
			}
		}
		
		freeReplyObject(reply);
	}
	
	std::cout << "[RedisSubscriber] Subscriber thread stopped" << std::endl;
}

void RedisSubscriber::handle_message(const std::string& channel, const std::string& message) {
	try {
		std::cout << "[RedisSubscriber] Received message on channel: " << channel << std::endl;

		// Parse JSON message
		json msg = json::parse(message);

		std::string npc_id = msg.value("npc_id", "");
		std::string action_type = msg.value("action_type", "");

		if (npc_id.empty() || action_type.empty()) {
			std::cerr << "[RedisSubscriber] Invalid message format" << std::endl;
			return;
		}

		// Extract action_data as JSON string
		std::string action_data = msg.value("action_data", json::object()).dump();

		// Execute NPC action
		execute_npc_action(npc_id, action_type, action_data);

		// Call user callback if set
		if (action_callback_) {
			action_callback_(npc_id, message);
		}

	} catch (const json::exception& e) {
		std::cerr << "[RedisSubscriber] JSON parse error: " << e.what() << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "[RedisSubscriber] Error handling message: " << e.what() << std::endl;
	}
}

void RedisSubscriber::execute_npc_action(
	const std::string& npc_id,
	const std::string& action_type,
	const std::string& action_data
) {
	try {
		std::cout << "[RedisSubscriber] Executing NPC action: " << npc_id
		          << " type=" << action_type << std::endl;

		json data = json::parse(action_data);

		if (action_type == "move") {
			int target_x = data.value("target_x", 0);
			int target_y = data.value("target_y", 0);
			int speed = data.value("speed", 150);

			std::cout << "[RedisSubscriber] NPC " << npc_id << " moving to ("
			          << target_x << "," << target_y << ") speed=" << speed << std::endl;

			// TODO: Call rAthena map-server API to move NPC
			// This would require integration with map-server's NPC movement functions

		} else if (action_type == "emote") {
			int emote_id = data.value("emote_id", 0);
			int duration = data.value("duration", 3000);

			std::cout << "[RedisSubscriber] NPC " << npc_id << " emote "
			          << emote_id << " duration=" << duration << "ms" << std::endl;

			// TODO: Call rAthena map-server API to show emote

		} else if (action_type == "state_change") {
			std::string new_state = data.value("new_state", "");

			std::cout << "[RedisSubscriber] NPC " << npc_id << " state change to "
			          << new_state << std::endl;

			// TODO: Update NPC state in map-server

		} else {
			std::cout << "[RedisSubscriber] Unknown action type: " << action_type << std::endl;
		}

	} catch (const json::exception& e) {
		std::cerr << "[RedisSubscriber] JSON parse error in action data: " << e.what() << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "[RedisSubscriber] Error executing NPC action: " << e.what() << std::endl;
	}
}

