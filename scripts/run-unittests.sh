#!/bin/bash -x

main () {
    cd "$(dirname "$(realpath "$0")")/.."
    local os=$(uname -s)
    if [ -n "$FSARCHS" ]; then
        local archs=()
        IFS=, read -ra archs <<< "$FSARCHS"
        for arch in "${archs[@]}" ; do
            run-tests "$arch"
        done
    elif [ x$os = xLinux ]; then
        local cpu=$(uname -m)
        if [ "x$cpu" == xx86_64 ]; then
            run-tests linux64
        elif [ "x$cpu" == xi686 ]; then
            run-tests linux32
        else
            echo "$0: Unknown CPU: $cpu" >&2
            exit 1
        fi
    elif [ "x$os" = xDarwin ]; then
        run-tests darwin
    else
        echo "$0: Unknown OS architecture: $os" >&2
        exit 1
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
        darwin)
            "$@"
            ;;
        *)
            valgrind -q --leak-check=full --error-exitcode=1 "$@"
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
    run-test $arch $test_dir/test_rename stage/$arch/workdir &&
    run-test $arch $test_dir/test_renameat stage/$arch/workdir &&
    run-test $arch $test_dir/test_unix_listen stage/$arch/workdir/socket
}

main "$@"
