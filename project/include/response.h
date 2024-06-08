#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "parse.h"
#include <netinet/in.h>
#include <netinet/ip.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include "cgi.h"

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
int response_message_length;


char *convertTimestampToDate(time_t timestamp)
{
    static char formattedDate[80];
    struct tm *timeinfo;
    timeinfo = gmtime(&timestamp);
    strftime(formattedDate, sizeof(formattedDate), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    return formattedDate;
}

const char *get_mime_type(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return "text/plain";
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".js") == 0)
        return "application/javascript";
    return "text/plain";
}

void strccpy(char *dest, const char *src, size_t max, char stopChar) {
    size_t i;
    for (i = 0; i < max - 1; ++i) { // -1 为了保留空间给结尾的空字符
        if ((src[i] == stopChar) || (src[i] == '\0')) {
            break;
        }
        dest[i] = src[i];
    }
    dest[i] = '\0'; // 确保字符串以空字符结尾
}

void Response(char *message_buffer, int complete_message_length, int client_sock, char *log)
{
    struct stat buf;
    char response_length_str[10];
    time_t now;
    time(&now);
    void *response_message;
    Request *request = parse(message_buffer, complete_message_length, client_sock);

    if (request == NULL)
    {
        response_message = malloc(strlen(RESPONSE_400));
        memset(response_message, 0, strlen(RESPONSE_400));
        memcpy(response_message, RESPONSE_400, strlen(RESPONSE_400));
        response_message_length = strlen(RESPONSE_400);
        memcpy(log, "400 Bad request", strlen("400 Bad request"));
    }
    else if (strcmp(request->http_version, "HTTP/1.1") != 0)
    {
        response_message = malloc(strlen(RESPONSE_505));
        memset(response_message, 0, strlen(RESPONSE_505));
        memcpy(response_message, RESPONSE_505, strlen(RESPONSE_505));
        response_message_length = strlen(RESPONSE_505);
        memcpy(log, "505 HTTP Version not supported", strlen("505 HTTP Version not supported"));
    }
    else if (strcmp(request->http_method, "GET") == 0)
    {
        /* 获取客户端请求的文件路径 */
        char head_URL[256];
        memset(head_URL, 0, sizeof(head_URL));
        strcat(head_URL, root_path);

        // 处理路径中的？
        char *question_mark = strchr(request->http_uri, '?');
        if (question_mark != NULL)
        {
            *question_mark = '\0';
        }
        // 将?前后的字符串分别存储在path和query中
        char path[URL_MAX_SIZE];
        char query[URL_MAX_SIZE];
        memset(path, 0, sizeof(path));
        memset(query, 0, sizeof(query));
        strcpy(path, request->http_uri);
        if (question_mark != NULL)
        {
            strcpy(query, question_mark + 1);
        }

        printf("path: %s\n", path);
        printf("query: %s\n", query);

        if (strcmp(path, "/") == 0)
        {
            strcat(head_URL, file_path); // 将index.html加入到head_URL中
        }
        else
        {
            strcat(head_URL, path); // 将请求的文件加入到head_URL中
        }


        char *mime_type = get_mime_type(head_URL); // 获取客户端请求的文件类型
        /* 获取客户端请求的文件路径 */
        // 如果客户端是cgi请求
        if (strstr(path, "cgi-bin") != NULL)
        {
            printf("cgi-bin\n");
            execute_cgi(client_sock, path, query);
            return;
        }
        /* 读取处理过后的文件路径 */
        if (stat(head_URL, &buf) == -1)
        {
            // 根据不同的错误判断是Permission denied还是Not Found还是IO error 还是Out of memory 还是 UNknown error
            if (errno == EACCES)
            {
                response_message = malloc(strlen("HTTP/1.1 404 Permission Denied\r\n\r\n"));
                memset(response_message, 0, strlen("HTTP/1.1 404 Permission Denied\r\n\r\n"));
                memcpy(response_message, "HTTP/1.1 404 Permission Denied\r\n\r\n", strlen("HTTP/1.1 404 Permission Denied\r\n\r\n"));
                response_message_length = strlen("HTTP/1.1 404 Permission Denied\r\n\r\n");
                memcpy(log, "404 Permission Denied", strlen("404 Permission Denied"));
            }
            else if (errno == ENOENT)
            {
                response_message = malloc(strlen("HTTP/1.1 404 Not Found\r\n\r\n"));
                memset(response_message, 0, strlen("HTTP/1.1 404 Not Found\r\n\r\n"));
                memcpy(response_message, "HTTP/1.1 404 Not Found\r\n\r\n", strlen("HTTP/1.1 404 Not Found\r\n\r\n"));
                response_message_length = strlen("HTTP/1.1 404 Not Found\r\n\r\n");
                memcpy(log, "404 Not Found", strlen("404 Not Found"));
            }
            else if (errno == EIO)
            {
                response_message = malloc(strlen("HTTP/1.1 404 IO Error\r\n\r\n"));
                memset(response_message, 0, strlen("HTTP/1.1 404 IO Error\r\n\r\n"));
                memcpy(response_message, "HTTP/1.1 404 IO Error\r\n\r\n", strlen("HTTP/1.1 404 IO Error\r\n\r\n"));
                response_message_length = strlen("HTTP/1.1 404 IO Error\r\n\r\n");
                memcpy(log, "404 IO Error", strlen("404 IO Error"));
            }
            else if (errno == ENOMEM)
            {
                response_message = malloc(strlen("HTTP/1.1 404 Out of Memory\r\n\r\n"));
                memset(response_message, 0, strlen("HTTP/1.1 404 Out of Memory\r\n\r\n"));
                memcpy(response_message, "HTTP/1.1 404 Out of Memory\r\n\r\n", strlen("HTTP/1.1 404 Out of Memory\r\n\r\n"));
                response_message_length = strlen("HTTP/1.1 404 Out of Memory\r\n\r\n");
                memcpy(log, "404 Out of Memory", strlen("404 Out of Memory"));
            }
            else
            {
                response_message = malloc(strlen("HTTP/1.1 404 Unknown Error\r\n\r\n"));
                memset(response_message, 0, strlen("HTTP/1.1 404 Unknown Error\r\n\r\n"));
                memcpy(response_message, "HTTP/1.1 404 Unknown Error\r\n\r\n", strlen("HTTP/1.1 404 Unknown Error\r\n\r\n"));
                response_message_length = strlen("HTTP/1.1 404 Unknown Error\r\n\r\n");
                memcpy(log, "404 Unknown Error", strlen("404 Unknown Error"));
            }
        }
        else
        {
            FILE *fd_in;
            fd_in = fopen(head_URL, "rb+");
            if (fd_in == NULL)
            {
                response_message = malloc(strlen(RESPONSE_404));
                memset(response_message, 0, strlen(RESPONSE_404));
                memcpy(response_message, RESPONSE_404, strlen(RESPONSE_404));
                response_message_length = strlen(RESPONSE_404);
            }
            else
            {
                void *file_content = malloc(buf.st_size);
                memset(file_content, 0, buf.st_size);
                fread(file_content, 1, buf.st_size, fd_in);
                fclose(fd_in);

                void *response_header = malloc(256);

                sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\nDate: %s\r\nServer: Liso/1.1\r\n\r\n", mime_type, buf.st_size, convertTimestampToDate(now));

                response_message = malloc(strlen(response_header) + buf.st_size);
                memset(response_message, 0, strlen(response_header) + buf.st_size);
                memcpy(response_message, response_header, strlen(response_header));
                memcpy(response_message + strlen(response_header), file_content, buf.st_size);

                response_message_length = strlen(response_header) + buf.st_size;

                memcpy(log, "200 OK", strlen("200 OK"));
            }
        }
    }
    else if (strcmp(request->http_method, "HEAD") == 0)
    {
        /* 获取客户端请求的文件路径 */
        char head_URL[256];
        memset(head_URL, 0, sizeof(head_URL));
        strcat(head_URL, root_path);
        if (strcmp(request->http_uri, "/") == 0)
        {
            strcat(head_URL, file_path); // 将index.html加入到head_URL中
        }
        else
        {
            strcat(head_URL, request->http_uri); // 将请求的文件加入到head_URL中
        }
        char *mime_type = get_mime_type(head_URL); // 获取客户端请求的文件类型
        /* 获取客户端请求的文件路径 */

        /* 读取处理过后的文件路径 */
        if (stat(head_URL, &buf) == -1)
        {
            response_message = malloc(strlen(RESPONSE_404));
            memset(response_message, 0, strlen(RESPONSE_404));
            memcpy(response_message, RESPONSE_404, strlen(RESPONSE_404));
            response_message_length = strlen(RESPONSE_404);
        }
        else
        {
            FILE *fd_in;
            fd_in = fopen(head_URL, "rb+");
            if (fd_in == NULL)
            {
                response_message = malloc(strlen(RESPONSE_404));
                memset(response_message, 0, strlen(RESPONSE_404));
                memcpy(response_message, RESPONSE_404, strlen(RESPONSE_404));
                response_message_length = strlen(RESPONSE_404);
            }
            else
            {
                void *response_header = malloc(256);
                sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\nDate: %s\r\nServer: Liso/1.1\r\n\r\n", mime_type, buf.st_size, convertTimestampToDate(now));

                response_message = malloc(strlen(response_header));
                memset(response_message, 0, strlen(response_header));
                memcpy(response_message, response_header, strlen(response_header));
                response_message_length = strlen(response_header);

                memcpy(log, "200 OK", strlen("200 OK"));
            }
        }
        /* 读取处理过后的文件路径 */
    }
    else if (strcmp(request->http_method, "POST") == 0)
    {
        response_message = malloc(complete_message_length);
        memset(response_message, 0, complete_message_length);
        memcpy(response_message, message_buffer, complete_message_length);
        response_message_length = complete_message_length;
        memcpy(log, "POST!!", strlen("POST!!"));
    }
    else
    {
        response_message = RESPONSE_501;
        response_message_length = strlen(RESPONSE_501);
        memcpy(log, "501 Not Implemented", strlen("501 Not Implemented"));
    }

    free(request);
    sprintf(response_length_str, "%d", response_message_length);
    strcat(log, "\tSIZE:");
    strcat(log, response_length_str);
    strcat(log, "\0");
    send(client_sock, response_message, response_message_length, 0);
}
