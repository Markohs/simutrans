/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include "root.h"
#include "../simversion.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../dataobj/environment.h"
#include "../simsys.h"

#include "eventmanager.h"
#include "framelistener.h"
#include "guimanager.h"
#include "logmanager.h"
#include "systemmanager.h"
#include "taskmanager.h"
#include "resourcemanager.h"

#define MIN_FRAME_TIME_MS 33

template<> root_t* singleton<root_t>::static_singleton = 0;

void root_t::add_framelistener(framelistener_t* listener)
{
	printf("Adding framelistener\n");
	framelisteners.insert(listener);
}

void root_t::remove_framelistener(framelistener_t* listener)
{
	printf("Removing framelistener\n");
	int deleted = framelisteners.erase(listener);
	if (deleted == 0)
		printf("******ERROR, deleted %i framelisteners ******\n",deleted);

}


void root_t::start(void)
{
 /*       assert(mActiveRenderer != 0);

		mActiveRenderer->_initRenderTargets();

		// Clear event times
		clearEventTimes();

		// Infinite loop, until broken out of by frame listeners
		// or break out by calling queueEndRendering()
		mQueuedEnd = false;
		*/
	eventmanager_t &evm = eventmanager_t::get_singleton();
	taskmanager_t &tkm = taskmanager_t::get_singleton();

	unsigned long next_frame_start = dr_time();

	while( !env_t::quit_simutrans ){
		//Pump messages in all registered RenderWindow windows
		//WindowEventUtilities::messagePump();

		unsigned long start_t;

		long wait_time = next_frame_start - dr_time() - 1;

		// Only sleep if we have more than 5 ms

		if (wait_time > 4){
			dr_sleep(wait_time);
		}

		while ((start_t = dr_time())<next_frame_start){
			//NOTHING, ACTIVE WAIT
		}

		dr_prepare_flush();
		evm.message_pump();
		fire_framestarted();

		unsigned long end_t = dr_time();

		long remaining_time = MIN_FRAME_TIME_MS - (end_t - start_t);
		if (remaining_time <= 0){
			remaining_time = 0;
		}

		tkm.schedule_execution(remaining_time);

		fire_frameended();
		dr_flush();

		next_frame_start = start_t + MIN_FRAME_TIME_MS;
	}
}

void root_t::fire_framestarted()
{
	std::set<framelistener_t*>::iterator i;

	for (i= framelisteners.begin(); i != framelisteners.end(); ++i)
	{
		(*i)->framestarted();
	}
}

void root_t::fire_frameended()
{
}


root_t::root_t(int argc, char** argv)
{
	printf("ROOT_T STARTED\n");

#ifdef REVISION
	version = "Simutrans version " VERSION_NUMBER " from " VERSION_DATE " r" QUOTEME(REVISION) "\n";
#else
	version = "Simutrans version " VERSION_NUMBER " from " VERSION_DATE "\n";
#endif

	this->argc = argc;
	this->argv = argv;

	logmanager = new logmanager_t();
	systemmanager = new systemmanager_t();
	eventmanager = new eventmanager_t();
	resourcemanager = new resourcemanager_t();
	guimanager = new guimanager_t();
	taskmanager = new taskmanager_t();

}

root_t::~root_t()
{
	printf("ROOT_T DESTROYED\n");

	delete taskmanager;
	delete guimanager;
	delete resourcemanager;
	delete eventmanager;
	delete systemmanager;
	delete logmanager;
}

const char *root_t::get_version() const{
	return version;
}


const char *root_t::gimme_arg(const char *arg, int off)
{
	for(  int i = 1;  i < argc;  i++  ) {
		printf("i = %i\n",i);
		if(strcmp(argv[i], arg) == 0  &&  i < argc - off  ) {
			return argv[i + off];
		}
	}
	return NULL;
}