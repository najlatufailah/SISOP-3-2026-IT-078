#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define ADMIN_PASSWORD "protocol7"

void log_history(const char *role, const char *message);

#endif
