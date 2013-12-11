/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

/*
 * GUI Manager
 */

#ifndef guimanager_h
#define guimanager_h

#include "singleton.h"
#include "framelistener.h"
#include "eventlistener.h"

#include "../gui/gui_frame.h"

class guimanager_t : public singleton<guimanager_t>, public framelistener_t, public eventlistener_t
{
public:
	guimanager_t();
	~guimanager_t();

	void add_simple_frame(gui_frame_t *item);

	void framestarted();

	void frameended();

	bool process_event(event_t *ev);

private:
	gui_frame_t *gui_root;

};

#endif