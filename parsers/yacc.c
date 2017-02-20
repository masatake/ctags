/*
*   Copyright (c) 2001-2002, Nick Hibma <n_hibma@van-laarhoven.org>
*
*   This source code is released for free distribution under the terms of the
*   GNU General Public License version 2 or (at your option) any later version.
*
*   This module contains functions for generating tags for YACC language files.
*/

/*
*   INCLUDE FILES
*/
#include "general.h"  /* must always come first */

#include <string.h>
#include "parse.h"
#include "routines.h"
#include "read.h"
#include "mio.h"
#include "promise.h"
#include "tokeninfo.h"
#include "keyword.h"

static bool not_in_grammar_rules = true;

enum yaccParserState {
	YACC_TOP_LEVEL,
	YACC_C_PROLOGUE,
	YACC_UNION,
	YACC_TOKEN,
	YACC_TYPE,
	YACC_GRAMMER,
	YACC_C_EPILOGUE,
};

typedef enum {
	/* label */
	K_TOKEN,
	K_TYPE,
} yaccKind;

static kindOption yaccKinds [] = {
	{ true, 't', "token", "tokens" },
	{ true, 'T', "type",  "types"  },
};

static tagRegexTable yaccTagRegexTable [] = {
	{"^([A-Za-z][A-Za-z_0-9]+)[ \t]*:", "\\1",
	 "l,label,labels", NULL, &not_in_grammar_rules },
	{"^([A-Za-z][A-Za-z_0-9]+)[ \t]*$", "\\1",
	 "l,label,labels", NULL, &not_in_grammar_rules },
};

static  void change_section (const char *line CTAGS_ATTR_UNUSED,
			     const regexMatch *matches CTAGS_ATTR_UNUSED,
			     unsigned int count CTAGS_ATTR_UNUSED,
			     void *data)
{
	enum yaccParserState *state = data;
	not_in_grammar_rules = !not_in_grammar_rules;

	if (*state == YACC_GRAMMER)
		*state = YACC_C_EPILOGUE;
	else
		*state = YACC_GRAMMER;
}

static void enter_c_prologue (const char *line CTAGS_ATTR_UNUSED,
			      const regexMatch *matches CTAGS_ATTR_UNUSED,
			      unsigned int count CTAGS_ATTR_UNUSED,
			      void *data)
{
	enum yaccParserState *state = data;
	*state = YACC_C_PROLOGUE;
}

static void leave_c_prologue (const char *line CTAGS_ATTR_UNUSED,
			      const regexMatch *matches CTAGS_ATTR_UNUSED,
			      unsigned int count CTAGS_ATTR_UNUSED,
			      void *data)
{
	enum yaccParserState *state = data;
	*state = YACC_TOP_LEVEL;
}

static void enter_token (const char *line CTAGS_ATTR_UNUSED,
			 const regexMatch *matches,
			 unsigned int count CTAGS_ATTR_UNUSED,
			 void *data)
{
	enum yaccParserState *state = data;
	if (*state == YACC_TOP_LEVEL)
		*state = YACC_TOKEN;
}

static void enter_type (const char *line CTAGS_ATTR_UNUSED,
			  const regexMatch *matches,
			  unsigned int count CTAGS_ATTR_UNUSED,
			  void *data)
{
	enum yaccParserState *state = data;
	if (*state == YACC_TOP_LEVEL)
		*state = YACC_TYPE;
}

static void enter_union (const char *line CTAGS_ATTR_UNUSED,
			 const regexMatch *matches,
			 unsigned int count CTAGS_ATTR_UNUSED,
			 void *data)
{
	enum yaccParserState *state = data;

	if (*state == YACC_TOP_LEVEL)
		*state = YACC_UNION;
}

static void leave_union (const char *line CTAGS_ATTR_UNUSED,
			 const regexMatch *matches,
			 unsigned int count CTAGS_ATTR_UNUSED,
			 void *data)
{
	enum yaccParserState *state = data;

	if (*state == YACC_UNION)
		*state = YACC_TOP_LEVEL;
}

static long yaccReadline ()
{
	int count = 0;
	while (1)
	{
		int c = getcFromInputFile ();
		if (c == EOF)
			return count;
		else if (c == '\n')
		{
			count++;
			return count;
		}
		else
			count++;
	}
}

static void make_promise_for_epilogue (void)
{
	long tmp, last;
	long endCharOffset;
	unsigned long c_start;
	unsigned long c_source_start;
	unsigned long c_end;

	c_start = getInputLineNumber ();
	c_source_start = getSourceLineNumber();

	/* Skip the lines for finding the EOF. */
	endCharOffset = 0;
	last = 0;
	while ((tmp = yaccReadline ()))
		last = tmp;
	if (last)
		endCharOffset = last;
	/* (NEVER HAPPEN)
	   If `last' is too long, strlen returns a too large value
	   for the positive area of `endCharOffset'. */
	if (endCharOffset < 0)
		endCharOffset = 0;

	c_end = getInputLineNumber ();

	makePromise ("C", c_start, 0, c_end, endCharOffset, c_source_start);
}

enum eTokenType {
	TOKEN_EOF = 256,
	TOKEN_UNDEFINED,
	TOKEN_IDENTIFIER,
	TOKEN_NUMBER,
	TOKEN_KEYWORD,		/* Never used */
};

struct tokenTypePair yaccTypePairs [] = {
	{ '<', '>' },
};

static void readToken (tokenInfo *const token, void *data)
{
	int c;

	enum yaccParserState *state = data;

	if (*state != YACC_TOKEN)
	{
		token->type = TOKEN_EOF;
		return;
	}
	
	do {
		c = getcFromInputFile ();
	} while (c == ' ' || c== '\t' || c == '\f' || c == '\r' || c == '\n');

	if (*state != YACC_TOKEN)
	{
		token->type = TOKEN_EOF;
		return;
	}

	token->type = TOKEN_UNDEFINED;
	token->keyword	= KEYWORD_NONE;
	vStringClear (token->string);
	token->lineNumber   = getInputLineNumber ();
	token->filePosition = getInputFilePosition ();

	switch (c)
	{
	case EOF:
		token->type = TOKEN_EOF;
		break;
	case '%':
		ungetcToInputFile (c);
		token->type = TOKEN_EOF;
		break;
	case '<':
		token->type = '<';
		break;
	case '>':
		token->type = '>';
		break;
	default:
		if (isdigit (c))
		{
			token->type = TOKEN_NUMBER;
			tokenPutc(token, c);
			while ((c = getcFromInputFile()))
			{
				if (isdigit (c))
					tokenPutc(token, c);
				else
				{
					ungetcToInputFile (c);
					break;
				}
			}
		}
		else if (isalpha(c) || c == '_')
		{
			token->type = TOKEN_IDENTIFIER;
			tokenPutc(token, c);
			while ((c = getcFromInputFile()))
			{
				if (isalnum(c) || c == '_')
					tokenPutc(token, c);
				else
				{
					ungetcToInputFile (c);
					break;
				}

			}
		}
		else
			token->type = c;
		break;
	}
}

struct tokenInfoClass yaccTokenInfoClass = {
	.nPreAlloc = 4,
	.typeForUndefined = TOKEN_UNDEFINED,
	.typeForKeyword   = TOKEN_KEYWORD,
	.typeForEOF       = TOKEN_EOF,
	.extraSpace       = 0,
	.pairs            = yaccTypePairs,
	.pairCount        = ARRAY_SIZE (yaccTypePairs),
	.read             = readToken,
};

static void parseTokens (enum yaccParserState *state)
{
	tokenInfo *t = newToken (&yaccTokenInfoClass);

	while (1)
	{
		tokenReadFull (t, state);
		if (tokenIsEOF(t))
			break;
		else if (tokenIsType (t, IDENTIFIER))
		{
			if (yaccKinds[K_TOKEN].enabled)
				makeSimpleTag (t->string, yaccKinds, K_TOKEN);
		}
		else if (t->type == '<')
			tokenSkipOverPair (t);
	}
	tokenDestroy (t);
}


static enum yaccParserState parserState;

static void initializeYaccParser (langType language)
{
	/*
	  YACC_TOP_LEVEL
	   %{ ...
		YACC_C_PROLOGUE
	   %}
	   YACC_TOP_LEVEL
	   %union {
	   	YACC_UNION
	   }
	   YACC_TOP_LEVEL		
	   %%
		YACC_GRAMMER
	   %%
	   	YACC_C_EPILOGUE
	*/

	addCallbackRegex (language, "^%\\{", "{exclusive}", enter_c_prologue, NULL, &parserState);
	addCallbackRegex (language, "^%\\}", "{exclusive}", leave_c_prologue, NULL, &parserState);

	addCallbackRegex (language, "^%%", "{exclusive}", change_section, NULL, &parserState);

	addCallbackRegex (language, "^%union", "{exclusive}", enter_union, NULL, &parserState);
	addCallbackRegex (language, "^}",      "{exclusive}", leave_union, NULL, &parserState);

	addCallbackRegex (language, "^%token", "{exclusive}", enter_token, NULL, &parserState);
	addCallbackRegex (language, "^%type",  "{exclusive}", enter_type,  NULL, &parserState);
}

static void runYaccParser (void)
{
	enum yaccParserState last_state;

	unsigned long c_input = 0;
	unsigned long c_source = 0;

	not_in_grammar_rules = true;

	c_input = 0;
	c_source = 0;
	parserState = YACC_TOP_LEVEL;
	last_state = parserState;

	while (yaccReadline ())
	{
		if (last_state == YACC_TOP_LEVEL &&
		    parserState == YACC_C_PROLOGUE)
		{
			if (yaccReadline ())
			{
				c_input  = getInputLineNumber ();
				c_source = getSourceLineNumber ();
			}
		}
		else if (last_state == YACC_C_PROLOGUE
			 && parserState == YACC_TOP_LEVEL)
		{
			unsigned long c_end = getInputLineNumber ();
			makePromise ("C", c_input, 0, c_end, 0, c_source);
			c_input = 0;
			c_source = 0;
		}
		else if (last_state == YACC_TOP_LEVEL
			 && parserState == YACC_UNION)
		{
			c_input = getInputLineNumber ();
			c_source = getInputLineNumber ();
		}
		else if (last_state == YACC_UNION
			 && parserState == YACC_TOP_LEVEL)
		{
			unsigned long c_end = getInputLineNumber ();
			makePromise ("C", c_input, strlen ("%"), c_end, strlen ("}"),
				     c_source);
			c_input = 0;
			c_source = 0;
		}
		else if (last_state == YACC_TOP_LEVEL
			 && parserState == YACC_TOKEN)
			parseTokens (&parserState);
		else if (parserState == YACC_C_EPILOGUE)
		{
			if (yaccReadline ())
				make_promise_for_epilogue ();
		}
		last_state = parserState;
	}
}

extern parserDefinition* YaccParser (void)
{
	static const char *const extensions [] = { "y", NULL };
	parserDefinition* const def = parserNew ("YACC");
	def->extensions = extensions;
	def->initialize = initializeYaccParser;
	def->method     = METHOD_REGEX;
	def->parser     = runYaccParser;
	def->tagRegexTable = yaccTagRegexTable;
	def->tagRegexCount = ARRAY_SIZE (yaccTagRegexTable);
	def->kinds      = yaccKinds;
	def->kindCount  = ARRAY_SIZE (yaccKinds);
	return def;
}
