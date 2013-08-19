#include <CUnit/CUnit.h>
#include <main/php.h>
#include "embed.h"
#include "php_phalcon.h"
#include "kernel/memory.h"
#include "test_memory.h"

#if ZEND_DEBUG
static char* __zend_filename = __FILE__;
static char* __zend_orig_filename = __FILE__;
static int __zend_lineno = 0;
static int __zend_orig_lineno = 0;
#endif

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

		x   = NULL;
		str = NULL;

		PHALCON_MM_GROW();
			PHALCON_INIT_NVAR_PNULL(x);
			str = (char*)estrdup("satyaM vada dharmaM chara svAdhyAyAnmA pramadaH");
			ZVAL_STRING(x, str, 0);
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

void test_phalcon_cpy_wrt(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* dest;
		zval* src;

		PHALCON_MM_GROW();
		/* dest is not observed by Phalcon */
			MAKE_STD_ZVAL(dest);
			ZVAL_STRING(dest, "R^itaM cha svAdhyAyapravachane cha", 1);

			PHALCON_INIT_VAR(src);
			ZVAL_STRING(src, "satyaM cha svAdhyAyapravachane cha", 1);

			PHALCON_CPY_WRT(dest, src);

			CU_ASSERT_PTR_EQUAL(dest, src);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(src), 2);
			CU_ASSERT_EQUAL(Z_TYPE_P(src), IS_STRING);
		PHALCON_MM_RESTORE();

		CU_ASSERT_EQUAL(_mem_block_check(dest, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_PTR_EQUAL(dest, src);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(dest), 1);
		CU_ASSERT_EQUAL(Z_TYPE_P(src), IS_STRING);
		CU_ASSERT_STRING_EQUAL(Z_STRVAL_P(src), "satyaM cha svAdhyAyapravachane cha");
		zval_ptr_dtor(&dest);

		PHALCON_MM_GROW();
		/* dest will be observed by Phalcon */
			dest = NULL;

			PHALCON_INIT_VAR(src);
			ZVAL_STRING(src, "satyaM cha svAdhyAyapravachane cha", 1);

			PHALCON_CPY_WRT(dest, src);

			CU_ASSERT_PTR_EQUAL(dest, src);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(src), 2);
			CU_ASSERT_EQUAL(Z_TYPE_P(src), IS_STRING);
		PHALCON_MM_RESTORE();
		/* At this point dest will be destroyed */

	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}

void test_phalcon_cpy_wrt_ctor(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* dest;
		zval* src;

		PHALCON_MM_GROW();
		/* dest is not observed by Phalcon */
			MAKE_STD_ZVAL(dest);
			ZVAL_STRING(dest, "R^itaM cha svAdhyAyapravachane cha", 1);

			PHALCON_INIT_VAR(src);
			ZVAL_STRING(src, "satyaM cha svAdhyAyapravachane cha", 1);

			PHALCON_CPY_WRT_CTOR(dest, src);

			CU_ASSERT_PTR_NOT_EQUAL(dest, src);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(src), 1);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(dest), 1);
			CU_ASSERT_EQUAL(Z_TYPE_P(src), IS_STRING);
			CU_ASSERT_EQUAL(Z_ISREF_P(src), 0);
			CU_ASSERT_EQUAL(Z_ISREF_P(dest), 0);
		PHALCON_MM_RESTORE();

		CU_ASSERT_EQUAL(_mem_block_check(dest, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_PTR_NOT_EQUAL(dest, src);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(dest), 1);
		CU_ASSERT_STRING_EQUAL(Z_STRVAL_P(dest), "satyaM cha svAdhyAyapravachane cha");
		zval_ptr_dtor(&dest);

		PHALCON_MM_GROW();
		/* dest will be observed by Phalcon */
			dest = NULL;

			PHALCON_INIT_VAR(src);
			ZVAL_STRING(src, "satyaM cha svAdhyAyapravachane cha", 1);

			PHALCON_CPY_WRT_CTOR(dest, src);

			CU_ASSERT_PTR_NOT_EQUAL(dest, src);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(src), 1);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(dest), 1);
			CU_ASSERT_EQUAL(Z_TYPE_P(src), IS_STRING);
			CU_ASSERT_EQUAL(Z_ISREF_P(src), 0);
			CU_ASSERT_EQUAL(Z_ISREF_P(dest), 0);
		PHALCON_MM_RESTORE();
		/* At this point dest will be destroyed */

	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}

void test_phalcon_obs_var(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* x;
		zval* y;

		PHALCON_MM_GROW();
			PHALCON_OBS_VAR(x);
			ALLOC_INIT_ZVAL(x);

			CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 1);
			CU_ASSERT_EQUAL(Z_ISREF_P(x), 0);

			PHALCON_OBS_VAR(y);
			ALLOC_INIT_ZVAL(y);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(y), 1);
			CU_ASSERT_EQUAL(Z_ISREF_P(y), 0);
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

void test_phalcon_obs_nvar(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* x = NULL;

		PHALCON_MM_GROW();
			PHALCON_OBS_NVAR(x);
			ALLOC_INIT_ZVAL(x);

			CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 1);
			CU_ASSERT_EQUAL(Z_ISREF_P(x), 0);

			PHALCON_OBS_NVAR(x);
			ALLOC_INIT_ZVAL(x);

			CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 1);
			CU_ASSERT_EQUAL(Z_ISREF_P(x), 0);
		PHALCON_MM_RESTORE();
	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}

void test_phalcon_separate(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* x;
		zval* orig;

		MAKE_STD_ZVAL(x);
		ZVAL_LONG(x, 0xB61964F6l);
		Z_ADDREF_P(x);
		Z_ADDREF_P(x);
		Z_ADDREF_P(x);
		orig = x;

		CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 4);
		CU_ASSERT_EQUAL(Z_ISREF_P(x), 0);

		PHALCON_SEPARATE(x);

		CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 1);
		CU_ASSERT_EQUAL(Z_ISREF_P(x), 0);
		CU_ASSERT_PTR_NOT_EQUAL(x, orig);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(orig), 3);
		CU_ASSERT_EQUAL(Z_ISREF_P(orig), 0);

		CU_ASSERT_EQUAL(Z_TYPE_P(x), IS_LONG);
		CU_ASSERT_EQUAL(Z_LVAL_P(x), 0xB61964F6l);

		CU_ASSERT_EQUAL(Z_TYPE_P(x), Z_TYPE_P(orig));
		CU_ASSERT_EQUAL(Z_LVAL_P(x), Z_LVAL_P(orig));

		zval_ptr_dtor(&orig);
		CU_ASSERT_EQUAL(_mem_block_check(x, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_EQUAL(_mem_block_check(orig, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(orig), 2);

		zval_ptr_dtor(&x);
		CU_ASSERT_EQUAL(_mem_block_check(orig, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(orig), 2);

		zval_ptr_dtor(&orig);
		CU_ASSERT_EQUAL(_mem_block_check(orig, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(orig), 1);

		zval_ptr_dtor(&orig);
	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}

void test_phalcon_separate_param(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* x;
		zval* orig;

		PHALCON_MM_GROW();
			MAKE_STD_ZVAL(x);
			ZVAL_LONG(x, 0xB61964F6l);
			Z_ADDREF_P(x);
			Z_ADDREF_P(x);
			Z_ADDREF_P(x);
			orig = x;

			CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 4);
			CU_ASSERT_EQUAL(Z_ISREF_P(x), 0);

			PHALCON_SEPARATE_PARAM(x);

			CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 1);
			CU_ASSERT_EQUAL(Z_ISREF_P(x), 0);
			CU_ASSERT_PTR_NOT_EQUAL(x, orig);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(orig), 4);
			CU_ASSERT_EQUAL(Z_ISREF_P(orig), 0);

			CU_ASSERT_EQUAL(Z_TYPE_P(x), IS_LONG);
			CU_ASSERT_EQUAL(Z_LVAL_P(x), 0xB61964F6l);

			CU_ASSERT_EQUAL(Z_TYPE_P(x), Z_TYPE_P(orig));
			CU_ASSERT_EQUAL(Z_LVAL_P(x), Z_LVAL_P(orig));

			zval_ptr_dtor(&orig);
			CU_ASSERT_EQUAL(_mem_block_check(x, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
			CU_ASSERT_EQUAL(_mem_block_check(orig, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(orig), 3);

			zval_ptr_dtor(&orig);
			CU_ASSERT_EQUAL(_mem_block_check(x, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
			CU_ASSERT_EQUAL(_mem_block_check(orig, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(orig), 2);

			zval_ptr_dtor(&orig);
			CU_ASSERT_EQUAL(_mem_block_check(x, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
			CU_ASSERT_EQUAL(_mem_block_check(orig, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(orig), 1);

			zval_ptr_dtor(&orig);
			CU_ASSERT_EQUAL(_mem_block_check(x, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		PHALCON_MM_RESTORE();
	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}
