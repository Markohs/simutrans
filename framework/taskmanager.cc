/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include "taskmanager.h"

#include "../simsys.h"
#include "../simdebug.h"
#include <list>

template<> taskmanager_t* singleton<taskmanager_t>::static_singleton = 0;

taskmanager_t::taskmanager_t()
{
	printf("TASK MANAGER STARTED\n");
}

taskmanager_t::~taskmanager_t()
{
	printf("TASK MANAGER STARTED\n");
}

void taskmanager_t::add_task( task_t *task )
{
	new_task_list.push_back(task);
}

void taskmanager_t::remove_task( task_t *task )
{
	/*task_list.erase(task);*/
}

void taskmanager_t::schedule_execution( long delta )
{

	unsigned long start = dr_time();
	unsigned long max_t = start + delta;

	/*
	 * Process newly added tasks
	 */
	while(!new_task_list.empty()){
		task_t *item = new_task_list.front();
		new_task_list.pop_front();

		unsigned long start = dr_time();

		if(item->get_state()==task_t::STATE_NEEDS_INIT){
			item->init_task();
		}

		unsigned long end = dr_time();

		if((end-start) > 33) {
			DBG_DEBUG4("taskmanager_t::schedule_execution()","Initialization of task \"%s\" took %lu milliseconds, SLOW!!!\n",item->get_name(),end-start);
			printf("taskmanager_t::schedule_execution(): Initialization of task \"%s\" took %lu milliseconds, SLOW!!!\n",item->get_name(),end-start);
		}

		if(item->get_state() == task_t::STATE_RUNNABLE){
			current_task_list.insert(item);
		}
		else {
			printf("Error initializing task \"%s\"",item->get_name());
		}


	}

	/*
	 * Run queue
	 */

	std::list<task_t*> to_remove;

	for (std::set<task_t*>::iterator i= current_task_list.begin(); i != current_task_list.end(); ++i)
	{
		long remaining_time = max_t - dr_time();

		if (remaining_time <= 0) {
			continue;
		}

		if( (*i)->get_state() == task_t::STATE_RUNNABLE ) {
			(*i)->run(remaining_time);

			if( (*i)->get_state() == task_t::STATE_DONE ) {
				to_remove.push_front(*i);
			}
		}
	}

	// remove completed tasks
	while(!to_remove.empty()){
		task_t *item = to_remove.front();
		to_remove.pop_front();
		current_task_list.erase(item);
	}

	unsigned long end = dr_time();

	if(end > max_t) {
		DBG_DEBUG4("taskmanager_t::schedule_execution()","Execution took too long");
		printf("taskmanager_t::schedule_execution(): Execution took too long, %lu ms\n",end - start);
	}
	else {
//		printf("taskmanager_t::schedule_execution(): Execution *OK*, took %lu ms\n",end - start);
	}

}
