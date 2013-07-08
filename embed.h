#ifndef EMBED_H_
#define EMBED_H_

#include <main/php.h>

#ifdef ZTS
extern void*** tsrm_ls;
#endif

extern zend_uint leaks;
extern void zend_message_dispatcher(long message, void* data TSRMLS_DC);

void startup_php(const char* func);
void shutdown_php(void);

#endif /* EMBED_H_ */
