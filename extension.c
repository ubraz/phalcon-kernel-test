#include <main/php.h>
#include "php_phalcon.h"
#include "extension.h"

ZEND_DECLARE_MODULE_GLOBALS(phalcon)

static PHP_MINIT_FUNCTION(phalcon)
{
	PHALCON_INIT(Phalcon_Exception);
	return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(phalcon)
{
	if (PHALCON_GLOBAL(active_memory) != NULL) {
		phalcon_clean_shutdown_stack(TSRMLS_C);
	}

	if (PHALCON_GLOBAL(function_cache) != NULL) {
		zend_hash_destroy(PHALCON_GLOBAL(function_cache));
		FREE_HASHTABLE(PHALCON_GLOBAL(function_cache));
		PHALCON_GLOBAL(function_cache) = NULL;
	}

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
	if (PHALCON_GLOBAL(active_memory) != NULL) {
		phalcon_clean_shutdown_stack(TSRMLS_C);
	}

	if (PHALCON_GLOBAL(function_cache) != NULL) {
		zend_hash_destroy(PHALCON_GLOBAL(function_cache));
		FREE_HASHTABLE(PHALCON_GLOBAL(function_cache));
		PHALCON_GLOBAL(function_cache) = NULL;
	}

	phalcon_orm_destroy_cache(TSRMLS_C);

	return SUCCESS;
}

static PHP_GINIT_FUNCTION(phalcon)
{
	php_phalcon_init_globals(phalcon_globals TSRMLS_CC);
}

static PHP_GSHUTDOWN_FUNCTION(phalcon)
{
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

