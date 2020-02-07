#include "unixkit.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    const char *pidfile = argv[1];
    intptr_t id;
    pid_t pid = unixkit_daemon_create(make_list(), "/", pidfile, &id);
    assert(pid >= 0);
    if (pid == 0) {
        char *wd = getcwd(NULL, 0);
        assert(!strcmp(wd, "/"));
        assert(unixkit_daemon_notify(id));
        free(wd);
        _exit(0);
    }
    FILE *f = fopen(pidfile, "r");
    assert(f);
    char buf[128];
    size_t count = fread(buf, 1, sizeof buf, f);
    fclose(f);
    assert(count > 0);
    buf[count] = 0;
    long child_pid = strtol(buf, NULL, 0);
    assert(child_pid == pid);
    return EXIT_SUCCESS;
}
