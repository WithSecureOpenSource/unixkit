#include "unixkit.h"
#include <assert.h>
#include <fcntl.h>
#include <string.h>

int main()
{
    int fd = open("/dev/null", O_RDONLY);
    assert(fd);
    char *fd_path = unixkit_get_fd_path(fd);
    assert(!strcmp(fd_path, "/dev/null"));
    fsfree(fd_path);

    return EXIT_SUCCESS;
}
