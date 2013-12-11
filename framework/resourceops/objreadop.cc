/*
 *  Copyright (c) 2013 The Simutrans Community
 */

#include "objreadop.h"


void objreadop_t::exec()
{
	obj_reader_t::read_file(filename);
	set_complete();
}

objreadop_t::objreadop_t( const char *filename )
{
	this->filename = filename;
}
