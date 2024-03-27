#!/bin/bash
#
# Copyright (c) 2022      Advanced Micro Devices, Inc. All rights reserved.
#

OPTIONS=" --mca coll ^hcoll --mca pml ^ucx --mca osc ^ucx --mca btl ^openib,uct "

ExecTest() {

    for NUMELEMS in $3 ; do
	for MEM1 in $4 ; do
	    for MEM2 in $4 ; do
		let COUNTER=COUNTER+1
		mpirun $OPTIONS -np $2 ../src/$1 -s $MEM1 -r $MEM2 -n $NUMELEMS
		if [ $? -eq 0 ]
		then
		    let SUCCESS=SUCCESS+1
		else
		    let FAILED=FAILED+1
		fi
	    done
	done
    done
}

let COUNTER=0
let SUCCESS=0
let FAILED=0

if [ "1"  = "1" ] ; then
    ExecTest "hip_query_test"         "1" "1"          "D"
fi
ExecTest "hip_pt2pt_bl"             "2" "32 1048576" "D H M O R"
ExecTest "hip_pt2pt_bsend"          "2" "32 1048576" "D H M O R"
ExecTest "hip_pt2pt_ssend"          "2" "32 1048576" "D H M O R"
ExecTest "hip_pt2pt_nb"             "2" "32 1048576" "D H M O R"
ExecTest "hip_pt2pt_nb_testall"     "2" "32 1048576" "D H M O R"
ExecTest "hip_pt2pt_persistent"     "2" "32 1048576" "D H M O R"
ExecTest "hip_sendtoself"           "1" "32 1048576" "D H M O R"
ExecTest "hip_pack"                 "1" "32"         "D H M O R"
ExecTest "hip_unpack"               "1" "32"         "D H M O R"
ExecTest "hip_type_resized_short"   "2" "32"         "D H M O R"
ExecTest "hip_type_resized_long"    "2" "32"         "D H M O R"
ExecTest "hip_type_struct_short"    "2" "32"         "D H M O R"
ExecTest "hip_type_struct_long"     "2" "32"         "D H M O R"
ExecTest "hip_allreduce"            "4" "32 1048576" "D"
ExecTest "hip_reduce"               "4" "32 1048576" "D"
ExecTest "hip_alltoall"             "4" "1024"       "D H"
ExecTest "hip_alltoallv"            "4" "1024"       "D H"
ExecTest "hip_allgather"            "4" "1024"       "D H"
ExecTest "hip_allgatherv"           "4" "1024"       "D H"
ExecTest "hip_gather"               "4" "1024"       "D H"
ExecTest "hip_gatherv"              "4" "1024"       "D H"
ExecTest "hip_scatter"              "4" "1024"       "D H"
ExecTest "hip_scatterv"             "4" "1024"       "D H"
ExecTest "hip_reduce_scatter"       "4" "1024"       "D H"
ExecTest "hip_reduce_scatter_block" "4" "1024"       "D H"
ExecTest "hip_pt2pt_nb_stress"      "2" "32 1048576" "D H M O R"
ExecTest "hip_sendtoself_stress"    "1" "32 1048576" "D H M O R"
ExecTest "hip_pt2pt_bl"             "2" "10 876 19680 980571" "D H"
ExecTest "hip_pt2pt_bl_mult"        "2" "1024" "D H"
printf "\n Executed %d Tests (%d passed %d failed)\n" $COUNTER $SUCCESS $FAILED
