/*
 * Copyright (c) Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

/*
 * Resource Operation
 */

#ifndef resource_operation_h
#define resource_operation_h

class resourceop_t {
public:
	resourceop_t()
		:complete(0)
	{};
	virtual void exec() = 0;
	void set_complete() {complete=true;};
	bool is_complete()  {return complete;};
private:
	bool complete;
};
#endif