/*
*   Copyright (c) 2016, Masatake YAMATO <yamato@redhat.com>
*
*   This source code is released for free distribution under the terms of the
*   GNU General Public License version 2 or (at your option) any later version.
*
*   This module contains functions for generating tags for Python language
*   files.
*/

#include "atoken.h"
#include "read.h"
#include "routines.h"

static void* createToken (void *createArg)
{
	struct abstTokenClass *klass = createArg;
	abstToken *token;

	token = eMalloc (sizeof  (abstToken) + klass->extraSpace);
	token->klass = klass;
	token->string  = vStringNew ();
	token->refCount = 0;
	return token;
}

static void clearToken (void *data)
{
	abstToken *token = data;

	if (token->klass->clear)
		token->klass->clear (token);

	token->type = token->klass->tokenUndefined;
	token->keyword = token->klass->keywordNone;
	vStringClear (token->string);
	token->lineNumber = getInputLineNumber ();
	token->filePosition = getInputFilePosition ();
}

static void deleteToken (void *data)
{
	abstToken *token = data;

	if (token->klass->delete)
		token->klass->delete (token);

	vStringDelete (token->string);
	eFree (token);
}

void *newAbstToken      (struct abstTokenClass *klass)
{
	abstToken *token;

 retry:
	if (klass->pool)
		token = objPoolGet (klass->pool);
	else if (klass->nPreAlloc > 0)
	{
		klass->pool = objPoolNew (klass->nPreAlloc,
					  createToken,
					  clearToken,
					  deleteToken,
					  klass);
		goto retry;
	}
	else
		token = createToken (klass);

	token->refCount = 1;
	return token;
}

void *abstTokenRef      (abstToken *token)
{
	token->refCount++;
	return token;
}

void *abstTokenUnref    (abstToken *token)
{
	token->refCount--;
	if (token->refCount == 0)
	{
		if (token->klass->pool)
			objPoolPut (token->klass->pool, token);
		else
			deleteToken(token->klass->pool);
	}
	return token;
}


void abstTokenReadFull  (abstToken *token, void *data)
{
	token->klass->read (token, data);
}

void abstTokenRead      (abstToken *token)
{
	abstTokenReadFull (token, NULL);
}

void *abstTokenGetExtra (abstToken *token)
{
	return token->extra;
}
