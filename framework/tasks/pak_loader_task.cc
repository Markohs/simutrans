/*
 * Copyright (c) 1997 - 2003 Hansjörg Malthaner
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include "pak_loader_task.h"
#include "../../besch/reader/obj_reader.h"
#include "../../display/simgraph.h"

#include "../../besch/skin_besch.h"	// just for the logo
#include "../../besch/grund_besch.h"	// for the error message!
#include "../../simskin.h"

#include "../../simsys.h"

// normal stuff
#include "../../dataobj/translator.h"
#include "../../dataobj/environment.h"

#include "../resourceops/searchfolderop.h"
#include "../resourceops/objreadop.h"
#include "../resourcemanager.h"

pakloader_task_t::pakloader_task_t( )
	:task_long_steppable_t("pakloader_task_t")
{
}

bool pakloader_task_t::init_task_impl()
{

	const char *path = env_t::objfilename.c_str();
	const char *message=translator::translate("Loading paks ...");
	std::string name = find->complete(path, "dat");
	drawing=is_display_init();

	// Start the find operation, find addons too if requested
	resourcemanager_t& rm = resourcemanager_t::get_singleton();
	op_paks = new searchfolderop_t(path,"pak");
	rm.queue_operation(op_paks);
	if(  env_t::default_settings.get_with_private_paks()  ) {
		op_addons = new searchfolderop_t( ("addons/" + env_t::objfilename).c_str() , "pak" );
		rm.queue_operation(op_addons);
	}
	else {
		op_addons = NULL;
	}

	// Get the logo
	if(drawing  &&  skinverwaltung_t::biglogosymbol==NULL) {
		display_fillbox_wh( 0, 0, display_get_width(), display_get_height(), COL_BLACK, true );
		obj_reader_t::read_file((name+"symbol.BigLogo.pak").c_str());
		DBG_MESSAGE("obj_reader_t::load()","big logo %p", skinverwaltung_t::biglogosymbol);
	}

	// Init the loading screen

	ls = new loadingscreen_t( message, 0, true );


	// Extract copyright, to update the loading screen

	if(  grund_besch_t::ausserhalb==NULL  ) {
		// defining the pak tile witdh ....
		obj_reader_t::read_file((name+"ground.Outside.pak").c_str());
		if(grund_besch_t::ausserhalb==NULL) {
			dbg->warning("obj_reader_t::load()","ground.Outside.pak not found, cannot guess tile size! (driving on left will not work!)");
		}
		else {
			if (char const* const copyright = grund_besch_t::ausserhalb->get_copyright()) {
				ls->set_info(copyright);
			}
		}
	}

	current_state = STATE_WAITFIND;

	DBG_MESSAGE("obj_reader_t::load()", "reading from '%s'", name.c_str());

	return true;
}

void pakloader_task_t::check_addons()
{
	bool need_more_process = false;
	if(  env_t::default_settings.get_with_private_paks()  ) {
		// try to read addons from private directory
		chdir( env_t::user_dir );

		find = new searchfolder_t();
		const int max = find->search(("addons/" + env_t::objfilename).c_str(), "pak");

		if(max != 0)		{
			need_more_process = true;
			ls->set_what(translator::translate("Loading addon paks ..."));
			ls->set_max(max);

			iterator = find->begin();
			n=0;

			current_state = STATE_ADDON;
		}
		else{
			fprintf(stderr, "reading addon object data failed (disabling).\n");
			env_t::default_settings.set_with_private_paks( false );
		}

		chdir( env_t::program_dir );
	}

	if(!need_more_process){
		complete();
	}
}

void pakloader_task_t::state_pak()
{

	resourcemanager_t& rm = resourcemanager_t::get_singleton();

	// Pending operation?
	if(op_read != NULL) {
		// We have a queued operation, check if its completed
		if(op_read->is_complete()){
			// We completed it, update the bar
			ls->set_progress(++n);
			// Advance the iterator
			iterator++;
			// Go idle
			delete op_read;
			op_read = NULL;
		}
		else {
			// Nothing to do, yield our CPU.
			return;
		}
	}

	// Are we done?
	if(iterator == find->end()){
		check_addons();
		return;
	}

	if(op_read == NULL) {
		// We are idle, queue one operation
		// Create a operation to read the file
		op_read = new objreadop_t(*iterator);
		// Start the operation
		rm.queue_operation(op_read);
	}
}

void pakloader_task_t::state_addon(){

	if(iterator == find->end()){
		complete();
		return;
	}

	obj_reader_t::read_file(*iterator);

	ls->set_progress(++n);

	iterator++;

}

void pakloader_task_t::step()
{
	switch(current_state){
	case  STATE_WAITFIND:
		state_waitfind();
		break;
	case STATE_PAK:
		state_pak();
		break;
	case STATE_ADDON:
		state_addon();
	}
}


void pakloader_task_t::state_waitfind()
{
	// Do we have the file listing ready?
	if(!op_paks->is_complete()){
		return;
	}

	// Set-up file reading environment, and change state.
	size_t i;
	n = 0;

	find=op_paks->get_searchfolder();

	ls->set_max(find->get_num_entries());

	iterator = find->begin();

	op_read = NULL;

	current_state = STATE_PAK;

}


pakloader_task_t::~pakloader_task_t()
{
	if(ls){
		delete ls;
	}

	if(op_paks){
		delete op_paks;
	}

}
