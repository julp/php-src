#ifndef ZEND_ENCODINGS_H

# define ZEND_ENCODINGS_H

typedef struct _Encoding Encoding;
typedef const Encoding * EncodingPtr;

extern EncodingPtr enc_unassociated;
extern EncodingPtr enc_ascii;
extern EncodingPtr enc_iso_8859_1;
extern EncodingPtr enc_utf8;
extern EncodingPtr enc_binary;

# define Z_STRENC(zval)       (zval).value.str.enc
# define Z_STRENC_P(zval_p)   Z_STRENC(*zval_p)
# define Z_STRENC_PP(zval_pp) Z_STRENC(**zval_pp)

/*
# define ZVAL_STRING_ENC(z, s, enc, duplicate) \
    do { \
        const char *__s = (s); \
        zval *__z = (z); \
        Z_STRLEN_P(__z) = strlen(__s); \
        Z_STRVAL_P(__z) = (duplicate ? estrndup(__s, Z_STRLEN_P(__z)) : (char*) __s); \
        Z_TYPE_P(__z) = IS_STRING; \
        Z_STRENC_P(__z) = enc; \
    } while (0)

# define ZVAL_STRINGL_ENC(z, s, l, enc, duplicate) \
    do { \
        const char *__s = (s); \
        int __l = l; \
        zval *__z = (z); \
        Z_STRLEN_P(__z) = __l; \
        Z_STRVAL_P(__z) = (duplicate ? estrndup(__s, __l) : (char*) __s); \
        Z_TYPE_P(__z) = IS_STRING; \
        Z_STRENC_P(__z) = enc; \
    } while (0)
*/

# define ZVAL_STRING_UTF8(z, s, duplicate) \
    ZVAL_STRING_ENC(z, s, enc_utf8, duplicate)

# define ZVAL_STRINGL_UTF8(z, s, l, duplicate) \
    ZVAL_STRINGL_ENC(z, s, l, enc_utf8, duplicate)
    
# define RETVAL_STRING_ENC(s, enc, duplicate) \
    ZVAL_STRING_ENC(return_value, s, enc, duplicate)

# define RETVAL_STRING_UTF8(s, duplicate) \
    ZVAL_STRING_UTF8(return_value, s, duplicate)

# define RETVAL_STRINGL_ENC(s, l, enc, duplicate) \
    ZVAL_STRINGL_ENC(return_value, s, l, enc, duplicate)

# define RETVAL_STRINGL_UTF8(s, l, duplicate) \
    ZVAL_STRINGL_UTF8(return_value, s, l, duplicate)

#define RETURN_STRING_ENC(s, enc, duplicate) \
    do { \
        RETVAL_STRING_ENC(s, enc, duplicate); \
        return; \
    } while (0);

#define RETURN_STRING_UTF8(s, duplicate) \
    do { \
        RETVAL_STRING_UTF8(s, duplicate); \
        return; \
    } while (0);

#define RETURN_STRINGL_ENC(s, l, enc, duplicate) \
    do { \
        RETVAL_STRINGL_ENC(s, l, enc, duplicate); \
        return; \
    } while (0);

#define RETURN_STRINGL_UTF8(s, l, duplicate) \
    do { \
        RETVAL_STRINGL_UTF8(s, l, duplicate); \
        return; \
    } while (0);

ZEND_API EncodingPtr enc_by_name(const char *);
ZEND_API int enc_are_incompatible(EncodingPtr, EncodingPtr);
ZEND_API EncodingPtr enc_for_filesystem(void);
ZEND_API const char *enc_name(EncodingPtr);
ZEND_API EncodingPtr enc_guess(const char *, int);
ZEND_API int8_t enc_max_bytes_per_cp(EncodingPtr);

#endif /* ZEND_ENCODINGS_H */
