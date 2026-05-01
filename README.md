# SISOP-3-2026-IT-078

# Soal-1 #
**protocol.h**
File protocol.h berfungsi sebagai:
* temoat menyimpan konstanta global
* interface untuk fungsi logging
* penghubung antar file dalam program
```c
#ifndef PROTOCOL_H
#define PROTOCOL_H
  
#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define ADMIN_NAME "Najla"
#define ADMIN_PASSWORD "protocol7"

#define TYPE_NORMAL 0
#define TYPE_ADMIN 1

void log_history(const char *role, const char *message);

#endif

```
**protocol.c**
Pada file protocol.h ini mencatat sistem file log_history, lalu membuka filenya,jika gagal dibuka maka program langsung keluar, dan juga ditambhkan kode untuk ambil waktu saat ini dalam bentuk timestamp, timestamp menjadi (tahun,bulan,hari,jam,menit) jika di print.

```c
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
```
**wired.c**


library yang dipakai dalam wired.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>
#include "protocol.h"
```
berfungsi untuk menyimpan socket client, menyimpan nama user, waktu server mulai
```c
int clients[MAX_CLIENTS];
char usernames[MAX_CLIENTS][50];
time_t server_start;
```
mengecek apakah username sudah pernah dipakai, dan bekerja dengan cara looping, dan lalu mengirim pesan ke semua client kecuali pengirim, dan menghapus client saat disconnect
```c
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

```
bagian ini digunakan untuk menyimpan variabel utama yang dibutuhkan server
```c
int server_fd, new_socket;
struct sockaddr_in address;
fd_set readfds;
int addrlen = sizeof(address);
```
pada server_start digunakan untuk mencatat waktu awal server berjalan(dipakai untuk fitur uptime), dan loop digunakan untuk mengosongkan daftar client, memastikan tidak ada data lama tersisa
```c
server_start = time(NULL);

for(int i=0;i<MAX_CLIENTS;i++)
{
    clients[i]=0;
    usernames[i][0]='\0';
}
```
lalu membuat socket TCP yang akan digunakan sebagai server untuk menerima koneksi dari client
```c
server_fd = socket(AF_INET, SOCK_STREAM, 0);
```
lalu konfigurasi adders
```c
address.sin_family = AF_INET;
address.sin_addr.s_addr = INADDR_ANY;
address.sin_port = htons(PORT);
```
pada kode ini *bind* untuk menghubungkan socket ke alamat dan port dan *listen* membuat server siap menerima koneksi
```c
bind(server_fd, (struct sockaddr *)&address, sizeof(address));
listen(server_fd, 10);
```
lalu masuk loop utama server 
```c
while(1)
{
```
setup select() untuk multi client
* FD_ZERO untuk mengosongkan set
* FD_SET untuk menambahkan server socket
* max_sd menyimpan nilai descriptor terbesar
```c
FD_ZERO(&readfds);
FD_SET(server_fd, &readfds);

int max_sd = server_fd;
```
lalu menambahkan semua client ke select, yang mana loop digunakan untuk memasukkan semua client aktif ke dalam select dan menentukan nilai maksimum descriptor
```c
for(int i=0;i<MAX_CLIENTS;i++)
{
    int sd = clients[i];
    if(sd > 0) FD_SET(sd, &readfds);
    if(sd > max_sd) max_sd = sd;
}
```
fungsi select() akan menunggu hingga aktivitas pada socket dan bisa dari server client baru atau client chat
```c
select(max_sd+1, &readfds, NULL, NULL, NULL);
```
jika server socket aktif bearti ada client masuk dan akan membuat socket baru khusus client tersebut
```c
if(FD_ISSET(server_fd, &readfds))
{
    new_socket = accept(server_fd,(struct sockaddr*)&address,(socklen_t*)&addrlen);
```
sever meerima username dari client dan ditambahkan \0 agar menajdi string valid
```c
char name[50];
int len = recv(new_socket, name, sizeof(name)-1, 0);
name[len] = '\0';
```
lalu validasi username, server menolak client dan koneksi ditutup
```c
if(username_exists(name))
```
lalu menambahkan client baru, menyimpan socket dan username
```c
clients[i]=new_socket;
strcpy(usernames[i],name);
```
server mengirim pesan sambutan ke client baru
```c
sprintf(welcome,"Welcome to The Wired, %s\n",name);
send(new_socket,welcome,strlen(welcome),0);
```
broadcast user join, menginformasikan ke semua client bahwa user baru telah bergabung
```c
sprintf(msg,"[System] %s has connected to The Wired.\n",name);
broadcast(msg,new_socket);
```
lalu hendling pesan client (server menerima pesan dari client) dan jika client terputus server broadcast pesan disconnect dan menghapus client dari daftar
```c
int valread = recv(sd, buffer, BUFFER_SIZE-1, 0);
if(valread <= 0)
```
jika client adalah admin maka server akan memproses command khusus , jika bukan admin pesan dikrim ke semua client
```c
if(strcmp(sender,"The Knights")==0)
sprintf(msg,"[%s]: %s",sender,buffer);
broadcast(msg,sd);
```
**navi.c**
libary yang dipakai pada navi.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "protocol.h"
```
variabel global yang menyimpan socket client untuk komunikasi dengan server
```c
int sock;
```
fungsi ini berjalan sebgai thread yang bertugas untuk menerima pesan dari server secara terus-menurus dan menampilkan pesan ke layar
```c
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
```
lalau deklarasi variable di main
* server_addr = menyimpan alamat server
* thread_id = idenrifier thread
* name = menyimpan username
```c
struct sockaddr_in server_addr;
pthread_t thread_id;
char name[50];
```
membuat socket TCP untuk client
```c
sock = socket(AF_INET, SOCK_STREAM, 0);
```
lalu kongfigurasi server
```c
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(PORT);
inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
```
client mencoba terhubung ke server
```c
connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
```
lalu input username
```c
printf("Enter your name: ");
fgets(name, sizeof(name), stdin);
name[strcspn(name, "\n")] = 0;
```
validasi admin dan password yang mana jika username adalah admin, client akan meminta password terlebih dahalu, jika password salah maka program langsung berhenti. jika benar akan menampilkan menu khusus untuk admin
```c
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
```
lalu mengerim usernmae ke server setelah validasi selesai dan membuat thread untuk menjlankan fungsi receive_message
```c
send(sock, name, strlen(name), 0);
pthread_create(&thread_id, NULL, receive_message, NULL);
```
loop untuk membaca input user dan mengirim pesan ke server
```c
while(1)
{
    char message[BUFFER_SIZE];
    scanf(" %[^\n]", message);
```
mapping menu admin ke RPC 
```c
if(strcmp(name,"The Knights")==0)
{
    if(strcmp(message,"1")==0) strcpy(message,"/active");
    else if(strcmp(message,"2")==0) strcpy(message,"/uptime");
    else if(strcmp(message,"3")==0) strcpy(message,"/shutdown");
    else if(strcmp(message,"4")==0) strcpy(message,"/exit");
}
```
lalu mengirim pesan(chat atau command) ke server
```c
send(sock, message, strlen(message), 0);
```
dan keluar dari program, jika user memilih kelua maka socket ditutup dan program berhenti
```c
if(strcmp(message,"/exit")==0)
{
    close(sock);
    break;
}
```

