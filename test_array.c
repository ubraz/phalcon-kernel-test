#include <CUnit/CUnit.h>
#include <main/php.h>
#include "embed.h"
#include "php_phalcon.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "test_array.h"

static char* __zend_filename = __FILE__;
static char* __zend_orig_filename = __FILE__;
static int __zend_lineno = 0;
static int __zend_orig_lineno = 0;

void test_phalcon_array_unshift(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* arr;
		zval* val;
		zval** tmp;

		ALLOC_INIT_ZVAL(arr);
		ALLOC_INIT_ZVAL(val);

		array_init_size(arr, 4);
		add_next_index_long(arr, 1);
		add_next_index_long(arr, 2);
		add_next_index_long(arr, 3);

		ZVAL_LONG(val, 0);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(val), 1);
		phalcon_array_unshift(arr, val);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(val), 2);

		CU_ASSERT_EQUAL(Z_TYPE_P(arr), IS_ARRAY);
		CU_ASSERT_EQUAL(zend_hash_num_elements(Z_ARRVAL_P(arr)), 4);
		CU_ASSERT_TRUE(zend_hash_index_exists(Z_ARRVAL_P(arr), 0));
		CU_ASSERT_TRUE(zend_hash_index_exists(Z_ARRVAL_P(arr), 1));
		CU_ASSERT_TRUE(zend_hash_index_exists(Z_ARRVAL_P(arr), 2));
		CU_ASSERT_TRUE(zend_hash_index_exists(Z_ARRVAL_P(arr), 3));

		CU_ASSERT_EQUAL(SUCCESS, zend_hash_index_find(Z_ARRVAL_P(arr), 0, (void**)&tmp));
		CU_ASSERT_EQUAL(_mem_block_check(*tmp, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_EQUAL(Z_TYPE_PP(tmp), IS_LONG);
		CU_ASSERT_EQUAL(Z_ISREF_PP(tmp), 0);
		CU_ASSERT_EQUAL(Z_REFCOUNT_PP(tmp), 2);
		CU_ASSERT_EQUAL(Z_LVAL_PP(tmp), 0);
		ZVAL_LONG(val, -1);
		CU_ASSERT_EQUAL(Z_LVAL_PP(tmp), -1);

		CU_ASSERT_EQUAL(SUCCESS, zend_hash_index_find(Z_ARRVAL_P(arr), 1, (void**)&tmp));
		CU_ASSERT_EQUAL(_mem_block_check(*tmp, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_EQUAL(Z_TYPE_PP(tmp), IS_LONG);
		CU_ASSERT_EQUAL(Z_ISREF_PP(tmp), 0);
		CU_ASSERT_EQUAL(Z_REFCOUNT_PP(tmp), 1);
		CU_ASSERT_EQUAL(Z_LVAL_PP(tmp), 1);

		CU_ASSERT_EQUAL(SUCCESS, zend_hash_index_find(Z_ARRVAL_P(arr), 2, (void**)&tmp));
		CU_ASSERT_EQUAL(_mem_block_check(*tmp, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_EQUAL(Z_TYPE_PP(tmp), IS_LONG);
		CU_ASSERT_EQUAL(Z_ISREF_PP(tmp), 0);
		CU_ASSERT_EQUAL(Z_REFCOUNT_PP(tmp), 1);
		CU_ASSERT_EQUAL(Z_LVAL_PP(tmp), 2);

		CU_ASSERT_EQUAL(SUCCESS, zend_hash_index_find(Z_ARRVAL_P(arr), 3, (void**)&tmp));
		CU_ASSERT_EQUAL(_mem_block_check(*tmp, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_EQUAL(Z_TYPE_PP(tmp), IS_LONG);
		CU_ASSERT_EQUAL(Z_ISREF_PP(tmp), 0);
		CU_ASSERT_EQUAL(Z_REFCOUNT_PP(tmp), 1);
		CU_ASSERT_EQUAL(Z_LVAL_PP(tmp), 3);

		CU_ASSERT_EQUAL(FAILURE, zend_hash_index_find(Z_ARRVAL_P(arr), 4, (void**)&tmp));
		CU_ASSERT_EQUAL(_mem_block_check(*tmp, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_EQUAL(Z_TYPE_PP(tmp), IS_LONG);
		CU_ASSERT_EQUAL(Z_ISREF_PP(tmp), 0);
		CU_ASSERT_EQUAL(Z_REFCOUNT_PP(tmp), 1);
		CU_ASSERT_EQUAL(Z_LVAL_PP(tmp), 3);

		zval_ptr_dtor(&arr);
		zval_ptr_dtor(&val);
	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}

void test_phalcon_array_values(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* arr;
		zval* res;
		zval** tmp;

		ALLOC_INIT_ZVAL(arr);
		ALLOC_INIT_ZVAL(res);

		array_init_size(arr, 4);
		add_index_long(arr, 10, 1);
		add_index_long(arr, 20, 2);
		add_index_long(arr, 30, 3);
		add_index_long(arr, 40, 4);

		phalcon_array_values(res, arr);

		CU_ASSERT_EQUAL(Z_TYPE_P(res), IS_ARRAY);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(res), 1);
		CU_ASSERT_EQUAL(Z_ISREF_P(res), 0);
		CU_ASSERT_EQUAL(zend_hash_num_elements(Z_ARRVAL_P(arr)), zend_hash_num_elements(Z_ARRVAL_P(res)));

		CU_ASSERT_EQUAL(zend_hash_num_elements(Z_ARRVAL_P(res)), 4);
		CU_ASSERT_TRUE(zend_hash_index_exists(Z_ARRVAL_P(res), 0));
		CU_ASSERT_TRUE(zend_hash_index_exists(Z_ARRVAL_P(res), 1));
		CU_ASSERT_TRUE(zend_hash_index_exists(Z_ARRVAL_P(res), 2));
		CU_ASSERT_TRUE(zend_hash_index_exists(Z_ARRVAL_P(res), 3));

		CU_ASSERT_FALSE(zend_hash_index_exists(Z_ARRVAL_P(arr), 0));
		CU_ASSERT_FALSE(zend_hash_index_exists(Z_ARRVAL_P(arr), 1));
		CU_ASSERT_FALSE(zend_hash_index_exists(Z_ARRVAL_P(arr), 2));
		CU_ASSERT_FALSE(zend_hash_index_exists(Z_ARRVAL_P(arr), 3));

		CU_ASSERT_FALSE(zend_hash_index_exists(Z_ARRVAL_P(res), 10));
		CU_ASSERT_FALSE(zend_hash_index_exists(Z_ARRVAL_P(res), 20));
		CU_ASSERT_FALSE(zend_hash_index_exists(Z_ARRVAL_P(res), 30));
		CU_ASSERT_FALSE(zend_hash_index_exists(Z_ARRVAL_P(res), 40));

		CU_ASSERT_TRUE(zend_hash_index_exists(Z_ARRVAL_P(arr), 10));
		CU_ASSERT_TRUE(zend_hash_index_exists(Z_ARRVAL_P(arr), 20));
		CU_ASSERT_TRUE(zend_hash_index_exists(Z_ARRVAL_P(arr), 30));
		CU_ASSERT_TRUE(zend_hash_index_exists(Z_ARRVAL_P(arr), 40));

		CU_ASSERT_EQUAL(SUCCESS, zend_hash_index_find(Z_ARRVAL_P(res), 0, (void**)&tmp));
		CU_ASSERT_EQUAL(_mem_block_check(*tmp, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_EQUAL(Z_TYPE_PP(tmp), IS_LONG);
		CU_ASSERT_EQUAL(Z_ISREF_PP(tmp), 0);
		CU_ASSERT_EQUAL(Z_REFCOUNT_PP(tmp), 2);
		CU_ASSERT_EQUAL(Z_LVAL_PP(tmp), 1);

		CU_ASSERT_EQUAL(SUCCESS, zend_hash_index_find(Z_ARRVAL_P(res), 1, (void**)&tmp));
		CU_ASSERT_EQUAL(_mem_block_check(*tmp, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_EQUAL(Z_TYPE_PP(tmp), IS_LONG);
		CU_ASSERT_EQUAL(Z_ISREF_PP(tmp), 0);
		CU_ASSERT_EQUAL(Z_REFCOUNT_PP(tmp), 2);
		CU_ASSERT_EQUAL(Z_LVAL_PP(tmp), 2);

		CU_ASSERT_EQUAL(SUCCESS, zend_hash_index_find(Z_ARRVAL_P(res), 2, (void**)&tmp));
		CU_ASSERT_EQUAL(_mem_block_check(*tmp, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_EQUAL(Z_TYPE_PP(tmp), IS_LONG);
		CU_ASSERT_EQUAL(Z_ISREF_PP(tmp), 0);
		CU_ASSERT_EQUAL(Z_REFCOUNT_PP(tmp), 2);
		CU_ASSERT_EQUAL(Z_LVAL_PP(tmp), 3);

		CU_ASSERT_EQUAL(SUCCESS, zend_hash_index_find(Z_ARRVAL_P(res), 3, (void**)&tmp));
		CU_ASSERT_EQUAL(_mem_block_check(*tmp, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
		CU_ASSERT_EQUAL(Z_TYPE_PP(tmp), IS_LONG);
		CU_ASSERT_EQUAL(Z_ISREF_PP(tmp), 0);
		CU_ASSERT_EQUAL(Z_REFCOUNT_PP(tmp), 2);
		CU_ASSERT_EQUAL(Z_LVAL_PP(tmp), 4);

		zval_ptr_dtor(&arr);
		zval_ptr_dtor(&res);
	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}

void test_phalcon_array_key_exists(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* arr;
		zval** tmp;
		zval idx;

		ALLOC_INIT_ZVAL(arr);
		INIT_ZVAL(idx);

		array_init_size(arr, 4);
		add_index_long(arr, 10, 1);
		add_index_long(arr, 20, 2);

		ZVAL_NULL(&idx);
		CU_ASSERT_FALSE(phalcon_array_key_exists(arr, &idx TSRMLS_CC));

		add_assoc_long(arr, "", 5);
		CU_ASSERT_TRUE(phalcon_array_key_exists(arr, &idx TSRMLS_CC));

		ZVAL_STRING(&idx, "test", 0);
		CU_ASSERT_FALSE(phalcon_array_key_exists(arr, &idx TSRMLS_CC));

		add_assoc_long(arr, "test", 6);
		CU_ASSERT_TRUE(phalcon_array_key_exists(arr, &idx TSRMLS_CC));

		ZVAL_LONG(&idx, 0);
		CU_ASSERT_FALSE(phalcon_array_key_exists(arr, &idx TSRMLS_CC));

		ZVAL_LONG(&idx, 1);
		CU_ASSERT_FALSE(phalcon_array_key_exists(arr, &idx TSRMLS_CC));

		ZVAL_LONG(&idx, 10);
		CU_ASSERT_TRUE(phalcon_array_key_exists(arr, &idx TSRMLS_CC));

		ZVAL_LONG(&idx, 20);
		CU_ASSERT_TRUE(phalcon_array_key_exists(arr, &idx TSRMLS_CC));

		zval_ptr_dtor(&arr);
	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}
