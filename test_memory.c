#include <CUnit/CUnit.h>
#include <main/php.h>
#include "embed.h"
#include "php_phalcon.h"
#include "kernel/memory.h"

char* __zend_filename = __FILE__;
char* __zend_orig_filename = __FILE__;
int __zend_lineno = 0;
int __zend_orig_lineno = 0;

void test_phalcon_alloc_zval(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* x;

		PHALCON_MM_GROW();
			PHALCON_ALLOC_ZVAL(x);

			CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 1);
			CU_ASSERT_EQUAL(Z_ISREF_P(x), 0);
			CU_ASSERT_EQUAL(Z_TYPE_P(x), IS_NULL);
		PHALCON_MM_RESTORE();

		/* zvals allocated by PHALCON_ALLOC_ZVAL() should not be affected by PHALCON_MM_RESTORE() */
		CU_ASSERT_EQUAL(_mem_block_check(x, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 1);

		zval_ptr_dtor(&x);
	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}

void test_phalcon_init_var(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* x;
		zval* y;

		PHALCON_MM_GROW();
			PHALCON_INIT_VAR(x);

			CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 1);
			CU_ASSERT_EQUAL(Z_ISREF_P(x), 0);
			CU_ASSERT_EQUAL(Z_TYPE_P(x), IS_NULL);

			PHALCON_INIT_VAR(y);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(y), 1);
			CU_ASSERT_EQUAL(Z_ISREF_P(y), 0);
			CU_ASSERT_EQUAL(Z_TYPE_P(y), IS_NULL);
			Z_ADDREF_P(y);
		PHALCON_MM_RESTORE();

		/* y has two references => should not be freed */
		CU_ASSERT_EQUAL(_mem_block_check(y, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(y), 1);
		zval_ptr_dtor(&y);
	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}

void test_phalcon_init_nvar(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* x = NULL;

		PHALCON_MM_GROW();
			PHALCON_INIT_NVAR(x);

			CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 1);
			CU_ASSERT_EQUAL(Z_ISREF_P(x), 0);
			CU_ASSERT_EQUAL(Z_TYPE_P(x), IS_NULL);

			ZVAL_LONG(x, 0x12345678ul);

			PHALCON_INIT_NVAR(x);

			CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 1);
			CU_ASSERT_EQUAL(Z_ISREF_P(x), 0);
			CU_ASSERT_EQUAL(Z_TYPE_P(x), IS_NULL);
		PHALCON_MM_RESTORE();
	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}

void test_phalcon_init_nvar_pnull(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* x   = NULL;
		char* str = NULL;

		PHALCON_MM_GROW();
			PHALCON_INIT_NVAR(x);

			CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 1);
			CU_ASSERT_EQUAL(Z_ISREF_P(x), 0);
			CU_ASSERT_EQUAL(Z_TYPE_P(x), IS_NULL);

			str = (char*)estrdup("satyaM vada dharmaM chara svAdhyAyAnmA pramadaH");
			ZVAL_STRING(x, str, 0);

			PHALCON_INIT_NVAR_PNULL(x);
			CU_ASSERT_EQUAL(_mem_block_check(str, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 1);
			CU_ASSERT_EQUAL(Z_ISREF_P(x), 0);
			CU_ASSERT_EQUAL(Z_TYPE_P(x), IS_NULL);
		PHALCON_MM_RESTORE();

		CU_ASSERT_EQUAL(_mem_block_check(str, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		efree(str);
	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}
