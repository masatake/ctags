/*
 *
 *  Copyright (c) 2015, Red Hat, Inc.
 *  Copyright (c) 2015, Masatake YAMATO
 *
 *  Author: Masatake YAMATO <yamato@redhat.com>
 *
 *   This source code is released for free distribution under the terms of the
 *   GNU General Public License version 2 or (at your option) any later version.
 *
 */
#ifndef CTAGS_MAIN_FIELD_H
#define CTAGS_MAIN_FIELD_H

/*
*   INCLUDE FILES
*/

#include "general.h"
#include "types.h"

#include "vstring.h"

/*
*   DATA DECLARATIONS
*/

typedef enum eFieldType { /* extension field content control */
	FIELD_UNKNOWN = -1,

	/* BASIC FIELDS */
	FIELD_NAME,
	FIELD_INPUT_FILE,
	FIELD_PATTERN,

	FIELD_ECTAGS_START,
	FIELD_COMPACT_INPUT_LINE = FIELD_ECTAGS_START,

	/* EXTENSION FIELDS */
	FIELD_JSON_LOOP_START,
	FIELD_FILE_SCOPE = FIELD_JSON_LOOP_START,
	FIELD_KIND_LONG,
	FIELD_KIND,
	FIELD_LANGUAGE,
	FIELD_LINE_NUMBER,
	FIELD_SCOPE,
	FIELD_TYPE_REF,
	FIELD_KIND_KEY,
	FIELD_ECTAGS_LOOP_START,
	FIELD_INHERITANCE = FIELD_ECTAGS_LOOP_START,
	FIELD_ACCESS,
	FIELD_IMPLEMENTATION,
	FIELD_SIGNATURE,
	FIELD_ECTAGS_LOOP_LAST = FIELD_SIGNATURE,

	/* Extension fields newly introduced in Universal Ctags. */
	FIELDS_UCTAGS_START,
	FIELD_REF_MARK = FIELDS_UCTAGS_START,
	FIELD_SCOPE_KEY,
	FIELD_SCOPE_KIND_LONG,
	FIELD_UCTAGS_LOOP_START,
	FIELD_ROLES = FIELD_UCTAGS_LOOP_START,
	FIELD_EXTRAS,
	FIELD_XPATH,
	FIELD_END_LINE,
	FIELD_EPOCH,
	FIELD_NTH,

	FIELD_BUILTIN_LAST = FIELD_NTH,
} fieldType ;

#define fieldDataTypeFlags "sib" /* used in --list-fields */
typedef enum eFieldDataType {
	FIELDTYPE_STRING  = 1 << 0,
	FIELDTYPE_INTEGER = 1 << 1,
	FIELDTYPE_BOOL    = 1 << 2,

	/* used in --list-fields */
	FIELDTYPE_END_MARKER = 1 << 3,

	/* If you want to allow a field to be accessed from code
	 * written in optscript, append FIELDTYPE_SCRIPTABLE to
	 * dataType member of the field's fieldDefinition.
	 *
	 * For the field defined in optlib, set {datatype=TYPE} flag to
	 * --_fielddef-<LANG>=... option. Just specifying a type is
	 * enough; FIELDTYPE_SCRIPTABLE is automatically append to the
	 * filed definition. If you don't set the flag explicitly,
	 * FIELDTYPE_SCRIPTABLE is not appended. */
	FIELDTYPE_SCRIPTABLE = FIELDTYPE_END_MARKER,
} fieldDataType;
/* Interpretation of VALUE of field
 *
 * With attachParserField() declared in entry.h, you can set a C string
 * as a VALUE for the specified field.
 *
 * The VALUE is interpreted very differently depending on the output
 * format: ctags, xref, and json.
 *
 * For FIELDTYPE_STRING
 * =====================================================================
 *
 * output and getter
 * ---------------------------------------------------------------------
 *
 * Consider if you set "str" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-------------
 *  ctags | foo:str
 *   xref | str
 *   json | "foo": "str"
 * -------+-------------
 *   :foo | (foo) => (foo) true
 *
 * Consider if you set "" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-------------
 *  ctags | foo:
 *   xref | (nothing)
 *   json | "foo": ""
 * -------+-------------
 *   :foo | () => () true
 *
 * Consider if you don't set the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-------------
 *  ctags | (nothing)
 *   xref | (nothing)
 *   json | (nothing)
 * -------+-------------
 *   :foo | null => false
 *
 * setter
 * ---------------------------------------------------------------------
 *
 *     stack | C sting stored to the field
 * ----------+----------------------------
 *   . (str) | "str"
 *      . () | ""
 *  . object | ERROR:typecheck
 *
 *
 * For FIELDTYPE_STRING|FIELDTYPE_BOOL
 * =====================================================================
 *
 * output and getter
 * ---------------------------------------------------------------------
 *
 * If a field holds "" (empty C string), the json writer prints it as
 * false, and the xref writer prints it as -.  In the xref format,
 * there is no way to distinguish the output for the value "" and
 * "-". Both are printed as "-".  If the value is a non-empty C
 * string, Both json writer and xref writer print it as-is.
 *
 * Consider if you set "str" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-------------
 *  ctags | foo:str
 *   xref | str
 *   json | "foo": "str"
 * -------+-------------
 *   :foo | (str) => (str) true
 *
 * Consider if you set "" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-------------
 *  ctags | foo:
 *   xref | - (as specified as FIELD_NULL_LETTER_CHAR/FIELD_NULL_LETTER_STRING)
 *   json | "foo": false
 * -------+-------------
 *   :foo | false => false true
 *
 * Consider if you don't set the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-------------
 *  ctags | (nothing)
 *   xref | (nothing)
 *   json | (nothing)
 * -------+-------------
 *   :foo | null => false
 *
 * setter
 * ---------------------------------------------------------------------
 *
 *     stack | C sting stored to the field
 * ----------+----------------------------
 *   . (str) | "str"
 *      . () | ""
 *   . false | ""
 *    . true | ERROR:typecheck
 *  . object | ERROR:typecheck
 *
 *
 * For FIELDTYPE_BOOL
 * =====================================================================
 *
 * output and getter
 * ---------------------------------------------------------------------
 *
 * Whether a field holds "" (an empty C string) or a non-epmty C string,
 * the json writer prints it as true. In the same condition, the xref
 * writer prints the name of the field.
 *
 * If a value is not set, the field is treated as if it holds false.
 * The json writer prints nothing for the field holding false.
 * The xref writer prints - for the field holding false.

 * Consider if you set "str" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-------------
 *  ctags | foo:
 *   xref | foo
 *   json | "foo": true
 * -------+-------------
 *   :foo | true => true true
 *
 * Consider if you set "" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-------------
 *  ctags | foo:
 *   xref | foo
 *   json | "foo": true
 * -------+-------------
 *   :foo | true => true true
 *
 * Consider if you don't set the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-------------
 *  ctags | (nothing)
 *   xref | - (as specified as FIELD_NULL_LETTER_CHAR/FIELD_NULL_LETTER_STRING)
 *   json | (nothing)
 * -------+-------------
 *   :foo | null => false
 *
 * setter
 * ---------------------------------------------------------------------
 *
 *     stack | C sting stored to the field
 * ----------+----------------------------
 *   . false | do nothing if the field was not set.
 *           | ERROR:fieldreset if the field was already set.
 *           |                  This is a limit of current implementation.
 *    . true | ""
 *  . object | ERROR:typecheck
 *
 *
 * For FIELDTYPE_INTEGER
 * =====================================================================
 *
 * output and getter
 * ---------------------------------------------------------------------
 *
 * If a field holds "" (an empty C string), the all writers print it as
 * 0. If a field holds a string that strtol(3) cannot convert to an integer,
 * all the writers print it as 1.
 *
 * Consider if you set "99" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-------------
 *  ctags | foo:99
 *   xref | 99
 *   json | "foo": 99
 * -------+-------------
 *   :foo | 99 => 99 true

 * Consider if you set "str" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-------------
 *  ctags | foo:1
 *   xref | 1
 *   json | "foo": 1
 * -------+-------------
 *   :foo | 1 => 1 true

 * Consider if you set "" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-------------
 *  ctags | foo:0
 *   xref | 0
 *   json | "foo": 0
 * -------+-------------
 *   :foo | 0 => 0 true
 *
 * Consider if you don't set the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-----------
 *  ctags | (nothing)
 *   xref | (nothing)
 *   json | (nothing)
 * -------+-------------
 *   :foo | null => false
 *
 * setter
 * ---------------------------------------------------------------------
 *
 *     stack | C sting stored to the field
 * ----------+----------------------------
 *     . int | int
 *       . 1 | "1" (as an example)
 *  . object | ERROR:typecheck
 *
 *
 * The other data types and the combinations of types are not implemented yet.
 *
 */

typedef const char* (*fieldRenderer)(const tagEntryInfo *const,
									 const char *,
									 vString *);

struct sFieldDefinition {
	/* letter, and ftype are initialized in the main part,
	   not in a parser. */
#define NUL_FIELD_LETTER '\0'
	unsigned char letter;
	const char* name;
	const char* description;
	bool enabled;

	fieldRenderer render;
	fieldRenderer renderNoEscaping;
	bool (* doesContainAnyChar) (const tagEntryInfo *const, const char*, const char *);

	bool (* isValueAvailable) (const tagEntryInfo *const, const fieldDefinition *);

	const char * getterValueType; /* used in --_list-operators */
	struct _EsObject * (* getValueObject) (const tagEntryInfo *, const fieldDefinition *);
	const char * setterValueType; /* used in --_list-operators */

	/* Return es_false if passed value is acceptable.
	   Return an error object is unacceptable. */
	struct _EsObject * (* checkValueForSetter) (const fieldDefinition *, const struct _EsObject *);
	struct _EsObject * (* setValueObject) (tagEntryInfo *, const fieldDefinition *, const struct _EsObject *);

	fieldDataType dataType; /* used in json output. See OP column in --list-fields. */

	unsigned int    version;

	unsigned int ftype;	/* Given from the main part */
};


/*
*   FUNCTION PROTOTYPES
*/

extern bool isFieldEnabled (fieldType type);

extern bool isValueAvailableGeneric (const tagEntryInfo *const tag, const fieldDefinition *fdef);
extern struct _EsObject* getFieldValueGeneric (const tagEntryInfo *tag, const fieldDefinition *fdef);
extern struct _EsObject* setFieldValueGeneric (tagEntryInfo *tag, const fieldDefinition *fdef, const struct _EsObject *obj);

#endif	/* CTAGS_MAIN_FIELD_H */
