/* Generated wolfSSL user_settings.h file for Arduino */
#ifndef ARDUINO_USER_SETTINGS_H
#define ARDUINO_USER_SETTINGS_H

/* Platform */
#define WOLFSSL_ARDUINO

/* Math library (remove this to use normal math)*/
#define USE_FAST_MATH
#define TFM_NO_ASM

/* RNG DEFAULT !!FOR TESTING ONLY!! */
/* comment out the error below to get started w/ bad entropy source
 * This will need fixed before distribution but is OK to test with */
/* #error "needs solved, see: https://www.wolfssl.com/docs/porting-guide/" */
#define WOLFSSL_GENSEED_FORTEST
#define TFM_TIMING_RESISTANT
#define WOLFSSL_SMALL_STACK
#define WOLFSSL_SHA3_SMALL
//#define WOLFSSL_SMALL_CERT_VERIFY
/* The remaining added by me: */
#define USE_CERT_BUFFERS_2048 
#define NO_ASN_TIME
#define SINGLE_THREADED 
#define NO_RSA
#define WOLFSSL_SHA384
#define RSA_LOW_MEM
#define ECC_USER_CURVES
#define HAVE_SNI
#define FP_MAX_BITS 512
#define XTIME fnSecondsSinceEpoch
#define XGMTIME
#define ALT_ECC_SIZE
#define HAVE_CHACHA
#define HAVE_POLY1305
#define HAVE_ONE_TIME_AUTH
#define GCM_SMALL
#define CURVED25519_SMALL
#define ED25519_SMALL
#define USE_SLOW_SHA
#define USE_SLOW_SHA256
#define USE_SLOW_SHA512
#define CURVE25519_SMALL
#define WOLFSSL_TLS13
#define HAVE_TLS_EXTENSIONS
#define HAVE_SUPPORTED_CURVES
#define HAVE_ECC
#define HAVE_HKDF
#define WC_RSA_PSS
#define NO_DH 
#define HAVE_AESGCM

#endif /* ARDUINO_USER_SETTINGS_H */