/*
 * Copyright (c) 1997 - 2003 Hansjörg Malthaner
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include "eventmanager.h"
#include <assert.h>
#include <stdio.h>

template<> eventmanager_t* singleton<eventmanager_t>::static_singleton = 0;

eventmanager_t::eventmanager_t()
{
	printf("EVENT MANAGER STARTED\n");
}

eventmanager_t::~eventmanager_t()
{
	printf("EVENT MANAGER STOPPED\n");
}

void eventmanager_t::add_eventlistener( eventlistener_entry_t &listener )
{
	printf("Adding eventlistener\n");
	eventlisteners.insert(listener);
}

void eventmanager_t::remove_eventlistener( eventlistener_entry_t &listener )
{
	printf("Removing eventlistener\n");
	int deleted = eventlisteners.erase(listener);
	if (deleted == 0)
		printf("******ERROR, deleted %i eventlisteners ******\n",deleted);
}

void eventmanager_t::message_pump()
{

	event_t ev;

	display_poll_event(&ev);

	while (ev.ev_class != EVENT_NONE){
		fire_listeners(&ev);
		display_poll_event(&ev);
	}
}

void eventmanager_t::fire_listeners( event_t *ev )
{
	std::set<eventlistener_entry_t>::iterator i;

	for (i= eventlisteners.begin(); i != eventlisteners.end(); ++i)
	{
		if((*i).listener->process_event(ev)){
			break;
		}
	}
}
