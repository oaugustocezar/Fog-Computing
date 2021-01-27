#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit, atoi, malloc, free */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <curl/curl.h>



void error(const char *msg) { perror(msg); exit(0); }

int main(int argc,char *argv[])
{
    CURL *curl;
  CURLcode res;
  
  char *latitude = "40";
  char *longitude = "35";
  char *velocity = "38";
  char body[500];
  
  sprintf( body, "{\"latitude\" : \"%s\", \"longitude\" : \"%s\", \"velocity\" : \"%s\"}",latitude,longitude,velocity);
  
  /* In windows, this will init the winsock stuff */ 
  curl_global_init(CURL_GLOBAL_ALL);
 
  /* get a curl handle */ 
    curl = curl_easy_init();
  /* set headers */
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charset: utf-8");
  if(curl) {
    /* First set the URL that is about to receive our POST. This URL can
       just as well be a https:// URL if that is what should receive the
       data. */ 
    curl_easy_setopt(curl, CURLOPT_URL, "backend-fogcomp.herokuapp.com/data");
    /* set the request method */
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, argv[1]);
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); 
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
 
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return 0;
}
