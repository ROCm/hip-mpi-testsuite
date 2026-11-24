#!/bin/bash
###############################################################################
# Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
###############################################################################

OPTIONS0=" --timeout 300 --mca coll ^hcoll"
OPTIONS1=" --timeout 300 --memory-alloc-kinds system,mpi --mca coll ^hcoll"
OPTIONS2=" --timeout 300 --memory-alloc-kinds system,mpi,rocm --mca coll ^hcoll"
OPTIONS3=" --timeout 300 --memory-alloc-kinds system,mpi,rocm:device --mca coll ^hcoll"
OPTIONS4=" --timeout 300 --memory-alloc-kinds system,mpi,rocm,nonsense:host --mca coll ^hcoll"


ExecTest() {

    let COUNTER=COUNTER+1
    mpirun $3 -np $2 ../src/$1 $4
    if [ $? -eq 0 ]
    then
	let SUCCESS=SUCCESS+1
    else
	let FAILED=FAILED+1
    fi
}

let COUNTER=0
let SUCCESS=0
let FAILED=0

ExecTest "hip_memkind"  "2" "$OPTIONS0" "0"
ExecTest "hip_memkind"  "2" "$OPTIONS1" "1"
ExecTest "hip_memkind"  "2" "$OPTIONS2" "2"
ExecTest "hip_memkind"  "2" "$OPTIONS3" "3"
ExecTest "hip_memkind"  "2" "$OPTIONS4" "4"
ExecTest "hip_memkind_sessions"  "2" "$OPTIONS0" "0"

printf "\n Executed %d Tests (%d passed %d failed)\n" $COUNTER $SUCCESS $FAILED
