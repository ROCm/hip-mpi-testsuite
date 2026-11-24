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

# disabling proto v2 at the moment. the read_all_2D test failes with
# proto v2 otherwise with UCX 1.17.0.
# Alternative is to use RNDV_SCHEME=put_zcopy if one wants to use proto v2
OPTIONS=" --timeout 300 --mca pml ^ucx --mca osc ^ucx --mca smsc_accelerator_priority 80 --mca coll ^hcoll"

ExecTest() {

    let COUNTER=COUNTER+1
    mpirun $OPTIONS -np $2 ../src/$1
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

ExecTest "hip_file_write"        "1"
ExecTest "hip_file_iwrite"       "1"
ExecTest "hip_file_iwrite_mult"  "1"
ExecTest "hip_file_write_all"    "4"
ExecTest "hip_file_write_all_2D" "4"

ExecTest "hip_file_read"         "1"
ExecTest "hip_file_iread"        "1"
ExecTest "hip_file_iread_mult"   "1"
ExecTest "hip_file_read_all"     "4"
ExecTest "hip_file_read_all_2D"  "4"

printf "\n Executed %d Tests (%d passed %d failed)\n" $COUNTER $SUCCESS $FAILED
