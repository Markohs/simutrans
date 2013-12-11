/*
 * Copyright (c) 1997 - 2001 Hj. Malthaner
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#ifndef simworldeventmanager_h
#define simworldeventmanager_h

#include "framework/eventlistener.h"
#include "simworld.h"

class simworldeventmanager_t : public eventlistener_t
{
public:
	simworldeventmanager_t(karte_t *world);

	bool process_event( event_t *ev );

private:
	karte_t *world;
};

#endif
