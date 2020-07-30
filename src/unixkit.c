#include <fsdyn/charstr.h>
#include <fsdyn/fsalloc.h>
#include <fsdyn/integer.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "unixkit.h"

#ifdef __APPLE__
#define FD_DIR "/dev/fd"
#else
#define FD_DIR "/proc/self/fd"
#endif

list_t *unixkit_get_open_fds()
{
    int dirfd = open(FD_DIR, O_RDONLY);
    if (dirfd < 0)
        return NULL;
    DIR *dir = fdopendir(dirfd);
    list_t *open_fds = make_list();
    for (;;) {
        struct dirent *entry = readdir(dir);
        if (!entry)
            break;
        char *end;
        int fd = strtol(entry->d_name, &end, 10);
        if (!*end && fd != dirfd)
            list_append(open_fds, as_integer(fd));
    }
    closedir(dir);
    return open_fds;
}

static void close_open_fds(list_t *keep_fds)
{
    list_t *open_fds = unixkit_get_open_fds();
    while (!list_empty(open_fds)) {
        int fd = as_intptr(list_pop_first(open_fds));
        if (!list_get(keep_fds, as_integer(fd)))
            close(fd);
    }
    destroy_list(open_fds);
}

pid_t unixkit_fork(list_t *keep_fds)
{
    sigset_t newmask, oldmask;
    sigfillset(&newmask);
    pthread_sigmask(SIG_BLOCK, &newmask, &oldmask);
    pid_t pid = fork();
    if (pid == 0) {
        struct sigaction act = {
            .sa_handler = SIG_DFL,
        };
        for (int i = 1; i < NSIG; i++)
            sigaction(i, &act, NULL);
        pthread_sigmask(SIG_SETMASK, &oldmask, NULL);
        close_open_fds(keep_fds);
        destroy_list(keep_fds);
        return pid;
    }
    pthread_sigmask(SIG_SETMASK, &oldmask, NULL);
    destroy_list(keep_fds);
    return pid;
}

static void terminate(pid_t pid)
{
    kill(pid, SIGKILL);
    for (;;) {
        int status;
        if (waitpid(pid, &status, 0) == pid)
            break;
        if (errno != EINTR)
            break;
    }
}

static bool write_pidfile(const char *pidfile)
{
    bool error = false;
    FILE *fp = fopen(pidfile, "w");
    if (!fp)
        return false;
    error |= fprintf(fp, "%ld", (long) getpid()) < 0;
    error |= fclose(fp) == EOF;
    return !error;
}

pid_t unixkit_daemon_create(list_t *keep_fds,
                            const char *workdir,
                            const char *pidfile,
                            intptr_t *id)
{
    assert(id != NULL);
    *id = -1;
    int sync_fd[2];
    if (pipe(sync_fd) < 0)
        return -1;

    list_append(keep_fds, as_integer(sync_fd[1]));
    pid_t pid = unixkit_fork(keep_fds);
    switch (pid) {
        case -1:
            close(sync_fd[0]);
            close(sync_fd[1]);
            return -1;
        case 0:
            break;
        default:
            close(sync_fd[1]);
            int value;
            ssize_t count = read(sync_fd[0], &value, sizeof value);
            close(sync_fd[0]);
            if (count != sizeof value) {
                terminate(pid);
                errno = EPROTO;
                return -1;
            }
            return pid;
    }

    if (setsid() < 0)
        _exit(0);
    umask(022);
    if (!write_pidfile(pidfile))
        _exit(0);
    if (chdir(workdir) < 0)
        _exit(0);
    *id = sync_fd[1];
    return 0;
}

bool unixkit_daemon_notify(intptr_t id)
{
    int value = 0;
    ssize_t count = write(id, &value, sizeof value);
    close(id);
    return count == sizeof value;
}

struct unixkit_filewriter {
    FILE *f;
    char *path;
    char *tmp_path;
};

unixkit_filewriter_t *unixkit_filewriter_open(const char *path,
                                              mode_t mode)
{
    char *tmp_path = charstr_printf("%s.XXXXXX", path);
    int fd = mkstemp(tmp_path);
    if (fd >= 0) {
        assert(fchmod(fd, mode) == 0);
        unixkit_filewriter_t *fw = fsalloc(sizeof *fw);
        fw->f = fdopen(fd, "w");
        fw->path = charstr_dupstr(path);
        fw->tmp_path = tmp_path;
        return fw;
    } else {
        fsfree(tmp_path);
        return NULL;
    }
}

FILE *unixkit_filewriter_get_stream(unixkit_filewriter_t *fw)
{
    return fw->f;
}

static bool sync_file(const char *path)
{
    bool error = true;
    int fd = open(path, O_RDONLY);
    if (fd) {
        error = fsync(fd) < 0;
        close(fd);
    }
    return !error;
}

bool unixkit_filewriter_close(unixkit_filewriter_t *fw)
{
    bool error = ferror(fw->f) != 0;
    error |= fclose(fw->f) == EOF;
    if (!error)
        error = !sync_file(fw->tmp_path);
    if (!error)
        error = rename(fw->tmp_path, fw->path) < 0;
    if (!error)
        error = !sync_file(dirname(fw->path));
    if (error)
        unlink(fw->tmp_path);
    fsfree(fw->path);
    fsfree(fw->tmp_path);
    fsfree(fw);
    return !error;
}

static size_t common_prefix_length(const char *a, const char *b)
{
    size_t len = 0;
    while (*a && *a++ == *b++)
        len++;
    return len;
}

static bool path_starts_with_(const char *path, const char *prefix, size_t len)
{
    return !prefix[len]
        && (!path[len] || path[len] == '/'
            || (len > 0 && prefix[len - 1] == '/'));
}

bool unixkit_path_starts_with(const char *path, const char *prefix)
{
    return path_starts_with_(path, prefix, common_prefix_length(path, prefix));
}

avl_elem_t *unixkit_path_get_lowest_ancestor(const char *path,
                                             avl_tree_t *paths)
{
    /* This somewhat tricky algorithm pays off if there are dozens of
     * path prefixes to match. Given a path P and a set S of paths,
     * the algorithm finds the predecessor P̅ of P in S WRT
     * lexicographical order. There are three cases:
     *
     * 1. P̅ does not exist
     *
     * No path in S is an ancestor of P.
     *
     * 2a. P̅ = P
     *
     * 2b. P = P̅ '/' Y
     *
     * 2c. P̅ = X '/'
     *     P = X '/' Y
     *
     * P̅ is the lowest ancestor of P in S.
     *
     * 3. P = X c Y
     *    P̅ = X c̅ Z
     *    c ≠ c̅
     *
     * If c = '/', no ancestor of P longer than X is in S. The
     * algorithm then recursively finds the lowest ancestor of X in S.
     *
     * If c ≠ '/', let X̅ be the longest prefix of X ending with a '/',
     * or ɛ if X does not contain a '/'. No ancestor of P longer than
     * X̅ is in S. The algorithm then recursively finds the lowest
     * ancestor of X̅ in S.
     */

    char *copy = charstr_dupstr(path);
    for (;;) {
        avl_elem_t *element = avl_tree_get_on_or_before(paths, copy);
        if (!element) {
            fsfree(copy);
            return NULL;
        }
        const char *prefix = avl_elem_get_key(element);
        size_t n = common_prefix_length(prefix, copy);
        if (path_starts_with_(copy, prefix, n)) {
            fsfree(copy);
            return element;
        }
        if (copy[n] != '/')
            while (n > 0 && copy[n - 1] != '/')
                n--;
        copy[n] = '\0';
    }
}

bool unixkit_path_starts_with_any(const char *path, avl_tree_t *paths)
{
    avl_elem_t *element = unixkit_path_get_lowest_ancestor(path, paths);
    return element != NULL;
}
