#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

static inline int unixkit_unix_listen_posix(const char *path, mode_t mode)
{
    struct sockaddr_un address = { .sun_family = AF_UNIX };
    if (strlen(path) >= sizeof address.sun_path) {
        errno = EOVERFLOW;
        return -1;
    }
    strcpy(address.sun_path, path);
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;
    mode_t old_umask = umask(S_IRWXU | S_IRWXG | S_IRWXO);
    int ret = bind(fd, (struct sockaddr *) &address, sizeof address);
    umask(old_umask);
    if (ret != 0 || chmod(path, mode) < 0 || listen(fd, 128) != 0) {
        int error = errno;
        close(fd);
        errno = error;
        return -1;
    }
    return fd;
}

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

static inline bool unixkit_socketpair_posix(int domain, int type, int protocol,
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
