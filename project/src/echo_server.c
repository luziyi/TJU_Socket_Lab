/******************************************************************************
 * echo_server.c                                                               *
 *                                                                             *
 * Description: This file contains the C source code for an echo server.  The  *
 *              server runs on a hard-coded port and simply write back anything*
 *              sent to it by connected clients.  It does not support          *
 *              concurrent clients.                                            *
 *                                                                             *
 * Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
 *          Wolf Richter <wolf@cs.cmu.edu>                                     *
 *                                                                             *
 *******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "parse.h"
#define ECHO_PORT 9999
#define BUF_SIZE 4096

// #define DEBUG

char c_get[50] = "GET";
char c_post[50] = "POST";
char c_head[50] = "HEAD";
char message_buffer[BUF_SIZE * 10]; // 一个足够大的缓冲区来保存累积的数据
int message_length = 0;

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int sock, client_sock;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char buf[BUF_SIZE];

    fprintf(stdout, "----- Echo Server -----\n");

    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)))
    {
        close_socket(sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }

    if (listen(sock, 5))
    {
        close_socket(sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    /* finally, loop waiting for input and then write it back */
    while (1)
    {
        cli_size = sizeof(cli_addr);
        if ((client_sock = accept(sock, (struct sockaddr *)&cli_addr, &cli_size)) == -1)
        {
            close(sock);
            fprintf(stderr, "Error accepting connection.\n");
            return EXIT_FAILURE;
        }

        readret = 0;

        while ((readret = recv(client_sock, buf, BUF_SIZE, 0)) >= 1)
        {
#ifdef DEBUG
            fprintf(stdout, "Received %d bytes.\n", (int)readret);
            fprintf(stdout, "Received message: %.*s\n", (int)readret, buf);
#endif
            // 将接收到的数据追加到消息缓冲区
            if (message_length + readret < sizeof(message_buffer))
            {
                memcpy(message_buffer + message_length, buf, readret);
                message_length += readret;
                message_buffer[message_length] = '\0'; // 确保缓冲区以NULL结尾
            }
            else
            {
                fprintf(stderr, "Message buffer overflow.\n");
                break;
            }

            // 检查是否接收到完整的HTTP请求（以\r\n\r\n作为结束标志）
            char *end_of_message = strstr(message_buffer, "\r\n\r\n");
            if (end_of_message != NULL)
            {
                // 计算完整消息的长度
                int complete_message_length = end_of_message - message_buffer + 4;

                // 解析完整的HTTP请求
                Request *request = parse(message_buffer, complete_message_length, client_sock);
                if (request == NULL)
                {
                    fprintf(stderr, "Failed to parse request.\n");
                    // 发送错误响应给客户端
                    char error_msg[] = "HTTP/1.1 400 Bad Request\r\n\r\n";
                    send(client_sock, error_msg, sizeof(error_msg) - 1, 0);
                }
                else 
                if(!strcmp(request->http_method, c_get) || !strcmp(request->http_method, c_head) || !strcmp(request->http_method, c_post))
                {
                    // 发送成功响应
                    char success_msg[] = "HTTP/1.1 200 OK\r\n\r\nParsed Successfully";
                     send(client_sock,  message_buffer, sizeof( message_buffer) - 1, 0);

                    
                    //send(client_sock, success_msg, sizeof(success_msg) - 1, 0);

                    // 释放请求对象
                    free(request->headers);
                    free(request);
                }
                else {
                     char success_msg[] = "HTTP/1.1 501 Not Implemented\r\n\r\n";
                    send(client_sock, success_msg, sizeof(success_msg) - 1, 0);
                    free(request->headers);
                    free(request);
                }

                // 将剩余未处理的数据移动到消息缓冲区的开头
                int remaining_data_length = message_length - complete_message_length;
                memmove(message_buffer, message_buffer + complete_message_length, remaining_data_length);
                message_length = remaining_data_length;
                message_buffer[message_length] = '\0';
            }
        }

        if (readret < 0)
        {
            perror("recv");
        }

        // 关闭客户端套接字
        // close(client_sock);

        if (readret == -1) // 如果读取失败
        {
            close_socket(client_sock);
            close_socket(sock);
            fprintf(stderr, "Error reading from client socket.\n");
            return EXIT_FAILURE;
        }

        if (close_socket(client_sock))
        {
            close_socket(sock);
            fprintf(stderr, "Error closing client socket.\n");
            return EXIT_FAILURE;
        }
    }

    close_socket(sock);

    return EXIT_SUCCESS;
}
