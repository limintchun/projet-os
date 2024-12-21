#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>  // struct sockaddr_in, INADDR_ANY

#define DEFAULT_PORT 1234
#define MAX_SIZE 1024
#define MAX_CLIENT 1000
#define MAX_CHAR 30

typedef struct {
	int port;
	struct sockaddr_in addr;
	char pseudo[MAX_CHAR];
	int connect;
	int socket_fd;
} client_t;


int create_socket();
void addClients(int client_fd, struct sockaddr_in client_addr);
void initializeClients();
void searchClient(char message[MAX_SIZE], struct sockaddr_in client_addr);
void initialize_address_data(struct sockaddr_in *address);
int link_to_client(int server_fd, struct sockaddr_in *address);
void* write_thread(void* arg);
void* read_thread(void* arg);

#endif // SERVER_H
