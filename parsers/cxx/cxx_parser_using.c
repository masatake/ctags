/*
*   Copyright (c) 2016, Szymon Tomasz Stefanek
*
*   This source code is released for free distribution under the terms of the
*   GNU General Public License version 2 or (at your option) any later version.
*
*   This module contains functions for parsing and scanning C++ source files
*/
#include "cxx_parser.h"
#include "cxx_parser_internal.h"

#include "cxx_debug.h"
#include "cxx_keyword.h"
#include "cxx_token.h"
#include "cxx_token_chain.h"
#include "cxx_scope.h"

#include "parse.h"
#include "vstring.h"
#include "read.h"

boolean cxxParserParseUsingClause(void)
{
	CXX_DEBUG_ENTER();
	
	// using-directives for namespaces and using-declarations for namespace members
	// using-declarations for class members
	// type alias and alias template declaration (since C++11)
	
	// using namespace ns_name;	(5)	// whole namespace
	// using ns_name::name;	(6)	 // only symbol name
	// using B::g; // inside class, using method g from base class B
	// using identifier attr(optional) = type-id ; <-- this is equivalent to a typedef!
	
	cxxTokenChainClear(g_cxx.pTokenChain);

	// skip to the next ; without leaving scope.
	if(!cxxParserParseUpToOneOf(CXXTokenTypeSemicolon | CXXTokenTypeClosingBracket | CXXTokenTypeEOF))
	{
		CXX_DEBUG_LEAVE_TEXT("Failed to parse up to the next ;");
		return FALSE;
	}

	if(!cxxTokenTypeIs(g_cxx.pToken,CXXTokenTypeSemicolon))
	{
		CXX_DEBUG_LEAVE_TEXT("This is a syntax error but we tolerate it");
		return TRUE;
	}

	cxxTokenChainDestroyLast(g_cxx.pTokenChain);

	if(g_cxx.pTokenChain->iCount < 1)
	{
		CXX_DEBUG_LEAVE_TEXT("This is a syntax error but we tolerate it");
		return TRUE;
	}
	
	CXXToken * pAssignment = cxxTokenChainFirstTokenOfType(g_cxx.pTokenChain,CXXTokenTypeAssignment);
	
	if(pAssignment)
	{
		CXXToken * pFirst = cxxTokenChainFirst(g_cxx.pTokenChain);

		if(cxxTokenTypeIs(pFirst,CXXTokenTypeIdentifier))
		{
			// It's a typedef. Reorder the tokens in the chain so it really looks like a typedef
			// and pass it to the specialized extraction routine
			cxxTokenChainTake(g_cxx.pTokenChain,pFirst);

			while(cxxTokenChainFirst(g_cxx.pTokenChain) != pAssignment)
				cxxTokenChainDestroyFirst(g_cxx.pTokenChain);
			cxxTokenChainDestroyFirst(g_cxx.pTokenChain); // kill assignment itself

			cxxTokenChainAppend(g_cxx.pTokenChain,pFirst); // in typedefs it's at the end

			cxxParserExtractTypedef(g_cxx.pTokenChain,FALSE);
		}
	} else {
		CXX_DEBUG_ASSERT(g_cxx.pTokenChain->iCount > 0,"The token chain should be non empty at this point");
	
		CXXToken * t = cxxTokenChainFirst(g_cxx.pTokenChain);
	
		boolean bUsingNamespace = cxxTokenTypeIs(t,CXXTokenTypeKeyword) && (t->eKeyword == CXXKeywordNAMESPACE);
	
		if(bUsingNamespace)
			cxxTokenChainDestroyFirst(g_cxx.pTokenChain);
	
		if(g_cxx.pTokenChain->iCount > 0)
		{
			cxxTokenChainCondense(g_cxx.pTokenChain,0);
	
			CXXToken * pFirst = cxxTokenChainFirst(g_cxx.pTokenChain);
			CXX_DEBUG_ASSERT(pFirst,"Condensation of a non empty chain should produce a token!");
			
			tagEntryInfo * tag;
			
			if(bUsingNamespace)
			{
				CXX_DEBUG_PRINT("Found usingns clause '%s'",vStringValue(pFirst->pszWord));
	
				tag = cxxTagBegin(
						vStringValue(pFirst->pszWord),
						CXXTagKindUSINGNS,
						pFirst
					);
			} else {
				CXX_DEBUG_PRINT("Found usingsym clause '%s'",vStringValue(pFirst->pszWord));
	
				tag = cxxTagBegin(
						vStringValue(pFirst->pszWord),
						CXXTagKindUSINGSYM,
						pFirst
					);
			}
	
			if(tag)
			{
				tag->isFileScope = (cxxScopeGetKind() == CXXTagKindNAMESPACE) && !isInputHeaderFile();
				cxxTagCommit();
			}
		}
	}

	CXX_DEBUG_LEAVE();
	return TRUE;
}
