/*
*
*   Copyright (c) 2014, Red Hat, Inc.
*   Copyright (c) 2014, Masatake YAMATO
*
*   This source code is released for free distribution under the terms of the
*   GNU General Public License version 2 or (at your option) any later version.
*
*   Defines hashtable
*/
#ifndef CTAGS_MAIN_HTABLE_H
#define CTAGS_MAIN_HTABLE_H

#include "general.h"
#include <stdint.h>

typedef struct sHashTable hashTable;
typedef unsigned int (* hashTableHashFunc)  (const void * const key);
typedef bool      (* hashTableEqualFunc) (const void* a, const void* b);
typedef void         (* hashTableFreeFunc)  (void * ptr);

/* To continue the interation, return false.
 * To break the interation, return true.
 */
typedef bool         (* hashTableForeachFunc) (const void *key, void *value, void *user_data);

unsigned int hashPtrhash (const void * x);
bool hashPtreq (const void * a, const void * constb);

unsigned int hashCstrhash (const void * x);
bool hashCstreq (const void * a, const void * b);

unsigned int hashCstrcasehash (const void * x);
bool hashCstrcaseeq (const void * a, const void * b);

unsigned int hashInthash (const void * x);
bool hashInteq (const void * a, const void * b);

extern hashTable* hashTableNew         (unsigned int size,
					hashTableHashFunc hashfn,
					hashTableEqualFunc equalfn,
					hashTableFreeFunc keyfreefn,
					hashTableFreeFunc valfreefn);

extern void       hashTableDelete      (hashTable *htable);
extern void       hashTableClear       (hashTable *htable);
extern void       hashTablePutItem     (hashTable *htable, void *key, void *value);
extern void*      hashTableGetItem     (hashTable *htable, const void * key);
extern bool    hashTableHasItem     (hashTable * htable, const void * key);
extern bool    hashTableDeleteItem  (hashTable *htable, const void *key);

/* hashTableForeachItem returns false if PROC returns false for all values.
 * True returned by hashTableForeachItem means PROC requests to stop the iteration.
 * If PROC never retruns true, or if PROC is not called because HTABLE is empty,
 * hashTableForeachItem returns false.
 */
extern bool       hashTableForeachItem (hashTable *htable, hashTableForeachFunc proc, void *user_data);

extern int        hashTableCountItem   (hashTable *htable);

extern hashTable* hashTableIntNew (unsigned int size,
								   hashTableHashFunc hashfn,
								   hashTableEqualFunc equalfn,
								   hashTableFreeFunc keyfreefn);
#define HT_PTR_TO_INT(P) ((int)(intptr_t)(P))
#define HT_INT_TO_PTR(P) ((void*)(intptr_t)(P))

#endif	/* CTAGS_MAIN_HTABLE_H */
