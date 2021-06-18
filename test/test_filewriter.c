#include <assert.h>
#include <string.h>
#include <sys/stat.h>

#include "unixkit.h"

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "w");
    assert(f);
    fclose(f);
    struct stat sb;
    assert(stat(argv[1], &sb) == 0);
    ino_t ino = sb.st_ino;
    mode_t mode = 0642;
    char content[] = "test";
    unixkit_filewriter_t *fw = unixkit_filewriter_open(argv[1], mode);
    assert(fw);
    f = unixkit_filewriter_get_stream(fw);
    fputs(content, f);
    assert(unixkit_filewriter_close(fw));
    assert(stat(argv[1], &sb) == 0);
    assert(ino != sb.st_ino);
    assert((sb.st_mode & 0777) == mode);
    f = fopen(argv[1], "r");
    assert(f);
    char buf[128];
    size_t count = fread(buf, 1, sizeof buf, f);
    fclose(f);
    assert(count == sizeof content - 1);
    assert(!memcmp(content, buf, count));
}
