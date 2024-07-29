#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<stdio.h>
#include<string.h>
#include<netdb.h>

#define SERVER_PORT     9000

int main(int argc, char *argv[])
{
    const char localhost[] = "localhost";
    int server_socket, result, client_address_len;
    struct sockaddr_in address, client_address;
    struct hostent *host;
    address.sin_family = AF_INET;
    address.sin_port = htons (SERVER_PORT);

    if((host = gethostbyname(localhost)) == NULL)
    {
        printf("Cannot get localhost address\n");
        return -1;
    }

    memcpy(&address.sin_addr, host->h_addr_list[0], host->h_length);

    server_socket = socket(AF_INET , SOCK_STREAM , 0);
	
	if (server_socket == -1)
	{
		printf("Socket not created\n");
        return -1;
	}

    result = bind(server_socket, (struct sockaddr *) &address, sizeof (address));

	if (result == -1)
	{
		printf("Bind failed\n");
        return -1;
	}

    result = listen(server_socket, 5);

	if (result == -1)
	{
		printf("Listen failed\n");
        return -1;
	}

    client_address_len = sizeof(client_address);
    result = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len);
    if (result < 0)
    {
		printf("Listen failed\n");
        return -1;
    }

    printf("XXXXXXXXX\n");
    
    return 0;
}
//int listen(int sockfd, int backlog);

       /*#define MY_SOCK_PATH "/somepath"
       #define LISTEN_BACKLOG 50

       #define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

           struct sockaddr_un  my_addr, peer_addr;

           sfd = socket(AF_UNIX, SOCK_STREAM, 0);
           if (sfd == -1)
               handle_error("socket");

           memset(&my_addr, 0, sizeof(my_addr));
           my_addr.sun_family = AF_UNIX;
           strncpy(my_addr.sun_path, MY_SOCK_PATH,
                   sizeof(my_addr.sun_path) - 1);

           if (bind(sfd, (struct sockaddr *) &my_addr,
                    sizeof(my_addr)) == -1)
               handle_error("bind");*/