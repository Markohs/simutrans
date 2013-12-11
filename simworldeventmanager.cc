/*
 * Copyright (c) 1997 - 2001 Hj. Malthaner
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include "simworldeventmanager.h"

#include "framework/eventmanager.h"
#include "dataobj/environment.h"
#include "gui/simwin.h"
#include "simtools.h"
#include "simevent.h"
#include "siminteraction.h"

simworldeventmanager_t::simworldeventmanager_t( karte_t *world )
	:eventlistener_t(EV_PRIO_INTERACTIVE)
{
	this->world = world;
}

bool simworldeventmanager_t::process_event( event_t *ev )
{
	 world->get_eventmanager()->process_event(*ev);
	 return true;
}
