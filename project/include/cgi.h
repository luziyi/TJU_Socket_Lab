#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 函数用于设置CGI环境变量

void execute_cgi(int client_sock, char *cgi_script, char *query_string)
{
    char sock[10];
    printf("sock    %d\n", client_sock);
    sprintf(sock, "%d", client_sock);
    setenv("REQUEST_METHOD", "GET", 1);
    setenv("SOCKET", sock, 1);
    if (query_string != NULL)
    {
        setenv("QUERY_STRING", query_string, 1);
    }
    else
    {
        setenv("QUERY_STRING", "", 1);
    }


    // 拼接CGI脚本路径 /workspaces/TJU-Socket-Lab/project/cgi-bin/login
    char *cgi_script_path = (char *)malloc(strlen(cgi_script) + 35);
    strcpy(cgi_script_path, "/workspaces/TJU-Socket-Lab/project");
    strcat(cgi_script_path, cgi_script);
    strcat(cgi_script_path, "\0");
    if(cgi_script_path != NULL)
    {
        setenv("SCRIPT_NAME", cgi_script_path, 1);
    }
    else
    {
        setenv("SCRIPT_NAME", "", 1);
    }
    // 创建子进程
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // 子进程
        // 重定向标准输出到客户端套接字
        dup2(client_sock, STDOUT_FILENO);
        // 执行CGI脚本
        execl(cgi_script_path, "cgi-script", NULL);
        // 如果execl返回，说明发生了错误
        perror("execl");
        exit(EXIT_FAILURE);
    }
    else
    {
        // 父进程
        // 等待子进程结束
        waitpid(pid, NULL, 0);
        // 获取子进程的输出
    }

    return 0;
}