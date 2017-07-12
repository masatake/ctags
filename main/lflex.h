/*
 *
 *  Copyright (c) 2017, Red Hat, Inc.
 *  Copyright (c) 2017, Masatake YAMATO
 *
 *  Author: Masatake YAMATO <yamato@redhat.com>
 *
 *   This source code is released for free distribution under the terms of the
 *   GNU General Public License version 2 or (at your option) any later version.
 *
 */

#ifndef CTAGS_MAIN_LFLEX_H
#define CTAGS_MAIN_LFLEX_H

#include "read.h"

#define CTAGS_FLEX_LEX(prefix) \
	prefix##_yylex()

#define YY_INPUT(buf,result,max_size)			\
	do {										\
		int i = 0;								\
		if (max_size == 0)						\
			result = YY_NULL;					\
		else									\
		{										\
			while (i < max_size)				\
			{									\
				int c = getcFromInputFile ();	\
				if (c == EOF)					\
					break;						\
				else							\
				{								\
					buf[i++] = c;				\
				}								\
			}									\
			result = i;							\
			if (result == 0)					\
				result = YY_NULL;				\
		}										\
	}											\
	while (0)

#endif
