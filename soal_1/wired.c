#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>
#include "protocol.h"

int clients[MAX_CLIENTS];
char usernames[MAX_CLIENTS][50];
time_t server_start;

int username_exists(char *name)
{
    for(int i = 0; i < MAX_CLIENTS; i++)
        if(strcmp(usernames[i], name) == 0)
            return 1;
    return 0;
}

void broadcast(char *msg, int sender)
{
    for(int i = 0; i < MAX_CLIENTS; i++)
        if(clients[i] != 0 && clients[i] != sender)
            send(clients[i], msg, strlen(msg), 0);
}

void remove_client(int sock)
{
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        if(clients[i] == sock)
        {
            clients[i] = 0;
            usernames[i][0] = '\0';
        }
    }
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    fd_set readfds;
    int addrlen = sizeof(address);

    server_start = time(NULL);

    for(int i=0;i<MAX_CLIENTS;i++)
    {
        clients[i]=0;
        usernames[i][0]='\0';
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);

    printf("Server running...\n");

    while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        int max_sd = server_fd;

        for(int i=0;i<MAX_CLIENTS;i++)
        {
            int sd = clients[i];
            if(sd > 0) FD_SET(sd, &readfds);
            if(sd > max_sd) max_sd = sd;
        }

        select(max_sd+1, &readfds, NULL, NULL, NULL);

        // === CLIENT BARU ===
        if(FD_ISSET(server_fd, &readfds))
        {
            new_socket = accept(server_fd,(struct sockaddr*)&address,(socklen_t*)&addrlen);

            char name[50];
            int len = recv(new_socket, name, sizeof(name)-1, 0);
            if(len <= 0) { close(new_socket); continue; }

            name[len] = '\0';

            if(username_exists(name))
            {
                char msg[]="[System] Username already used.\n";
                send(new_socket,msg,strlen(msg),0);
                close(new_socket);
            }
            else
            {
                for(int i=0;i<MAX_CLIENTS;i++)
                {
                    if(clients[i]==0)
                    {
                        clients[i]=new_socket;
                        strcpy(usernames[i],name);
                        break;
                    }
                }

                char welcome[100];
                sprintf(welcome,"Welcome to The Wired, %s\n",name);
                send(new_socket,welcome,strlen(welcome),0);

                char msg[100];
                sprintf(msg,"[System] %s has connected to The Wired.\n",name);
                broadcast(msg,new_socket);
            }
        }

        // === CHAT ===
        for(int i=0;i<MAX_CLIENTS;i++)
        {
            int sd = clients[i];

            if(sd > 0 && FD_ISSET(sd, &readfds))
            {
                char buffer[BUFFER_SIZE];
                int valread = recv(sd, buffer, BUFFER_SIZE-1, 0);

                if(valread <= 0)
                {
                    char msg[100];
                    sprintf(msg,"[System] %s has disconnected.\n",usernames[i]);
                    broadcast(msg,sd);

                    close(sd);
                    remove_client(sd);
                }
                else
                {
                    buffer[valread]='\0';

                    char sender[50];
                    strcpy(sender,usernames[i]);

                    // === ADMIN ===
                    if(strcmp(sender,"The Knights")==0)
                    {
                        if(strcmp(buffer,"/active")==0)
                        {
                            int count=0;
                            for(int j=0;j<MAX_CLIENTS;j++)
                                if(clients[j]!=0) count++;

                            char msg[100];
                            sprintf(msg,"Active users: %d\n",count);
                            send(sd,msg,strlen(msg),0);
                        }
                        else if(strcmp(buffer,"/uptime")==0)
                        {
                            int sec = (int)(time(NULL)-server_start);

                            char msg[100];
                            sprintf(msg,"Uptime: %d sec\n",sec);
                            send(sd,msg,strlen(msg),0);
                        }
                        else if(strcmp(buffer,"/shutdown")==0)
                        {
                            printf("Server shutdown by admin\n");
                            exit(0);
                        }
                    }
                    else
                    {
                        if(strcmp(buffer,"/exit")==0)
                        {
                            char msg[100];
                            sprintf(msg,"[System] %s disconnected.\n",sender);
                            broadcast(msg,sd);

                            close(sd);
                            remove_client(sd);
                        }
                        else
                        {
                            char msg[BUFFER_SIZE];
                            sprintf(msg,"[%s]: %s",sender,buffer);
                            broadcast(msg,sd);
                        }
                    }
                }
            }
        }
    }
}
