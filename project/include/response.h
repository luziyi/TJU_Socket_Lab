#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "parse.h"
#include <netinet/in.h>
#include <netinet/ip.h>
#include <fcntl.h>
#include <time.h>

#define MAX_MESSAGE_LENGTH 4096
#define URL_MAX_SIZE 256
#define O_RDONLY 00
#define S_ISREG 0100000
#define S_IRUSR 00400
#define BUF_SIZE 4096

char RESPONSE_400[4096] = "HTTP/1.1 400 Bad request\r\n\r\n";
char RESPONSE_404[4096] = "HTTP/1.1 404 Not Found\r\n\r\n";
char RESPONSE_501[4096] = "HTTP/1.1 501 Not Implemented\r\n\r\n";
char RESPONSE_505[4096] = "HTTP/1.1 505 HTTP Version not supported\r\n\r\n";
char RESPONSE_200[4096] = "HTTP/1.1 200 OK\r\n\r\n";
char http_version_now[50] = "HTTP/1.1";
char root_path[50] = "./static_site";
char file_path[50] = "/index.html";

char *convertTimestampToDate(time_t timestamp)
{
    static char formattedDate[80];
    struct tm *timeinfo;
    timeinfo = gmtime(&timestamp);
    strftime(formattedDate, sizeof(formattedDate), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    return formattedDate;
}

char *Response(char *message_buffer, int complete_message_length, int client_sock)
{
    struct stat buf;
    time_t now;
    time(&now);

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

        char head_URL[BUF_SIZE];
        memset(head_URL, 0, sizeof(head_URL));
        strcat(head_URL, root_path);

        /* 获取客户端请求的文件路径 */
        if (strcmp(request->http_uri, "/") == 0)
        {
            strcat(head_URL, file_path); // 将index.html加入到head_URL中
        }
        else if (sizeof(request->http_uri) + sizeof(root_path) < URL_MAX_SIZE)
        {
            strcat(head_URL, request->http_uri); // 将请求的文件加入到head_URL中
        }
        else
        {
            response_message = RESPONSE_404;
        }
        /* 获取客户端请求的文件路径 */

        /* 读取处理过后的文件路径 */
        if (stat(head_URL, &buf) == -1)
        {
            response_message = RESPONSE_404;
        }
        else
        {

            int fd_in = open(head_URL, O_RDONLY);

            if (fd_in < 0)
            {
                printf("Failed to open the file\n");
                response_message = RESPONSE_404;
            }
            else
            {

                char response1[BUF_SIZE];
                char response2[BUF_SIZE];

                ssize_t bytesRead = read(fd_in, response2, BUF_SIZE);
                if (bytesRead < 0)
                {
                    printf("Failed to read the file\n");
                    close(fd_in);
                    response_message = RESPONSE_404;
                }
                else
                {
                    memcpy(response1, "HTTP/1.1 200 OK\r\n\r\n", sizeof(response1));
                    strncat(response1, response2, bytesRead);
                    response_message = response1;
                    close(fd_in);
                }
            }
        }
        /* 读取处理过后的文件路径 */
    }
    else if (strcmp(request->http_method, "HEAD") == 0)
    {
        char head_URL[BUF_SIZE];
        memset(head_URL, 0, sizeof(head_URL));
        strcat(head_URL, root_path);

        /* 获取客户端请求的文件路径 */
        if (strcmp(request->http_uri, "/") == 0)
        {
            strcat(head_URL, file_path); // 将index.html加入到head_URL中
        }
        else if (sizeof(request->http_uri) + sizeof(root_path) < URL_MAX_SIZE)
        {
            strcat(head_URL, request->http_uri); // 将请求的文件加入到head_URL中
        }
        else
        {
            response_message = RESPONSE_404;
        }
        /* 获取客户端请求的文件路径 */

        /* 读取处理过后的文件路径 */
        if (stat(head_URL, &buf) == -1)
        {
            response_message = RESPONSE_404;
        }
        else
        {

            int fd_in = open(head_URL, O_RDONLY);

            if (fd_in < 0)
            {
                printf("Failed to open the file\n");
                response_message = RESPONSE_404;
            }
            else
            {

                char response1[BUF_SIZE];
                char response2[BUF_SIZE];

                ssize_t bytesRead = read(fd_in, response2, BUF_SIZE);
                if (bytesRead < 0)
                {
                    printf("Failed to read the file\n");
                    close(fd_in);
                    response_message = RESPONSE_404;
                }
                else
                {
                    response_message = RESPONSE_200;
                    close(fd_in);
                }
            }
        }
        /* 读取处理过后的文件路径 */
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
