/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

/*
 * GUI Manager
 */

#ifndef taskmanager_h
#define taskmanager_h

#include "singleton.h"
#include "task_long_steppable.h"

#include <list>
#include <set>

class taskmanager_t : public singleton<taskmanager_t>
{
public:
	taskmanager_t();
	~taskmanager_t();

	void add_task(task_t *task);
	void remove_task(task_t *task);
	void schedule_execution(long delta);
private:
	std::list<task_t *> new_task_list;
	std::set<task_t *>	current_task_list;
};

#endif