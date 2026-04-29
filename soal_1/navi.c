#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "protocol.h"

int sock;

void *receive_message(void *arg)
{
    char buffer[BUFFER_SIZE];

    while(1)
    {
        int valread = recv(sock, buffer, BUFFER_SIZE-1, 0);
        if(valread > 0)
        {
            buffer[valread] = '\0';
            printf("%s\n", buffer);
        }
    }
}

int main()
{
    struct sockaddr_in server_addr;
    pthread_t thread_id;
    char name[50];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    //  ADMIN PASSWORD DULU
    if(strcmp(name, "The Knights") == 0)
    {
        char password[50];

        printf("Enter password: ");
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = 0;

        if(strcmp(password, ADMIN_PASSWORD) != 0)
        {
            printf("Wrong password!\n");
            return 0;
        }

        printf("1. Check Active Entities\n");
        printf("2. Check Server Uptime\n");
        printf("3. Execute Emergency Shutdown\n");
        printf("4. Disconnect\n");
    }

    //  BARU KIRIM KE SERVER
    send(sock, name, strlen(name), 0);

    pthread_create(&thread_id, NULL, receive_message, NULL);

    while(1)
    {
        char message[BUFFER_SIZE];
        scanf(" %[^\n]", message);

        if(strcmp(name,"The Knights")==0)
        {
            if(strcmp(message,"1")==0) strcpy(message,"/active");
            else if(strcmp(message,"2")==0) strcpy(message,"/uptime");
            else if(strcmp(message,"3")==0) strcpy(message,"/shutdown");
            else if(strcmp(message,"4")==0) strcpy(message,"/exit");
        }

        send(sock, message, strlen(message), 0);

        if(strcmp(message,"/exit")==0)
        {
            close(sock);
            break;
        }
    }
}
