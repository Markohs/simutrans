/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include "game_loop_task.h"
#include "../../simsys.h"
#include "../../simworld.h"


game_loop_task_t::game_loop_task_t()
	:task_steppable_t("game_loop_task_t")
{

}

bool game_loop_task_t::run( unsigned long delta )
{
	printf("OMG\n");
	return false;
}
