#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include<TinyGPS.h>
#include <wolfssl.h>
#include <user_settings.h>
#include <wolfssl/ssl.h>
#include <wolfssl/certs_test.h>
#include <string.h>
#define PORT 443
#define HOST "www.backend-fogcomp.online"
#define SSID "HausOfGaga_2G"
#define PASS "BornThisWay"
SoftwareSerial mySerial(13, 15); // RX = D7, TX  = D8
TinyGPS gps;
WiFiClient client;
WOLFSSL_METHOD* method;
WOLFSSL_CTX* ctx = NULL;
WOLFSSL* ssl = NULL;
int WiFiSend(WOLFSSL* ssl_cli, char* msg, int sz, void* ctx_cli);
int WiFiReceive(WOLFSSL* ssl_cli, char* reply, int sz, void* ctx_cli);



void setup() {
  char errBuf[80];
  int err            = 0;
  WiFi.begin(SSID, PASS);
  Serial.begin(9600);
  mySerial.begin(9600);

  Serial.printf("Connecting to %s ", SSID);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  if (client.connect(HOST, PORT)) {
    Serial.println("Server connected");
    wolfSSL_Init();
    method = wolfTLSv1_3_client_method(); /* use TLS v1.3 */
    if ((ctx = wolfSSL_CTX_new(method)) == NULL) {
      Serial.println("wolfSSL_CTX_new error");
    }
    err = wolfSSL_CTX_load_verify_buffer(ctx, (const byte*)cert_fog_der_2048, sizeof_cert_fog_der_2048, SSL_FILETYPE_ASN1);
    if (err != SSL_SUCCESS) {
      Serial.println("Error loading certs");
      Serial.println(err);
    }

    err = wolfSSL_CTX_UseSNI(ctx, WOLFSSL_SNI_HOST_NAME, HOST, XSTRLEN(HOST));
    if (err != WOLFSSL_SUCCESS) {
      sprintf(errBuf, "Setting host name failed with error condition: %d and reason %s\n", err , wolfSSL_ERR_error_string(err , errBuf));
      Serial.print(errBuf);
    }

    // initialize wolfSSL using callback functions
    wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);
    wolfSSL_SetIOSend(ctx, WiFiSend);
    wolfSSL_SetIORecv(ctx, WiFiReceive);

  } else {
    Serial.println("Não foi possível conectar");
  }
}
void loop() {
  float flat, flon, fvel;
  unsigned long age;
  int err            = 0;
  int input          = 0;
  int ret;
  char body[100];
  char msg[500];
  const char* cipherName;
  int msgSz;
  char errBuf[80];
  char reply[500];
  int flagWrite;  
  ssl = wolfSSL_new(ctx);
  if (ssl == NULL) {
    Serial.println("Unable to allocate SSL object");
    return;
  } else {
    Serial.println("SSL object allocate");
  }
  err = wolfSSL_connect_TLSv13(ssl);
  if (err != WOLFSSL_SUCCESS) {
    err = wolfSSL_get_error(ssl, 0);
    wolfSSL_ERR_error_string(err, errBuf);
    Serial.print("TLS Connect Error: ");
    Serial.println(errBuf);
  }
  Serial.print("SSL version is ");
  Serial.println(wolfSSL_get_version(ssl));
  cipherName = wolfSSL_get_cipher(ssl);
  Serial.print("SSL cipher suite is ");
  Serial.println(cipherName);

  while (mySerial.available()) {
    gps.encode(mySerial.read());
    gps.f_get_position(&flat, &flon, &age);
    fvel = gps.f_speed_kmph();
    Serial.print("Latitude");Serial.println(flat);
    smartdelay(0);
    sprintf(body, "{\"latitude\" : \"%.6f\", \"longitude\" : \"%.6f\", \"velocity\" : \"%.6f\"}", flat, flon , fvel );
    sprintf(msg, "POST /data HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s", HOST, strlen(body), body);
    msgSz = (int)strlen(msg);
    flat = 0.0; flon = 0.0; fvel = 0.0; 
    flagWrite = 0;
    flagWrite = wolfSSL_write(ssl, msg, msgSz);
    if (flagWrite > 0 ) {
      input = 0;
      input = wolfSSL_read(ssl, reply, sizeof(reply) - 1);
      if (input < 0) {
        err = wolfSSL_get_error(ssl, 0);
        wolfSSL_ERR_error_string(err, errBuf);
        Serial.print("TLS Read Error: ");
        Serial.println(errBuf);
      } else if (input > 0) {
        reply[input] = '\0';
        Serial.print(reply);
        Serial.println("Connection complete.");
        delay(1000);
      } else {
        Serial.println();
      }
    } else if (flagWrite < 0 || flagWrite == 0) {
      err = wolfSSL_get_error(ssl, 0);
      wolfSSL_ERR_error_string(err, errBuf);
      Serial.print("TLS Write Error: ");
      Serial.println(errBuf);
    }
  }

  wolfSSL_shutdown(ssl);
  wolfSSL_free(ssl);

}
int WiFiSend(WOLFSSL* ssl_cli, char* msg, int sz, void* ctx_cli) {
  int sent = 0;
  sent = client.write(msg, sz);
  return sent;
}
int WiFiReceive(WOLFSSL* ssl_cli, char* reply, int sz, void* ctx_cli) {
  int ret = 0;
  while (!client.available()) {}
  while (client.available() > 0 && ret < sz) {
    reply[ret++] = client.read();
    yield();
  }
  return ret;
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (mySerial.available()) {
      gps.encode(mySerial.read());
      yield();
    }
  } while (millis() - start < ms);
}
