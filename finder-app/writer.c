#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int fd;
    int result;
    if (argc == 1)
    {
        syslog(LOG_ERR, "path and text string not defined");
        return 1;
    }
    else if (argc == 2)
    {
        syslog(LOG_ERR, "text string not defined");
        return 1;
    }
    else
    {
        fd = open(argv[1], O_CREAT|O_RDWR, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);
        if(fd < 0)
        {
            syslog(LOG_ERR, "Cannot open or create file %s", argv[1]);
            return 1;
        }
        else
        {
            result = write(fd, argv[2], strlen(argv[2]));
            if(result < 0)
            {
                syslog(LOG_ERR, "Cannot write to file %s", argv[1]);
                close(fd);
                return 1;
            }
            close(fd);
            syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
        }
    }
    return 0;
}
