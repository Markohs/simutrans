/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include "world_step_task.h"
#include "../taskmanager.h"

#include "../../simsys.h"
#include "../../simworld.h"


world_step_task_t::world_step_task_t( karte_t *world )
	:task_long_steppable_t("world_step_task_t")
{
	this->world = world;
}

bool world_step_task_t::init_task_impl()
{
	return true;
}

void world_step_task_t::step()
{
	world->step();
	complete();
}

void world_step_task_t::reset()
{
	taskmanager_t &tm = taskmanager_t::get_singleton();

	state = STATE_RUNNABLE;

	tm.add_task(this);
}
