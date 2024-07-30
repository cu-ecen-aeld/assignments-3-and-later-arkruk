#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define SERVER_PORT 9000
#define BUFSIZE 100
char* file_name = "/var/tmp/aesdsocketdata";
char* daemon_arg = "-d";
int accept_socket, server_socket;
int run_as_daemon = 0;

void signal_handler(int signal)
{
    if (run_as_daemon == 0)
    {
        printf("signal receive\n");
    }
    remove(file_name);
    syslog(LOG_DEBUG, "Caught signal, exiting");
    close(accept_socket);
    close(server_socket);
    closelog();
    exit(0);
}

int main(int argc, char *argv[])
{
    openlog(NULL, 0, LOG_USER);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (argc == 2)
    {
        if (0 == strcmp(argv[1], daemon_arg))
        {
            printf("run as daemon\n");
            run_as_daemon = 1;
        }
    }

    printf("Server started\n");
    const char localhost[] = "localhost";
    int server_socket, result, client_address_len, result_size;
    struct sockaddr_in address, client_address;
    struct hostent *host;
    bzero((char *) &address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons (SERVER_PORT);

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

    if (run_as_daemon == 1)
    {

        pid_t  pid = fork();

        if (pid < 0)
        {
            exit(EXIT_FAILURE);
        }

        if (pid > 0)
        {
            exit(EXIT_SUCCESS);
        }

    }

    for(;;)
    {
        client_address_len = sizeof(client_address);
        accept_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len);
        if (result < 0)
        {
            if (run_as_daemon == 0)
            {
                printf("Accept failed\n");
            }
            close(accept_socket);
            close(server_socket);
            return -1;
        }

        char *client_add = inet_ntoa(client_address.sin_addr);
        syslog(LOG_DEBUG, "Accepted connection from %s", client_add);
        if (run_as_daemon == 0)
        {
            printf("Accepted connection from %s\n", client_add);
        }

        // read

        FILE * fd;
        fd = fopen(file_name, "a");
        if(fd < 0)
        {
            if (run_as_daemon == 0)
            {
                printf("cannot create file %s\n", file_name);
            }
            closelog();
            close(accept_socket);
            close(server_socket);
            return -1;
        }
        else
        {
            for (;;)
            {
                bzero(received_message, BUFSIZE);
    
                result_size = read(accept_socket, received_message, BUFSIZE);
                result = fwrite(received_message, sizeof(char), result_size, fd);
    
                if(result < 0)
                {
                    if (run_as_daemon == 0)
                    {
                        printf("write failed\n");
                    }
                    close(fd);
                    closelog();
                    close(accept_socket);
                    close(server_socket);
                    return -1;
                }
    
                if (received_message[result_size - 1] == '\n')
                {
                    if (run_as_daemon == 0)
                    {
                        printf("data finished\n");
                    }
                    break;
                }
            }

            fclose(fd);
            closelog();
        }

        // send
        FILE * fp;
        char * read_line = NULL;
        size_t line_size = 0;
        fp = fopen(file_name, "r");
        if(fp < 0)
        {
            if (run_as_daemon == 0)
            {
                printf("cannot open file %s\n", file_name);
            }
            closelog();
            close(accept_socket);
            close(server_socket);
            return -1;
        }
        else
        {
            for (;;)
            {
                result_size = getline(&read_line, &line_size, fp);
                if (result_size == -1)
                {
                    if (run_as_daemon == 0)
                    {
                        printf("end of file\n");
                    }
                    break;
                }
    
                result = send(accept_socket, read_line, result_size, 0);
    
                if(result < 0)
                {
                    if (run_as_daemon == 0)
                    {
                        printf("send failed\n");
                    }
                    close(fd);
                    closelog();
                    close(accept_socket);
                    close(server_socket);
                    return -1;
                }
            }
    
            fclose(fp);
            if (read_line)
            {
                free(read_line);
            }
        }
        close(accept_socket);
        syslog(LOG_DEBUG, "Closed connection from %s", client_add);
    }
    closelog();
    close(server_socket);

    return 0;
}
