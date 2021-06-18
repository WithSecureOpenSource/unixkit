#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>

#include "unixkit.h"

int main()
{
    int fd[2];
    int status = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    assert(status >= 0);
    uid_t uid;
    gid_t gid;
    assert(unixkit_get_peer_credentials(fd[1], &uid, &gid));
    assert(uid == getuid());
    assert(gid == getgid());
    return EXIT_SUCCESS;
}
