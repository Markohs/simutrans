/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

/*
 * Framework root singleton
 */

#ifndef eventmanager_h
#define eventmanager_h

#include "singleton.h"
#include "eventlistener.h"
#include <set>

struct eventlistener_entry_compare_t
{
	bool operator() (const eventlistener_entry_t& x,const eventlistener_entry_t& y) const {
		if(x.priority == y.priority){
			return x.listener < y.listener;
		}
		
		return (x.priority < y.priority);
	}
};

class eventmanager_t : public singleton<eventmanager_t>
{
public:
	eventmanager_t();
	~eventmanager_t();

	void add_eventlistener(eventlistener_entry_t &listener);
	void remove_eventlistener(eventlistener_entry_t &listener);

	void message_pump();

private:

	void fire_listeners(event_t *ev);

	std::set<eventlistener_entry_t,eventlistener_entry_compare_t> eventlisteners;
};

#endif