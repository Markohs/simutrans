/*
 *  Copyright (c) 2013 The Simutrans Community
 *
 */

#ifndef SEARCHFOLDEROP_H
#define SEARCHFOLDEROP_H

#include "../resourceop.h"
#include "../../utils/searchfolder.h"

class searchfolderop_t : public resourceop_t {
public:
	void exec();
	searchfolderop_t(const std::string &filepath, const std::string &extension, const bool only_directories = false, const bool prepend_path = true);
	~searchfolderop_t();
	searchfolder_t *get_searchfolder();
private:
	searchfolder_t *sf;
	const std::string filepath;
	const std::string extension;
	const bool only_directories;
	const bool prepend_path;

	int result;
};

#endif