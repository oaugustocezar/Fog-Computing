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

/* The remaining added by me: */
//#define DEBUG_WOLFSSL
#define USE_CERT_BUFFERS_2048
#define NO_ASN_TIME
#define SINGLE_THREADED

#define XTIME fnSecondsSinceEpoch
#define XGMTIME
#define ALT_ECC_SIZE

#define WOLFSSL_TLS13
#define HAVE_TLS_EXTENSIONS
#define HAVE_SUPPORTED_CURVES
#define HAVE_ECC
#define HAVE_HKDF
#define WC_RSA_PSS
#define NO_DH 

#define HAVE_AESGCM

#endif /* ARDUINO_USER_SETTINGS_H */