/*
 * Copyright (c) 1997 - 2003 Hansjörg Malthaner
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#ifndef obj_reader_task_h
#define obj_reader_task_h

#include "../task_long_steppable.h"
#include "../../utils/searchfolder.h"
#include "../../simloadingscreen.h"

class searchfolderop_t;
class objreadop_t;

class pakloader_task_t : public task_long_steppable_t
{
public:
	pakloader_task_t();
	~pakloader_task_t();
	bool init_task_impl();
	void step();
private:

	enum state_t {
		STATE_WAITFIND,
		STATE_PAK,
		STATE_ADDON
	};

	void state_waitfind();
	void state_pak();
	void check_addons();
	void state_addon();
	state_t current_state;

	int progressbar_step;

	loadingscreen_t *ls;

	searchfolder_t *find;

	searchfolderop_t *op_paks;
	searchfolderop_t *op_addons;

	objreadop_t *op_read;

	searchfolder_t::const_iterator iterator;

	uint n;
	bool drawing;

};

#endif