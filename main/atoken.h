/*
*   Copyright (c) 2016, Masatake YAMATO <yamato@redhat.com>
*
*   This source code is released for free distribution under the terms of the
*   GNU General Public License version 2 or (at your option) any later version.
*
*   This module contains functions for generating tags for Python language
*   files.
*/

#include "general.h"  /* must always come first */
#include "mio.h"
#include "objpool.h"
#include "vstring.h"

#ifndef CTAGS_MAIN_ATOKEN_H
#define CTAGS_MAIN_ATOKEN_H

struct abstTokenClass;

#define abstTokenMembers			\
	int		type;			\
	int		keyword;		\
	vString *	string;			\
	struct abstTokenClass *klass;           \
	unsigned int    refCount;		\
	unsigned long	lineNumber;		\
	MIOPos		filePosition

typedef struct sAbstToken {
	abstTokenMembers;
	char extra[];
} abstToken;

#define ABST_TOKEN(X)  ((abstToken *)X)
#define ABST_TOKENX(X)  (abstTokenGetExtra((abstToken *)X))

struct abstTokenClass {
	unsigned int nPreAlloc;
	int tokenUndefined;
	int keywordNone;
	size_t extraSpace;
	void (*read)   (abstToken *token, void *extra);
	void (*clear)  (abstToken *token);
	void (*delete) (abstToken *token);
	objPool *pool;
};

void *newAbstToken      (struct abstTokenClass *klass);
void *abstTokenRef      (abstToken *token);
void *abstTokenUnref    (abstToken *token);

void abstTokenReadFull  (abstToken *token, void *data);
void abstTokenRead      (abstToken *token);

void *abstTokenGetExtra (abstToken *token);

#endif
