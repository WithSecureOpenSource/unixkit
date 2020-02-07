#include "unixkit.h"
#include <fsdyn/charstr.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>

static bool create_and_rename(const char *old, const char *new)
{
    FILE *f = fopen(old, "w");
    assert(f);
    fclose(f);
    return unixkit_rename(old, new);
}

int main(int argc, char **argv)
{
    char *old = charstr_printf("%s/old", argv[1]);
    char *new = charstr_printf("%s/new", argv[1]);
    unlink(new);
    assert(create_and_rename(old, new));
    struct stat sb;
    assert(!stat(new, &sb));
    assert(!create_and_rename(old, new) && errno == EEXIST);
    fsfree(old);
    fsfree(new);
    return EXIT_SUCCESS;
}
