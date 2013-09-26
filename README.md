My thoughts

* PHP files: see `declare(encoding=...)` and `zend.script_encoding` (last one is specific to UTF-8/UTF-16/UTF-32 and doesn't associate any data to charsets)
* for CLI get stdin charset (Unix: use locale)
* HTTP (GET, POST, etc):
  + input: check if this is a valid UTF-8 and fallback to an ASCII-8bit encoding?
  + output: -
* mysql (mysql, mysqli, mysqlnd, pdo_mysql):
  + input: as is?
  + output: charset can be known on the fly with MYSQL_FIELD structure, member charsetnr (see example below for mysqlnd)
  + Note: a `SET NAMES` (or equivalent) in the middle of fetching operation does not affect new lines. In other words, charset is set one for all at query execution.
* postgresql (pgsql + pdo_pgsql)
  + input: as is? (check if compatible with PQclientEncoding?)
  + output: see member client_encoding of structure PGResult (but it is "private") + pg_encoding_to_char
  + Note: a `SET NAMES` (or equivalent) in the middle of fetching operation does not affect new lines. In other words, charset is set one for all at query execution.
* sqlite3: I/O in UTF-8 (even if SQLite >= 3 also permits UTF-16)
* PCRE:
  + input: indifferent (except modifier u implies UTF-8)
  + output: same as input
* libxml2 related extensions (DOM, SimpleXML, XMLReader, XMLWriter):
  + input (paths excluded, see filesystem): UTF-8 implied
  + output: UTF-8
* intl, json:
  + input: UTF-8 implied
  + output: all outputs are in UTF-8
* zip: due to an old ZIP format, a MSDOS code page should be used (like CP850 for Western Europe - see GetOEMCP() on windows)
* filesystem (paths only):
  + Unix: guess it from locale or use an INI directive?
  + Windows: current ANSI code page see GetACP() (PHP does not use (yet) Unicode (UTF-16) functions of windows API)
* mbstring: removal of mb_internal_encoding (duplicate/conflict with zend.script_encoding/declare(encoding=...))
* iconv: same as mbstring with iconv.\*\_encoding
* conversions (iconv, mbstring, intl/UConverter, utf8_(en|de)code): we know I/O charsets (redondant in long term)

# Introduction

A good compromise before an hypothetic use of Unicode (UTF-8 like UTF-16 - previous attempt of PHP 6) as internal charset for PHP can be to:
1. identify the charset of any string
2. associate a charset to any string

It permits us to:
* in long term, do charset conversion
* immediately, make charset checks (eg: utf8_encode of an UTF-8 string will throw a warning)
* throw a notice when using a non multibyte function (strlen with an UTF-8 string)
* allow `[]` operator to access to a code point instead of a byte
* ...

# Implementation

* For this, a change to zval is required (just add a pointer - or int ? - to zvalue_value.str)
* Have a way to get script encoding (through `declare(encoding=...)` ?)
* Have a kind of virtual hierarchy of charsets, something like this:

```
|- binary (for blob, varbinary from databases, raw md5, content of binary file, etc)
|- unassociated (undefined/unknown/default charset)
|- ascii
   |- iso-8859-1
   |- cp1252
   |- utf-8
|- utf-16 (le/be)
|- utf-32 (le/be)
```

# Backward Incompatible Changes

None (as far I know) but it will be great to remove iconv.\*\_encoding and mb_internal_encoding in long term (first depreciate them ? - emit a E_DEPRECATED and do nothing)

# Internal API changes

Rewrite all [ZVAL|RETVAL]\_\* macros. In order to not break anything, introduce new macros with a "ENC" suffix which expect a pointer to a charset. "Old" [ZVAL|RETVAL]\_\* macros are redirected to their ENC equivalent with the default encoding. Eg:

```
#define ZVAL_STRING_ENC(z, s, enc, duplicate) /* not shown */
#define ZVAL_STRING(z, s, duplicate) ZVAL_STRING_ENC(z, s, ENC_UNASSOCIATED, duplicate)
```

Add modifiers to zend_parse_arg_impl:
* 's', a string without an associated encoding (or we don't care about its encoding): `char **ptr, int *ptr_len`, is kept as is for compatibility (at least temporarily)
* 'e', a string encoded or compatible with a given encoding: ` char **ptr, int *ptr_len, EncodingPtr enc`
* 'u', for convenience, an utf-8 string: `char **ptr, int *ptr_len`
* 'E', same as 's' but we want to get its encoding: `char **ptr, int *ptr_len, EncodingPtr *enc`

Add convenient function for array insertion (add_[assoc|index]_stringl?_enc)

TODO: need to keep charset associated to identifiers but hashtables for symbols use a char * as key, not a zval *.

# Extension specific

## pdo

new callbacks needed

## pgsql

```C
/* an ugly hach for a missing function:
int PQencoding(PGresult *res)
{
       return ((pg_result *) res)->client_encoding;
}
*/
static int PQencoding(PGresult *res)
{
#if PG_VERSION_NUM >= 80300
# define CMDSTATUS_LEN 64
#else
# define CMDSTATUS_LEN 40
#endif /* Postgre >= 8.3.0 */

       typedef struct {
               /*PQnoticeReceiver*/ void *noticeRec;
               void *noticeRecArg;
               /*PQnoticeProcessor*/ void *noticeProc;
               void *noticeProcArg;
       } PGNoticeHooks;

       struct pg_result
       {
               int ntups;
               int numAttributes;
               /*PGresAttDesc*/ void *attDescs;
               /*PGresAttValue*/ void **tuples;
               int tupArrSize;
#if PG_VERSION_NUM >= 80200
               int numParameters;
               /*PGresParamDesc*/ void *paramDescs;
#endif /* Postgre >= 8.2.0 */
               ExecStatusType resultStatus;
               char cmdStatus[CMDSTATUS_LEN];
               int binary;
               PGNoticeHooks noticeHooks;
#if PG_VERSION_NUM >= 80400
               /*PGEvent*/ void *events;
               int nEvents;
#endif /* Postgre >= 8.4.0 */
               int client_encoding;
               char *errMsg;
               /*PGMessageField*/ void *errFields;
               char null_field[1];
               /*PGresult_data*/ void *curBlock;
               int curOffset;
               int spaceLeft;
       };

       return ((struct pg_result *) res)->client_encoding;
}
```

## mysql(i)

```C
/* SELECT CHARACTER_SET_NAME, GROUP_CONCAT(id) from INFORMATION_SCHEMA.COLLATIONS GROUP BY CHARACTER_SET_NAME; */

PHPAPI int mysql_is_string_type(
#ifdef MYSQLI_USE_MYSQLND
    enum_mysqlnd_field_types
#else
    enum_field_types
#endif
type) {
    switch (type) {
        /* charset dependant */
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_VAR_STRING:
        case MYSQL_TYPE_STRING:
        /* binary */
        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
        case MYSQL_TYPE_BLOB:
            return 1;
        default:
            return 0;
    }
}

PHPAPI EncodingPtr mysql_enc_to_php(unsigned int charsetnr)
{
    EncodingPtr enc = null_or_a_default_encoding;

    if (63 == charsetnr) {
        return BINARY_ENCODING; // binary (for a field of type *BLOB or (VAR)BINARY)
    } else {
#ifdef MYSQLI_USE_MYSQLND
        const MYSQLND_CHARSET *charset;
#else
        const char *charset;
#endif /* MYSQLI_USE_MYSQLND */

#ifdef MYSQLI_USE_MYSQLND
        /* mysqlnd/mysqlnd.h: const MYSQLND_CHARSET * mysqlnd_find_charset_nr(unsigned int charsetnr); */
        if (NULL == (charset = mysqlnd_find_charset_nr(charsetnr))) {
#else
        /* mysql/my_sys.h: const char *get_charset_name(uint cs_number); */
        if (NULL == (charset = get_charset_name(charsetnr))) {
#endif /* MYSQLI_USE_MYSQLND */
            return enc; // undetermined, return default encoding
        } else {
            // do our own mapping from name
#ifdef MYSQLI_USE_MYSQLND
            if (NULL == (enc = enc_by_name(charset->name))) {
#else
            if (NULL == (enc = enc_by_name(charset))) {
#endif /* MYSQLI_USE_MYSQLND */
                return null_or_a_default_encoding; // no match, return default encoding
            } else {
                // rewrite latin1 into CP1252 instead of ISO-8859-1 (MySQL feature)
                //if (0 == zend_binary_strcasecmp(enc->name, strlen(enc->name), "latin1", sizeof("latin1") - 1)) {
                if (0 == strcasecmp(enc->name, "latin1")) {
                    return CP1252_ENCODING;
                }
            }
            return enc;
        }
    }
}
```

## Filesystem

* Windows: GetACP
* Unix: locale based and/or by an ini setting ?
See: Zend/encoding_fs_overrides.h

## intl, json, sqlite3, xmlreader, xmlwriter, simplexml, dom

I/O imply UTF-8 (except for paths): almost nothing to do

## pcre

Take back haystack charset

Note: modifier u would become obsolete

## iconv, mbstring, intl/UConverter

I/O charsets are known because user gives them to us.

Note: from_encoding parameter would become obsolete

## GET/POST/COOKIE

Check if ASCII or UTF-8?

## SESSION + (un)serialize

* Keep format s:\<string length\>:\<string content\> as is
* Add format e:\<charset name length\>:\<charset name\>:\<string length\>:\<string content\>
?

## zip

Use the bit 11 of general purpose bit flag?

See: [.ZIP File Format Specification](http://www.pkware.com/documents/casestudies/APPNOTE.TXT) (APPENDIX D in particular)

# New functions

* `string str_encoding(string $str)` return current encoding of given string
* `bool str_force_encoding(mixed &$variable, string $encoding)` true if the new encoding was successfully associated to the string variable

# Examples

```PHP
<?php
declare(encoding='UTF-8');

$ascii = 'a';
var_dump(
        str_encoding(utf8_decode('é')), # => string(10) "ISO-8859-1"
        str_force_encoding($ascii, 'ASCII'), # ok
        $ascii . 'é', # ok
        'é' . utf8_decode('é'), # => Warning: concatenation of incompatible charsets found: 'UTF-8' and 'ISO-8859-1'
        trim(" é "), # => Warning: trim() is not compatible with multibyte strings (string with charset 'UTF-8' found)
        NULL
);
```
