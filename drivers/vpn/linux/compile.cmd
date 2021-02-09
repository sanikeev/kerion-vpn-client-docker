#!/bin/bash

: ${KERNEL_HEADERS:=/lib/modules/$(uname -r)/build}

echo ${KERNEL_HEADERS}

make -C ${KERNEL_HEADERS} SUBDIRS=$PWD modules || exit 1

exit
