#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ucred.h>
#include <sys/un.h>

#include <fsdyn/fsalloc.h>

#include "unixkit-posix.h"

char *unixkit_get_fd_path(int fd)
{
    char *buf = fsalloc(MAXPATHLEN);
    if (fcntl(fd, F_GETPATH, buf) != -1)
        return buf;
    int err = errno;
    fsfree(buf);
    errno = err;
    return NULL;
}

bool unixkit_get_peer_credentials(int socket, uid_t *uid, gid_t *gid)
{
    struct xucred peer_credentials;
    socklen_t option_len = sizeof peer_credentials;
    if (getsockopt(socket, SOL_LOCAL, LOCAL_PEERCRED, &peer_credentials,
                   &option_len) == 0) {
        *uid = peer_credentials.cr_uid;
        *gid = peer_credentials.cr_gid;
        return true;
    }
    return false;
}

int unixkit_unix_listen(const char *path, mode_t mode)
{
    return unixkit_unix_listen_posix(path, mode);
}

bool unixkit_rename(const char *old, const char *new)
{
    if (!renamex_np(old, new, RENAME_EXCL))
        return true;
    if (errno == ENOTSUP) {
        struct stat sb;
        if (stat(new, &sb) < 0 && errno == ENOENT)
            return !rename(old, new);
        errno = EEXIST;
    }
    return false;
}

bool unixkit_renameat(int old_dirfd, const char *old, int new_dirfd,
                      const char *new)
{
    if (!renameatx_np(old_dirfd, old, new_dirfd, new, RENAME_EXCL))
        return true;
    if (errno == ENOTSUP) {
        struct stat sb;
        if (fstatat(new_dirfd, new, &sb, 0) < 0 && errno == ENOENT)
            return !renameat(old_dirfd, old, new_dirfd, new);
        errno = EEXIST;
    }
    return false;
}

bool unixkit_pipe(int pipefd[2])
{
    return unixkit_pipe_posix(pipefd);
}

bool unixkit_socketpair(int domain, int type, int protocol, int pairfd[2])
{
    return unixkit_socketpair_posix(domain, type, protocol, pairfd);
}
