/******************************************************************************
 * echo_client.c                                                               *
 *                                                                             *
 * Description: This file contains the C source code for an echo client.  The  *
 *              client connects to an arbitrary <host,port> and sends input    *
 *              from stdin.                                                    *
 *                                                                             *
 * Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
 *          Wolf Richter <wolf@cs.cmu.edu>                                     *
 *                                                                             *
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define ECHO_PORT 8888 // 端口
#define BUF_SIZE 4096  // 缓冲区大小

// #define DEBUG // DEBUG开关

int main(int argc, char *argv[])
{
    if (argc <= 3)
    {
        fprintf(stderr, "usage: %s <server-ip> <port> <file-path>", argv[0]);
        return EXIT_FAILURE;
    }
    // 获取参数个数

    char buf[BUF_SIZE]; // 数据缓冲区

    int status, sock; // status: getaddrinfo return value, sock: socket file descriptor
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo)); // 初始化
    struct addrinfo *servinfo;                  // will point to the results
    hints.ai_family = AF_INET;                  // IPv4
    hints.ai_socktype = SOCK_STREAM;            // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;                // fill in my IP for me

    if ((status = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) // 如果getaddrinfo返回非0，说明出错，获取服务器地址失败
    {
        fprintf(stderr, "getaddrinfo error: %s \n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    if ((sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) // 如果socket返回-1，说明出错，出错是因为没有成功连接到服务器
    {
        fprintf(stderr, "Socket failed");
        return EXIT_FAILURE;
    }

    if (connect(sock, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        fprintf(stderr, "Connect"); // 连接失败？
        return EXIT_FAILURE;
    }

#ifdef DEBUG
    fprintf(stdout, "Connected to server\n"); // 调试信息，如果成功连接则输出
    printf("file path: %s\n", argv[3]);
#endif

    for (int i = 3; i < argc; i++)
    {
/* 从文件读取请求 并发送*/
#ifdef DEBUG
        printf("file path: %s\n", argv[i]);
#endif
        FILE *file = fopen(argv[i], "r");

        if (file == NULL)
        {
            fprintf(stderr, "Failed to open file\n");
            return EXIT_FAILURE;
        }
        char msg[BUF_SIZE];
        size_t bytes_read;
        while ((bytes_read = fread(msg, 1, sizeof(msg), file)) > 0)
        {
            send(sock, msg, bytes_read, 0); // 向服务端发送数据
            fprintf(stdout, "Sending %s", msg);
        }
        memset(msg, 0, sizeof(msg));
        fclose(file);
        /* 从文件读取请求 并发送 */

        /* 接收服务器发送的相应 */
        int bytes_received;
        while ((bytes_received = recv(sock, buf, BUF_SIZE, 0)) > 1) // 从服务器获得的消息，服务端经过解析之后发送的
        {
            buf[bytes_received] = '\0';

            fprintf(stdout, "Received %s", buf);
        }
        /* 接收服务器发送的相应 */
    }

    freeaddrinfo(servinfo);
    close(sock);
    return EXIT_SUCCESS;
}
