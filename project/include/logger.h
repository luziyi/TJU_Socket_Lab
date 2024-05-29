

void log(char* file_name, char* message)
{
    FILE *file = fopen(file_name, "w");
    if (file == NULL)
    {
        file = fopen(file_name, "w");
        if (file == NULL)
        {
            // Handle error: unable to create file
            printf("Unable to create file\n");
            return;
        }
    }
    fprintf(file, "%s", message);
    fclose(file);
}