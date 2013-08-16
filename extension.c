#include <main/php.h>
#include "php_phalcon.h"
#include "extension.h"
#include "embed.h"

ZEND_DECLARE_MODULE_GLOBALS(phalcon)

static PHP_MINIT_FUNCTION(phalcon)
{
	PHALCON_INIT(Phalcon_Exception);
	return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(phalcon)
{
	assert(PHALCON_GLOBAL(function_cache) == NULL);
	assert(PHALCON_GLOBAL(orm).parser_cache == NULL);
	assert(PHALCON_GLOBAL(orm).ast_cache == NULL);

	phalcon_orm_destroy_cache(TSRMLS_C);

	return SUCCESS;
}

static PHP_RINIT_FUNCTION(phalcon)
{
	php_phalcon_init_globals(PHALCON_VGLOBAL TSRMLS_CC);

	return SUCCESS;
}

static PHP_RSHUTDOWN_FUNCTION(phalcon)
{
	if (PHALCON_GLOBAL(start_memory) != NULL) {
		phalcon_clean_restore_stack(TSRMLS_C);
	}

	if (PHALCON_GLOBAL(function_cache) != NULL) {
		zend_hash_destroy(PHALCON_GLOBAL(function_cache));
		FREE_HASHTABLE(PHALCON_GLOBAL(function_cache));
		PHALCON_GLOBAL(function_cache) = NULL;
	}

	phalcon_orm_destroy_cache(TSRMLS_C);
	memclean_called = 1;

	return SUCCESS;
}

static PHP_GINIT_FUNCTION(phalcon)
{
	phalcon_memory_entry *start;

	php_phalcon_init_globals(phalcon_globals TSRMLS_CC);

	start = (phalcon_memory_entry *) pecalloc(1, sizeof(phalcon_memory_entry), 1);
/* pecalloc() will take care of these members
	start->pointer      = 0;
	start->hash_pointer = 0;
	start->prev = NULL;
	start->next = NULL;
*/
	start->addresses       = pecalloc(24, sizeof(zval*), 1);
	start->capacity        = 24;
	start->hash_addresses  = pecalloc(8, sizeof(zval*), 1);
	start->hash_capacity   = 8;

	phalcon_globals->start_memory = start;
}

static PHP_GSHUTDOWN_FUNCTION(phalcon)
{
	assert(phalcon_globals->start_memory != NULL);

	pefree(phalcon_globals->start_memory->hash_addresses, 1);
	pefree(phalcon_globals->start_memory->addresses, 1);
	pefree(phalcon_globals->start_memory, 1);
	phalcon_globals->start_memory = NULL;
}

zend_module_entry phalcon_module_entry = {
	STANDARD_MODULE_HEADER,
	"phalcon-kernel",
	NULL,
	PHP_MINIT(phalcon),
	PHP_MSHUTDOWN(phalcon),
	PHP_RINIT(phalcon),
	PHP_RSHUTDOWN(phalcon),
	NULL,
	PHP_PHALCON_VERSION,
	PHP_MODULE_GLOBALS(phalcon),
	PHP_GINIT(phalcon),
	PHP_GSHUTDOWN(phalcon),
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

