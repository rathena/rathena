// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "discord.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/timer.hpp>
#include <common/strlib.hpp>

#include "itemdb.hpp"
#include "map.hpp"
#include "mob.hpp"
#include "pc.hpp"

using namespace std;

#define MVP_MESSAGE_FILE "log/mvp_messages.txt"
#define CARD_MESSAGE_FILE "log/card_messages.txt"

// Global strings for Discord webhooks
static string discord_mvp_webhook_url;
static string discord_card_webhook_url;
static string discord_server_webhook_url;
static string discord_server_cached_label;

// Forward declarations
static string url_encode(const string& value);
static string json_escape(const string& s);
static bool send_discord_webhook(const string& webhook_url, const string& json_payload);
static void save_message_id(const char* filename, const string& message_id);
static vector<string> load_message_ids(const char* filename);
static void delete_discord_message(const string& webhook_url, const string& message_id);

void discord_set_mvp_webhook(const char* url) {
if (url && url[0] != '\0') {
discord_mvp_webhook_url = url;
}
}

void discord_set_card_webhook(const char* url) {
if (url && url[0] != '\0') {
discord_card_webhook_url = url;
}
}

void discord_set_server_webhook(const char* url) {
if (url && url[0] != '\0') {
discord_server_webhook_url = url;
}
}

void discord_set_server_label(const char* label) {
if (label && label[0] != '\0') {
discord_server_cached_label = label;
}
}

const char* discord_get_server_label() {
	return discord_server_cached_label.c_str();
}

// Handle configuration from log_athena.conf
bool discord_handle_config(const char* key, const char* value) {
	if (strcmpi(key, "discord_mvp_webhook") == 0) {
		discord_set_mvp_webhook(value);
		return true;
	}
	else if (strcmpi(key, "discord_card_webhook") == 0) {
		discord_set_card_webhook(value);
		return true;
	}
	else if (strcmpi(key, "discord_server_webhook") == 0) {
		discord_set_server_webhook(value);
		return true;
	}
	else if (strcmpi(key, "discord_server_label") == 0) {
		discord_set_server_label(value);
		return true;
	}
	return false;  // Not a Discord config
}

// URL-encode a string for safe transmission
static string url_encode(const string& value) {
ostringstream escaped;
for (size_t i = 0; i < value.length(); ++i) {
unsigned char c = value[i];
if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
escaped << c;
} else {
escaped << '%' << uppercase << hex << (int)c;
}
}
return escaped.str();
}

// Escape JSON special characters
static string json_escape(const string& s) {
ostringstream escaped;
for (size_t i = 0; i < s.length(); ++i) {
char c = s[i];
if (c == '\"') escaped << "\\\"";
else if (c == '\\') escaped << "\\\\";
else if (c == '\b') escaped << "\\b";
else if (c == '\f') escaped << "\\f";
else if (c == '\n') escaped << "\\n";
else if (c == '\r') escaped << "\\r";
else if (c == '\t') escaped << "\\t";
else escaped << c;
}
return escaped.str();
}

// Send a Discord webhook with retry logic
static bool send_discord_webhook(const string& webhook_url, const string& json_payload) {
if (webhook_url.empty()) {
return false;
}

// Write JSON payload to temporary file
const char* temp_file = "discord_payload.json";
FILE* f = fopen(temp_file, "w");
if (!f) {
ShowError("discord: Failed to create temporary JSON file\n");
return false;
}
fprintf(f, "%s", json_payload.c_str());
fclose(f);

// Build curl command
char command[4096];
snprintf(command, sizeof(command),
"curl -s -X POST -H \"Content-Type: application/json\" -d @%s \"%s\" -o discord_response.txt 2>/dev/null",
temp_file, webhook_url.c_str());

// Execute curl
int result = system(command);

// Clean up temporary file
remove(temp_file);

if (result != 0) {
ShowError("discord: Failed to execute curl command (result: %d)\n", result);
return false;
}

return true;
}

// Save a Discord message ID to file
static void save_message_id(const char* filename, const string& message_id) {
if (message_id.empty()) return;

ofstream out(filename, ios::app);
if (out.is_open()) {
out << message_id << endl;
out.close();
}
}

// Load all message IDs from file
static vector<string> load_message_ids(const char* filename) {
vector<string> ids;
ifstream in(filename);
if (in.is_open()) {
string line;
while (getline(in, line)) {
if (!line.empty()) {
ids.push_back(line);
}
}
in.close();
}
return ids;
}

// Delete a Discord message by ID
static void delete_discord_message(const string& webhook_url, const string& message_id) {
if (webhook_url.empty() || message_id.empty()) return;

// Extract webhook ID and token from URL
// URL format: https://discord.com/api/webhooks/WEBHOOK_ID/WEBHOOK_TOKEN
size_t pos1 = webhook_url.find("/webhooks/");
if (pos1 == string::npos) return;
pos1 += 10; // length of "/webhooks/"

size_t pos2 = webhook_url.find("/", pos1);
if (pos2 == string::npos) return;

string webhook_id = webhook_url.substr(pos1, pos2 - pos1);
string webhook_token = webhook_url.substr(pos2 + 1);

// Build delete URL
char delete_url[512];
snprintf(delete_url, sizeof(delete_url),
"https://discord.com/api/webhooks/%s/%s/messages/%s",
webhook_id.c_str(), webhook_token.c_str(), message_id.c_str());

// Execute DELETE request
char command[1024];
snprintf(command, sizeof(command),
"curl -s -X DELETE \"%s\" 2>/dev/null",
delete_url);

system(command);
}

// Clear all MVP messages on server start
void discord_clear_mvp_messages_on_start() {
if (discord_mvp_webhook_url.empty()) return;

vector<string> message_ids = load_message_ids(MVP_MESSAGE_FILE);
for (const string& id : message_ids) {
delete_discord_message(discord_mvp_webhook_url, id);
}

// Clear the file
remove(MVP_MESSAGE_FILE);
}

// Clear all card messages on server start
void discord_clear_card_messages_on_start() {
if (discord_card_webhook_url.empty()) return;

vector<string> message_ids = load_message_ids(CARD_MESSAGE_FILE);
for (const string& id : message_ids) {
delete_discord_message(discord_card_webhook_url, id);
}

// Clear the file
remove(CARD_MESSAGE_FILE);
}

// Extract message ID from Discord API response
static string extract_message_id_from_response(const char* response_file) {
ifstream in(response_file);
if (!in.is_open()) return "";

string content((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
in.close();

// Look for "id":"MESSAGE_ID" in JSON response
size_t pos = content.find("\"id\":\"");
if (pos == string::npos) return "";

pos += 6; // length of "\"id\":\""
size_t end_pos = content.find("\"", pos);
if (end_pos == string::npos) return "";

return content.substr(pos, end_pos - pos);
}

// Notify MVP kill
void discord_notify_mvp_kill(
const char* mvp_name,
const char* map_name,
const char* killer_name,
int kill_duration_ms,
const char* top1_name, int64 top1_damage,
const char* top2_name, int64 top2_damage,
const char* top3_name, int64 top3_damage,
t_itemid mvp_reward_item_id,
t_exp mvp_reward_exp
) {
if (discord_mvp_webhook_url.empty()) return;

// Build damage ranking text
ostringstream damage_ranking;
if (top1_name && top1_damage > 0) {
damage_ranking << "**1.** " << json_escape(top1_name) << " - " << top1_damage << " dano\\n";
}
if (top2_name && top2_damage > 0) {
damage_ranking << "**2.** " << json_escape(top2_name) << " - " << top2_damage << " dano\\n";
}
if (top3_name && top3_damage > 0) {
damage_ranking << "**3.** " << json_escape(top3_name) << " - " << top3_damage << " dano";
}

// Build reward text
ostringstream reward_text;
bool has_rewards = false;

if (mvp_reward_item_id > 0) {
shared_ptr<item_data> item = item_db.find(mvp_reward_item_id);
if (item) {
reward_text << "**Item:** " << json_escape(item->ename.c_str());
has_rewards = true;
}
}

if (mvp_reward_exp > 0) {
if (has_rewards) reward_text << "\\n";
reward_text << "**EXP:** " << mvp_reward_exp;
has_rewards = true;
}

if (!has_rewards) {
reward_text << "Nenhuma recompensa";
}

// Convert kill duration to minutes and seconds
int kill_minutes = kill_duration_ms / 60000;
int kill_seconds = (kill_duration_ms % 60000) / 1000;

// Build JSON payload
ostringstream json;
json << "{"
<< "\"embeds\": [{"
<< "\"title\": \"" << json_escape(mvp_name) << " foi derrotado!\","
<< "\"description\": \"Servidor: **" << json_escape(discord_server_cached_label) << "**\","
<< "\"color\": 15844367,"  // Gold color
<< "\"fields\": ["
<< "{\"name\": \"Mapa\", \"value\": \"" << json_escape(map_name) << "\", \"inline\": true},"
<< "{\"name\": \"Matador\", \"value\": \"" << json_escape(killer_name) << "\", \"inline\": true},"
<< "{\"name\": \"Tempo\", \"value\": \"" << kill_minutes << "m " << kill_seconds << "s\", \"inline\": true},"
<< "{\"name\": \"Recompensas MVP\", \"value\": \"" << reward_text.str() << "\", \"inline\": false},"
<< "{\"name\": \"Ranking de Dano\", \"value\": \"" << damage_ranking.str() << "\", \"inline\": false}"
<< "],"
<< "\"footer\": {\"text\": \"rAthena Discord Bot\"}"
<< "}]"
<< "}";

if (send_discord_webhook(discord_mvp_webhook_url, json.str())) {
// Extract and save message ID
string message_id = extract_message_id_from_response("discord_response.txt");
if (!message_id.empty()) {
save_message_id(MVP_MESSAGE_FILE, message_id);
}
remove("discord_response.txt");
}
}

// Notify MVP respawn
void discord_notify_mvp_respawn(struct mob_data* md) {
	if (discord_mvp_webhook_url.empty() || !md) return;

	const char* mvp_name = md->name;
	const char* map_name = ::map[md->m].name;  // Use global map array

// Get respawn time from spawn data
uint32 respawn_time_ms = 0;
if (md->spawn) {
respawn_time_ms = md->spawn->delay1;
if (md->spawn->delay2) {
respawn_time_ms += (md->spawn->delay2 / 2); // Average variance
}
}

int respawn_minutes = respawn_time_ms / 60000;
int respawn_seconds = (respawn_time_ms % 60000) / 1000;

// Build JSON payload
ostringstream json;
json << "{"
<< "\"embeds\": [{"
<< "\"title\": \"" << json_escape(mvp_name) << " renasceu!\","
<< "\"description\": \"Servidor: **" << json_escape(discord_server_cached_label) << "**\","
<< "\"color\": 5763719,"  // Blue-green color
<< "\"fields\": ["
<< "{\"name\": \"Mapa\", \"value\": \"" << json_escape(map_name) << "\", \"inline\": true},"
<< "{\"name\": \"Tempo de Respawn\", \"value\": \"~" << respawn_minutes << "m " << respawn_seconds << "s\", \"inline\": true}"
<< "],"
<< "\"footer\": {\"text\": \"rAthena Discord Bot\"}"
<< "}]"
<< "}";

if (send_discord_webhook(discord_mvp_webhook_url, json.str())) {
// Extract and save message ID
string message_id = extract_message_id_from_response("discord_response.txt");
if (!message_id.empty()) {
save_message_id(MVP_MESSAGE_FILE, message_id);
}
remove("discord_response.txt");
}
}

// Notify card drop
void discord_notify_card_drop(const char* card_name, const char* mob_name, const char* map_name, const char* player_name) {
if (discord_card_webhook_url.empty()) return;

ostringstream json;
json << "{"
<< "\"embeds\": [{"
<< "\"title\": \"Carta dropada!\","
<< "\"description\": \"Servidor: **" << json_escape(discord_server_cached_label) << "**\","
<< "\"color\": 10181046,"  // Purple color
<< "\"fields\": ["
<< "{\"name\": \"Carta\", \"value\": \"" << json_escape(card_name) << "\", \"inline\": true},"
<< "{\"name\": \"Monstro\", \"value\": \"" << json_escape(mob_name) << "\", \"inline\": true},"
<< "{\"name\": \"Mapa\", \"value\": \"" << json_escape(map_name) << "\", \"inline\": true},"
<< "{\"name\": \"Jogador\", \"value\": \"" << json_escape(player_name) << "\", \"inline\": true}"
<< "],"
<< "\"footer\": {\"text\": \"rAthena Discord Bot\"}"
<< "}]"
<< "}";

if (send_discord_webhook(discord_card_webhook_url, json.str())) {
// Extract and save message ID
string message_id = extract_message_id_from_response("discord_response.txt");
if (!message_id.empty()) {
save_message_id(CARD_MESSAGE_FILE, message_id);
}
remove("discord_response.txt");
}
}

// Notify server online
void discord_notify_server_online() {
if (discord_server_webhook_url.empty()) return;

ostringstream json;
json << "{"
<< "\"embeds\": [{"
<< "\"title\": \"Servidor Online\","
<< "\"description\": \"**" << json_escape(discord_server_cached_label) << "** está online!\","
<< "\"color\": 3066993,"  // Green color
<< "\"footer\": {\"text\": \"rAthena Discord Bot\"}"
<< "}]"
<< "}";

send_discord_webhook(discord_server_webhook_url, json.str());
remove("discord_response.txt");
}

// Notify server offline
void discord_notify_server_offline() {
if (discord_server_webhook_url.empty()) return;

ostringstream json;
json << "{"
<< "\"embeds\": [{"
<< "\"title\": \"Servidor Offline\","
<< "\"description\": \"**" << json_escape(discord_server_cached_label) << "** está offline.\","
<< "\"color\": 15158332,"  // Red color
<< "\"footer\": {\"text\": \"rAthena Discord Bot\"}"
<< "}]"
<< "}";

send_discord_webhook(discord_server_webhook_url, json.str());
remove("discord_response.txt");
}

// Notify GM command execution
void discord_notify_gm_command(const char* gm_name, const char* command, const char* map_name) {
if (discord_server_webhook_url.empty()) return;

ostringstream json;
json << "{"
<< "\"embeds\": [{"
<< "\"title\": \"Comando GM Executado\","
<< "\"color\": 16776960,"  // Yellow color
<< "\"fields\": ["
<< "{\"name\": \"GM\", \"value\": \"" << json_escape(gm_name) << "\", \"inline\": true},"
<< "{\"name\": \"Comando\", \"value\": \"" << json_escape(command) << "\", \"inline\": true},"
<< "{\"name\": \"Mapa\", \"value\": \"" << json_escape(map_name) << "\", \"inline\": true}"
<< "],"
<< "\"footer\": {\"text\": \"rAthena Discord Bot\"}"
<< "}]"
<< "}";

send_discord_webhook(discord_server_webhook_url, json.str());
remove("discord_response.txt");
}

// Notify player login
void discord_notify_player_login(const char* player_name, int account_id) {
if (discord_server_webhook_url.empty()) return;

ostringstream json;
json << "{"
<< "\"embeds\": [{"
<< "\"title\": \"Jogador Conectado\","
<< "\"description\": \"" << json_escape(player_name) << " entrou no servidor.\","
<< "\"color\": 3447003,"  // Blue color
<< "\"fields\": ["
<< "{\"name\": \"Account ID\", \"value\": \"" << account_id << "\", \"inline\": true}"
<< "],"
<< "\"footer\": {\"text\": \"rAthena Discord Bot\"}"
<< "}]"
<< "}";

send_discord_webhook(discord_server_webhook_url, json.str());
remove("discord_response.txt");
}

// Notify player logout
void discord_notify_player_logout(const char* player_name, int account_id) {
if (discord_server_webhook_url.empty()) return;

ostringstream json;
json << "{"
<< "\"embeds\": [{"
<< "\"title\": \"Jogador Desconectado\","
<< "\"description\": \"" << json_escape(player_name) << " saiu do servidor.\","
<< "\"color\": 10197915,"  // Orange color
<< "\"fields\": ["
<< "{\"name\": \"Account ID\", \"value\": \"" << account_id << "\", \"inline\": true}"
<< "],"
<< "\"footer\": {\"text\": \"rAthena Discord Bot\"}"
<< "}]"
<< "}";

send_discord_webhook(discord_server_webhook_url, json.str());
remove("discord_response.txt");
}

// Notify vending shop opened
void discord_notify_vending_open(const char* player_name, const char* shop_name, const char* map_name, int item_count) {
if (discord_server_webhook_url.empty()) return;

ostringstream json;
json << "{"
<< "\"embeds\": [{"
<< "\"title\": \"Loja de Vendedor Aberta\","
<< "\"color\": 15105570,"  // Light blue color
<< "\"fields\": ["
<< "{\"name\": \"Vendedor\", \"value\": \"" << json_escape(player_name) << "\", \"inline\": true},"
<< "{\"name\": \"Loja\", \"value\": \"" << json_escape(shop_name) << "\", \"inline\": true},"
<< "{\"name\": \"Mapa\", \"value\": \"" << json_escape(map_name) << "\", \"inline\": true},"
<< "{\"name\": \"Itens\", \"value\": \"" << item_count << "\", \"inline\": true}"
<< "],"
<< "\"footer\": {\"text\": \"rAthena Discord Bot\"}"
<< "}]"
<< "}";

send_discord_webhook(discord_server_webhook_url, json.str());
remove("discord_response.txt");
}

// Notify rare item crafted
void discord_notify_rare_craft(const char* player_name, const char* item_name, int success_rate) {
if (discord_server_webhook_url.empty()) return;

ostringstream json;
json << "{"
<< "\"embeds\": [{"
<< "\"title\": \"Item Raro Criado!\","
<< "\"color\": 16766720,"  // Gold color
<< "\"fields\": ["
<< "{\"name\": \"Jogador\", \"value\": \"" << json_escape(player_name) << "\", \"inline\": true},"
<< "{\"name\": \"Item\", \"value\": \"" << json_escape(item_name) << "\", \"inline\": true},"
<< "{\"name\": \"Taxa de Sucesso\", \"value\": \"" << success_rate << "%\", \"inline\": true}"
<< "],"
<< "\"footer\": {\"text\": \"rAthena Discord Bot\"}"
<< "}]"
<< "}";

send_discord_webhook(discord_server_webhook_url, json.str());
remove("discord_response.txt");
}

// Notify refine success/failure
void discord_notify_refine(const char* player_name, const char* item_name, int refine_level, bool success) {
if (discord_server_webhook_url.empty()) return;

const char* result = success ? "Sucesso" : "Falha";
int color = success ? 3066993 : 15158332; // Green or Red

ostringstream json;
json << "{"
<< "\"embeds\": [{"
<< "\"title\": \"Refinamento: " << result << "\","
<< "\"color\": " << color << ","
<< "\"fields\": ["
<< "{\"name\": \"Jogador\", \"value\": \"" << json_escape(player_name) << "\", \"inline\": true},"
<< "{\"name\": \"Item\", \"value\": \"" << json_escape(item_name) << "\", \"inline\": true},"
<< "{\"name\": \"Nível\", \"value\": \"+" << refine_level << "\", \"inline\": true}"
<< "],"
<< "\"footer\": {\"text\": \"rAthena Discord Bot\"}"
<< "}]"
<< "}";

send_discord_webhook(discord_server_webhook_url, json.str());
remove("discord_response.txt");
}

// Notify enchant success
void discord_notify_enchant(const char* player_name, const char* item_name, const char* enchant_name) {
if (discord_server_webhook_url.empty()) return;

ostringstream json;
json << "{"
<< "\"embeds\": [{"
<< "\"title\": \"Encantamento Aplicado!\","
<< "\"color\": 9127187,"  // Purple color
<< "\"fields\": ["
<< "{\"name\": \"Jogador\", \"value\": \"" << json_escape(player_name) << "\", \"inline\": true},"
<< "{\"name\": \"Item\", \"value\": \"" << json_escape(item_name) << "\", \"inline\": true},"
<< "{\"name\": \"Encantamento\", \"value\": \"" << json_escape(enchant_name) << "\", \"inline\": true}"
<< "],"
<< "\"footer\": {\"text\": \"rAthena Discord Bot\"}"
<< "}]"
<< "}";

send_discord_webhook(discord_server_webhook_url, json.str());
remove("discord_response.txt");
}

// Notify guild castle conquered
void discord_notify_castle_conquered(const char* castle_name, const char* guild_name) {
if (discord_server_webhook_url.empty()) return;

ostringstream json;
json << "{"
<< "\"embeds\": [{"
<< "\"title\": \"Castelo Conquistado!\","
<< "\"color\": 16711680,"  // Bright red
<< "\"fields\": ["
<< "{\"name\": \"Castelo\", \"value\": \"" << json_escape(castle_name) << "\", \"inline\": true},"
<< "{\"name\": \"Guilda\", \"value\": \"" << json_escape(guild_name) << "\", \"inline\": true}"
<< "],"
<< "\"footer\": {\"text\": \"rAthena Discord Bot\"}"
<< "}]"
<< "}";

send_discord_webhook(discord_server_webhook_url, json.str());
remove("discord_response.txt");
}

// Notify WoE start
void discord_notify_woe_start() {
if (discord_server_webhook_url.empty()) return;

ostringstream json;
json << "{"
<< "\"embeds\": [{"
<< "\"title\": \"War of Emperium Iniciou!\","
<< "\"description\": \"A batalha pelos castelos começou!\","
<< "\"color\": 16711680,"  // Bright red
<< "\"footer\": {\"text\": \"rAthena Discord Bot\"}"
<< "}]"
<< "}";

send_discord_webhook(discord_server_webhook_url, json.str());
remove("discord_response.txt");
}

// Notify WoE end
void discord_notify_woe_end() {
if (discord_server_webhook_url.empty()) return;

ostringstream json;
json << "{"
<< "\"embeds\": [{"
<< "\"title\": \"War of Emperium Terminou!\","
<< "\"description\": \"A batalha pelos castelos acabou!\","
<< "\"color\": 8421504,"  // Gray
<< "\"footer\": {\"text\": \"rAthena Discord Bot\"}"
<< "}]"
<< "}";

send_discord_webhook(discord_server_webhook_url, json.str());
remove("discord_response.txt");
}
