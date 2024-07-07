#!/bin/bash
#
# Copyright (c) 2024      Advanced Micro Devices, Inc. All rights reserved.
#

# disabling proto v2 at the moment. the read_all_2D test failes with
# proto v2 otherwise with UCX 1.17.0.
# Alternative is to use RNDV_SCHEME=put_zcopy if one wants to use proto v2
OPTIONS="-x UCX_PROTO_ENABLE=n --mca pml ucx --mca osc ucx"

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
