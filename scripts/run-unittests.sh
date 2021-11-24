#!/usr/bin/env bash

set -x

main () {
    cd "$(dirname "$(realpath "$0")")/.."
    if [ -n "$FSARCHS" ]; then
        local archs=()
        IFS=, read -ra archs <<< "$FSARCHS"
        for arch in "${archs[@]}" ; do
            run-tests "$arch"
        done
    else
        local os=$(uname -m -s)
        case $os in
            "Darwin arm64")
                run-tests darwin;;
            "Darwin x86_64")
                run-tests darwin;;
            "FreeBSD amd64")
                run-tests freebsd_amd64;;
            "Linux i686")
                run-tests linux32;;
            "Linux x86_64")
                run-tests linux64;;
            "Linux aarch64")
                run-tests linux64;;
            "OpenBSD amd64")
                run-tests openbsd_amd64;;
            *)
                echo "$0: Unknown OS architecture: $os" >&2
                exit 1
        esac
    fi
}

realpath () {
    # reimplementation of "readlink -fv" for OSX
    python -c "import os.path, sys; print(os.path.realpath(sys.argv[1]))" "$1"
}

run-test () {
    local arch=$1
    shift
    case $arch in
        linux32 | linux64)
            valgrind -q --leak-check=full --error-exitcode=1 "$@"
            ;;
        *)
            "$@"
            ;;
    esac
}

run-tests () {
    local arch=$1
    local test_dir=stage/$arch/build/test
    mkdir -p stage/$arch/workdir &&
    run-test $arch $test_dir/test_daemon stage/$arch/workdir/pidfile &&
    run-test $arch $test_dir/test_filewriter stage/$arch/workdir/test &&
    run-test $arch $test_dir/test_fork &&
    run-test $arch $test_dir/test_get_fd_path &&
    run-test $arch $test_dir/test_get_peer_credentials &&
    run-test $arch $test_dir/test_path &&
    run-test $arch $test_dir/test_pipe &&
    run-test $arch $test_dir/test_rename stage/$arch/workdir &&
    run-test $arch $test_dir/test_renameat stage/$arch/workdir &&
    run-test $arch $test_dir/test_socketpair &&
    run-test $arch $test_dir/test_unix_listen stage/$arch/workdir/socket
}

main "$@"
