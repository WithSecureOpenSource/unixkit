#include "unixkit.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

static bool create_and_rename(int dirfd, const char *old, const char *new)
{
    int fd = openat(dirfd, old, O_CREAT | O_TRUNC, 0644);
    assert(fd >= 0);
    close(fd);
    return unixkit_renameat(dirfd, old, dirfd, new);
}

int main(int argc, char **argv)
{
    int dirfd = open(argv[1], O_DIRECTORY);
    assert(dirfd >= 0);
    unlinkat(dirfd, "new", 0);
    assert(create_and_rename(dirfd, "old", "new"));
    struct stat sb;
    assert(!fstatat(dirfd, "new", &sb, 0));
    assert(!create_and_rename(dirfd, "old", "new") && errno == EEXIST);
    close(dirfd);
    return EXIT_SUCCESS;
}
