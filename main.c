#include <stdlib.h>
#include <CUnit/Basic.h>
#include "test_array.h"
#include "test_assert.h"
#include "test_memory.h"
#include "test_variables.h"
#include "testbed.h"

int main(int argc, char** argv)
{
	CU_initialize_registry();

	CU_pSuite suite_memory = CU_add_suite("Memory", NULL, NULL);
	CU_ADD_TEST(suite_memory, test_phalcon_alloc_zval);
	CU_ADD_TEST(suite_memory, test_phalcon_init_var);
	CU_ADD_TEST(suite_memory, test_phalcon_init_nvar);
	CU_ADD_TEST(suite_memory, test_phalcon_init_nvar_pnull);
	CU_ADD_TEST(suite_memory, test_phalcon_cpy_wrt);
	CU_ADD_TEST(suite_memory, test_phalcon_cpy_wrt_ctor);
	CU_ADD_TEST(suite_memory, test_phalcon_obs_var);
	CU_ADD_TEST(suite_memory, test_phalcon_obs_nvar);
	CU_ADD_TEST(suite_memory, test_phalcon_separate);
	CU_ADD_TEST(suite_memory, test_phalcon_separate_param);

	CU_pSuite suite_assert = CU_add_suite("Assert", setup_assert, teardown_assert);
	CU_ADD_TEST(suite_assert, test_phalcon_assert_class);

	CU_pSuite suite_array = CU_add_suite("Array", NULL, NULL);
	CU_ADD_TEST(suite_array, test_phalcon_array_unshift);
	CU_ADD_TEST(suite_array, test_phalcon_array_values);
	CU_ADD_TEST(suite_array, test_phalcon_array_key_exists);

	CU_pSuite suite_variables = CU_add_suite("Variables", NULL, NULL);
	CU_ADD_TEST(suite_variables, test_phalcon_serialize);
	CU_ADD_TEST(suite_variables, test_phalcon_unserialize);

	CU_pSuite suite_testbed = CU_add_suite("Testbed", NULL, NULL);
	CU_ADD_TEST(suite_testbed, test_increment);
	CU_ADD_TEST(suite_testbed, test_phalcon_mvc_router_route__construct);
	CU_ADD_TEST(suite_testbed, test_array_mess);
	CU_ADD_TEST(suite_testbed, test_exceptions);
	CU_ADD_TEST(suite_testbed, test_unclean_shutdown);

	initialize_embed_wrapper();

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();

	CU_cleanup_registry();
	return CU_get_number_of_tests_failed() ? EXIT_FAILURE : EXIT_SUCCESS;
}
