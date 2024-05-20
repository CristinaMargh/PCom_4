#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *authorization)
{
    char *message = calloc(BUFLEN, sizeof(char));
    DIE(message == NULL, "calloc failed!\n");
    char *line = calloc(LINELEN, sizeof(char));
    DIE(line == NULL, "calloc failed!\n");

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host:%s", host);
    compute_message(message, line);
    
    // Cookies
    if (cookies && cookies_count > 0) {
        strcpy(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
        }
        strcat(line, "\r\n");
        strcat(message, line);
    }
      if (authorization != NULL && strlen(authorization) > 0) {
        sprintf(line, "Authorization: %s\r\n", authorization);
        strcat(message, line);
    }
    // Step 4: add final new line
    compute_message(message, "");
    free(line);
    return message;
}
char *compute_delete_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *authorization)
{
    char *message = calloc(BUFLEN, sizeof(char));
    DIE(message == NULL, "calloc failed!\n");
    char *line = calloc(LINELEN, sizeof(char));
    DIE(line == NULL, "calloc failed!\n");

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host:%s", host);
    compute_message(message, line);
    
    // Cookies
    if (cookies && cookies_count > 0) {
        strcpy(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
        }
        strcat(line, "\r\n");
        strcat(message, line);
    }
      if (authorization != NULL && strlen(authorization) > 0) {
        sprintf(line, "Authorization: %s\r\n", authorization);
        strcat(message, line);
    }
    // Step 4: add final new line
    compute_message(message, "");
    free(line);
    return message;
}

char *compute_post_request(char *host, char *url, char *content_type, char **body_data,
                           int body_data_fields_count, char **cookies, int cookies_count, char* authorization) {
    char *message = calloc(BUFLEN, sizeof(char));
    DIE(message == NULL, "calloc failed!\n");
    char *line = calloc(LINELEN, sizeof(char));
    DIE(line == NULL, "calloc failed!\n");
    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host:%s", host);
    compute_message(message, line);

    /* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);
    sprintf(line, "Content-Length: %ld\r\n", strlen(*body_data));
    strcat(message, line);

    // Handle cookies, assuming first cookie is session and optionally second is bearer token
    if (cookies && cookies_count > 0) {
            strcpy(line, "Cookie: ");
            for (int i = 0; i < cookies_count; i++) {
                strcat(line, cookies[i]);
            }
            strcat(line, "\r\n");
            strcat(message, line);
        }
      if (authorization != NULL && strlen(authorization) > 0) {
        sprintf(line, "Authorization: %s\r\n", authorization);
        strcat(message, line);
     }

    // Step 5: add new line at end of header
    strcat(message, "\r\n");
    strcat(message, *body_data); 

    free(line);
    return message;
}

