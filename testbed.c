#include <CUnit/CUnit.h>
#include <main/php.h>
#include "embed.h"
#include "php_phalcon.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/operators.h"
#include "testbed.h"

void test_increment(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* x = NULL;
		zval* y = NULL;
		zval* z;
		zval** a;

		CU_ASSERT_EQUAL(SUCCESS, zend_eval_string("class TestClass { public $id; } $a = new TestClass;", NULL, "sample class" TSRMLS_CC));
		CU_ASSERT_EQUAL(SUCCESS, zend_hash_find(EG(active_symbol_table), ZEND_STRS("a"), (void**)&a));
		CU_ASSERT_TRUE(Z_TYPE_PP(a) == IS_OBJECT);

		PHALCON_MM_GROW();

		PHALCON_INIT_NVAR(x);
		ZVAL_LONG(x, 0);
		PHALCON_CPY_WRT(y, x);

		/* x and y share the same value here */
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 2);

		/* Now x, y and $a->id share the same value */
		phalcon_update_property_zval(*a, ZEND_STRL("id"), y TSRMLS_CC);

		CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 3);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(y), 3);

		/* Therefore if we increment x, then y and $a->id will also be affected  */
		CU_ASSERT_EQUAL(SUCCESS, increment_function(y));

		CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 3);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(y), 3);
		CU_ASSERT_EQUAL(Z_LVAL_P(x), Z_LVAL_P(y));
		CU_ASSERT_EQUAL(Z_LVAL_P(x), 1);

		/* zend_read_property() won't modify the reference counter */
		z = zend_read_property(Z_OBJCE_PP(a), *a, ZEND_STRL("id"), 1 TSRMLS_CC);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(z), 3);
		CU_ASSERT_EQUAL(Z_LVAL_P(z), 1);

		/* The conclusion is that we need to separate y if we need to increment it independently */
		PHALCON_SEPARATE(y);
		CU_ASSERT_EQUAL(SUCCESS, increment_function(y));

		CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 2);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(y), 1);
		CU_ASSERT_EQUAL(Z_LVAL_P(x), 1);
		CU_ASSERT_EQUAL(Z_LVAL_P(y), 2);
		CU_ASSERT_EQUAL(Z_LVAL_P(z), 1);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(z), 2);

		PHALCON_MM_RESTORE();
	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}

void test_phalcon_mvc_router_route__construct(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* y = NULL;
		zval* z;
		zval** a;
		zval** b;

		CU_ASSERT_EQUAL(SUCCESS, zend_eval_string("class TestClass1 { public static $_uniqueId = null; public $_id; } $a = new TestClass1();", NULL, "sample class 1" TSRMLS_CC));
		CU_ASSERT_EQUAL(SUCCESS, zend_eval_string("class TestClass2 { public static $_uniqueId = null; public $_id; } $b = new TestClass2();", NULL, "sample class 2" TSRMLS_CC));

		CU_ASSERT_EQUAL(SUCCESS, zend_hash_find(EG(active_symbol_table), ZEND_STRS("a"), (void**)&a));
		CU_ASSERT_EQUAL(SUCCESS, zend_hash_find(EG(active_symbol_table), ZEND_STRS("b"), (void**)&b));
		CU_ASSERT_TRUE(Z_TYPE_PP(a) == IS_OBJECT);
		CU_ASSERT_TRUE(Z_TYPE_PP(b) == IS_OBJECT);

		/* Original code */
		{
			/* Imagine this is the first invocation of Phalcon\Mvc\Router\Route::__construct() */
			{
				zval* unique_id = NULL;
				zval* route_id = NULL;
				zval* one;
				zval* next_id;

				PHALCON_MM_GROW();

				PHALCON_OBS_VAR(unique_id);
				phalcon_read_static_property(&unique_id, ZEND_STRL("testclass1"), ZEND_STRL("_uniqueId") TSRMLS_CC);

				CU_ASSERT_EQUAL(Z_TYPE_P(unique_id), IS_NULL);
				if (Z_TYPE_P(unique_id) == IS_NULL) {
					PHALCON_INIT_NVAR(unique_id);
					ZVAL_LONG(unique_id, 0);
				}

				CU_ASSERT_EQUAL(Z_REFCOUNT_P(unique_id), 1);
				CU_ASSERT_EQUAL(Z_TYPE_P(unique_id), IS_LONG);

				PHALCON_CPY_WRT(route_id, unique_id);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(unique_id), 2);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(route_id), 2);

				phalcon_update_property_zval(*a, ZEND_STRL("_id"), route_id TSRMLS_CC);

				CU_ASSERT_EQUAL(Z_REFCOUNT_P(unique_id), 3);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(route_id), 3);

				PHALCON_INIT_VAR(one);
				ZVAL_LONG(one, 1);

				PHALCON_INIT_VAR(next_id);
				phalcon_add_function(next_id, unique_id, one TSRMLS_CC);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(next_id), 1);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(unique_id), 3);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(route_id), 3);

				phalcon_update_static_property(ZEND_STRL("testclass1"), ZEND_STRL("_uniqueId"), next_id TSRMLS_CC);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(next_id), 2);

				PHALCON_MM_RESTORE();
			}

			zval* id       = zend_read_property(Z_OBJCE_PP(a), *a, ZEND_STRL("_id"), 1 TSRMLS_CC);
			zval* uniqueid = zend_read_static_property(Z_OBJCE_PP(a), ZEND_STRL("_uniqueId"), 1 TSRMLS_CC);

			CU_ASSERT_EQUAL(Z_REFCOUNT_P(id), 1);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(uniqueid), 1);
			CU_ASSERT_EQUAL(Z_LVAL_P(id), 0);
			CU_ASSERT_EQUAL(Z_LVAL_P(uniqueid), 1);

			/* Imagine this is the second invocation of Phalcon\Mvc\Router\Route::__construct() */
			{
				zval* unique_id = NULL;
				zval* route_id = NULL;
				zval* one;
				zval* next_id;

				PHALCON_MM_GROW();

				PHALCON_OBS_VAR(unique_id);
				phalcon_read_static_property(&unique_id, ZEND_STRL("testclass1"), ZEND_STRL("_uniqueId") TSRMLS_CC);

				CU_ASSERT_EQUAL(Z_TYPE_P(unique_id), IS_LONG);
				CU_ASSERT_EQUAL(Z_LVAL_P(unique_id), 1);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(unique_id), 2);

				PHALCON_CPY_WRT(route_id, unique_id);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(unique_id), 3);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(route_id), 3);

				phalcon_update_property_zval(*a, ZEND_STRL("_id"), route_id TSRMLS_CC);

				CU_ASSERT_EQUAL(Z_REFCOUNT_P(unique_id), 4);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(route_id), 4);

				PHALCON_INIT_VAR(one);
				ZVAL_LONG(one, 1);

				PHALCON_INIT_VAR(next_id);
				phalcon_add_function(next_id, unique_id, one TSRMLS_CC);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(next_id), 1);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(unique_id), 4);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(route_id), 4);

				phalcon_update_static_property(ZEND_STRL("testclass1"), ZEND_STRL("_uniqueId"), next_id TSRMLS_CC);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(next_id), 2);

				PHALCON_MM_RESTORE();
			}

			id       = zend_read_property(Z_OBJCE_PP(a), *a, ZEND_STRL("_id"), 1 TSRMLS_CC);
			uniqueid = zend_read_static_property(Z_OBJCE_PP(a), ZEND_STRL("_uniqueId"), 1 TSRMLS_CC);

			CU_ASSERT_EQUAL(Z_REFCOUNT_P(id), 1);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(uniqueid), 1);
			CU_ASSERT_EQUAL(Z_LVAL_P(id), 1);
			CU_ASSERT_EQUAL(Z_LVAL_P(uniqueid), 2);
		}

		/* Optimized code */
		{
			/* Imagine this is the first invocation of Phalcon\Mvc\Router\Route::__construct() */
			{
				zval* unique_id = NULL;
				zval* route_id = NULL;

				PHALCON_MM_GROW();

				PHALCON_OBS_VAR(unique_id);
				phalcon_read_static_property(&unique_id, ZEND_STRL("testclass2"), ZEND_STRL("_uniqueId") TSRMLS_CC);

				CU_ASSERT_EQUAL(Z_TYPE_P(unique_id), IS_NULL);
				if (Z_TYPE_P(unique_id) == IS_NULL) {
					ZVAL_LONG(unique_id, 0);
				}

				CU_ASSERT_EQUAL(Z_REFCOUNT_P(unique_id), 2);
				CU_ASSERT_EQUAL(Z_TYPE_P(unique_id), IS_LONG);

				PHALCON_CPY_WRT_CTOR(route_id, unique_id);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(unique_id), 2);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(route_id), 1);

				phalcon_update_property_zval(*b, ZEND_STRL("_id"), route_id TSRMLS_CC);

				CU_ASSERT_EQUAL(Z_REFCOUNT_P(unique_id), 2);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(route_id), 2);

				CU_ASSERT_EQUAL(SUCCESS, increment_function(unique_id));
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(unique_id), 2);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(route_id), 2);

				CU_ASSERT_EQUAL(Z_LVAL_P(unique_id), 1);
				CU_ASSERT_EQUAL(Z_LVAL_P(route_id), 0);

				PHALCON_MM_RESTORE();
			}

			zval* id       = zend_read_property(Z_OBJCE_PP(a), *b, ZEND_STRL("_id"), 1 TSRMLS_CC);
			zval* uniqueid = zend_read_static_property(Z_OBJCE_PP(b), ZEND_STRL("_uniqueId"), 1 TSRMLS_CC);

			CU_ASSERT_EQUAL(Z_REFCOUNT_P(id), 1);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(uniqueid), 1);
			CU_ASSERT_EQUAL(Z_LVAL_P(id), 0);
			CU_ASSERT_EQUAL(Z_LVAL_P(uniqueid), 1);

			/* Imagine this is the second invocation of Phalcon\Mvc\Router\Route::__construct() */
			{
				zval* unique_id = NULL;
				zval* route_id = NULL;

				PHALCON_MM_GROW();

				PHALCON_OBS_VAR(unique_id);
				phalcon_read_static_property(&unique_id, ZEND_STRL("testclass2"), ZEND_STRL("_uniqueId") TSRMLS_CC);

				CU_ASSERT_EQUAL(Z_TYPE_P(unique_id), IS_LONG);
				CU_ASSERT_EQUAL(Z_LVAL_P(unique_id), 1);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(unique_id), 2);

				PHALCON_CPY_WRT_CTOR(route_id, unique_id);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(unique_id), 2);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(route_id), 1);

				phalcon_update_property_zval(*b, ZEND_STRL("_id"), route_id TSRMLS_CC);

				CU_ASSERT_EQUAL(Z_REFCOUNT_P(unique_id), 2);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(route_id), 2);

				CU_ASSERT_EQUAL(SUCCESS, increment_function(unique_id));
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(unique_id), 2);
				CU_ASSERT_EQUAL(Z_REFCOUNT_P(route_id), 2);

				CU_ASSERT_EQUAL(Z_LVAL_P(unique_id), 2);
				CU_ASSERT_EQUAL(Z_LVAL_P(route_id), 1);

				PHALCON_MM_RESTORE();
			}

			id       = zend_read_property(Z_OBJCE_PP(a), *b, ZEND_STRL("_id"), 1 TSRMLS_CC);
			uniqueid = zend_read_static_property(Z_OBJCE_PP(b), ZEND_STRL("_uniqueId"), 1 TSRMLS_CC);

			CU_ASSERT_EQUAL(Z_REFCOUNT_P(id), 1);
			CU_ASSERT_EQUAL(Z_REFCOUNT_P(uniqueid), 1);
			CU_ASSERT_EQUAL(Z_LVAL_P(id), 1);
			CU_ASSERT_EQUAL(Z_LVAL_P(uniqueid), 2);
		}
	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);

}
