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
int response_message_length;

char *convertTimestampToDate(time_t timestamp)
{
    static char formattedDate[80];
    struct tm *timeinfo;
    timeinfo = gmtime(&timestamp);
    strftime(formattedDate, sizeof(formattedDate), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    return formattedDate;
}

char *base64_encode(const char *data, int data_len)
{
    // base64编码表
    char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    // 返回值
    char *res = (char *)malloc(sizeof(char) * (data_len * 4 / 3 + 4));
    // 每次取3个字节，编码成4个字符
    int pos = 0;
    for (int i = 0; i < data_len; i += 3)
    {
        // 取3个字节
        int val = data[i] << 16;
        if (i + 1 < data_len)
            val |= data[i + 1] << 8;
        if (i + 2 < data_len)
            val |= data[i + 2];
        // 编码成4个字符
        res[pos++] = table[(val >> 18) & 0x3F];
        res[pos++] = table[(val >> 12) & 0x3F];
        if (i + 1 < data_len)
            res[pos++] = table[(val >> 6) & 0x3F];
        else
            res[pos++] = '=';
        if (i + 2 < data_len)
            res[pos++] = table[val & 0x3F];
        else
            res[pos++] = '=';
    }
    res[pos] = '\0';
    return res;
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

char *Response(char *message_buffer, int complete_message_length, int client_sock)
{
    char *log;
    struct stat buf;
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
        char response_message_length_str[10];
        sprintf(response_message_length_str, "%d", response_message_length);
        log = malloc(strlen(RESPONSE_400) + strlen("Message size:") + strlen(response_message_length_str) + 1);
        sprintf(log, "%s Message size:%d", RESPONSE_400, response_message_length);
    }
    else if (strcmp(request->http_version, "HTTP/1.1") != 0)
    {
        response_message = malloc(strlen(RESPONSE_505));
        memset(response_message, 0, strlen(RESPONSE_505));
        memcpy(response_message, RESPONSE_505, strlen(RESPONSE_505));
        response_message_length = strlen(RESPONSE_505);
        char response_message_length_str[10];
        sprintf(response_message_length_str, "%d", response_message_length);
        log = malloc(strlen(RESPONSE_505) + strlen("Message size:") + strlen(response_message_length_str) + 1);
        sprintf(log, "%s Message size:%d", RESPONSE_505, response_message_length);
    }
    else if (strcmp(request->http_method, "GET") == 0)
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

                free(file_content);
                free(response_header);

                char response_message_length_str[10];
                sprintf(response_message_length_str, "%d", response_message_length);
                log = malloc(strlen(RESPONSE_200) + strlen("Message size:") + strlen(response_message_length_str) + 1);
                sprintf(log, "%s Message size:%d", RESPONSE_200, response_message_length);


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

                free(response_header);

                char response_message_length_str[10];
                sprintf(response_message_length_str, "%d", response_message_length);
                log = malloc(strlen(RESPONSE_200) + strlen("Message size:") + strlen(response_message_length_str) + 1);
                sprintf(log, "%s Message size:%d", RESPONSE_200, response_message_length);

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

        char response_message_length_str[10];
        sprintf(response_message_length_str, "%d", response_message_length);
        log = malloc(strlen(RESPONSE_200) + strlen("Message size:") + strlen(response_message_length_str) + 1);
        sprintf(log, "%s Message size:%d", RESPONSE_200, response_message_length);
    }
    else
    {
        response_message = RESPONSE_501;
        response_message_length = strlen(RESPONSE_501);
        char response_message_length_str[10];
        sprintf(response_message_length_str, "%d", response_message_length);
        log = malloc(strlen(RESPONSE_501) + strlen("Message size:") + strlen(response_message_length_str) + 1);
        sprintf(log, "%s Message size:%d", RESPONSE_501, response_message_length);
    }

    free(request);
    send(client_sock, response_message, response_message_length, 0);
    free(response_message);
    return log;
}
