#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_USER_LEN 50
#define MAX_PASS_LEN 50
#define RESPONSE_SIZE 1024

#define USER_FILE "./users.txt" 

void send_response(int sockfd, const char *message);
char *get_password(const char *username, const char *userfile);

// 主函数
int main(void) {
    char *method = getenv("REQUEST_METHOD"); // 应为"GET"或"POST"
    char *query_string = getenv("QUERY_STRING");
    char *client_sock = getenv("SOCKET");
    char username[MAX_USER_LEN], password[MAX_PASS_LEN];
    char response[RESPONSE_SIZE];

    if (method == NULL) {
        fprintf(stderr, "Error: REQUEST_METHOD not set.\n");
        return EXIT_FAILURE;
    }
    if (strcmp(method, "GET") == 0) {
        // 解析查询字符串
        sscanf(query_string, "uName=%50[^&]&uPass=%50[^&]", username, password);
    } else {
        sprintf(response, "\n{\"Code\": \"405\", \"Msg\": \"Method Not Allowed\"}");
        send_response(atoi(client_sock), response);
        exit(EXIT_FAILURE);
    }

    // 检查用户名和密码是否匹配
    
    char *stored_password = get_password(username, USER_FILE);
    if (stored_password != NULL) {
        // 判断密码是否相同
        if (strcmp(password, stored_password) == 0) {
            sprintf(response, "\n{\"Code\": \"200\", \"Msg\": \"Login successful\",\"Results\":{\"username\":\"%s\",\"password\":\"%s\"}}", username, password);
        } else {
            sprintf(response, "\n{\"Code\": \"400\", \"Msg\": \"Incorrect password\",\"Results\":{\"username\":\"%s\",\"password\":\"%s\"}}", username, stored_password);
        }
    } else {
        
    }
    send_response(atoi(client_sock), response);
    exit(EXIT_SUCCESS);
}

// 获取存储的密码 密码在文件中存储的格式是 [username],[password]
char *get_password(const char *username, const char *userfile) {
    FILE *file = fopen(userfile, "r");
    if (!file) {
        perror("Error opening user file");
        exit(EXIT_FAILURE);
    }
    char line[128];
    while (fgets(line, sizeof(line), file)) {
        char stored_username[MAX_USER_LEN], stored_password_hash[MAX_PASS_LEN];
        if (sscanf(line, "%50[^,],%50[^\n]", stored_username, stored_password_hash) == 2) {
            if (strcmp(username, stored_username) == 0) {
            fclose(file);
            char *result = malloc(strlen(stored_password_hash) + 1);  // Allocate memory
            strcpy(result, stored_password_hash);  // Copy the password hash
            return result;
            }
        }
    }
    fclose(file);
    return NULL;
}

// 发送JSON格式的响应


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