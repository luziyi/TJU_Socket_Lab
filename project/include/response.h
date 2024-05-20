#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"

#define MAX_MESSAGE_LENGTH 4096

char RESPONSE_400[4096] = "HTTP/1.1 400 Bad request\r\n\r\n";
char RESPONSE_404[4096] = "HTTP/1.1 404 Not Found\r\n\r\n";
char RESPONSE_501[4096] = "HTTP/1.1 501 Not Implemented\r\n\r\n";
char RESPONSE_505[4096] = "HTTP/1.1 505 HTTP Version not supported\r\n\r\n";
char RESPONSE_200[4096] = "HTTP/1.1 200 OK\r\n\r\n";

char *Response(char *message_buffer, int complete_message_length, int client_sock)
{
    char *response_message;
    Request *request = parse(message_buffer, complete_message_length, client_sock);
    
    if (request == NULL)
    {
        response_message = RESPONSE_400;
    }
    else if (strcmp(request->http_version, "HTTP/1.1") != 0)
    {
        response_message = RESPONSE_505;
        printf(response_message);
    }
    else if (strcmp(request->http_method, "GET") == 0)
    {
        // Handle GET request
        FILE *file = fopen("/webServerStartCodes-new/static_site/index.html", "r");
        if (file == NULL) {
            response_message = RESPONSE_404;
        } else {
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            
            response_message = (char *) malloc(file_size + sizeof(RESPONSE_200));
            strcpy(response_message, RESPONSE_200);
            fread(response_message + sizeof(RESPONSE_200) - 1, file_size, 1, file);
            fclose(file);
        }   
    }
    else if (strcmp(request->http_method, "HEAD") == 0)
    {
        // Handle HEAD request
        FILE *file = fopen("/webServerStartCodes-new/static_site/index.html", "r");
        if (file == NULL) {
            response_message = RESPONSE_404;
        } else {
            fclose(file);
            response_message = RESPONSE_200;
        }
    }
    else if (strcmp(request->http_method, "POST") == 0)
    {
        // Handle POST request
        response_message = message_buffer;
    }
    else
    {
        response_message = RESPONSE_501;
    }
    
    free(request);
    printf("Response message: %s\n", response_message);
    return response_message;
}
