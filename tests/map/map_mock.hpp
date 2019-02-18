#include "../../src/map/map.hpp"

#define map_id2sd map_id2sd_mock

struct map_session_data* map_id2sd_mock(int id) {
	return (struct map_session_data*)0x0;
}
