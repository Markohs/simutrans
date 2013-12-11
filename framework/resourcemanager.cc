/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include "resourcemanager.h"
#include "resourceop.h"
#include <stdio.h>

template<> resourcemanager_t* singleton<resourcemanager_t>::static_singleton = 0;

void *bootstrap( void *ptr )
{
	resourcemanager_t::get_singleton_ptr()->worker_thread();
	return 0;
}

resourcemanager_t::resourcemanager_t()
{
	printf("RESOURCE MANAGER STARTED\n");
	
	//ignored error code
	if(pthread_create( &worker_id, NULL, bootstrap, 0)){
		printf("********WE CAN'T CREATE WORKER THREAD!!!!!!!********\n");
	}
	sem_init(&worker_semaphore, 0, 0);

}

resourcemanager_t::~resourcemanager_t()
{
	printf("RESOURCE MANAGER STOPPED\n");
	sem_destroy(&worker_semaphore);
}

void resourcemanager_t::queue_operation( resourceop_t *operation )
{
	queued_tasks.push_back(operation);
	sem_post(&worker_semaphore);
}

void resourcemanager_t::worker_thread()
{
	while(true) {
		sem_wait(&worker_semaphore);

		resourceop_t *op = queued_tasks.front();
		queued_tasks.pop_front();

		op->exec();

	}
}
