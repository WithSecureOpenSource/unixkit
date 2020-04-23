#define _GNU_SOURCE

#include <fsdyn/fsalloc.h>

#include <errno.h>
#include <limits.h>
#include <linux/fs.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/un.h>
#include <unistd.h>

static bool resolve_path(const char *path, char *buf, size_t len)
{
    ssize_t count = readlink(path, buf, len);
    if (count < 0 || count >= len)
        return false;
    buf[count] = '\0';
    return true;
}

char *unixkit_get_fd_path(int fd)
{
    char proc_path[64];
    sprintf(proc_path, "/proc/self/fd/%d", fd);
    char *buf = fsalloc(PATH_MAX);
    if (resolve_path(proc_path, buf, PATH_MAX))
        return buf;
    int err = errno;
    fsfree(buf);
    errno = err;
    return NULL;
}

bool unixkit_get_peer_credentials(int socket, uid_t *uid, gid_t *gid)
{
    struct ucred peer_credentials;
    socklen_t option_len = sizeof peer_credentials;
    if (getsockopt(socket,
                   SOL_SOCKET,
                   SO_PEERCRED,
                   &peer_credentials,
                   &option_len)
        == 0) {
        *uid = peer_credentials.uid;
        *gid = peer_credentials.gid;
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
    if (fchmod(fd, mode) != 0) {
        int error = errno;
        close(fd);
        errno = error;
        return -1;
    }
    mode_t old_umask = umask(0);
    int ret = bind(fd, (struct sockaddr *)&address, sizeof address);
    umask(old_umask);
    if (ret != 0 || listen(fd, 128) != 0) {
        int error = errno;
        close(fd);
        errno = error;
        return -1;
    }
    return fd;
}

/* CentOS 6 linux headers are too old and lack these definitions. */
#ifndef SYS_renameat2
#ifdef __x86_64__
#define SYS_renameat2 316
#else
#define SYS_renameat2 353
#endif
#endif

#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif

#ifndef RENAME_NOREPLACE
#define RENAME_NOREPLACE (1 << 0)
#endif

bool unixkit_rename(const char *old, const char *new)
{
    if (!syscall(SYS_renameat2, AT_FDCWD, old, AT_FDCWD, new, RENAME_NOREPLACE))
        return true;
    if (errno == ENOSYS || errno == EINVAL) {
        struct stat sb;
        if (stat(new, &sb) < 0 && errno == ENOENT)
            return !rename(old, new);
        errno = EEXIST;
    }
    return false;
}

bool unixkit_renameat(int old_dirfd,
                      const char *old,
                      int new_dirfd,
                      const char *new)
{
    if (!syscall(
            SYS_renameat2, old_dirfd, old, new_dirfd, new, RENAME_NOREPLACE))
        return true;
    if (errno == ENOSYS || errno == EINVAL) {
        struct stat sb;
        if (fstatat(new_dirfd, new, &sb, 0) < 0 && errno == ENOENT)
            return !renameat(old_dirfd, old, new_dirfd, new);
        errno = EEXIST;
    }
    return false;
}
