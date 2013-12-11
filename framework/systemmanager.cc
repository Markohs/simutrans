/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include "systemmanager.h"
#include "root.h"
#include "../simsys.h"
#include "../simdebug.h"
#include "../display/simgraph.h"

template<> systemmanager_t* singleton<systemmanager_t>::static_singleton = 0;

systemmanager_t::systemmanager_t()
{
	printf("SYSTEM MANAGER STARTED\n");

	root_t &root = root_t::get_singleton();

	static const sint16 resolutions[][2] = {
		{  640,  480 },
		{  800,  600 },
		{ 1024,  768 },
		{ 1280, 1024 },
		{  704,  560 } // try to force window mode with allegro
	};

	sint16 disp_width = 0;
	sint16 disp_height = 0;
	sint16 fullscreen = false;

	// likely only the program without graphics was downloaded
	if (root.gimme_arg("-res", 0) != NULL) {
		const char* res_str = root.gimme_arg("-res", 1);
		const int res = *res_str - '1';

		switch (res) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			fullscreen = (res<=4);
			disp_width  = resolutions[res][0];
			disp_height = resolutions[res][1];
			break;

		default:
			fprintf(stderr,
				"invalid resolution, argument must be 1,2,3 or 4\n"
				"1=640x480, 2=800x600, 3=1024x768, 4=1280x1024, 5=windowed\n"
				);
			// return 0; TODO
		}
	}

	fullscreen |= (root.gimme_arg("-fullscreen", 0) != NULL);

	if(root.gimme_arg("-screensize", 0) != NULL) {
		const char* res_str = root.gimme_arg("-screensize", 1);
		int n = 0;

		if (res_str != NULL) {
			n = sscanf(res_str, "%hdx%hd", &disp_width, &disp_height);
		}

		if (n != 2) {
			fprintf(stderr,
				"invalid argument for -screensize option\n"
				"argument must be of format like 800x600\n"
				);
			//return 1; TODO
		}
	}

	int parameter[2];
	parameter[0] = root.gimme_arg("-async", 0) != NULL;
	parameter[1] = root.gimme_arg("-use_hw", 0) != NULL;
	if (!dr_os_init(parameter)) {
		dr_fatal_notify("Failed to initialize backend.\n");
		//return EXIT_FAILURE; TODO
	}

	// Get optimal resolution.
	if (disp_width == 0 || disp_height == 0) {
		resolution const res = dr_query_screen_resolution();
		if (fullscreen) {
			disp_width  = res.w;
			disp_height = res.h;
		} else {
			disp_width  = min(704, res.w);
			disp_height = min(560, res.h);
		}
	}

	dbg->important("Preparing display ...");
	DBG_MESSAGE("simmain", "simgraph_init disp_width=%d, disp_height=%d, fullscreen=%d", disp_width, disp_height, fullscreen);
	simgraph_init(disp_width, disp_height, fullscreen);
	DBG_MESSAGE("simmain", ".. results in disp_width=%d, disp_height=%d", display_get_width(), display_get_height());

}

systemmanager_t::~systemmanager_t()
{
	printf("SYSTEM MANAGER STOPPED\n");
}