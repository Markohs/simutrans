/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include "task_steppable.h"
#include "taskmanager.h"
#include "../simsys.h"

task_steppable_t::task_steppable_t( const char *task_name,bool auto_schedule )
	:task_t(task_name,auto_schedule)
{

}

task_steppable_t::~task_steppable_t()
{

}

void task_steppable_t::complete()
{
	state = task_t::STATE_DONE;
}

bool task_steppable_t::init_task()
{
	state = task_t::STATE_RUNNABLE;
	return true;
}

