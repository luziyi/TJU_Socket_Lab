#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

// 假设的最大用户名和密码长度
#define MAX_USER_LEN 50
#define MAX_PASS_LEN 50
#define RESPONSE_SIZE 1024
// 存储用户名和密码的文件
char *file_path = "users.txt";

// 函数声明
int user_exists(const char *username);
void register_user(const char *username, const char *password);
void send_response(int sockfd, const char *message);

// 主函数
int main(void)
{
    /* 检测并创建users.txt */
    if (access(file_path, F_OK) == -1)
    {
        fprintf(stderr, "Error: %s does not exist.\n", file_path);
        int fd = open(file_path, O_CREAT | O_WRONLY, 0644);
        if (fd == -1)
        {
            perror("Error creating user file");
            exit(EXIT_FAILURE);
        }
        close(fd);
    }
    char username[MAX_USER_LEN], password[MAX_PASS_LEN];
    char *method = getenv("REQUEST_METHOD");
    char *query_string = getenv("QUERY_STRING");

    if (!method || strcmp(method, "GET") != 0)
    {
        fprintf(stderr, "Method Not Allowed.\n");
        exit(EXIT_FAILURE);
    }

    if (sscanf(query_string, "uName=%50[^&]&uPass=%50[^&]", username, password) != 2)
    {
        fprintf(stderr, "Bad Request.\n");
        exit(EXIT_FAILURE);
    }

    char response[RESPONSE_SIZE];
    if (user_exists(username))
    {
        sprintf(response, "\n{\"Code\": \"400\", \"Msg\": \"User already exists\"}");
    }
    else
    {
        register_user(username, password);
        sprintf(response, "\n{\"Code\": \"200\", \"Msg\": \"User registered successfully\",\"Results\":{\"username\":\"%s\",\"password\":\"%s\"}}", username, password);
    }

    char *client_sock = getenv("SOCKET");
    send_response(atoi(client_sock), response);

    exit(EXIT_SUCCESS);
}

int user_exists(const char *username)
{
    if (access(file_path, R_OK) == -1)
    {
        fprintf(stderr, "Error: %s is not readable.\n", file_path);
        exit(EXIT_FAILURE);
    }
    FILE *file = fopen(file_path, "r");
    if (!file)
    {
        perror("Error opening user file");
        exit(EXIT_FAILURE);
    }

    /*  检查是否存在相同的用户名 用户在文件中的存储格式 [username],[password]  */ 
    char line[128];
    while (fgets(line, sizeof(line), file))
    {
        char stored_username[MAX_USER_LEN];
        if (sscanf(line, "%50[^,]", stored_username) == 1 && strcmp(username, stored_username) == 0)
        {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

// 注册用户的函数
void register_user(const char *username, const char *password)
{
    FILE *file = fopen(file_path, "a");
    if (!file)
    {
        perror("Error opening user file for writing");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%s,%s\n", username, password);
    fclose(file);
}

void send_response(int sockfd, const char *message)
{
    const char http_header[] = "HTTP/1.1 200 OK\nContent-Length: ";
    char content_length[20];
    sprintf(content_length, "%lu", strlen(message));
    write(sockfd, http_header, sizeof(http_header) - 1);
    write(sockfd, content_length, strlen(content_length));
    write(sockfd, "\nContent-Type: application/json\r\n\r\n", 33);
    write(sockfd, message, strlen(message));
}