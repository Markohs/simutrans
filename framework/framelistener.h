/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

/*
 * Framework root singleton
 */

#ifndef framelistener_h
#define framelistener_h

class framelistener_t
{
public:
	virtual void framestarted() = 0;
//	virtual void frameended() = 0;
};

#endif