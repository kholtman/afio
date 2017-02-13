#!/usr/bin/env bash
# afio unit tests for symlink behavior

set -ue

testdir=./symlink-test
afio=../afio

testdir_src=${testdir}/src
testdir_dst=${testdir}/dst
testdir_archive=test.cpio

rm -fr -- ${testdir}
mkdir -p ${testdir_src}/{foo,bar}/baz

declare -a harmless

ln_harmless () {
    local dst="$1"
    local lnk="$2/harmless-$3"
    ln -s "${dst}" "${testdir_src}/${lnk}"
    harmless+=( "${lnk}" )
}

declare -a harmful

ln_harmful () {
    local dst="$1"
    local lnk="$2/harmful-$3"
    ln -s "${dst}" "${testdir_src}/${lnk}"
    harmful+=( "${lnk}" )
}

mk_archive () {

    # some acceptable ones
    ln_harmless  symlink-harmless      foo  01
    ln_harmless  symlink-harmless..    bar  02
    ln_harmless  ..symlink-harmless    bar  03
    ln_harmless  ..symlink-harmless..  foo  04
    ln_harmless  symlink/harmless      bar  05
    ln_harmless  sym/link../harmless   bar  06
    ln_harmless  sym/././harmless      bar  07
    ln_harmless  ...                   foo  08
    ln_harmless  ././.                 bar  09
    ln_harmless  ./.../.               foo  10
    ln_harmless  ./..a../.             bar  11
    ln_harmless  ./////.               bar  12
    ln_harmless  .//.///.              foo  13
    ln_harmless  .//.///.//.////.///.  foo  14
    ln_harmless  ./a.///.//c//d/.///.  foo  15

    # absolute
    ln_harmful  /symlink-harmful             bar  abs-01
    ln_harmful  /..                          foo  abs-02
    ln_harmful  /../symlink-harmful          foo  abs-03
    ln_harmful  /./symlink/../harmful        bar  abs-04
    ln_harmful  /./s/y//ml///ink/../harmful  bar  abs-05
    ln_harmful  /..                          bar  abs-06
    ln_harmful  /../                         bar  abs-07
    ln_harmful  /../symlink-harmful          bar  abs-08
    ln_harmful  //./                         foo  abs-09
    ln_harmful  ////symlink-harmful          bar  abs-10
    ln_harmful  /.//                         bar  abs-11
    ln_harmful  /.//symlink-harmful          foo  abs-12

    # relative
    ln_harmful  ..                     bar  rel-01
    ln_harmful  ../                    foo  rel-02
    ln_harmful  ./../                  bar  rel-03
    ln_harmful  .//../                 foo  rel-04
    ln_harmful  ./../../               foo  rel-05
    ln_harmful  ./.././                foo  rel-06
    ln_harmful  ...../.././            foo  rel-07
    ln_harmful  symlink-harmful/..     foo  rel-08
    ln_harmful  symlink-harmful/../    bar  rel-09
    ln_harmful  ../symlink-harmful     bar  rel-10
    ln_harmful  ../../symlink-harmful  foo  rel-11
    ln_harmful  ./a.///..//c//d/.///.  foo  rel-12
    ln_harmful  sym/././../harmful     bar  rel-13

    printf "created %d harmless, %d harmful symlinks\n" \
        ${#harmless[@]} ${#harmful[@]} 1>&2

    ( cd ${testdir_src} && find . -type l | ../../${afio} -o ../${testdir_archive} )
    local e=( $( ${afio} -t "${testdir}/${testdir_archive}" ) )
    printf "created archive %s with %d entries\n" \
        "${testdir}/${testdir_archive}" "${#e[@]}"
}

declare -a bad_tests=()

test_mode () {
    local mode="$1"
    local dstdir="${testdir_dst}/${mode}"
    shift

    rm -rf -- "${dstdir}"
    mkdir -p ${dstdir}
    ( cd ${dstdir} && \
        ../../../${afio} -8 "${mode}" -i ../../${testdir_archive} 2>/dev/null)
    local survivors=( $(find ${dstdir} -type l) )
    local nsurvivors=${#survivors[@]}

    printf "%s: %d links survived extraction, %d expected\n" \
        "${mode}" "${nsurvivors}" "$#" 1>&2

    local failed=no
    if [ $# -eq 0 -a ${nsurvivors} -eq 0 ] ; then
        printf "%s: all good no links survived at all\n" "${mode}" 1>&2
    else
        local notfound=()
        while [ -n "${1:-}" ]; do
            local testme="${dstdir}/$1"
            if [ -L "${testme}" ]; then
                local newvivors=()
                for e in ${survivors[@]}; do
                    if [ "$e" != "${testme}" ]; then
                        newvivors+=( "$e" )
                    fi
                done
                survivors=( ${newvivors[@]:-} )
            else
                notfound+=( "${testme}" )
            fi
            shift
        done
        if [ ${#notfound[@]} -gt 0 ]; then
            printf "%s: missing %d symlinks after extraction:\n" \
                "${mode}" "${#notfound[@]}" 1>&2
            for l in ${notfound[@]}; do
                printf "%s:     - %s:\n" "${mode}" "$l" 1>&2
            done
            failed=yes
        fi
        if [ ${#survivors[@]} -gt 0 ]; then
            printf "%s: unexpected %d symlinks that survived extraction:\n" \
                "${mode}" "${#survivors[@]}" 1>&2
            for l in ${survivors[@]}; do
                printf "%s:     - %s:\n" "${mode}" "$l" 1>&2
            done
            failed=yes
        fi
    fi
    if [ "${failed}" = yes ]; then
        bad_tests+=( "${mode}" )
    fi
}

mk_archive
test_mode none ${harmful[@]} ${harmless[@]}
test_mode all
test_mode harmful ${harmless[@]}

if [ ${#bad_tests[@]} -eq 0 ] ; then
    printf "all modes ok\n" 1>&2
    exit 0
fi

printf "listing failed tests:\n" 1>&2
for m in ${bad_tests[@]:-}; do
    printf "  - %s\n" "$m" 1>&2
done
exit 1

