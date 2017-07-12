%{
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

#include "general.h"
#include "lflex.h"

%}

%option noyywrap noinput nounput 

%x COMMENT

%%

"/*"          { BEGIN COMMENT; }
<COMMENT>"*/" { BEGIN INITIAL; }
<COMMENT>.    { ; }
<COMMENT>"\n" { ; }
.             { ECHO; }

%%

static void runLexParser (void)
{
	CTAGS_FLEX_LEX(lex);
}

extern parserDefinition* LexParser (void)
{
	static const char *const extensions [] = { "l", NULL };
	parserDefinition* const def = parserNew ("LEX");
	def->extensions = extensions;
	def->parser     = runLexParser;
	return def;
}
