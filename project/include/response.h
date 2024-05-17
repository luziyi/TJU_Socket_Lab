#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"

char c_get[50] = "GET";
char c_post[50] = "POST";
char c_head[50] = "HEAD";

// #define DEBUG
#ifdef DEBUG
#define LOG(...) fprintf(stdout, __VA_ARGS__)
#endif

char* Response(char *message_buffer, int complete_message_length, int client_sock)
{
    char *response_message;
    Request *request = parse(message_buffer, complete_message_length, client_sock);
    printf("Request: %s %s %s\n", request->http_method, request->http_uri, request->http_version);
    if (request == NULL)
    {
        fprintf(stderr, "Failed to parse request.\n");
        // 发送错误响应给客户端
        response_message = "HTTP/1.1 400 Bad Request\r\n\r\n";
    }
    else if (!strcmp(request->http_method, c_get) || !strcmp(request->http_method, c_head) || !strcmp(request->http_method, c_post))
    {
        // 发送成功响应
        // char success_msg[] = "HTTP/1.1 200 OK\r\n\r\nParsed Successfully";
        response_message = "HTTP/1.1 200 OK\r\n\r\nParsed Successfully";
        response_message = message_buffer;
        // send(client_sock, message_buffer, sizeof(message_buffer) - 1, 0);
        // send(client_sock, success_msg, sizeof(success_msg) - 1, 0);
        free(request->headers);
        free(request);
    }
    else
    {
        // 发送错误响应给客户端
        response_message = "HTTP/1.1 501 Not Implemented\r\n\r\n";
        // send(client_sock, success_msg, sizeof(success_msg) - 1, 0);
        free(request->headers);
        free(request);
    }
    printf("Response message: %s\n", response_message);
    return response_message;
}
