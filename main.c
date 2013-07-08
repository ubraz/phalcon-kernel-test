#include <stdlib.h>
#include <CUnit/Basic.h>
#include "test_memory.h"

int main(int argc, char** argv)
{
	CU_initialize_registry();
	CU_pSuite suite_memory = CU_add_suite("Memory", NULL, NULL);
	CU_ADD_TEST(suite_memory, test_phalcon_alloc_zval);
	CU_ADD_TEST(suite_memory, test_phalcon_alloc_zval_mm);
	CU_ADD_TEST(suite_memory, test_phalcon_init_var);
	CU_ADD_TEST(suite_memory, test_phalcon_init_nvar);
	CU_ADD_TEST(suite_memory, test_phalcon_init_nvar_pnull);
	CU_ADD_TEST(suite_memory, test_phalcon_cpy_wrt);
	CU_ADD_TEST(suite_memory, test_phalcon_cpy_wrt_ctor);
	CU_ADD_TEST(suite_memory, test_phalcon_obs_var);
	CU_ADD_TEST(suite_memory, test_phalcon_obs_nvar);
	CU_ADD_TEST(suite_memory, test_phalcon_separate);

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();

	CU_cleanup_registry();
	return CU_get_number_of_tests_failed() ? EXIT_FAILURE : EXIT_SUCCESS;
}
