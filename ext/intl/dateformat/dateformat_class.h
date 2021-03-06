/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Kirti Velankar <kirtig@yahoo-inc.com>                       |
   +----------------------------------------------------------------------+
*/
#ifndef DATE_FORMAT_CLASS_H
#define DATE_FORMAT_CLASS_H

#include <php.h>

#include "intl_common.h"
#include "intl_error.h"
#include "intl_data.h"
#include "dateformat_data.h"

typedef struct {
	zend_object		zo;
	dateformat_data	datef_data;
	int				date_type;
	int				time_type;
	int				calendar;
	char			*requested_locale;
} IntlDateFormatter_object;

void dateformat_register_IntlDateFormatter_class( TSRMLS_D );
extern zend_class_entry *IntlDateFormatter_ce_ptr;

/* Auxiliary macros */

#define DATE_FORMAT_METHOD_INIT_VARS	INTL_METHOD_INIT_VARS(IntlDateFormatter, dfo)
#define DATE_FORMAT_METHOD_FETCH_OBJECT_NO_CHECK	INTL_METHOD_FETCH_OBJECT(IntlDateFormatter, dfo)
#define DATE_FORMAT_METHOD_FETCH_OBJECT				\
		DATE_FORMAT_METHOD_FETCH_OBJECT_NO_CHECK;	\
	if (dfo->datef_data.udatf == NULL)				\
	{												\
		intl_errors_set(&dfo->datef_data.error, U_ILLEGAL_ARGUMENT_ERROR, "Found unconstructed IntlDateFormatter", 0, TSRMLS_C); \
		RETURN_FALSE;								\
	}

#define DATE_FORMAT_OBJECT(dfo)		(dfo)->datef_data.udatf

#endif // #ifndef DATE_FORMAT_CLASS_H
