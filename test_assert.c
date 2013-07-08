#include <stdio.h>
#include <CUnit/CUnit.h>
#include <main/php.h>
#include "embed.h"
#include "php_phalcon.h"
#include "kernel/memory.h"
#include "kernel/assert.h"
#include "kernel/debug.h"
#include "test_assert.h"

int setup_assert(void)
{
	phalcon_log = fopen("/dev/null", "w");
	return phalcon_log == NULL;
}

int teardown_assert(void)
{
	if (phalcon_log) {
		fclose(phalcon_log);
		phalcon_log = 0;
	}

	return 0;
}

void test_phalcon_assert_class(void)
{
	startup_php(__func__);
	zend_first_try {
		zval* x;

		PHALCON_MM_GROW();
			PHALCON_INIT_VAR(x);

			CU_ASSERT_EQUAL(phalcon_assert_class(x, "" TSRMLS_CC), FAILURE);

			object_init(x);
			CU_ASSERT_EQUAL(phalcon_assert_class(x, "stdClass" TSRMLS_CC), SUCCESS);
			CU_ASSERT_EQUAL(phalcon_assert_class(x, "object" TSRMLS_CC), FAILURE);
		PHALCON_MM_RESTORE();
	}
	zend_catch {
		CU_ASSERT(0);
	}
	zend_end_try();

	shutdown_php();
	CU_ASSERT_EQUAL(leaks, 0);
}
