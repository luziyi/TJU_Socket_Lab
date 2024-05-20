/******************************************************************************
 * echo_server.c                                                               *
 *                                                                             *
 * Description: This file contains the C source code for an echo server.  The  *
 *              server runs on a hard-coded port and simply write back anything*
 *              sent to it by connected clients.  It supports persistent       *
 *              connections.                                                   *
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
#include "response.h"
#define ECHO_PORT 9999
#define BUF_SIZE 4096

// #define DEBUG

char message_buffer[BUF_SIZE * 2]; // 一个足够大的缓冲区来保存累积的数据
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

    /* 服务器监听套接字以等待连接请求 */
    if (listen(sock, 5)) // listen
    {
        close_socket(sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    /* finally, loop waiting for input and then write it back */
    while (1)
    {
        /**
         * 从客户端接受传入的连接。
         *
         * @param sock 服务器套接字描述符。
         * @param cli_addr 客户端地址结构。
         * @param cli_size 客户端地址结构的大小。
         * @return 如果成功，则返回客户端套接字描述符；如果发生错误，则返回-1。
         */
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
            // 如果message_buffer末尾的4个字符是\r\n\r\n，则表示消息结束
#ifdef DEBUG
            printf("Message buffer: %s\n", message_buffer);
#endif
            char *end_of_message = strstr(buf, "\r\n\r\n");
            // 将第一个请求放入待解析区域
            if (end_of_message != NULL)
            {
// 输出缓冲区内容
#ifdef DEBUG
                printf("Message buffer: %s\n", message_buffer);
#endif
                char *response_message;
                int complete_message_length = end_of_message - buf + 4;      // 计算完整消息的长度
                response_message = Response(buf, complete_message_length, client_sock); // 解析完整的HTTP请求
                // printf("Response message: %s\n", response_message);
                send(client_sock, response_message, strlen(response_message), 0); // 发送响应
                // 清空缓冲区
                memset(message_buffer, 0, sizeof(message_buffer));
                message_length = 0;
            memset(buf, 0, BUF_SIZE);   

                // close_socket(client_sock);
            }
        }

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
