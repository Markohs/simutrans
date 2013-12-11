/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#ifndef world_step_task_h
#define world_step_task_h

#include "../task_long_steppable.h"
class karte_t;

class world_step_task_t : public task_long_steppable_t
{
public:
	world_step_task_t( karte_t *world );
	~world_step_task_t();

	void reset();

private:
	bool init_task_impl();
	void step();

	karte_t *world;
	unsigned long last_time;
};

#endif