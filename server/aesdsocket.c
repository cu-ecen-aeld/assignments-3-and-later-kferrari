#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <errno.h>
#include <syslog.h>

#define PORT "9000"
#define BUF_SIZE 1024
#define FPATH "/var/tmp/aesdsocketdata"

int sockfd;

void signal_handler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        syslog(LOG_INFO, "Caught signal, exiting");
        close(sockfd);
        remove(FPATH);
        syslog(LOG_INFO, "Closed server socket and deleted file %s", FPATH);
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    int ret = 0;
    int is_daemon = 0;
    struct addrinfo hints, *result;
    struct sigaction sigact;

    // optional input flag '-d' to run as daemon
    if ((argc == 2) && (strcmp(argv[1], "-d") == 0))
    {
        is_daemon = 1;
    }

    memset(&sigact, 0, sizeof(sigact));
    sigact.sa_handler = signal_handler;
    sigfillset(&sigact.sa_mask);

    ret = sigaction(SIGINT, &sigact, NULL);
    if (ret == -1)
    {
        perror("cannot handle SIGINT");
        exit(1);
    }

    ret = sigaction(SIGTERM, &sigact, NULL);
    if (ret == -1)
    {
        perror("cannot handle SIGTERM");
        exit(1);
    }

    openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_USER);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = AI_PASSIVE;     /* For wildcard IP address */

    ret = getaddrinfo(NULL, PORT, &hints, &result);
    if (ret != 0)
    {
        printf("getaddrinfo failed: %s\n", gai_strerror(ret));
        closelog();
        exit(EXIT_FAILURE);
    }

    // socket
    sockfd = socket(result->ai_family, result->ai_socktype, 0);
    if (sockfd == -1)
    {
        perror("socket failed");
        closelog();
        freeaddrinfo(result);
        exit(1);
    }

    // to reuse the same
    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    if (ret == -1)
    {
        perror("setsockopt failed");
        closelog();
        close(sockfd);
        freeaddrinfo(result);
        exit(1);
    }

    // bind
    ret = bind(sockfd, result->ai_addr, result->ai_addrlen);
    if (ret == -1)
    {
        perror("bind failed");
        closelog();
        close(sockfd);
        freeaddrinfo(result);
        exit(1);
    }
    freeaddrinfo(result);

    // creating a daemon after binding is ensured
    if (is_daemon)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("Forking failed\n");
            closelog();
            close(sockfd);
            exit(1);
        }
        else if (pid > 0)
        { // parent process
            closelog();
            close(sockfd);
            exit(0);
        }
        printf("running in daemon mode: process id : %d\n", getpid());
    }

    // listening
    ret = listen(sockfd, 10);
    if (ret == -1)
    {
        perror("listen failed");
        closelog();
        close(sockfd);
        exit(1);
    }

    // accept connections forever, unless interrupted
    int client_sockfd;
    char buffer[BUF_SIZE] = {0};
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(addrlen);
    FILE *file;

    while (1)
    {
        client_sockfd = accept(sockfd, (struct sockaddr *)&address, &addrlen);
        if (client_sockfd == -1)
        {
            perror("accept failed");
            closelog();
            close(sockfd);
            exit(1);
        }

        syslog(LOG_INFO, "Accepted connection from %s", inet_ntoa(address.sin_addr));

        file = fopen(FPATH, "a");
        if (file == NULL)
        {
            perror("fopen failed");
            closelog();
            close(client_sockfd);
            close(sockfd);
            exit(1);
        }

        size_t bytes_recv;
        while ((bytes_recv = recv(client_sockfd, buffer, BUF_SIZE, 0)) > 0)
        {
            fwrite(buffer, 1, bytes_recv, file);
            if (buffer[bytes_recv - 1] == '\n')
                break;
        }

        fclose(file);

        file = fopen(FPATH, "r");
        if (file == NULL)
        {
            perror("fopen failed");
            closelog();
            close(client_sockfd);
            close(sockfd);
            exit(1);
        }

        while (fgets(buffer, BUF_SIZE, file) != NULL)
        {
            send(client_sockfd, buffer, strlen(buffer), 0);
        }
        fclose(file);

        close(client_sockfd);
        syslog(LOG_INFO, "Closed connection from %s", inet_ntoa(address.sin_addr));
    }

    return 0;
}