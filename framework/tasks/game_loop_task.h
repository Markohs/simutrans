/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#ifndef game_loop_task_h
#define game_loop_task_h

#include "../task_steppable.h"
#include "../../simworld.h"

class game_loop_task_t : public task_steppable_t
{
public:
	game_loop_task_t();
	~game_loop_task_t();
	bool run(unsigned long delta);
private:
//	karte_t *world;
//	unsigned long last_time;
};

#endif