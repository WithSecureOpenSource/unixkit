#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "unixkit.h"

int main()
{
    int fd = open("/dev/null", O_RDONLY);
    assert(fd);
    char *fd_path = unixkit_get_fd_path(fd);
    if (fd_path) {
        assert(!strcmp(fd_path, "/dev/null"));
        fsfree(fd_path);
    } else
        assert(errno == ENOSYS);

    return EXIT_SUCCESS;
}
