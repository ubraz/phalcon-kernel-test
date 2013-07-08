#include <sapi/embed/php_embed.h>
#include "embed.h"
#include "extension.h"

#ifdef ZTS
void*** tsrm_ls;
#endif

zend_uint leaks;

void zend_message_dispatcher(long message, void* data TSRMLS_DC)
{
	switch (message) {
		case ZMSG_MEMORY_LEAK_DETECTED: {
			zend_leak_info* leak = (zend_leak_info*)data;
			fprintf(stderr, "%p %zu [%s:%u] [%s:%u]\n", leak->addr, leak->size, leak->filename, leak->lineno, leak->orig_filename, leak->orig_lineno);
			break;
		}

		case ZMSG_MEMORY_LEAK_REPEATED: {
			zend_uintptr_t repeated = (zend_uintptr_t)data;
			fprintf(stderr, "Last leak repeated %ld times\n", repeated);
			break;
		}

		case ZMSG_MEMORY_LEAKS_GRAND_TOTAL: {
			zend_uint total = *(zend_uint*)data;
			leaks = total;
			fprintf(stderr, "%u leaks detected\n", total);
			break;
		}
	}
}

void startup_php(const char* func)
{
	const char* argv[2] = { func, NULL };
	php_embed_init(1, (char**)argv PTSRMLS_CC);

	zend_startup_module(&phalcon_module_entry);
	leaks = 0;
}

void shutdown_php(void)
{
	php_embed_shutdown(TSRMLS_C);
}
