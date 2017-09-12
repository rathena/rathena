// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _RANDOM_H_
#define _RANDOM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cbasetypes.h"

void rnd_init(void);
void rnd_seed(uint32);

int32 rnd(void);// [0, SINT32_MAX]
uint32 rnd_roll(uint32 dice_faces);// [0, dice_faces)
int32 rnd_value(int32 min, int32 max);// [min, max]
double rnd_uniform(void);// [0.0, 1.0)
double rnd_uniform53(void);// [0.0, 1.0)

#ifdef __cplusplus
}
#endif

#endif /* _RANDOM_H_ */
