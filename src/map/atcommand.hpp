#ifndef ATCOMMAND_HPP
#define ATCOMMAND_HPP

#include "../common/cbasetypes.hpp"

struct map_session_data;

// Command definitions
#define ATCOMMAND_LENGTH 50
#define ACMD(x) int atcommand_##x(const int fd, struct map_session_data* sd, const char* command, const char* message)
#define ACMD_DEF(x) ACMD(x)
#define ACMD_FUNC(x) int atcommand_ ## x(const int fd, struct map_session_data* sd, const char* command, const char* message)

typedef int (*AtCommandFunc)(const int fd, struct map_session_data* sd, const char* command, const char* message);

bool is_atcommand(const int fd, struct map_session_data* sd, const char* message, int type);
void add_atcommand(const char* name, AtCommandFunc func);

#endif // ATCOMMAND_HPP
