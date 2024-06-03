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
#include "logger.h"
#define MAX_CLIENT 1024
#define ECHO_PORT 9999
#define BUF_SIZE 81920

// #define DEBUG

char message_buffer[BUF_SIZE * 2]; // 一个足够大的缓冲区来保存累积的数据
int message_length = 0;
int bias = 0;
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

    fprintf(stdout, "==========Echo Server==========\n");

    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        log("error.log", "Failed creating socket.", LOG_LEVEL_ERROR, NULL);
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
        log("error.log", "Failed binding socket.", LOG_LEVEL_ERROR, NULL);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }

    /* 服务器监听套接字以等待连接请求 */
    if (listen(sock, 5)) // listen
    {
        close_socket(sock);
        log("error.log", "Error listening on socket.", LOG_LEVEL_ERROR, NULL);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    // 获取客户端ip
    //fd初始化
     int fd_client[MAX_CLIENT];
    int client_count = 0;
    fd_set tmp_fd;
    fd_set ready_fd;
    
    int max_fd = sock;
    FD_ZERO(&tmp_fd);
    FD_ZERO(&ready_fd);
    FD_SET(sock, &ready_fd);

    for (int i = 0; i < MAX_CLIENT; i++)
        fd_client[i] = -1;
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

        tmp_fd = ready_fd;
        int num_connect = select(max_fd + 1, &tmp_fd, NULL, NULL, NULL);
        if (num_connect < 0)
        {
            log("error.log", "num<0.", LOG_LEVEL_ERROR, NULL);
            return EXIT_FAILURE;
        }

        else if (num_connect == 0)
        {
            continue;
        }
        if (FD_ISSET(sock, &tmp_fd))//sock在tmp_fd中
        {
            cli_size = sizeof(cli_addr);
            client_sock = accept(sock, (struct sockaddr *)&cli_addr, &cli_size);//接受连接
            if (client_sock < 0)
                continue;
            for (int i = 0; i < MAX_CLIENT; i++)
            {
                if (fd_client[i] == -1)//找到一个空闲的位置
                {
                   // printf("insert in : %d\n", i);
                    fd_client[i] = client_sock;//插
                    FD_SET(client_sock, &ready_fd);
                    max_fd = (client_sock > max_fd) ? client_sock : max_fd;
                    break;
                }
                if(i == MAX_CLIENT - 1)
                {
                    log("error.log", "Clients Overflow!", LOG_LEVEL_ERROR, NULL);
                    return EXIT_FAILURE;
                }
            }    
        }
        


       /*cli_size = sizeof(cli_addr);
        if ((client_sock = accept(sock, (struct sockaddr *)&cli_addr, &cli_size)) == -1)
        {
            close(sock);
            log("error.log", "Error accepting connection.", LOG_LEVEL_ERROR, NULL);
            fprintf(stderr, "Error accepting connection.\n");
            return EXIT_FAILURE;
        }*/
        for (int i = 0; i < MAX_CLIENT; i++)
        {
            if (fd_client[i] < 0)
                continue;
            client_sock = fd_client[i];
            if (FD_ISSET(client_sock, &ready_fd))
            {
        readret = 0;
        char client_ip[64];
        inet_ntop(AF_INET, &(cli_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Connected client IP: %s\n", client_ip);
        //while ((readret = recv(client_sock, buf, BUF_SIZE, 0)) >= 1)
        //这里不用while 因为一个坏了不能坏其他的 
        readret = recv(client_sock, buf, BUF_SIZE, 0);
         if (readret <= 0)
                {
                    close_socket(client_sock);
                    FD_CLR(client_sock, &ready_fd);
                    fd_client[i] = -1;
                    log("error.log", "Error reading from client socket.", LOG_LEVEL_ERROR, client_ip);
                }
         else
        
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
                log("error.log", "Message buffer overflow.", LOG_LEVEL_ERROR, client_ip);
                fprintf(stderr, "Message buffer overflow.\n");
                break;
            }

            char response_buffer[1024];
            strncpy(response_buffer, message_buffer + bias, 1024);
            // 如果message_buffer末尾的4个字符是\r\n\r\n，则表示消息结束
            char *end_of_message = strstr(response_buffer, "\r\n\r\n");
            // 将第一个请求放入待解析区域
            while (end_of_message != NULL)
            {
                char *log_message;
                void *response_message;
                log_message = malloc(1024);
                int complete_message_length = end_of_message - response_buffer + 4;           // 计算完整消息的长度
                Response(response_buffer, complete_message_length, client_sock, log_message); // 解析完整的HTTP请求
                printf("log_message: %s\n", log_message);
                if (log_message != NULL)
                    log("access.log", log_message, LOG_LEVEL_INFO, client_ip);
                bias += complete_message_length;
                memset(response_buffer, 0, 1024);
                strncpy(response_buffer, message_buffer + bias, 1024);
                end_of_message = strstr(response_buffer, "\r\n\r\n");
            }

            if (readret == -1) // 如果读取失败
            {
                close_socket(client_sock);
                close_socket(sock);
                fprintf(stderr, "Error reading from client socket.\n");
                return EXIT_FAILURE;
            }
            /*
            if (close_socket(client_sock))
            {
                close_socket(sock);
                fprintf(stderr, "Error closing client socket.\n");
                return EXIT_FAILURE;
            }*/
        }
        FD_CLR(client_sock, &ready_fd);
        fd_client[i] = -1;
        close_socket(client_sock);
            }
        }
   } 
   close_socket(sock);
    return EXIT_SUCCESS;

}
