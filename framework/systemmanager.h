/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

/*
 * GUI Manager
 */

#ifndef systemmanager_h
#define systemmanager_h

#include "singleton.h"
#include "eventlistener.h"

#include "../simsys.h"

class systemmanager_t : public singleton<systemmanager_t>
{
public:
	systemmanager_t();
	~systemmanager_t();
};

#endif