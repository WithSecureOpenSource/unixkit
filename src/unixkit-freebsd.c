#include "unixkit-posix.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

char *unixkit_get_fd_path(int fd)
{
    errno = ENOSYS;
    return NULL;
}

bool unixkit_get_peer_credentials(int socket, uid_t *uid, gid_t *gid)
{
    return !getpeereid(socket, uid, gid);
}

int unixkit_unix_listen(const char *path, mode_t mode)
{
    return unixkit_unix_listen_posix(path, mode);
}

bool unixkit_rename(const char *old, const char *new)
{
    struct stat sb;
    if (stat(new, &sb) < 0 && errno == ENOENT)
        return !rename(old, new);
    errno = EEXIST;
    return false;
}

bool unixkit_renameat(int old_dirfd,
                      const char *old,
                      int new_dirfd,
                      const char *new)
{
    struct stat sb;
    if (fstatat(new_dirfd, new, &sb, 0) < 0 && errno == ENOENT)
        return !renameat(old_dirfd, old, new_dirfd, new);
    errno = EEXIST;
    return false;
}

bool unixkit_pipe(int pipefd[2])
{
    return !pipe2(pipefd, O_CLOEXEC);
}

bool unixkit_socketpair(int domain, int type, int protocol, int pairfd[2])
{
    return !socketpair(domain, type | SOCK_CLOEXEC, protocol, pairfd);
}
