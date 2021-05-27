/* wolfssl_client.ino
 *
 * Copyright (C) 2006-2020 wolfSSL Inc.
 *
 * This file is part of wolfSSL.
 *
 * wolfSSL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfSSL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <wolfssl.h>
#include <user_settings.h>
//#include <wolfssl/wolfcrypt/logging.h>
#include <wolfssl/ssl.h>
#include <wolfssl/certs_test.h>
#include <Ethernet.h>
SoftwareSerial mySerial(D7, D8); // RX = D7, TX  = D8
const char host[] = "www.backend-fogcomp.online"; // server to connect to
const int port = 443; // port on server to connect to


char msg[500];

WiFiClient client;
WOLFSSL_METHOD* method;
WOLFSSL_CTX* ctx = NULL;
WOLFSSL* ssl = NULL;
#define PORT 443
#define HOST "www.backend-fogcomp.online"
int EthernetSend(WOLFSSL* ssl_cli, char* msg, int sz, void* ctx_cli);
int EthernetReceive(WOLFSSL* ssl_cli, char* reply, int sz, void* ctx_cli);


void setup() {
  //wolfSSL_Debugging_ON();
  const char *ssid = "HausOfGaga_2G";
  const char *pass = "BornThisWay"; 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.begin(9600); //Start Serial
  
  pinMode(D7,INPUT); //d7 is RX, receiver, so define it as input
  pinMode(D8,OUTPUT); //d8 is TX, transmitter, so define it as output
  mySerial.begin(9600); //Start mySerial
  Serial.printf("Connecting to %s ", ssid);
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  

  if(client.connect(HOST,PORT)){
    Serial.println("Conectado ao servidor");   
    wolfSSL_Init();
    method = wolfTLSv1_2_client_method(); /* use TLS v1.3 */ 
    
    if(method == NULL){
      Serial.println("memoria insuficiente");
    }
   
    if((ctx = wolfSSL_CTX_new(method)) == NULL){
      Serial.println("Erro ao criar contexto");      
    }
     
   /*  if(wolfSSL_CTX_load_verify_buffer(ctx,(const byte*)cert_fog_der_2048,sizeof_cert_fog_der_2048,SSL_FILETYPE_ASN1)!= SSL_SUCCESS){
      Serial.println("Erro ao carregar certificado");
    }*/
    
  // initialize wolfSSL using callback functions
 // wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);
  
  wolfSSL_SetIOSend(ctx, EthernetSend);
  wolfSSL_SetIORecv(ctx, EthernetReceive);
  wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE,0);
  
  return;
}
}



void loop() {
  char body[500];
  sprintf( body, "{\"latitude\" : \"%d\", \"longitude\" : \"%d\", \"velocity\" : \"%d\"}",40,40,40);
  Serial.println("Passou 5");
  int err            = 0;
  int input          = 0;
  int total_input    = 0;
  char msg[500];
  sprintf(msg,"POST /data HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s",HOST,strlen(body),body);
  int msgSz          = (int)strlen(msg);
  char errBuf[80];
  char reply[80];
  const char* cipherName;
  //Serial.println(msg);
    

    
      Serial.print("Connected to ");
      Serial.println(HOST);
      
      ssl = wolfSSL_new(ctx);
      if (ssl == NULL) {
        Serial.println("Unable to allocate SSL object");
        return;
      }else{
        Serial.println("SSL object allocate");
      }
       wolfSSL_SetIOReadCtx(ssl, &client);
       wolfSSL_SetIOWriteCtx(ssl, &client);
       Serial.println("Passou 7");

     /* err = wolfSSL_connect(ssl);
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
      Serial.println(cipherName);*/

      if ((wolfSSL_write(ssl, msg, msgSz)) >0 ) {
        
        Serial.print("Server response: ");
        while (client.available() || wolfSSL_pending(ssl)) {
          input = wolfSSL_read(ssl, reply, sizeof(reply) - 1);
          total_input += input;
          if (input < 0) {
            err = wolfSSL_get_error(ssl, 0);
            wolfSSL_ERR_error_string(err, errBuf);
            Serial.print("TLS Read Error: ");
            Serial.println(errBuf);
            break;
          } else if (input > 0) {
            reply[input] = '\0';
            Serial.print(reply);
          } else {
            Serial.println();
          }
        } 
      } else {
        err = wolfSSL_get_error(ssl, 0);
        wolfSSL_ERR_error_string(err, errBuf);
        Serial.print("TLS Write Error: ");
        Serial.println(errBuf);
      }
      
      wolfSSL_shutdown(ssl);
      wolfSSL_free(ssl);

      client.stop();
      Serial.println("Connection complete.");
      
     
  
  delay(1000);
}

int EthernetSend(WOLFSSL* ssl_cli, char* msg, int sz, void* ctx_cli) {
  int sent = 0;
  Serial.print(msg);
  sent = client.print(msg);  
  //Serial.print(msg);
  return sent;
}

int EthernetReceive(WOLFSSL* ssl_cli, char* reply, int sz, void* ctx_cli) {
  int ret = 0;

  while (client.available() > 0 && ret < sz) {
    reply[ret++] = client.read();
  }

  return ret;
}
