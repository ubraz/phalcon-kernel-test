
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2013 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  +------------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_phalcon.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"

/*
 * Memory Frames/Virtual Symbol Scopes
 *------------------------------------
 *
 * Phalcon uses memory frames to track the variables used within a method
 * in order to free them or reduce their reference counting accordingly before
 * exit the method in execution.
 *
 * This adds a minimum overhead to execution but save us the work of
 * free memory in each method.
 *
 * The whole memory frame is an open double-linked list which start is an
 * allocated empty frame that points to the real first frame. The start
 * memory frame is globally accesed using PHALCON_GLOBAL(start_frame)
 *
 * Not all methods must grown/restore the phalcon_memory_entry.
 */

/**
 * Initializes/Reinitializes a variable
 */
inline void phalcon_init_nvar(zval **var TSRMLS_DC) {
	if (*var) {
		if (Z_REFCOUNT_PP(var) > 1) {
			Z_DELREF_PP(var);
			ALLOC_ZVAL(*var);
			Z_SET_REFCOUNT_PP(var, 1);
			Z_UNSET_ISREF_PP(var);
		} else {
			zval_ptr_dtor(var);
			PHALCON_ALLOC_ZVAL(*var);
		}
	} else {
		phalcon_memory_alloc(var TSRMLS_CC);
	}
}

/**
 * Copy/Write variables caring of reference counting
 */
inline void phalcon_cpy_wrt(zval **dest, zval *var TSRMLS_DC) {

	if (*dest) {
		if (Z_REFCOUNT_PP(dest) > 0) {
			zval_ptr_dtor(dest);
		}
	} else {
		phalcon_memory_observe(dest TSRMLS_CC);
	}

	Z_ADDREF_P(var);
	*dest = var;
}

/**
 * Copy/Write variables caring of reference counting also duplicating the origin ctor
 */
inline void phalcon_cpy_wrt_ctor(zval **dest, zval *var TSRMLS_DC) {

	if (*dest) {
		if (Z_REFCOUNT_PP(dest) > 0) {
			zval_ptr_dtor(dest);
		}
	} else {
		phalcon_memory_observe(dest TSRMLS_CC);
	}

	Z_ADDREF_P(var);
	*dest = var;
	zval_copy_ctor(*dest);
	Z_SET_REFCOUNT_PP(dest, 1);
	Z_UNSET_ISREF_PP(dest);
}

/**
 * Adds a memory frame in the current executed method
 */
int PHALCON_FASTCALL phalcon_memory_grow_stack(TSRMLS_D) {

	phalcon_memory_entry *entry, *start;
	zend_phalcon_globals *phalcon_globals_ptr = PHALCON_VGLOBAL;

	if (!phalcon_globals_ptr->start_memory) {
		start = (phalcon_memory_entry *) emalloc(sizeof(phalcon_memory_entry));
		start->pointer = -1;
		start->hash_pointer = -1;
		start->prev = NULL;
		start->next = NULL;
		phalcon_globals_ptr->start_memory = start;
		phalcon_globals_ptr->active_memory = start;
	}

	entry = (phalcon_memory_entry *) emalloc(sizeof(phalcon_memory_entry));
	entry->pointer = -1;
	entry->hash_pointer = -1;
	entry->prev = phalcon_globals_ptr->active_memory;
	phalcon_globals_ptr->active_memory->next = entry;
	phalcon_globals_ptr->active_memory = entry;

	return SUCCESS;
}

/**
 * Finishes the current memory stack by releasing allocated memory
 */
int PHALCON_FASTCALL phalcon_memory_restore_stack(TSRMLS_D) {

	register int i;
	phalcon_memory_entry *prev, *active_memory;
	phalcon_symbol_table *active_symbol_table;
	zend_phalcon_globals *phalcon_globals_ptr = PHALCON_VGLOBAL;

	active_memory = phalcon_globals_ptr->active_memory;
	if (active_memory == NULL) {
		return FAILURE;
	}

	/*#ifndef PHALCON_RELEASE
	//if(!PHALCON_GLOBAL(phalcon_stack_stats)){
	//	PHALCON_GLOBAL(phalcon_stack_stats) += active_memory->pointer;
	//	PHALCON_GLOBAL(phalcon_number_grows)++;
	//} else {
	//	if (active_memory->pointer > PHALCON_GLOBAL(phalcon_stack_stats)) {
	//		PHALCON_GLOBAL(phalcon_stack_stats) = active_memory->pointer;
	//	}
	//}
	#endif*/

	if (phalcon_globals_ptr->active_symbol_table) {
		active_symbol_table = phalcon_globals_ptr->active_symbol_table;
		if (active_symbol_table->scope == active_memory) {
			zend_hash_destroy(EG(active_symbol_table));
			FREE_HASHTABLE(EG(active_symbol_table));
			EG(active_symbol_table) = active_symbol_table->symbol_table;
			phalcon_globals_ptr->active_symbol_table = active_symbol_table->prev;
			efree(active_symbol_table);
		}
	}

	/**
	 * Check for non freed hash key zvals, mark as null to avoid string freeing
	 */
	if (active_memory->hash_pointer > -1) {
		for (i = active_memory->hash_pointer; i >= 0; i--) {
			if (Z_REFCOUNT_P(*active_memory->hash_addresses[i]) <= 1) {
				ZVAL_NULL(*active_memory->hash_addresses[i]);
			} else {
				zval_copy_ctor(*active_memory->hash_addresses[i]);
			}
		}
		efree(active_memory->hash_addresses);
	}

	/**
	 * Traverse all zvals allocated, reduce the reference counting or free them
	 */
	if (active_memory->pointer > -1) {

		//phalcon_globals_ptr->phalcon_stack_derivate[active_memory->pointer]++;

		for (i = active_memory->pointer; i >= 0; i--) {

			if (active_memory->addresses[i] == NULL) {
				continue;
			}

			if ((Z_REFCOUNT_PP(active_memory->addresses[i]) - 1) == 0) {
				zval_ptr_dtor(active_memory->addresses[i]);
				active_memory->addresses[i] = NULL;
			} else {
				Z_DELREF_PP(active_memory->addresses[i]);
				if (Z_REFCOUNT_PP(active_memory->addresses[i]) == 1) {
					active_memory->addresses[i] = NULL;
				}
			}
		}

		efree(active_memory->addresses);
	}

	prev = active_memory->prev;
	efree(phalcon_globals_ptr->active_memory);
	phalcon_globals_ptr->active_memory = prev;
	if (prev != NULL) {
		phalcon_globals_ptr->active_memory->next = NULL;
		if (phalcon_globals_ptr->active_memory == phalcon_globals_ptr->start_memory) {
			efree(phalcon_globals_ptr->active_memory);
			phalcon_globals_ptr->start_memory = NULL;
			phalcon_globals_ptr->active_memory = NULL;
		}
	} else {
		phalcon_globals_ptr->start_memory = NULL;
		phalcon_globals_ptr->active_memory = NULL;
	}

	return SUCCESS;
}

/**
 * Finishes memory stack when PHP throws a fatal error
 */
int PHALCON_FASTCALL phalcon_clean_shutdown_stack(TSRMLS_D) {

	zend_phalcon_globals *phalcon_globals_ptr = PHALCON_VGLOBAL;

	#if !ZEND_DEBUG && PHP_VERSION_ID <= 50400

	phalcon_memory_entry *prev, *active_memory = phalcon_globals_ptr->active_memory;

	while (active_memory != NULL) {

		prev = active_memory->prev;
		efree(active_memory);
		active_memory = prev;
		if (prev != NULL) {
			active_memory->next = NULL;
			if (active_memory == phalcon_globals_ptr->start_memory) {
				efree(active_memory);
				phalcon_globals_ptr->start_memory = NULL;
				active_memory = NULL;
			}
		} else {
			phalcon_globals_ptr->start_memory = NULL;
			active_memory = NULL;
		}

	}

	#endif

	phalcon_globals_ptr->start_memory = NULL;
	phalcon_globals_ptr->active_memory = NULL;

	return SUCCESS;
}

/**
 * Observes a memory pointer to release its memory at the end of the request
 */
int PHALCON_FASTCALL phalcon_memory_observe(zval **var TSRMLS_DC) {

	phalcon_memory_entry *active_memory = PHALCON_GLOBAL(active_memory);

	active_memory->pointer++;

	/** Incremental dynamic reallocation saving memory */
	if (active_memory->pointer <= 0) {
		active_memory->addresses = emalloc(sizeof(zval **) * 8);
	} else {
		if (active_memory->pointer >= 8 && active_memory->pointer < 20) {
			active_memory->addresses = erealloc(active_memory->addresses, sizeof(zval **) * 20);
		} else {
			if (active_memory->pointer >= 20 && active_memory->pointer < 36) {
				active_memory->addresses = erealloc(active_memory->addresses, sizeof(zval **) * 36);
			} else {
				if (active_memory->pointer >= 36) {
					active_memory->addresses = erealloc(active_memory->addresses, sizeof(zval **) * PHALCON_MAX_MEMORY_STACK);
				}
			}
		}
	}

	#ifndef PHALCON_RELEASE
	if (active_memory->pointer >= (PHALCON_MAX_MEMORY_STACK - 1)) {
		fprintf(stderr, "ERROR: Phalcon memory stack is too small %d\n", PHALCON_MAX_MEMORY_STACK);
		return FAILURE;
	}
	#endif

	active_memory->addresses[active_memory->pointer] = var;
	return SUCCESS;
}

/**
 * Observe a variable and allocates memory for it
 */
int PHALCON_FASTCALL phalcon_memory_alloc(zval **var TSRMLS_DC) {

	phalcon_memory_entry *active_memory = PHALCON_GLOBAL(active_memory);

	active_memory->pointer++;

	/** Incremental dynamic reallocation saving memory */
	if (active_memory->pointer <= 0) {
		active_memory->addresses = emalloc(sizeof(zval **) * 8);
	} else {
		if (active_memory->pointer >= 8 && active_memory->pointer < 20) {
			active_memory->addresses = erealloc(active_memory->addresses, sizeof(zval **) * 20);
		} else {
			if (active_memory->pointer >= 20 && active_memory->pointer < 36) {
				active_memory->addresses = erealloc(active_memory->addresses, sizeof(zval **) * 36);
			} else {
				if (active_memory->pointer >= 36) {
					active_memory->addresses = erealloc(active_memory->addresses, sizeof(zval **) * PHALCON_MAX_MEMORY_STACK);
				}
			}
		}
	}

	#ifndef PHALCON_RELEASE
	if (active_memory->pointer >= (PHALCON_MAX_MEMORY_STACK - 1)) {
		fprintf(stderr, "ERROR: Phalcon memory stack is too small %d\n", PHALCON_MAX_MEMORY_STACK);
		return FAILURE;
	}
	#endif

	active_memory->addresses[active_memory->pointer] = var;

	ALLOC_ZVAL(*var);
	INIT_PZVAL(*var);
	ZVAL_NULL(*var);

	return SUCCESS;
}

/**
 * Observe a variable and allocates memory for it
 * Marks hash key zvals to be nulled before be freed
 */
int PHALCON_FASTCALL phalcon_memory_alloc_pnull(zval **var TSRMLS_DC) {

	phalcon_memory_entry *active_memory = PHALCON_GLOBAL(active_memory);

	active_memory->pointer++;

	/** Incremental dynamic reallocation saving memory */
	if (active_memory->pointer <= 0) {
		active_memory->addresses = emalloc(sizeof(zval **) * 8);
	} else {
		if (active_memory->pointer >= 8 && active_memory->pointer < 20) {
			active_memory->addresses = erealloc(active_memory->addresses, sizeof(zval **) * 20);
		} else {
			if (active_memory->pointer >= 20 && active_memory->pointer < 36) {
				active_memory->addresses = erealloc(active_memory->addresses, sizeof(zval **) * 36);
			} else {
				if (active_memory->pointer >= 36) {
					active_memory->addresses = erealloc(active_memory->addresses, sizeof(zval **) * PHALCON_MAX_MEMORY_STACK);
				}
			}
		}
	}

	active_memory->hash_pointer++;

	/** Incremental dynamic reallocation saving memory */
	if (active_memory->hash_pointer <= 0) {
		active_memory->hash_addresses = emalloc(sizeof(zval **) * 4);
	} else {
		if (active_memory->hash_pointer >= 4) {
			active_memory->hash_addresses = erealloc(active_memory->hash_addresses, sizeof(zval **) * 16);
		}
	}

	#ifndef PHALCON_RELEASE
	if (active_memory->pointer >= (PHALCON_MAX_MEMORY_STACK - 1)) {
		fprintf(stderr, "ERROR: Phalcon memory stack is too small %d\n", PHALCON_MAX_MEMORY_STACK);
		return FAILURE;
	}
	#endif

	active_memory->addresses[active_memory->pointer] = var;
	active_memory->hash_addresses[active_memory->hash_pointer] = var;

	ALLOC_ZVAL(*var);
	INIT_PZVAL(*var);
	ZVAL_NULL(*var);

	return SUCCESS;
}

/**
 * Removes a memory pointer from the active memory pool
 */
int PHALCON_FASTCALL phalcon_memory_remove(zval **var TSRMLS_DC) {
	zval_ptr_dtor(var);
	*var = NULL;
	return SUCCESS;
}

/**
 * Cleans the phalcon memory stack recursivery
 */
int PHALCON_FASTCALL phalcon_clean_restore_stack(TSRMLS_D) {
	while (PHALCON_GLOBAL(active_memory) != NULL) {
		phalcon_memory_restore_stack(TSRMLS_C);
	}
	return SUCCESS;
}

/**
 * Copies a variable only if its refcount is greater than 1
 */
void PHALCON_FASTCALL phalcon_copy_ctor(zval *destiny, zval *origin) {
	if (Z_REFCOUNT_P(origin) > 1) {
		zval_copy_ctor(destiny);
	} else {
		ZVAL_NULL(origin);
	}
}

/**
 * Creates virtual symbol tables dynamically
 */
void phalcon_create_symbol_table(TSRMLS_D) {

	phalcon_symbol_table *entry;
	zend_phalcon_globals *phalcon_globals_ptr = PHALCON_VGLOBAL;
	HashTable *symbol_table;

	#ifndef PHALCON_RELEASE
	if (!phalcon_globals_ptr->active_memory) {
		fprintf(stderr, "ERROR: Trying to create a virtual symbol table without a memory frame");
		return;
	}
	#endif

	entry = (phalcon_symbol_table *) emalloc(sizeof(phalcon_symbol_table));
	entry->scope = phalcon_globals_ptr->active_memory;
	entry->symbol_table = EG(active_symbol_table);
	entry->prev = phalcon_globals_ptr->active_symbol_table;
	phalcon_globals_ptr->active_symbol_table = entry;

	ALLOC_HASHTABLE(symbol_table);
	zend_hash_init(symbol_table, 0, NULL, ZVAL_PTR_DTOR, 0);
	EG(active_symbol_table) = symbol_table;
}

/**
 * Restores all the virtual symbol tables
 */
void phalcon_clean_symbol_tables(TSRMLS_D) {

	/*unsigned int i;

	if (PHALCON_GLOBAL(symbol_tables)) {
		for (i = PHALCON_GLOBAL(number_symbol_tables); i > 0; i--) {
			EG(active_symbol_table) = PHALCON_GLOBAL(symbol_tables)[i - 1];
		}
		efree(PHALCON_GLOBAL(symbol_tables));
		PHALCON_GLOBAL(symbol_tables) = NULL;
	}*/
}

/**
 * Exports symbols to the active symbol table
 */
int phalcon_set_symbol(zval *key_name, zval *value TSRMLS_DC) {

	if (!EG(active_symbol_table)) {
		zend_rebuild_symbol_table(TSRMLS_C);
	}

	if (EG(active_symbol_table)) {
		if (Z_TYPE_P(key_name) == IS_STRING) {
			Z_ADDREF_P(value);
			zend_hash_update(EG(active_symbol_table), Z_STRVAL_P(key_name), Z_STRLEN_P(key_name) + 1, &value, sizeof(zval *), NULL);
			if (EG(exception)) {
				return FAILURE;
			}
		}
	}

	return SUCCESS;
}

/**
 * Exports a string symbol to the active symbol table
 */
int phalcon_set_symbol_str(char *key_name, unsigned int key_length, zval *value TSRMLS_DC) {

	if (!EG(active_symbol_table)) {
		zend_rebuild_symbol_table(TSRMLS_C);
	}

	if (&EG(symbol_table)) {
		Z_ADDREF_P(value);
		zend_hash_update(&EG(symbol_table), key_name, key_length, &value, sizeof(zval *), NULL);
		if (EG(exception)) {
			return FAILURE;
		}
	}

	return SUCCESS;
}