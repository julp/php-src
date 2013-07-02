#include "zend.h"
#include "zend_API.h"
#include "zend_encodings.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#define STR_LEN(str)      (ARRAY_SIZE(str) - 1)
#define STR_SIZE(str)     (ARRAY_SIZE(str))

static const Encoding __enc_unassociated = {
    "unassociated",
    ASCII_NON_COMPATIBLE,
    {
        NULL
    }
};

static const Encoding __enc_ascii = {
    "US-ASCII",
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
    ASCII_COMPATIBLE,
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
    ASCII_COMPATIBLE,
    {
        "ibm-5348_P100-1997",
        "ibm-5348",
        "windows-1252",
        NULL
    }
};

static const Encoding __enc_utf8 = {
    "UTF-8",
    ASCII_COMPATIBLE,
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
        NULL
    }
};

EncodingPtr enc_unassociated = &__enc_unassociated;
EncodingPtr enc_ascii = &__enc_ascii;
EncodingPtr enc_iso_8859_1 = &__enc_iso_8859_1;
EncodingPtr enc_utf8 = &__enc_utf8;

static EncodingPtr known_encodings[] = {
    &__enc_ascii,
    &__enc_iso_8859_1,
    &__enc_cp1252,
    &__enc_utf8
};

EncodingPtr enc_by_name(const char *name_or_alias)
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
