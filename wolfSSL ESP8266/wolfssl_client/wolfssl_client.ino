#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include<TinyGPS.h>
#include <wolfssl.h>
#include <user_settings.h>
#include <wolfssl/ssl.h>
#include <string.h>
#define PORT 443
#define HOST "backend-fogcomp.tk"
#define SSID "HausOfGaga_2G"
#define PASS "BornThisWay"
#define CIPHER1 "TLS13-AES128-GCM-SHA256"
#define CIPHER2 "TLS13-AES256-GCM-SHA384"
#define CIPHER3 "TLS13-CHACHA20-POLY1305-SHA256"

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

    err = wolfSSL_CTX_UseSNI(ctx, WOLFSSL_SNI_HOST_NAME, HOST, XSTRLEN(HOST));
    if (err != WOLFSSL_SUCCESS) {
      sprintf(errBuf, "Setting host name failed with error condition: %d and reason %s\n", err , wolfSSL_ERR_error_string(err , errBuf));
      Serial.print(errBuf);
    }
    if (wolfSSL_CTX_set_cipher_list(ctx, CIPHER1) != WOLFSSL_SUCCESS) {
      Serial.println("Deu ruim");
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
  char reply[1024];
  int flagWrite;
  unsigned long t0 = 0;
  unsigned long t1 = 0;

  ssl = wolfSSL_new(ctx);
  if (ssl == NULL) {
    Serial.println("Unable to allocate SSL object");
    return;
  } else {
    Serial.println("SSL object allocate");
  }
  unsigned long hst0 = millis();
  err = wolfSSL_connect_TLSv13(ssl);
  unsigned long hst1 = millis();

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
  sprintf(body, "{\"dispositivo\" : \"nodeMCU\", \"t0Handshake\" : \"%ul\",\"t1Handshake\" : \"%ul\" , \"ciphersuit\" : \"%s\"}",  hst0 , hst1, cipherName);
  sprintf(msg, "POST /data/hstime HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s", HOST, strlen(body), body);
  //Serial.println(msg);
  msgSz = (int)strlen(msg);
  wolfSSL_write(ssl, msg, msgSz);
  wolfSSL_read(ssl, reply, sizeof(reply) - 1);
  Serial.println(reply);
  Serial.println("imprimiu resposta 1");

  while (mySerial.available()) {   

    gps.encode(mySerial.read());
    gps.f_get_position(&flat, &flon, &age);
    fvel = gps.f_speed_kmph();
    sprintf(body, "{\"dispositivo\" : \"nodeMCU\",\"ciphersuit\" : \"%s\", \"latitude\" : \"%.6f\", \"longitude\" : \"%.6f\", \"velocity\" : \"%.6f\"}", cipherName, flat, flon , fvel );
    sprintf(msg, "POST /data HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s", HOST, strlen(body), body);
    msgSz = (int)strlen(msg);
    flat = 0.0; flon = 0.0; fvel = 0.0;
    flagWrite = 0;
    t0 = millis();
    flagWrite = wolfSSL_write(ssl, msg, msgSz);
    if (flagWrite > 0 ) {
      input = 0;
      input = wolfSSL_read(ssl, reply, sizeof(reply) - 1);
      t1 = millis();
      reply[input] = '\0';
      //Serial.print(reply);
      Serial.println("Connection complete 1");  
      Serial.println();
    
    }
    sprintf(body, "{\"dispositivo\" : \"nodeMCU\",\"ciphersuit\" : \"%s\", \"t0\" : \"%ul\", \"t1\" : \"%ul\"}", cipherName, t0, t1 );
    sprintf(msg, "POST /data/commtime HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s", HOST, strlen(body), body);
    msgSz = (int)strlen(msg);
    flagWrite = 0;
    flagWrite = wolfSSL_write(ssl, msg, msgSz);
    if (flagWrite > 0 ) {
      input = 0;
      input = wolfSSL_read(ssl, reply, sizeof(reply) - 1);
      t1 = millis();
      reply[input] = '\0';
      Serial.println("Connection complete 2");      
      delay(1000);
      Serial.println();      
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
      
    }
  } while (millis() - start < ms);
}
