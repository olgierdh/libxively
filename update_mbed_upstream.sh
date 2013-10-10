#!/bin/bash -x

rev=$(git rev-parse --short --no-symbolic ${1:-"HEAD"})
src=${2:-"https://xively@mbed.org/users/xively/code/libxively-test/"}
dst=${3:-"mbed_mercurial"}

hg clone ${src} ${dst}
rm -rf ${dst}/src
git archive ${rev} "src/libxively/*.[ch]" "src/libxively/comm_layers/mbed/" | tar x -C ${dst}
echo "#define XI_VERSION \"0.1.x-${rev}\"" > ${dst}/src/libxively/xi_version.h
hg commit --repository ${dst} --addremove --user xively --message "Update from git revision ${rev}"
hg push --repository ${dst}
rm -rf ${dst}
