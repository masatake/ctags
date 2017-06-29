/*
*   Copyright (c) 1998-2002, Darren Hiebert
*
*   This source code is released for free distribution under the terms of the
*   GNU General Public License version 2 or (at your option) any later version.
*
*   External interface to keyword.c
*/
#ifndef CTAGS_MAIN_KEYWORD_H
#define CTAGS_MAIN_KEYWORD_H

/*
*   INCLUDE FILES
*/
#include "general.h"  /* must always come first */

#include "types.h"
#include "vstring.h"

#include <stdio.h>


#define KEYWORD_NONE -1

/*
*   FUNCTION PROTOTYPES
*/

/* `string' should be allocated statically. */
extern void addKeyword (const char *const string, langType language, int value);

/* addKeywordStrdup does strdup `string'.
   Duplicated string is freed in  freeKeywordTable() */
extern void addKeywordStrdup (const char *const string, langType language, int value);
extern int lookupKeyword (const char *const string, langType language);
extern int lookupCaseKeyword (const char *const string, langType language);
extern void freeKeywordTable (void);

extern void dumpKeywordTable (FILE *fp);

#ifdef DEBUG
extern void printKeywordTable (void);
#endif

#endif  /* CTAGS_MAIN_KEYWORD_H */
