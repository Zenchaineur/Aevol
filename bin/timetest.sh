#!/usr/bin/env bash
# Given a set of Git refs (branch names, tags), the program compiles the code,
# runs it with the same set of parameters and returns a summary

TIME_EXEC="${TIME_EXEC:-/usr/bin/time}"
AMOUNT_TESTS="${AMOUNT_TESTS:-10}"
AMOUNT_ITERATIONS="${AMOUNT_ITERATIONS:-1000}"

#REF_LIST=(master parallelization_run_a_step_sebastian parallelization_promoter_at)
#REF_LIST=(master parallelization_run_a_step_sebastian)
REF_LIST=(master parallelization_promoter_at)

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
    cmake -DUSE_OMP=ON -DCMAKE_BUILD_TYPE=${BUILD_TYPE} .. &> /dev/null
    echo "Build de l'application"
    make &> /dev/null

    # Prepare for results
    mkdir -p ${RESULT_DIR}
    rm -f ${RESULT_DIR}/${ref}_${BUILD_TYPE}_${AMOUNT_ITERATIONS}its_results.csv
    echo 'cpu_s,real_s' >> ${RESULT_DIR}/${ref}_${BUILD_TYPE}_${AMOUNT_ITERATIONS}its_results.csv

}

function _execute_test() {
    i=0
    while [[ $i -lt ${AMOUNT_TESTS} ]]; do
        echo "Référence Git : $ref Exécution : $i"
        ${TIME_EXEC} -f "%U,%e" --output=${RESULT_DIR}/${ref}_${BUILD_TYPE}_${AMOUNT_ITERATIONS}its_results.csv --append ./micro_aevol_cpu -n ${AMOUNT_ITERATIONS} "$@" > /dev/null
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
    BUILD_TYPE="$1"
    if ! [[ "${BUILD_TYPE}" =~ ^(Debug|Release)$ ]]; then
        echo "User must provide either Debug or Release as an argument for build type"
        return 1
    fi
    shift  
    BASE_REF=$(git rev-parse --abbrev-ref HEAD)
    PROJECT_DIR="${PWD}"
    RESULT_DIR="${PROJECT_DIR}/results/timetest-$(date +%F-%H-%M-%S)"
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
