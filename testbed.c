#include <CUnit/CUnit.h>
#include <main/php.h>
#include <main/php_main.h>
#include <main/SAPI.h>
#include <Zend/zend_exceptions.h>
#include "embed.h"
#include "php_phalcon.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/operators.h"
#include "testbed.h"

#if ZEND_DEBUG
static char* __zend_filename = __FILE__;
static char* __zend_orig_filename = __FILE__;
static int __zend_lineno = 0;
static int __zend_orig_lineno = 0;
#endif

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

void test_array_mess(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* x;
		zval* y;
		zval** a;

		CU_ASSERT_EQUAL(SUCCESS, zend_eval_string("class TestClass { public $arr; } $a = new TestClass;", NULL, "sample class" TSRMLS_CC));
		CU_ASSERT_EQUAL(SUCCESS, zend_hash_find(EG(active_symbol_table), ZEND_STRS("a"), (void**)&a));
		CU_ASSERT_TRUE(Z_TYPE_PP(a) == IS_OBJECT);

		zend_update_property_null(Z_OBJCE_PP(a), *a, ZEND_STRL("arr") TSRMLS_CC);
		x = zend_read_property(Z_OBJCE_PP(a), *a, ZEND_STRL("arr"), 0 TSRMLS_CC);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 1);

		ALLOC_INIT_ZVAL(y);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(y), 1);
		zend_update_property(Z_OBJCE_PP(a), *a, ZEND_STRL("arr"), y TSRMLS_CC);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(y), 2);

		SEPARATE_ZVAL(&y);
		array_init(y);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(y), 1);
		zend_update_property(Z_OBJCE_PP(a), *a, ZEND_STRL("arr"), y TSRMLS_CC);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(y), 2);

		x = zend_read_property(Z_OBJCE_PP(a), *a, ZEND_STRL("arr"), 0 TSRMLS_CC);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 2);
		CU_ASSERT_EQUAL(Z_TYPE_P(x), IS_ARRAY);

		SEPARATE_ZVAL(&y);
		add_next_index_long(y, 1);
		zend_update_property(Z_OBJCE_PP(a), *a, ZEND_STRL("arr"), y TSRMLS_CC);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(y), 2);

		x = zend_read_property(Z_OBJCE_PP(a), *a, ZEND_STRL("arr"), 0 TSRMLS_CC);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 2);
		CU_ASSERT_EQUAL(Z_TYPE_P(x), IS_ARRAY);

		zend_update_property(Z_OBJCE_PP(a), *a, ZEND_STRL("arr"), y TSRMLS_CC);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(y), 2);
		CU_ASSERT_EQUAL(Z_REFCOUNT_P(x), 2);
		CU_ASSERT_EQUAL(Z_TYPE_P(x), IS_ARRAY);

		zval_ptr_dtor(&y);
	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}

PHP_FUNCTION(test_function)
{
//	zend_eval_string("throw new Exception();", NULL, "" TSRMLS_CC);
//	zend_eval_string("echo 'Test';", NULL, "" TSRMLS_CC);
//	CU_ASSERT_PTR_NOT_NULL(EG(exception));
//	zend_clear_exception(TSRMLS_C);
//	zend_eval_string("print_r(debug_backtrace());", NULL, "" TSRMLS_CC);
}

zend_function_entry fe[] = {
	PHP_FE(test_function, NULL)
	PHP_FE_END
};

int call_internal_function(zval* return_value, const char* func, uint func_len, uint pcnt, zval** params TSRMLS_DC)
{
	zend_function* function_handler;

	if (UNEXPECTED(!EG(active) || EG(exception))) {
		return FAILURE;
	}

	if (EXPECTED(zend_hash_find(EG(function_table), func, func_len, (void**)&function_handler) == SUCCESS)) {
		if (UNEXPECTED(function_handler->type != ZEND_INTERNAL_FUNCTION)) {
			return FAILURE;
		}

		zend_execute_data execute_data;
		zval* current_this;
		zend_class_entry* current_scope;
		zend_class_entry* current_called_scope;
		uint i;

		if (EG(current_execute_data)) {
			execute_data = *EG(current_execute_data);
		}

		EX(op_array) = NULL;
		EX(opline)   = NULL;
		EX(function_state).function = function_handler;

		ZEND_VM_STACK_GROW_IF_NEEDED(pcnt + 1);
		for (i=0; i<pcnt; ++i) {
#if PHP_VERSION_ID < 50500
			zend_vm_stack_push_nocheck(params[i] TSRMLS_CC);
#else
			zend_vm_stack_push(params[i] TSRMLS_CC);
#endif
		}

		EX(function_state).arguments = zend_vm_stack_top(TSRMLS_C);
#if PHP_VERSION_ID < 50500
		zend_vm_stack_push_nocheck((void*)(zend_uintptr_t)pcnt TSRMLS_CC);
#else
		zend_vm_stack_push((void*)(zend_uintptr_t)pcnt TSRMLS_CC);
#endif

		current_scope        = EG(scope);
		current_called_scope = EG(called_scope);
		current_this         = EG(This);

		EX(object) = NULL;
		EG(scope)  = NULL;
		EG(This)   = NULL;

		EX(prev_execute_data)    = EG(current_execute_data);
		EG(current_execute_data) = &execute_data;

		function_handler->internal_function.handler(pcnt, return_value, NULL, NULL, 1 TSRMLS_CC);
#if PHP_VERSION_ID < 50500
		zend_vm_stack_clear_multiple(TSRMLS_C);
#else
		zend_vm_stack_clear_multiple(0 TSRMLS_CC);
#endif

		EG(called_scope)         = current_called_scope;
		EG(scope)                = current_scope;
		EG(This)                 = current_this;
		EG(current_execute_data) = EX(prev_execute_data);

		if (EG(exception)) {
			if (!EG(current_execute_data)) {
				zend_exception_error(EG(exception), E_ERROR TSRMLS_CC);
				zend_error(E_ERROR, "Exception thrown without a stack frame");
			}

			if (zend_throw_exception_hook) {
				zend_throw_exception_hook(NULL TSRMLS_CC);
			}

			CU_ASSERT_TRUE(EG(current_execute_data)->opline == NULL);
		}

		return SUCCESS;
	}

	return FAILURE;
}

void test_exceptions(void)
{
	startup_php(__func__);
	zend_register_functions(NULL, fe, NULL, MODULE_PERSISTENT TSRMLS_CC);
	zend_first_try {
		zval* retval;
		ALLOC_INIT_ZVAL(retval);

//		call_internal_function(retval, ZEND_STRS("test_function"), 0, NULL TSRMLS_CC);
		zend_eval_string("try { test_function(); } catch (Exception $e) { echo 'Caught exception!', PHP_EOL; }", NULL, "" TSRMLS_CC);
		zval_ptr_dtor(&retval);
	}
	zend_catch {
		CU_ASSERT_FALSE(1);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}

void test_unclean_shutdown(void)
{
	size_t initial_memory_usage, final_memory_usage;
	int old_reporting;
	zval* z1;
	zval* z2;
	zval* z3;
	zval* z4;
	zval* z5;

	startup_php(__func__);

	/* We need to measure memory usage before request startup happens
	 * but after PHP initialization. startup_php() initializes the request
	 * and this is why we need to shut the request down
	 */
	php_request_shutdown(NULL);
	memclean_called = 0;

	/* Get memory usage before request startup happens */
	initial_memory_usage = zend_memory_usage(0 TSRMLS_CC);

	/* Perform request startup */
	SG(options) |= SAPI_OPTION_NO_CHDIR;
	SG(request_info).argc = 0;
	SG(request_info).argv = NULL;

	CU_ASSERT_EQUAL(php_request_startup(TSRMLS_C), SUCCESS);

	/**/
	old_reporting = EG(error_reporting);

	zend_first_try {
		PHALCON_MM_GROW();

		PHALCON_INIT_VAR(z1);
		PHALCON_INIT_VAR(z2);
		PHALCON_INIT_VAR(z3);
		PHALCON_INIT_VAR(z4);
		PHALCON_INIT_VAR(z5);

		array_init(z2);
		object_init(z3);
		add_next_index_zval(z2, z1);
		add_next_index_zval(z2, z3);
		add_next_index_zval(z2, z4);
		add_next_index_zval(z2, z5);

		Z_ADDREF_P(z1);
		Z_ADDREF_P(z3);
		Z_ADDREF_P(z4);
		Z_ADDREF_P(z5);

		EG(error_reporting) = 0;
		zend_eval_string("calling_sos();", NULL, "" TSRMLS_CC);
		CU_ASSERT_FALSE(1);

		PHALCON_MM_RESTORE();
	}
	zend_catch {
		CU_ASSERT_TRUE(1);
		EG(error_reporting) = old_reporting;
	}
	zend_end_try();

	CU_ASSERT_EQUAL(_mem_block_check(z1, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
	CU_ASSERT_EQUAL(_mem_block_check(z2, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
	CU_ASSERT_EQUAL(_mem_block_check(z3, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
	CU_ASSERT_EQUAL(_mem_block_check(z4, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);
	CU_ASSERT_EQUAL(_mem_block_check(z5, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC), 1);

	CU_ASSERT_EQUAL(memclean_called, 0);
	CU_ASSERT_EQUAL(PG(report_memleaks), 1);
	CU_ASSERT_EQUAL(CG(unclean_shutdown), 1);

	php_request_shutdown(NULL);
	CU_ASSERT_EQUAL(memclean_called, 1);
	CU_ASSERT_EQUAL(leaks, 0);

	/* Check memory usage after request shutdown */
	final_memory_usage = zend_memory_usage(0 TSRMLS_CC);
	CU_ASSERT_EQUAL(final_memory_usage, initial_memory_usage);
	/**/

	/* Launch an empty request */
	SG(options) |= SAPI_OPTION_NO_CHDIR;
	SG(request_info).argc = 0;
	SG(request_info).argv = NULL;

	CU_ASSERT_EQUAL(php_request_startup(TSRMLS_C), SUCCESS);

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}
