#include <stdio.h>
#include <time.h>
#include "protocol.h"

void log_history(const char *role, const char *message)
{
    FILE *file = fopen("history.log", "a");
    if (file == NULL) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(
        file,
        "[%04d-%02d-%02d %02d:%02d:%02d] [%s] [%s]\n",
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec,
        role,
        message
    );

    fclose(file);
}
