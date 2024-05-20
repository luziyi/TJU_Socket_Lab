#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"

char c_get[50] = "GET";
char c_post[50] = "POST";
char c_head[50] = "HEAD";

char RESPONSE_400[4096] = "HTTP/1.1 400 Bad request\r\n\r\n";
char RESPONSE_404[4096] = "HTTP/1.1 404 Not Found\r\n\r\n";
char RESPONSE_501[4096] = "HTTP/1.1 501 Not Implemented\r\n\r\n";
char RESPONSE_505[4096] = "HTTP/1.1 505 HTTP Version not supported\r\n\r\n";

// #define DEBUG

char *Response(char *message_buffer, int complete_message_length, int client_sock)
{
#ifdef DEBUG
    printf("Message buffer: %s\n", message_buffer);
#endif
    char *response_message;
    Request *request = parse(message_buffer, complete_message_length, client_sock);
    if (request == NULL)
    {
        // fprintf(stderr, "Failed to parse request.\n"); // 发送错误响应给客户端
        response_message = RESPONSE_400;
    }
    else if (strcmp(request->http_method, "HEAD") == 0)
    {
        // 处理HEAD请求
        // response_message = handle_head_request(request);
        response_message = "HTTP/1.1 400 Bad Request\r\n\r\n";
    }
    else if (strcmp(request->http_method, "POST") == 0)
    {
        // 处理POST请求
        // response_message = handle_post_request(request);
    }
    else
    {
        // 未知的请求方法
        response_message = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
    }
    free(request);
    printf("Response message: %s\n", response_message);
    return response_message;
}
