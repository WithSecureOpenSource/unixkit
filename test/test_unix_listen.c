#include "unixkit.h"

#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    unlink(argv[1]);
    umask(S_IRWXG);
    mode_t mode = S_IRWXU | S_IRWXG;
    int fd = unixkit_unix_listen(argv[1], mode);
    assert(fd >= 0);
    struct stat statbuf;
    assert(stat(argv[1], &statbuf) == 0);
    mode_t file_mode = statbuf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    assert(file_mode == mode);
    return EXIT_SUCCESS;
}
