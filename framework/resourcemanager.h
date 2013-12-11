/*
 * Copyright (c) Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

/*
 * Resource Manager, devoted to async I/O file operations.
 */

#ifndef resourcemanager_h
#define resourcemanager_h

#include "singleton.h"

#include <pthread.h>
#include <semaphore.h>
#include <list>

class resourceop_t;

enum resource_operation_type {
	OP_FIND = 1
};

struct resource_result {
	void *data;
	bool completed;
};

class resourcemanager_t : public singleton<resourcemanager_t>
{
	friend void *bootstrap( void *ptr );
public:
	resourcemanager_t();
	~resourcemanager_t();
	void queue_operation(resourceop_t *operation);
private:
	void worker_thread();
	std::list<resourceop_t*> queued_tasks;
	pthread_t worker_id;
	sem_t worker_semaphore;
};

#endif