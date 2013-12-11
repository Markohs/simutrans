/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include "guimanager.h"
#include "root.h"
#include <assert.h>
#include <stdio.h>

#include "../gui/simwin.h"
#include "../display/simgraph.h"
#include "../simsys.h"
#include "eventmanager.h"

template<> guimanager_t* singleton<guimanager_t>::static_singleton = 0;

guimanager_t::guimanager_t()
	:eventlistener_t(EV_PRIO_WINDOWMANAGER)
{
	printf("GUI MANAGER STARTED\n");
	gui_root = NULL;
	root_t::get_singleton_ptr()->add_framelistener(this);

}

guimanager_t::~guimanager_t()
{
	root_t::get_singleton_ptr()->remove_framelistener(this);
//	eventmanager_t::get_singleton_ptr()->remove_eventlistener(eventlistener_entry_t(EV_PRIO_WINDOWMANAGER,this));
	printf("GUI MANAGER STOPPED\n");
}

void guimanager_t::framestarted()
{
	if(gui_root) {
//		dr_prepare_flush();
		display_all_win();
//		gui_root->zeichnen(win_get_pos(gui_root), gui_root->get_fenstergroesse());
//		dr_flush();
	}
}

void guimanager_t::frameended()
{
	throw std::exception("The method or operation is not implemented.");
}

void guimanager_t::add_simple_frame( gui_frame_t *item )
{
	gui_root = item;

	koord pos( (display_get_width()-gui_root->get_fenstergroesse().x)/2 , (display_get_height()-gui_root->get_fenstergroesse().y)/2 );

	create_win( pos.x , pos.y , gui_root, w_info, magic_none );
}

bool guimanager_t::process_event( event_t *ev )
{
	// SIMWIN.CC <= RESIZE AND THEME RELOAD
	win_poll_event(ev);

	// SIMWIN.CC <= Window events
	return check_pos_win(ev);

//	printf("guimanager_t::process_event received message, class: %i\n",ev->ev_class);
}

