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
    if (argc == 1)
    {
        syslog(LOG_ERR, "path and text string not defined");
    }
    else if (argc == 2)
    {
        syslog(LOG_ERR, "text string not defined");
    }
    else
    {
        fd = open(argv[1], O_CREAT|O_RDWR, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);
        write(fd, argv[2], strlen(argv[2]));
        close(fd);
        syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
    }
    return 0;
}

/*
if [ $# -eq 0 ]; then
    echo "path and text string not defined"
    exit 1
elif [ $# -eq 1 ]; then
    echo "text string not defined"
    exit 1
else
    dir_name=$(dirname $1)
    mkdir -p $dir_name
    echo "$2" > $1
    if [ -f $1 ]; then
        exit 0
    else
        echo "file is not created"
        exit 1
    fi
fi
*/