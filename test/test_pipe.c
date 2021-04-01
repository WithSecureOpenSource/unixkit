#include "unixkit.h"
#include <assert.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    int pipefd[2];
    assert(unixkit_pipe(pipefd));
    int flags = fcntl(pipefd[0], F_GETFD, 0);
    assert(flags & FD_CLOEXEC);
    flags = fcntl(pipefd[1], F_GETFD, 0);
    assert(flags & FD_CLOEXEC);
    char c = '0';
    assert(write(pipefd[1], &c, 1) == 1);
    assert(read(pipefd[0], &c, 1) == 1);
    assert(c == '0');
    return EXIT_SUCCESS;
}
