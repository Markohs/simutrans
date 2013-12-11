/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

/*
 * Framework root singleton
 */

#ifndef root_h
#define root_h

#include "singleton.h"

#include <set>

class framelistener_t;

class logmanager_t;
class systemmanager_t;
class resourcemanager_t;
class guimanager_t;
class eventmanager_t;
class taskmanager_t;

class root_t : public singleton<root_t>
{
public:
	root_t(int argc, char** argv);
	~root_t();
	const char *get_version() const;

	void start();
	bool render_one_frame();

	void add_framelistener(framelistener_t* listener);
	void remove_framelistener(framelistener_t* listener);

	void fire_framestarted();
	void fire_frameended();

	const char *gimme_arg(const char *arg, int off);

private:
	logmanager_t *logmanager;
	systemmanager_t *systemmanager;
	guimanager_t *guimanager;
	resourcemanager_t *resourcemanager;
	eventmanager_t *eventmanager;
	taskmanager_t *taskmanager;

	std::set<framelistener_t*> framelisteners;

	int argc;
	char** argv;

protected:
	const char *version;
};

#endif