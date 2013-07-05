My thoughts

* sources: see `declare(encoding=...)` and `zend.script_encoding` (last one is specific to UTF-8/UTF-16/UTF-32 and doesn't associate any data to charsets)
* HTTP (GET, POST, etc):
  + input: check if this is a valid UTF-8 and fallback to an ASCII-8bit encoding?
  + output: -
* mysql (mysql, mysqli, mysqlnd, pdo_mysql):
  + input: as is?
  + output: charset can be known on the fly with MYSQL_FIELD structure, member charsetnr (see example below for mysqlnd)
  + Note: a `SET NAMES` (or equivalent) in the middle of fetching operation does not affect new lines. In other words, charset is set one for all at query execution.
* postgresql (pgsql + pdo_pgsql)
  + input: as is? (check if compatible with PQclientEncoding?)
  + output: see PQclientEncoding + pg_encoding_to_char
* sqlite3: I/O in UTF-8 **or** UTF-16
* PCRE:
  + input: indifferent (except modifier u implies UTF-8)
  + output: same as input
* libxml2 related extensions (DOM, SimpleXML, XMLReader, XMLWriter):
  + input (paths excluded, see filesystem): UTF-8 implied
  + output: UTF-8
* intl:
  + input: UTF-8 implied
  + output: all outputs are in UTF-8
* zip: due to an old ZIP format, a MSDOS code page should be used (like CP850 for Western Europe - see GetOEMCP() on windows)
* filesystem (paths only):
  + Unix: guess it from locale or use an INI directive?
  + Windows: current ANSI code page see GetACP() (PHP does not use (yet) Unicode (UTF-16) functions of windows API)
* mbstring: removal of mb_internal_encoding (duplicate/conflict with zend.script_encoding/declare(encoding=...))
* conversions (iconv, mbstring, intl/UConverter, utf8_(en|de)code): we know I/O charsets (redondant in long term)

Functions added:
* `string str_encoding(mixed &$variable)` return current encoding of a string variable
* `bool str_force_encoding(mixed &$variable, string $encoding)` true if the new encoding was successfully associated to the string variable

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

    if (63 == charset) {
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
            if (NULL == (enc = enc_get_by_name(charset->name))) {
#else
            if (NULL == (enc = enc_get_by_name(charset))) {
#endif /* MYSQLI_USE_MYSQLND */
                return null_or_a_default_encoding; // no match, return default encoding
            } else {
                // rewrite latin1 into CP1252 instead of ISO-8859-1 (MySQL feature)
                if (0 == strcasecmp(enc->name, "latin1")) {
                    return CP1252_ENCODING;
                }
            }
            return enc;
        }
    }
}
```
