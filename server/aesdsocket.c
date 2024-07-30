#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<string.h>
#include<netdb.h>
#include<syslog.h>
#include<fcntl.h>
#include<unistd.h>

#define SERVER_PORT 9000
#define BUFSIZE 100

int main(int argc, char *argv[])
{
    printf("Server started\n");
    const char localhost[] = "localhost";
    int server_socket, result, client_address_len, result_size, accept_socket;
    struct sockaddr_in address, client_address;
    struct hostent *host;
    bzero((char *) &address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons (SERVER_PORT);
    char* file_name = "/var/tmp/aesdsocketdata";

    char received_message[BUFSIZE];

    remove(file_name);

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

for(int i = 0; i < 5; i++)
{
    client_address_len = sizeof(client_address);
    accept_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len);
    if (result < 0)
    {
		printf("Accept failed\n");
        close(accept_socket);
        return -1;
    }

    openlog(NULL, 0, LOG_USER);

    char *client_add = inet_ntoa(client_address.sin_addr);
    syslog(LOG_DEBUG, "Accepted connection from %s", client_add);
    printf("Accepted connection from %s\n", client_add);

    printf("Start\n\n"); 
    // read
    FILE * fd;
    fd = fopen(file_name, "a");
    //open(file_name, O_CREAT|O_WRONLY|O_TRUNC, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);
    if(fd < 0)
    {
        printf("cannot create file %s\n", file_name);
        closelog();
        close(accept_socket);
        return -1;
    }
    else
    {
        for (;;)
        {
            bzero(received_message, BUFSIZE);

            result_size = read(accept_socket, received_message, BUFSIZE);
            if (result_size!=0)
            {
                printf ("DATA size: %d %d\n", result_size, i);
            }
            result = fwrite(received_message, sizeof(char), result_size, fd);

            //result = write(fd, received_message, result_size);

            if(result < 0)
            {
                printf("write failed\n");
                close(fd);
                closelog();
                close(accept_socket);
                return -1;
            }

            if (received_message[result_size - 1] == '\n')
            {
                printf("data finished\n");
                break;
            }
        }

        fclose(fd);
        closelog();
    }

    // send
    //fd = open(file_name, O_RDONLY | O_TRUNC, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);
printf("SEND\n");
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    fp = fopen(file_name, "r");
    if(fp < 0)
    {
        printf("111\n");
        printf("cannot open file %s\n", file_name);
        closelog();
        close(accept_socket);
        return -1;
    }
    else
    {
        printf("2222\n");
        for (;;)
        {
            //bzero(received_message, BUFSIZE);
            //result_size = read(fd, received_message, BUFSIZE);
            result_size = getline(&line, &len, fp);
            printf("size %d\n", result_size);
            if (result_size == -1)
            {
                printf("end of file\n");
                break;
            }
            for(int i = 0; i < result_size; i++)
            {
                printf ("%c", line[i]);
            }
            printf ("\n\n");
            printf ("%s %dfff\n", received_message, result_size);

            result = send(accept_socket, line, result_size, 0);

            if(result < 0)
            {
                printf("send failed\n");
                close(fd);
                closelog();
                close(accept_socket);
                return -1;
            }

            /*if (received_message[result_size - 1] == '\n')
            {
                printf("data finished\n");
                
                break;
            }*/
            
        }
        fclose(fp);
        if (line)
            free(line);
        

    }
        close(accept_socket);
    //sleep(2);
}
    closelog();

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