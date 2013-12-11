/*
 *  Copyright (c) 2013 The Simutrans Community
 *
 */

#ifndef OBJREADOP_H
#define OBJREADOP_H

#include "../resourceop.h"
#include "../../besch/reader/obj_reader.h"

class objreadop_t : public resourceop_t {
public:
	void exec();
	objreadop_t(const char *filename);
private:
	const char *filename;
	int result;
};

#endif