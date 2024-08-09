#include <sys/socket.h>
#include <sys/types.h>
#include <sys/queue.h>
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
#include <pthread.h>
#include <time.h>
#include <signal.h>

void receive_send_method(void* socket_arg);

#define SERVER_PORT 9000
#define BUFSIZE 100
#define SIG SIGRTMIN

char* file_name = "/var/tmp/aesdsocketdata";
char* daemon_arg = "-d";
int accept_socket, server_socket;
int run_as_daemon = 0;
pthread_mutex_t data_mutex;
pthread_mutex_t list_mutex;

struct thread_element
{
    pthread_t thread_id;
    int finished;
    int socket;
    LIST_ENTRY(thread_element) pointers;
};

LIST_HEAD(thread_list, thread_element);

static void timer_thread(union sigval sigval)
{
    char outstr[200];
    outstr[0] = 't';
    outstr[1] = 'i';
    outstr[2] = 'm';
    outstr[3] = 'e';
    outstr[4] = 's';
    outstr[5] = 't';
    outstr[6] = 'a';
    outstr[7] = 'm';
    outstr[8] = 'p';
    outstr[9] = ':';

    int result;
    time_t t;
    struct tm *tmp;
    t = time(NULL);
    tmp = localtime(&t);

    strftime(outstr+10, sizeof(outstr)-10, "%Y%m%d%H%M%S\n", tmp);
    pthread_mutex_lock(&data_mutex);

    FILE * fd;
    fd = fopen(file_name, "a");
    if(fd < 0)
    {
        if (run_as_daemon == 0)
        {
            printf("cannot create file %s\n", file_name);
        }
        fclose(fd);
        return;
    }
    else
    {
        result = fwrite(outstr, sizeof(char), strlen(outstr), fd);

        if(result < 0)
        {
            printf("Write file failed\n");
        }
        fclose(fd);
    }

    pthread_mutex_unlock(&data_mutex);
}

void signal_handler(int signal)
{
    if (signal == SIGALRM)
    {
        char outstr[200];
        printf("ccc\n");
        time_t t;
        printf("ddd\n");
        struct tm *tmp;
        printf("eee\n");
        t = time(NULL);
        printf("ffff\n");
        tmp = localtime(&t);
        printf("aaaa\n");
        strftime(outstr, sizeof(outstr), "%Y%m%d%H%M%S", tmp);
        printf("%s\n", outstr);
    }
    else
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
}

int main(int argc, char *argv[])
{
    char outstr[200];
    printf("ccc\n");
    time_t t;
    printf("ddd\n");
    struct tm *tmp;
    printf("eee\n");
    t = time(NULL);
    printf("ffff\n");
    tmp = localtime(&t);
    printf("aaaa\n");
    strftime(outstr, sizeof(outstr), "%Y%m%d%H%M%S", tmp);
    // year, month, day, hour (in 24 hour format) minute and second
    printf("bbbb/n");
    printf("%s\n", outstr);

    struct sigevent sev;
    timer_t timerid;
    struct itimerspec its;
    int clock_id = CLOCK_MONOTONIC;
    memset(&sev, 0, sizeof(struct sigevent));
    
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_value.sival_ptr = &timerid;
    sev.sigev_notify_function = timer_thread;
    if (timer_create(clock_id, &sev, &timerid) == -1)
    {
        printf("create timer failed\n");
        return -1;
    }

    its.it_value.tv_sec = 10;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    if (timer_settime(timerid, 0, &its, NULL) == -1)
    {
        printf("set timer failed\n");
        return -1;
    }

    struct thread_list head;
    LIST_INIT(&head);

    struct thread_element *element, *pointer;

    openlog(NULL, 0, LOG_USER);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGKILL, signal_handler);

    if (argc == 2)
    {
        if (0 == strcmp(argv[1], daemon_arg))
        {
            printf("run as daemon\n");
            run_as_daemon = 1;
        }
    }

    printf("Server started\n");
    const char localhost[] = "0.0.0.0";
    int result, client_address_len;
    struct sockaddr_in address, client_address;
    struct hostent *host;
    bzero((char *) &address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons (SERVER_PORT);

    remove(file_name);
  
    if((host = gethostbyname(localhost)) == NULL)
    {
        printf("Cannot get localhost address\n");
        return -1;
    }

    memcpy(&address.sin_addr, host->h_addr_list[0], host->h_length);

    server_socket = socket(AF_INET , SOCK_STREAM , 0);
	
    int enable = 1;
    result = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    if(result < 0)
    {
        close(server_socket);
        printf("Cannot set SO_REUSEADDR option\n");
        return -1;
    }

    result = setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable));

    if(result < 0)
    {
        close(server_socket);
        printf("Cannot set SO_REUSEPORT option\n");
        return -1;
    }

	if (server_socket == -1)
	{
        close(server_socket);
		printf("Socket not created\n");
        return -1;
	}

    result = bind(server_socket, (struct sockaddr *) &address, sizeof (address));

	if (result == -1)
	{
        close(server_socket);
		printf("Bind failed\n");
        return -1;
	}

    result = listen(server_socket, 5);

	if (result == -1)
	{
        close(server_socket);
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

        element = malloc(sizeof(struct thread_element));
        element->finished = 0;
        element->socket = accept_socket;
        LIST_INSERT_HEAD(&head, element, pointers);
        result = pthread_create(&element->thread_id, NULL, receive_send_method, (void *)element);
        if(result != 0)
        {
            printf("Thread create error");
            //TODO cleanup
        }

        pthread_mutex_lock(&list_mutex);
        LIST_FOREACH(pointer, &head, pointers)
        {
            if (pointer->finished == 1)
            {
                pthread_join(pointer->thread_id, NULL);
                pointer->finished = 0;
            }
        }
        pthread_mutex_unlock(&list_mutex);

      /*  (accept_socket);
          pthread_t hello_world_thread;
    int result = pthread_create(&hello_world_thread, NULL, hello_world, (void *) argv[0]);
*/
    }
    closelog();
    close(server_socket);

    return 0;
}

void receive_send_method(void* element)
{
    char received_message[BUFSIZE];
    int result_size;
    int result;
    int socket = ((struct thread_element*)element)->socket;

        // read
    pthread_mutex_lock(&data_mutex);
    FILE * fd;
    fd = fopen(file_name, "a");
    if(fd < 0)
    {
        if (run_as_daemon == 0)
        {
            printf("cannot create file %s\n", file_name);
        }
        closelog();
        close(socket);
        return -1;
    }
    else
    {
        for (;;)
        {
            bzero(received_message, BUFSIZE);

            result_size = read(socket, received_message, BUFSIZE);
            result = fwrite(received_message, sizeof(char), result_size, fd);

            if(result < 0)
            {
                if (run_as_daemon == 0)
                {
                    printf("write failed\n");
                }
                close(fd);
                closelog();
                close(socket);
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
        close(socket);
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
                    printf("file read is finished\n");
                }
                break;
            }

            result = send(socket, read_line, result_size, 0);
    
            if(result < 0)
            {
                if (run_as_daemon == 0)
                {
                    printf("send failed\n");
                }
                close(fd);
                closelog();
                close(socket);
                return -1;
            }
        }
    
        fclose(fp);
        if (read_line)
        {
            free(read_line);
        }
    }
    close(socket);
    pthread_mutex_unlock(&data_mutex);

    pthread_mutex_lock(&list_mutex);
    ((struct thread_element*)element)->finished = 1;
    pthread_mutex_unlock(&list_mutex);
    syslog(LOG_DEBUG, "Closed connection from");// %s", client_add);
}