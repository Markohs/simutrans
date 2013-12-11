/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include "task_long_steppable.h"
#include "taskmanager.h"
#include "../simsys.h"

task_long_steppable_t::task_long_steppable_t( const char *task_name,bool auto_schedule )
	:task_t(task_name,auto_schedule)
{

}

task_long_steppable_t::~task_long_steppable_t()
{

}

bool task_long_steppable_t::run( unsigned long delta )
{
	const unsigned long t_start = dr_time();
	const unsigned long t_max   = t_start + delta;

	while ( (state == task_t::STATE_RUNNABLE)  &&  (dr_time() < t_max)){
		step();
	}

	return true;
}

void task_long_steppable_t::complete()
{
	state = task_t::STATE_DONE;
}

bool task_long_steppable_t::init_task()
{
	if(init_task_impl()){
		state = task_t::STATE_RUNNABLE;
		return true;
	}
	else{
		state = task_t::STATE_ERROR;
		return false;
	}
}

