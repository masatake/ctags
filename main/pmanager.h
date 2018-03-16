/*
*   Copyright (c) 2018, Masatake YAMATO
*   Copyright (c) 2018, Red Hat, Inc.
*
*   This source code is released for free distribution under the terms of the
*   GNU General Public License version 2 or (at your option) any later version.
*
*   Managing parallel execution
*/
#ifndef CTAGS_MAIN_PMANAGER_H
#define CTAGS_MAIN_PMANAGER_H

/*
*   INCLUDE FILES
*/
#include "general.h"  /* must always come first */

struct PManager;

struct PManager* pmanager_new (int count, int argc, const char** argv);
int pmanager_dispatch(struct PManager *manager, const char *file);
int pmanager_delete  (struct PManager *manager);

#endif  /* CTAGS_MAIN_PMANAGER_H */
