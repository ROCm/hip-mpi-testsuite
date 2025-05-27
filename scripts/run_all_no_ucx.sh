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

OPTIONS1A=" --mca smsc_accelerator_priority 80 --mca mpi_accelerator_rocm_memcpy_async 0 --mca coll ^hcoll --mca pml ^ucx --mca osc ^ucx --mca btl ^openib,uct "
OPTIONS1B=" --mca smsc_accelerator_priority 80 --mca mpi_accelerator_rocm_memcpy_async 1 --mca coll ^hcoll --mca pml ^ucx --mca osc ^ucx --mca btl ^openib,uct "
OPTIONS2A=" --mca btl smcuda,tcp,self --mca mpi_accelerator_use_sync_memops true --mca coll ^hcoll --mca pml ^ucx --mca osc ^ucx "
OPTIONS2B=" --mca btl smcuda,tcp,self --mca mpi_accelerator_use_sync_memops false --mca coll ^hcoll --mca pml ^ucx --mca osc ^ucx "

ExecTestSm1A() {

    for NUMELEMS in $3 ; do
	for MEM1 in $4 ; do
	    for MEM2 in $4 ; do
		let COUNTER=COUNTER+1
		mpirun $OPTIONS1A -np $2 ../src/$1 -s $MEM1 -r $MEM2 -n $NUMELEMS
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

ExecTestSm1B() {

    for NUMELEMS in $3 ; do
	for MEM1 in $4 ; do
	    for MEM2 in $4 ; do
		let COUNTER=COUNTER+1
		mpirun $OPTIONS1B -np $2 ../src/$1 -s $MEM1 -r $MEM2 -n $NUMELEMS
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


ExecTestSmCuda2A() {

    for NUMELEMS in $3 ; do
	for MEM1 in $4 ; do
	    for MEM2 in $4 ; do
		let COUNTER=COUNTER+1
		mpirun $OPTIONS2A -np $2 ../src/$1 -s $MEM1 -r $MEM2 -n $NUMELEMS
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

ExecTestSmCuda2B() {

    for NUMELEMS in $3 ; do
	for MEM1 in $4 ; do
	    for MEM2 in $4 ; do
		let COUNTER=COUNTER+1
		mpirun $OPTIONS2B -np $2 ../src/$1 -s $MEM1 -r $MEM2 -n $NUMELEMS
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

#Run tests using btl/sm + smsc/accelerator + sync. hipMemcpy
#Note: this is expected to be the default configuration used
#      hence the more extensive testing
printf "\n Running tests with btl/sm + smsc/accelerator and rocm_memcpy_async = false\n"
ExecTestSm1A "hip_pt2pt_bl"             "2" "32 1048576" "D H M O R"
ExecTestSm1A "hip_pt2pt_bsend"          "2" "32 1048576" "D H M O R"
ExecTestSm1A "hip_pt2pt_ssend"          "2" "32 1048576" "D H M O R"
ExecTestSm1A "hip_pt2pt_nb"             "2" "32 1048576" "D H M O R"
ExecTestSm1A "hip_pt2pt_nb_testall"     "2" "32 1048576" "D H M O R"
ExecTestSm1A "hip_pt2pt_persistent"     "2" "32 1048576" "D H M O R"
ExecTestSm1A "hip_sendtoself"           "1" "32 1048576" "D H M O R"
ExecTestSm1A "hip_pack"                 "1" "32"         "D H M O R"
ExecTestSm1A "hip_unpack"               "1" "32"         "D H M O R"
ExecTestSm1A "hip_type_resized_short"   "2" "32"         "D H M O R"
ExecTestSm1A "hip_type_resized_long"    "2" "32"         "D H M O R"
ExecTestSm1A "hip_type_struct_short"    "2" "32"         "D H M O R"
ExecTestSm1A "hip_type_struct_long"     "2" "32"         "D H M O R"
ExecTestSm1A "hip_allreduce"            "4" "32 1048576" "D"
ExecTestSm1A "hip_reduce"               "4" "32 1048576" "D"
ExecTestSm1A "hip_reduce_local"         "1" "32"         "D"
ExecTestSm1A "hip_bcast"                "4" "32 1048576" "D"
ExecTestSm1A "hip_alltoall"             "4" "1024"       "D H"
ExecTestSm1A "hip_alltoallv"            "4" "1024"       "D H"
ExecTestSm1A "hip_allgather"            "4" "1024"       "D H"
ExecTestSm1A "hip_allgatherv"           "4" "1024"       "D H"
ExecTestSm1A "hip_gather"               "4" "1024"       "D H"
ExecTestSm1A "hip_gatherv"              "4" "1024"       "D H"
ExecTestSm1A "hip_scatter"              "4" "1024"       "D H"
ExecTestSm1A "hip_scatterv"             "4" "1024"       "D H"
ExecTestSm1A "hip_reduce_scatter"       "4" "1024"       "D H"
ExecTestSm1A "hip_reduce_scatter_block" "4" "1024"       "D H"
ExecTestSm1A "hip_scan"                 "4" "1024"       "D H"
ExecTestSm1A "hip_exscan"               "4" "1024"       "D H"
ExecTestSm1A "hip_pt2pt_nb_stress"      "2" "32 1048576" "D H"
ExecTestSm1A "hip_sendtoself_stress"    "1" "32 1048576" "D H"
ExecTestSm1A "hip_pt2pt_bl"             "2" "10 876 19680 980571" "D H"
ExecTestSm1A "hip_pt2pt_bl_mult"        "2" "1024" "D H"

#Run tests using btl/sm + smsc/accelerator + async. hipMemcpy
printf "\n Running tests with btl/sm + smsc/accelerator and rocm_memcpy_async = true\n"
ExecTestSm1B "hip_pt2pt_bl"             "2" "32 1048576" "D H M O R"
ExecTestSm1B "hip_pt2pt_nb"             "2" "32 1048576" "D H M O R"
ExecTestSm1B "hip_pt2pt_nb_stress"      "2" "32 1048576" "D H"
ExecTestSm1B "hip_pt2pt_bl_mult"        "2" "1024" "D H"

#Run some tests using btl/smcuda + mpi_accelerator_use_sync_memops= true
printf "\n Running tests with btl/smcuda and use_sync_memops = true\n"
ExecTestSmCuda2A "hip_pt2pt_bl"             "2" "32 1048576" "D H M O R"
ExecTestSmCuda2A "hip_pt2pt_nb"             "2" "32 1048576" "D H M O R"
ExecTestSmCuda2A "hip_pt2pt_nb_stress"      "2" "32 1048576" "D H"
ExecTestSmCuda2A "hip_pt2pt_bl_mult"        "2" "1024" "D H"

#Run some tests using btl/smcuda + mpi_accelerator_use_sync_memops=false
printf "\n Running tests with btl/smcuda and use_sync_memops = false\n"
ExecTestSmCuda2B "hip_pt2pt_bl"             "2" "32 1048576" "D H M O R"
ExecTestSmCuda2B "hip_pt2pt_nb"             "2" "32 1048576" "D H M O R"
ExecTestSmCuda2B "hip_pt2pt_nb_stress"      "2" "32 1048576" "D H"
ExecTestSmCuda2B "hip_pt2pt_bl_mult"        "2" "1024" "D H"

printf "\n Executed %d Tests (%d passed %d failed)\n" $COUNTER $SUCCESS $FAILED
