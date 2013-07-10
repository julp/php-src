#include "zend.h"
#include "zend_API.h"
#include "zend_encodings.h"

#ifdef PHP_WIN32
# include <win32/php_stdint.h>
#else
# include <inttypes.h>
#endif /* PHP_WIN32 */

#ifndef TRUE
# define TRUE 1
#endif /* !TRUE */
#ifndef FALSE
# define FALSE 0
#endif /* !FALSE */

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#define STR_LEN(str)      (ARRAY_SIZE(str) - 1)
#define STR_SIZE(str)     (ARRAY_SIZE(str))

typedef enum {
    ASCII_NON_COMPATIBLE,
    ASCII_REAL,
    ASCII_SUPERSET
} AsciiCompatibility;

#define WINDOWS_CODE_PAGE_NONE 0

struct _Encoding {
    const char *name;
    uint16_t windows_code_page; /* http://msdn.microsoft.com/en-us/library/windows/desktop/dd317756.aspx */
    AsciiCompatibility compat;
    const char *aliases[];
};

static const Encoding __enc_unassociated = {
    "unassociated",
    WINDOWS_CODE_PAGE_NONE,
    ASCII_NON_COMPATIBLE,
    {
        NULL
    }
};

static const Encoding __enc_binary = {
    "binary",
    WINDOWS_CODE_PAGE_NONE,
    ASCII_NON_COMPATIBLE,
    {
        NULL
    }
};

static const Encoding __enc_ascii = {
    "US-ASCII",
    20127,
    ASCII_REAL,
    {
        "ASCII",
        "ANSI_X3.4-1968",
        "ANSI_X3.4-1986",
        "ISO_646.irv:1991",
        "iso_646.irv:1983",
        "ISO646-US",
        "us",
        "csASCII",
        "iso-ir-6",
        "cp367",
        "ascii7",
        "646",
        "windows-20127",
        "ibm-367",
        "IBM367",
        NULL
    }
};

static const Encoding __enc_iso_8859_1 = {
    "ISO-8859-1",
    28591,
    ASCII_SUPERSET,
    {
        "ibm-819",
        "IBM819",
        "cp819",
        "latin1",
        "8859_1",
        "csISOLatin1",
        "iso-ir-100",
        "ISO_8859-1:1987",
        "l1",
        "819",
        NULL
    }
};

static const Encoding __enc_cp1252 = {
    "cp1252",
    1252,
    ASCII_SUPERSET,
    {
        "ibm-5348_P100-1997",
        "ibm-5348",
        "windows-1252",
        NULL
    }
};

static const Encoding __enc_utf8 = {
    "UTF-8",
    65001,
    ASCII_SUPERSET,
    {
        "ibm-1208",
        "ibm-1209",
        "ibm-5304",
        "ibm-5305",
        "ibm-13496",
        "ibm-13497",
        "ibm-17592",
        "ibm-17593",
        "windows-65001",
        "cp1208",
        "x-UTF_8J",
        "unicode-1-1-utf-8",
        "unicode-2-0-utf-8",
        /* MySQL */
        "utf8",
        "utf8mb4",
        NULL
    }
};

EncodingPtr enc_unassociated = &__enc_unassociated;
EncodingPtr enc_ascii = &__enc_ascii;
EncodingPtr enc_iso_8859_1 = &__enc_iso_8859_1;
EncodingPtr enc_utf8 = &__enc_utf8;
EncodingPtr enc_binary = &__enc_binary;

static EncodingPtr known_encodings[] = {
    &__enc_ascii,
    &__enc_iso_8859_1,
    &__enc_cp1252,
    &__enc_utf8
};

ZEND_API EncodingPtr enc_by_name(const char *name_or_alias)
{
    size_t i;

    for (i = 0; i < ARRAY_SIZE(known_encodings); i++) {
        const char **alias;

        if (0 == strcasecmp(known_encodings[i]->name, name_or_alias)) {
            return known_encodings[i];
        }
        for (alias = known_encodings[i]->aliases; NULL != *alias; alias++) {
            if (0 == strcasecmp(*alias, name_or_alias)) {
                return known_encodings[i];
            }
        }
    }

    return NULL;
}

ZEND_API const char *enc_name(EncodingPtr enc)
{
    return enc->name;
}

/*
AreFileApisANSI() ? GetACP() : GetOEMCP()
*/
ZEND_API EncodingPtr enc_for_filesystem()
{
#ifdef WIN32
    size_t i;

    for (i = 0; i < ARRAY_SIZE(known_encodings); i++) {
        if (GetACP() == known_encodings[i]->windows_code_page) {
            return known_encodings[i];
        }
    }
#endif /* WIN32 */
    return enc_unassociated;
}

ZEND_API int enc_are_incompatible(EncodingPtr reference, EncodingPtr tested)
{
    if (enc_unassociated == tested) {
        /* For now, if charset is undetermined, ignore them: let's they are compatible */
        return FALSE;
    } else {
        return reference != tested && reference->compat >= tested->compat;
    }
}
