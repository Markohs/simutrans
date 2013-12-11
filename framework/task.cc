/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include "task.h"
#include "taskmanager.h"

task_t::task_t( const char *task_name,bool auto_schedule )
{
	this->task_name = task_name;
	if(auto_schedule){
		taskmanager_t::get_singleton().add_task(this);
	}	
	state = STATE_NEEDS_INIT;
}

const char * task_t::get_name()
{
	return task_name;
}

task_t::state_enum_t task_t::get_state()
{
	return state;
}

task_t::~task_t()
{

}
