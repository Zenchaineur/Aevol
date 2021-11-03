#!/usr/bin/env bash
# Given a set of Git refs (branch names, tags), the program compiles the code,
# runs it with the same set of parameters and returns a summary

TIME_EXEC="${TIME_EXEC:-/usr/bin/time}"
AMOUNT_TESTS="${AMOUNT_TESTS:-10}"

REF_LIST=(master parallelization_run_a_step_sebastian)

function _change_ref() {
    REF="$1"
    if [[ -z $REF ]]; then
        echo "A Git ref needs to be provided to _change_ref"
        return 1
    fi

    (
        git stash
        git checkout $REF
        git pull
    ); [[ "$?" -ne 0 ]] || return 1

}

function _build_and_prepare() {
    
    # Compile code
    mkdir -p ${PROJECT_DIR}/build
    cd ${PROJECT_DIR}/build
    rm -rf ./*
    echo "Exécution de CMake"
    cmake -DUSE_OMP=ON -DCMAKE_BUILD_TYPE=Release .. &> /dev/null
    echo "Build de l'application"
    make &> /dev/null

    # Prepare for results
    mkdir -p ${PROJECT_DIR}/timetest_results
    rm -f ${PROJECT_DIR}/timetest_results/${ref}_results.txt
    echo 'cpu_s real_s' >> ${PROJECT_DIR}/timetest_results/${ref}_results.txt

}

function _execute_test() {
    i=0
    while [[ $i -lt ${AMOUNT_TESTS} ]]; do
        echo "Référence Git : $ref Exécution : $i"
        ${TIME_EXEC} -f "%U %e" --output=${PROJECT_DIR}/timetest_results/${ref}_results.txt --append ./micro_aevol_cpu "$@" > /dev/null
        if [[ $? -ne 0 ]]; then
            break
        fi
        i=$(($i+1))
        
    done
}

function timetest() {
    if ! [[ -f ExpManager.cpp ]]; then
        echo "This script must be called from the project root directory"
        return 1
    fi
    if ! [[ -f ${TIME_EXEC} ]]; then
        echo "time binary not found"
        return 1
    fi
    BASE_REF=$(git rev-parse --abbrev-ref HEAD)
    PROJECT_DIR="${PWD}"
    for ref in ${REF_LIST[@]}; do
        echo "Passage sur la branche $ref"
        _change_ref $ref
        _build_and_prepare
        _execute_test "$@"
    done
    _change_ref $BASE_REF
    git stash pop
}

timetest "$@"
