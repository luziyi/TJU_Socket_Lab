#include <stdio.h>  // Include the header file for FILE, fopen, fprintf, and fclose
#include <string.h> // Include the header file for strcmp

#define LOG_LEVEL_INFO "INFO"
#define LOG_LEVEL_ERROR "ERROR"
#define LOG_LEVEL_DEBUG "DEBUG"

void log(char *file_name, char *message, char *log_level)
{
    time_t now;
    time(&now);

    FILE *file = fopen(file_name, "a");
    if (file == NULL)
    {
        file = fopen(file_name, "a");
        if (file == NULL)
        {
            printf("Unable to create file\n");
            return;
        }
    }
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