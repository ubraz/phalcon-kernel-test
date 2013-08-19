#include <CUnit/CUnit.h>
#include <main/php.h>
#include "embed.h"
#include "php_phalcon.h"
#include "kernel/memory.h"
#include "kernel/variables.h"
#include "test_variables.h"

void test_phalcon_serialize(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* res;
		zval* var;

		MAKE_STD_ZVAL(res);
		MAKE_STD_ZVAL(var);

		ZVAL_LONG(var, 0x12345678l);

		phalcon_serialize(res, &var TSRMLS_CC);

		CU_ASSERT_EQUAL(Z_TYPE_P(res), IS_STRING);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(res), 1);
		CU_ASSERT_EQUAL(Z_ISREF_P(res), 0);

		zval_ptr_dtor(&res);
		zval_ptr_dtor(&var);
	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}

void test_phalcon_unserialize(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* res;
		zval* var;
		int old = EG(error_reporting);

		MAKE_STD_ZVAL(res);
		MAKE_STD_ZVAL(var);

		ZVAL_LONG(var, 0x12345678l);

		phalcon_unserialize(res, var TSRMLS_CC);
		CU_ASSERT_EQUAL(Z_TYPE_P(res), IS_BOOL);
		CU_ASSERT_EQUAL(Z_BVAL_P(res), 0);

		ZVAL_EMPTY_STRING(var);
		phalcon_unserialize(res, var TSRMLS_CC);
		CU_ASSERT_EQUAL(Z_TYPE_P(res), IS_BOOL);
		CU_ASSERT_EQUAL(Z_BVAL_P(res), 0);
		zval_dtor(var);

		ZVAL_STRING(var, "i:123;", 1);
		phalcon_unserialize(res, var TSRMLS_CC);
		CU_ASSERT_EQUAL(Z_TYPE_P(res), IS_LONG);
		CU_ASSERT_EQUAL(Z_LVAL_P(res), 123);
		zval_dtor(var);
		zval_dtor(res);

		EG(error_reporting) = 0;
		ZVAL_STRING(var, "i:123", 1);
		phalcon_unserialize(res, var TSRMLS_CC);
		CU_ASSERT_EQUAL(Z_TYPE_P(res), IS_BOOL);
		CU_ASSERT_EQUAL(Z_BVAL_P(res), 0);
		EG(error_reporting) = old;

		zval_ptr_dtor(&res);
		zval_ptr_dtor(&var);
	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}
