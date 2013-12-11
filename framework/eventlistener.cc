/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include "eventlistener.h"
#include "eventmanager.h"

eventlistener_t::eventlistener_t( eventlistener_prio priority )
{
	eventlistenerentry.priority=priority;
	eventlistenerentry.listener=this;

	eventmanager_t::get_singleton_ptr()->add_eventlistener(eventlistenerentry);
	//eventmanager_t::get_singleton().add_eventlistener(eventlistenerentry);
}

eventlistener_t::~eventlistener_t()
{
	eventmanager_t::get_singleton_ptr()->remove_eventlistener(eventlistenerentry);
}
