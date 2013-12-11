/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include "world_sync_task.h"
#include "../../simsys.h"
#include "../../simworld.h"



bool world_sync_task_t::run( unsigned long delta )
{
	unsigned long new_time = dr_time();

	unsigned long elapsed_time = new_time - last_time;

	last_time = new_time;

	world->sync_step( elapsed_time, true, true );
//	printf("Syncing %lu ms\n",elapsed_time);
	
	return true;
}

world_sync_task_t::world_sync_task_t( karte_t *world )
	:task_steppable_t("world_sync_task_t")
{
	this->world = world;

	last_time = dr_time();

}
