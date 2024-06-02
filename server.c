#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>

#define BUFLEN 81

int main()
{
    int sockMain, sockClient;
    struct sockaddr_in servAddr;

    if ((sockMain = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Ошибка открытия основного сокета.");
        exit(1);
    }

    bzero((char *)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = 0;

    if (bind(sockMain, (struct sockaddr *)&servAddr, sizeof(servAddr)))
    {
        perror("Ошибка привязывания.");
        exit(1);
    }
    int length = sizeof(servAddr);
    if (getsockname(sockMain, (struct sockaddr *)&servAddr, (socklen_t *)&length))
    {
        perror("getsockname call unsuccessful.");
        exit(1);
    }
    printf("Сервер: номер порта - %d\n", ntohs(servAddr.sin_port));

    listen(sockMain, 5);

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(sockMain, &read_fds);
    int max_socket = sockMain;

    while (1)
    {
        fd_set tmp_fds = read_fds;
        if (select(max_socket + 1, &tmp_fds, NULL, NULL, NULL) < 0)
        {
            perror("Ошибка select");
            continue;
        }

        for (int i = 0; i <= max_socket; i++)
        {
            if (FD_ISSET(i, &tmp_fds))
            {
                if (i == sockMain)
                {
                    if ((sockClient = accept(sockMain, 0, 0)) < 0)
                    {
                        printf("Неверный сокет клиента.\n");
        
                        exit(1);
                    }
                    FD_SET(sockClient, &read_fds);
                    max_socket = (sockClient > max_socket) ? sockClient : max_socket;
                }
                else
                {
                    char buf[BUFLEN] = {' '};
                    int msgLength = recv(i, buf, BUFLEN, 0);
                    if (msgLength <= 0)
                    {
                        printf("Произошла ошибка или сервер разорвал соединение.\n");
                        close(i);
                        FD_CLR(i, &read_fds);
                    }
                    else
                    {
                        printf("Получено сообщение от клиента %d: %s\n", i, buf);
                        for (int j = 0; j <= max_socket; j++)
                        {

                            if ((j != i) && (j != sockMain))
                            {
                                send(j, buf, strlen(buf), 0);
                            }
                        }
                    }
                }
            }
        }
    }

    close(sockMain);

    return 0;
}
