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
    ASCII_REAL,
    ASCII_SUPERSET,
    ASCII_NON_COMPATIBLE
} AsciiCompatibility;

/*
#define ASCII_REAL           0x00
#define ASCII_SUPERSET       0x01
#define ASCII_NON_COMPATIBLE 0x02
typedef uint8_t AsciiCompatibility;
*/

#define WINDOWS_CODE_PAGE_NONE 0

struct _Encoding {
    const char *name;
    int8_t max_bytes_per_cp;
    uint16_t windows_code_page; /* http://msdn.microsoft.com/en-us/library/windows/desktop/dd317756.aspx */
    AsciiCompatibility compat;
    const char *aliases[];
};

static const Encoding __enc_unassociated = {
    "unassociated",
    1,
    WINDOWS_CODE_PAGE_NONE,
    ASCII_NON_COMPATIBLE,
    {
        NULL
    }
};

static const Encoding __enc_binary = {
    "binary",
    1,
    WINDOWS_CODE_PAGE_NONE,
    ASCII_NON_COMPATIBLE,
    {
        NULL
    }
};

static const Encoding __enc_ascii = {
    "US-ASCII",
    1,
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
    1,
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
    1,
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
    4,
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

ZEND_API EncodingPtr enc_unassociated = &__enc_unassociated;
ZEND_API EncodingPtr enc_ascii = &__enc_ascii;
ZEND_API EncodingPtr enc_iso_8859_1 = &__enc_iso_8859_1;
ZEND_API EncodingPtr enc_utf8 = &__enc_utf8;
ZEND_API EncodingPtr enc_binary = &__enc_binary;

static EncodingPtr known_encodings[] = {
    &__enc_ascii,
    &__enc_iso_8859_1,
    &__enc_cp1252,
    &__enc_utf8
};

#define UTF8_3_4_IS_ILLEGAL(b) (((b) < 0x80) || ((b) > 0xBF))

#undef I
#undef R

#define I -1 /* Illegal */
#define R -2 /* Out of range */

static const int8_t utf8_count_bytes[256] = {
    /*      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
    /* 0 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 1 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 2 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 3 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 4 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 5 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 6 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 7 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 8 */ I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
    /* 9 */ I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
    /* A */ I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
    /* B */ I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
    /* C */ I, I, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* D */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* E */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    /* F */ 4, 4, 4, 4, R, R, R, R, R, R, R, R, R, R, R, R
};

#undef I
#undef R

static const uint8_t utf8_min_second_byte_value[256] = {
    /*      0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    A,    B,    C,    D,    E,    F */
    /* 0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 1 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 2 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 3 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 4 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 5 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 6 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 7 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 9 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* A */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* B */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* C */ 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    /* D */ 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    /* E */ 0xA0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    /* F */ 0x90, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t utf8_max_second_byte_value[256] = {
    /*      0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    A,    B,    C,    D,    E,    F */
    /* 0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 1 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 2 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 3 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 4 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 5 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 6 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 7 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 9 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* A */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* B */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* C */ 0x00, 0x00, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF,
    /* D */ 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF,
    /* E */ 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0x9F, 0xBF, 0xBF,
    /* F */ 0xBF, 0xBF, 0xBF, 0xBF, 0x8F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

ZEND_API EncodingPtr enc_guess(const char *string, int string_len)
{
    int i, ascii;
    int8_t cplen;
    uint8_t *end, *p;

    ascii = 1;
    p = (uint8_t *) string;
    end = ((uint8_t *) string) + string_len;
    while (end > p) {
        cplen = utf8_count_bytes[*p];
        ascii &= (cplen == 1);
        if (cplen < 1/* || cplen > U8_MAX_LENGTH*/) {
            return enc_unassociated;
        }
        if (end - p < cplen) {
            return enc_unassociated; // truncated UTF-8 string
        }
        if (1 == cplen) {
            p++;
        } else {
            if (p[1] < utf8_min_second_byte_value[*p] || p[1] > utf8_max_second_byte_value[*p]) {
                return enc_unassociated;
            }
            for (i = 2; i < cplen; i++) {
                if (UTF8_3_4_IS_ILLEGAL(p[i])) {
                    return enc_unassociated;
                }
            }
            p += cplen;
        }
    }

    return ascii ? enc_ascii : enc_utf8;
}

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

ZEND_API int8_t enc_max_bytes_per_cp(EncodingPtr enc)
{
    return enc->max_bytes_per_cp;
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
    /**
     * TODO: to avoid lookup at each call, set it as a global variable (a MINIT callback equivalent ?)
     **/
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
        return reference != tested && ((!((reference->compat & ASCII_SUPERSET) ^ (tested->compat & ASCII_SUPERSET))) || ((reference->compat & ASCII_NON_COMPATIBLE) | (tested->compat & ASCII_NON_COMPATIBLE)));
    }
}
