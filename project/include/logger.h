#include <stdio.h>  // Include the header file for FILE, fopen, fprintf, and fclose
#include <string.h> // Include the header file for strcmp
#include <time.h>   // Include the header file for time

#define LOG_LEVEL_INFO "INFO"
#define LOG_LEVEL_ERROR "ERROR"
#define LOG_LEVEL_DEBUG "DEBUG"

// Declare the function convertTimestampToDate
char *convertTimestampToDateForlogger(time_t timestamp)
{
    static char formattedDate[80];
    struct tm *timeinfo;
    timeinfo = gmtime(&timestamp);
    strftime(formattedDate, sizeof(formattedDate), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    return formattedDate;
}

void log(char* file_name, char* message, char* log_level, char* ip)
{
    time_t nowt;
    time(&nowt);
    char* now_time = convertTimestampToDateForlogger(nowt);
    FILE* file = fopen(file_name, "a");
    if (file == NULL)
    {
        file = fopen(file_name, "a");
        if (file == NULL)
        {
            printf("Unable to create file\n");
            return;
        }
    }
    fprintf(file, "[Date: %s] ", now_time);
    if (ip != NULL)
        fprintf(file, "[Client: %s] ", ip);
    else
        fprintf(file, "[Client: 0.0.0.0] ");
    if (strcmp(log_level, LOG_LEVEL_INFO) == 0)
    {
        fprintf(file, "[INFO] %s\n", message);
    }
    else if (strcmp(log_level, LOG_LEVEL_ERROR) == 0)
    {
        fprintf(file, "[ERROR] %s\n", message);
    }
    else if (strcmp(log_level, LOG_LEVEL_DEBUG) == 0)
    {
        fprintf(file, "[DEBUG] %s\n", message);
    }
    else
    {
        fprintf(file, "[INFO] %s\n", message);
    }

    fclose(file);
}