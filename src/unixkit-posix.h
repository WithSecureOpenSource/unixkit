#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <unistd.h>

static inline bool unixkit_pipe_posix(int pipefd[2])
{
    if (pipe(pipefd) < 0)
        return false;
    if (fcntl(pipefd[0], F_SETFD, FD_CLOEXEC) < 0 ||
        fcntl(pipefd[1], F_SETFD, FD_CLOEXEC) < 0) {
        int err = errno;
        close(pipefd[0]);
        close(pipefd[1]);
        errno = err;
        return false;
    }
    return true;
}

static inline bool unixkit_socketpair_posix(int domain,
                                            int type,
                                            int protocol,
                                            int pairfd[2])
{
    if (socketpair(domain, type, protocol, pairfd) < 0)
        return false;
    if (fcntl(pairfd[0], F_SETFD, FD_CLOEXEC) < 0 ||
        fcntl(pairfd[1], F_SETFD, FD_CLOEXEC) < 0) {
        int err = errno;
        close(pairfd[0]);
        close(pairfd[1]);
        errno = err;
        return false;
    }
    return true;
}
