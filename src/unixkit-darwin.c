#include <fsdyn/fsalloc.h>

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ucred.h>
#include <sys/un.h>
#include <unistd.h>

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
    if (getsockopt(socket,
                   SOL_LOCAL,
                   LOCAL_PEERCRED,
                   &peer_credentials,
                   &option_len)
        == 0) {
        *uid = peer_credentials.cr_uid;
        *gid = peer_credentials.cr_gid;
        return true;
    }
    return false;
}

int unixkit_unix_listen(const char *path, mode_t mode)
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
    int ret = bind(fd, (struct sockaddr *)&address, sizeof address);
    umask(old_umask);
    if (ret != 0 || chmod(path, mode) < 0 || listen(fd, 128) != 0) {
        int error = errno;
        close(fd);
        errno = error;
        return -1;
    }
    return fd;
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
