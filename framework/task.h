/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#ifndef task_h
#define task_h

class task_t
{
public:

	enum state_enum_t {
		STATE_NEEDS_INIT=0,
		STATE_RUNNABLE,
		STATE_ERROR,
		STATE_DONE
	};

	task_t(const char *task_name,bool auto_schedule=true);
	~task_t();
	const char *get_name();
	state_enum_t get_state();
	virtual bool init_task() {state=STATE_RUNNABLE;return true;};
	/**
	 * @param delta How much time do we have
	 */
	virtual bool run( unsigned long delta ) = 0;
private:
	const char *task_name;
protected:
	state_enum_t state;
};

#endif