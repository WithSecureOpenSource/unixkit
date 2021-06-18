#include <assert.h>
#include <fcntl.h>

#include "unixkit.h"

int main(int argc, char **argv)
{
    int pairfd[2];
    assert(unixkit_socketpair(AF_UNIX, SOCK_STREAM, 0, pairfd));
    int flags = fcntl(pairfd[0], F_GETFD, 0);
    assert(flags & FD_CLOEXEC);
    flags = fcntl(pairfd[1], F_GETFD, 0);
    assert(flags & FD_CLOEXEC);
    char c = '0';
    assert(write(pairfd[1], &c, 1) == 1);
    assert(read(pairfd[0], &c, 1) == 1);
    assert(c == '0');
    return EXIT_SUCCESS;
}
