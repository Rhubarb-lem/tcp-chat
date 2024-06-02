#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#define BUFLEN 81
int main(int argc, char **argv)
{
    if (argc < 4)
    {
        printf("ВВЕСТИ: имя_пользователя  имя_хоста порт\n");
        exit(1);
    }
    int sock;
    char exitt[] = "exit\n";
    struct sockaddr_in servAddr;
    struct hostent *hp, *gethostbyname();

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("He могу получить socket\n");
        exit(1);
    }
    bzero((char *)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    hp = gethostbyname(argv[2]);
    bcopy(hp->h_addr, &servAddr.sin_addr, hp->h_length);
    servAddr.sin_port = htons(atoi(argv[3]));
    if (connect(sock, (const struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("Клиент не может соединиться.\n");
        exit(1);
    }
    fd_set read_fds;

    for (;;)
    {

        FD_ZERO(&read_fds);

        FD_SET(sock, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        int activity = select(sock + 1, &read_fds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            perror("select");
            exit(1);
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            char buffer[BUFLEN] = {' '};
            fgets(buffer, sizeof(buffer), stdin);
            if (!strcmp(exitt, buffer))
            {
                printf("Чат завершил свою работу\n");
                break;
            }
            char name_message[1024 + 15];
            snprintf(name_message, BUFLEN + 15, "%s: %s", argv[1], buffer);

            send(sock, name_message, strlen(name_message), 0);
        }

        if (FD_ISSET(sock, &read_fds))
        {

            char buffer[1024];
            int n = recv(sock, buffer, sizeof(buffer), 0);
            if (n <= 0)
            {

                printf("Произошла ошибка или сервер разорвал соединение.\n");

                break;
            }

            printf("%s\n", buffer);
        }
    }
    close(sock);
    exit(0);
}