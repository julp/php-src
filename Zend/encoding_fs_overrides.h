#undef ZVAL_STRING
#undef ZVAL_STRINGL
#undef RETVAL_STRING
#undef RETVAL_STRINGL
#undef RETURN_STRING
#undef RETURN_STRINGL

#define ZVAL_STRING(z, s, duplicate) \
    ZVAL_STRING_ENC(z, s, enc_for_filesystem(), duplicate)

#define ZVAL_STRINGL(z, s, l, duplicate) \
    ZVAL_STRINGL_ENC(z, s, l, enc_for_filesystem(), duplicate)

#define RETVAL_STRING(s, duplicate) \
    RETVAL_STRING_ENC(s, enc_for_filesystem(), duplicate)

#define RETVAL_STRINGL(s, l, duplicate) \
    RETVAL_STRINGL_ENC(s, l, enc_for_filesystem(), duplicate)

#define RETURN_STRING(s, duplicate) \
    do { \
        RETVAL_STRING_ENC(s, enc_for_filesystem(), duplicate); \
        return; \
    } while (0);

#define RETURN_STRINGL(s, l, duplicate) \
    do { \
        RETVAL_STRINGL_ENC(s, l, enc_for_filesystem(), duplicate); \
        return; \
    } while (0);
