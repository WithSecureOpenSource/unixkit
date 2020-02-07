#include "unixkit.h"
#include <fsdyn/integer.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>

int main()
{
    list_t *open_fds = unixkit_get_open_fds();
    assert(open_fds);
    list_elem_t *elem = list_get_first(open_fds);
    while (elem) {
        int fd = as_intptr(list_elem_get_value(elem));
        fprintf(stderr, "fd: %d\n", fd);
        elem = list_next(elem);
    }
    list_t *keep_fds = make_list();
    list_append(keep_fds, as_integer(0));
    list_append(keep_fds, as_integer(1));
    list_append(keep_fds, as_integer(2));
    pid_t pid = unixkit_fork(keep_fds);
    assert(pid >= 0);
    if (pid == 0) {
        list_elem_t *elem = list_get_first(open_fds);
        while (elem) {
            int fd = as_intptr(list_elem_get_value(elem));
            int flags = fcntl(fd, F_GETFD, 0);
            if (fd >= 0 && fd <= 2)
                assert(flags >= 0);
            else if (flags >= 0)
                /* allow for valgrind protected file descriptors */
                assert(close(fd) < 0 && errno == EBADF);
            else
                assert(errno == EBADF);
            elem = list_next(elem);
        }
        destroy_list(open_fds);
        _exit(0);
    }
    destroy_list(open_fds);
    for (;;) {
        int wstatus;
        if (waitpid(pid, &wstatus, 0) == pid) {
            assert(WIFEXITED(wstatus));
            break;
        }
        assert(errno == EINTR);
    }
    return EXIT_SUCCESS;
}
