/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#ifndef task_long_steppable_h
#define task_long_steppable_h

#include "task.h"

class task_long_steppable_t : public task_t
{
public:
	task_long_steppable_t(const char *task_name,bool auto_schedule=true);
	~task_long_steppable_t();
	bool run( unsigned long delta );
	void complete();
	bool init_task();
	virtual bool init_task_impl() = 0;
	virtual void step()=0;
};

#endif