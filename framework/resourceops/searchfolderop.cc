/*
 *  Copyright (c) 2013 The Simutrans Community
 */

#include "searchfolderop.h"

searchfolderop_t::searchfolderop_t( const std::string &_filepath, const std::string &_extension, const bool _only_directories , const bool _prepend_path )
:filepath(_filepath),
extension(_extension),
only_directories(_only_directories),
prepend_path(_prepend_path)
{
	sf = new searchfolder_t;
}

void searchfolderop_t::exec()
{
	result = sf->search(filepath,extension,only_directories,prepend_path);
	set_complete();
}

searchfolder_t * searchfolderop_t::get_searchfolder()
{
	return sf;
}

searchfolderop_t::~searchfolderop_t()
{
	delete sf;
}

