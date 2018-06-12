// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _RANDOM_HPP_
#define _RANDOM_HPP_

#ifdef __cplusplus
extern "C" {
#endif

#include "cbasetypes.hpp"

void rnd_init(void);

int32 rnd(void);// [0, SINT32_MAX]
uint32 rnd_roll(uint32 dice_faces);// [0, dice_faces)
int32 rnd_value(int32 min, int32 max);// [min, max]

#ifdef __cplusplus
}
#endif

#endif /* _RANDOM_HPP_ */
