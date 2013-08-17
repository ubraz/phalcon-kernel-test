#ifndef EMBED_H_
#define EMBED_H_

#include <main/php.h>

#ifdef ZTS
extern void*** tsrm_ls;
#endif

extern zend_uint leaks;
extern int memclean_called;
#if PHP_VERSION_ID < 50400
ZEND_API void zend_message_dispatcher(long message, void* data TSRMLS_DC);
#else
ZEND_API void zend_message_dispatcher(long message, const void* data TSRMLS_DC);
#endif

void initialize_embed_wrapper(void);
void startup_php(const char* func);
void shutdown_php(void);

#endif /* EMBED_H_ */
