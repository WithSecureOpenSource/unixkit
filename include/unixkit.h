#ifndef UNIXKIT
#define UNIXKIT

#include <fsdyn/avltree.h>
#include <fsdyn/fsalloc.h>
#include <fsdyn/list.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/* In unixkit APIs, file descriptors are encoded in lists as objects
 * of type integer_t. */

/* Return a list of the file descriptors open in the calling
 * process. */
list_t *unixkit_get_open_fds(void);
/* This function acquires ownership of the list of file descriptors
 * argument. */
pid_t unixkit_fork(list_t *keep_fds);

/*
 * Create a daemon process. The return value of this function is
 * analogous to fork's. The new process is not guaranteed to be a
 * child of the calling process. On success, control is returned to
 * the calling process only after the new daemon process has either
 * terminated or has successfully executed unixkit_daemon_notify with
 * the returned id. On failure, -1 is returned to the calling process
 * and errno is set. This function acquires ownership of the list of
 * file descriptors argument.
 */
pid_t unixkit_daemon_create(list_t *keep_fds,
                            const char *workdir,
                            const char *pidfile,
                            intptr_t *id);
bool unixkit_daemon_notify(intptr_t id);

/* The returned value must be deallocated with fsfree. */
char *unixkit_get_fd_path(int fd);

/*
 * Get the user and group IDs of the peer connected to a UNIX domain
 * socket.
 */
bool unixkit_get_peer_credentials(int socket, uid_t *uid, gid_t *gid);

/*
 * Create a unix domain socket listening on the given path. On
 * failure, -1 is returned and errno is set.
 */
int unixkit_unix_listen(const char *path, mode_t mode);

/* Rename a file. On failure, errno is set. If the new path names a
 * file that already exists, this function fails and sets errno to
 * EEXIST.
 */
bool unixkit_rename(const char *old_path, const char *new_path);
bool unixkit_renameat(int old_dirfd,
                      const char *old_path,
                      int new_dirfd,
                      const char *new_path);

/* pipe() and socketpair() wrappers that set the close-on-exec flag on
 * the allocated file descriptors. */
bool unixkit_pipe(int pipefd[2]);
bool unixkit_socketpair(int domain, int type, int protocol, int pairfd[2]);

/*
 * Write files atomically. If the application calls clearerr, rewind
 * or fclose on the underlying stream the behaviour is undefined.
 */
typedef struct unixkit_filewriter unixkit_filewriter_t;
unixkit_filewriter_t *unixkit_filewriter_open(const char *path,
                                              mode_t mode);
FILE *unixkit_filewriter_get_stream(unixkit_filewriter_t *fw);
bool unixkit_filewriter_close(unixkit_filewriter_t *fw);

bool unixkit_path_starts_with(const char *path, const char *prefix);
/*
 * The avl_tree object must contain strings as keys and use strcmp as
 * comparator.
 */
bool unixkit_path_starts_with_any(const char *path, avl_tree_t *paths);
avl_elem_t *unixkit_path_get_lowest_ancestor(const char *path,
                                             avl_tree_t *paths);

#ifdef __cplusplus
}
#endif

#endif
