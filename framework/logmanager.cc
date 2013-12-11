/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "root.h"
#include "logmanager.h"

#include "../simsys.h"
#include "../simdebug.h"


#ifdef SYSLOG
#include <syslog.h>
#endif //SYSLOG

#ifdef MAKEOBJ
#define debuglevel (3)
#else
#ifdef NETTOOL
#define debuglevel (0)

#else
#define debuglevel (umgebung_t::verbose_debug)

// for display ...
#include "../gui/messagebox.h"
#include "../display/simgraph.h"
#include "../gui/simwin.h"

#include "../dataobj/environment.h"
#endif
#endif

template<> logmanager_t* singleton<logmanager_t>::static_singleton = 0;

logmanager_t::logmanager_t()
	:default_log(0)
{

	root_t &root = root_t::get_singleton();

	printf("LOGMANAGER STARTED\n");

	/*** Begin logging set up ***/
	/*** process parameters passed to simutrans, and create a default log ***/

#ifdef SYSLOG
	bool cli_syslog_enabled = (gimme_arg( argc, argv, "-syslog", 0 ) != NULL);
	const char* cli_syslog_tag = gimme_arg( argc, argv, "-tag", 1 );
#else //SYSLOG
	bool cli_syslog_enabled = false;
	const char* cli_syslog_tag = NULL;
#endif //SYSLOG

	env_t::verbose_debug = 0;
	if(  root.gimme_arg("-debug", 0) != NULL  ) {
		const char *s = root.gimme_arg("-debug", 1);
		int level = 4;
		if(s!=NULL  &&  s[0]>='0'  &&  s[0]<='9'  ) {
			level = atoi(s);
		}
		env_t::verbose_debug = level;
	}

	const char *version = root_t::get_singleton_ptr()->get_version();

	if (  cli_syslog_enabled  ) {
		printf("syslog enabled\n");
		if (  cli_syslog_tag  ) {
			printf("Init logging with syslog tag: %s\n", cli_syslog_tag);
			create_log( "syslog", true, true, true, version, cli_syslog_tag );
		}
		else {
			printf("Init logging with default syslog tag\n");
			create_log( "syslog", true, true, true, version, "simutrans" );
		}
	}
	else if (root.gimme_arg("-log", 0)) {
		chdir( env_t::user_dir );
		char temp_log_name[256];
		const char *logname = "simu.log";
		if(  root.gimme_arg("-server", 0)  ) {
			const char *p = root.gimme_arg("-server", 1);
			int portadress = p ? atoi( p ) : 13353;
			sprintf( temp_log_name, "simu-server%d.log", portadress==0 ? 13353 : portadress );
			logname = temp_log_name;
		}
		create_log( logname, true, root.gimme_arg("-log", 0 ) != NULL, true, version, NULL );
	}
	else if (root.gimme_arg("-debug", 0) != NULL) {
		create_log( "stderr", true, root.gimme_arg("-debug", 0 ) != NULL, true, version, NULL );
	}
	else {
		create_log(NULL, false, false, true, version, NULL);
	}
	/*** End logging set up ***/

}


log_t* logmanager_t::create_log(const char* logname, bool force_flush, bool log_debug, bool log_console, const char* greeting, const char* syslogtag )
{
	log_t *log = new log_t(logname, force_flush, log_debug, log_console, greeting, syslogtag);

	log_list.insert( log );

	if(!default_log) {
		default_log = log;
		dbg = log;
	}
	return log;
}

void logmanager_t::destroy_log(log_t *log)
{
	log_list.erase(log);
}


logmanager_t::~logmanager_t()
{
	std::set<log_t*>::iterator i;

	for ( i= log_list.begin(); i != log_list.end(); ++i ){
		delete *i;
	}
	printf("LOGMANAGER STOPPED\n");
}
