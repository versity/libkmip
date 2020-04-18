#!/bin/bash
#
# VSM - Versity Storage Management File System
#
# Copyright (c) 2017  Versity Software, Inc.
# All Rights Reserved
#

set -e

err_report() {
  echo "Error on line $1"
  exit 1
}

trap 'err_report $LINENO' ERR

# env flags to change RPM build behavior
DEV_BUILD=${DEV_BUILD:-"no"}

# build against the debug kernel
DEBUG_KERN=${DEBUG_KERN:-"no"}

# Distro to put into RPM names -- avoids el7.centos set in /etc/rpm/macros.dist
DISTRO=${DISTRO:-".el7"}

# the user passes in these things:
# argv $1: RPM_TAR_FILE  - .tar.gz ready for rpmbuild -ts, -ta, etc
RPM_TAR_FILE=$1

if [[ -z "$RPM_TAR_FILE" ]]; then
    echo "usage: $0 /path/to/rpm/ready/file.tar.gz"
    exit 1
fi

# Keep rpms local to us
RPM_DIR=${RPM_DIR:-"$(pwd)/rpmbuild"}
mkdir -p "${RPM_DIR}"/{SOURCES,BUILD,BUILDROOT,RPMS,SRPMS}

export RESULT_DIR RPM_DIR

# mock ignores $HOME/.rpmmacros, so set here
VENDOR="Versity Software, Inc."


# make sure we run /usr/bin/mock
PATH=/usr/bin:$PATH
export PATH

test_flag() {
    flag=$1

    if [ "$flag" == "yes" ]; then
        /bin/true
    else
        /bin/false
    fi
}

string_flag() {
    flag=$1

    if test_flag "$flag" == /bin/true; then
        echo "true"
    else
        echo "false"
    fi
}

# print flag value and status to command line/logs
print_vars_and_flags() {
    echo
    echo "Variables:"

    echo "RESULT_DIR: -> '$RESULT_DIR'"
    echo "RPM_DIR: -> '$RPM_DIR'"

    echo
    echo "Flags:"
    echo "DEV_BUILD: -> $DEV_BUILD ($(string_flag "$DEV_BUILD"))"
    echo "DEBUG_KERN: -> $DEBUG_KERN ($(string_flag "$DEBUG_KERN"))"
    echo

}

build_rpmbuild_flags() {
    local optargs

    # other macros to add ?
    optargs="--define 'vendor $VENDOR' --define '_topdir $RPM_DIR' --define 'dist $DISTRO'"

    if test_flag "$DEBUG_KERN"; then
        optargs="$optargs --define 'kerndebug 1'"
    fi

    echo "$optargs"
}


rpm_build () {
    local tar_file
    local rpm_flags

    tar_file="$1"

    test -f "$tar_file"

    rpm_flags=$(build_rpmbuild_flags)

    echo "RPM flags: $rpm_flags"

    eval rpmbuild -ts "$rpm_flags" "$tar_file"
    eval rpmbuild -tb "$rpm_flags" "$tar_file"
}


build () {
    print_vars_and_flags
    rpm_build "$RPM_TAR_FILE"
}

# We run against whichever environment we are in -- let the docker container control that.
build
