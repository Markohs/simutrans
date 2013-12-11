/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

/*
 * Framework root singleton
 */

#ifndef eventlistener_h
#define eventlistener_h

#include "../simevent.h"

class eventlistener_t;

enum eventlistener_prio {
	EV_PRIO_WINDOWMANAGER = 0,
	EV_PRIO_LOADINGSCREEN = 10,
	EV_PRIO_INTERACTIVE = 20
};

struct eventlistener_entry_t
{
	eventlistener_prio	priority;
	eventlistener_t		*listener;
};

class eventlistener_t
{
public:
	/**
	 * @return true, if this event listener wants to swallow the event
	 */
	virtual bool process_event(event_t *ev) = 0;

	eventlistener_t(eventlistener_prio priority);
	~eventlistener_t();
/*
	virtual void frameended() = 0;
	*/
private:

	eventlistener_entry_t eventlistenerentry;
};


#endif