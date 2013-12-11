/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#ifndef task_steppable_h
#define task_steppable_h

#include "task.h"

class task_steppable_t : public task_t
{
public:
	task_steppable_t(const char *task_name,bool auto_schedule=true);
	~task_steppable_t();
	virtual bool run( unsigned long delta ) = 0;
	void complete();
	bool init_task();
};

#endif