/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

/*
* Framework root singleton
 */

#ifndef logmanager_h
#define logmanager_h

#include "singleton.h"
#include "../simtypes.h"
#include "../utils/log.h"

#include <set>

class logmanager_t : public singleton<logmanager_t>
{
public:

	log_t *create_log(const char* logname, bool force_flush, bool log_debug, bool log_console, const char* greeting, const char* syslogtag=NULL );
	void destroy_log(log_t *log);

	/**
	 * Creates the log manager, also sets up the default logging according to the passed parameters
	 */
	logmanager_t();
	~logmanager_t();
protected:
	/// A list of all the logs the manager can access
	std::set<log_t *> log_list;


	log_t *default_log;
};

#endif