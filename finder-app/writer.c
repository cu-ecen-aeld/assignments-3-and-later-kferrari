#include <stdio.h>
#include <stdlib.h>

#include <syslog.h>


int main(int argc, char *argv[]) {
    openlog ("writer", 0, LOG_USER);

    if (argc < 3) {
        syslog(LOG_ERR, "Usage: %s <file> <content>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "w");
    if (file == NULL) {
        syslog(LOG_ERR, "Cannot create file %s\n", argv[1]);
        return 1;
    }

    syslog(LOG_DEBUG, "Writing %s to %s\n", argv[2], argv[1]);

    fprintf(file, "%s", argv[2]);
    fclose(file);

    closelog();

    return 0;
}