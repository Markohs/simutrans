/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#ifndef world_sync_task_h
#define world_sync_task_h

#include "../task_steppable.h"
#include "../../simworld.h"

class world_sync_task_t : public task_steppable_t
{
public:
	world_sync_task_t( karte_t *world );
	~world_sync_task_t();
	bool run(unsigned long delta);
private:
	karte_t *world;
	unsigned long last_time;
};

#endif