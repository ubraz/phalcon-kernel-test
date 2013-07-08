#include <main/php.h>
#include "php_phalcon.h"
#include "phalcon_exception.h"
#include "kernel/main.h"

zend_class_entry* phalcon_exception_ce;

PHALCON_INIT_CLASS(Phalcon_Exception)
{
	PHALCON_REGISTER_CLASS_EX(Phalcon, Exception, exception, "exception", NULL, 0);
	return SUCCESS;
}
