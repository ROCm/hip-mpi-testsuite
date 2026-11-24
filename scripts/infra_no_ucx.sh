#!/bin/bash
###############################################################################
# Infrastructure for No-UCX tests
###############################################################################

OPTIONS1A=" --timeout 300 --mca smsc_accelerator_priority 80 --mca mpi_accelerator_rocm_memcpy_async 0 --mca coll ^hcoll --mca pml ^ucx --mca osc ^ucx --mca btl ^openib,uct "
OPTIONS1B=" --timeout 300 --mca smsc_accelerator_priority 80 --mca mpi_accelerator_rocm_memcpy_async 1 --mca coll ^hcoll --mca pml ^ucx --mca osc ^ucx --mca btl ^openib,uct "
OPTIONS2A=" --timeout 300 --mca btl smcuda,tcp,self --mca mpi_accelerator_use_sync_memops true --mca coll ^hcoll --mca pml ^ucx --mca osc ^ucx "
OPTIONS2B=" --timeout 300 --mca btl smcuda,tcp,self --mca mpi_accelerator_use_sync_memops false --mca coll ^hcoll --mca pml ^ucx --mca osc ^ucx "

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
			    FAILED_LIST="$FAILED_LIST\n- $1 (np: $2, elements: $NUMELEMS, mem1: $MEM1, mem2: $MEM2) [Opt1A]"
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
			    FAILED_LIST="$FAILED_LIST\n- $1 (np: $2, elements: $NUMELEMS, mem1: $MEM1, mem2: $MEM2) [Opt1B]"
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
			    FAILED_LIST="$FAILED_LIST\n- $1 (np: $2, elements: $NUMELEMS, mem1: $MEM1, mem2: $MEM2) [Opt2A]"
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
			    FAILED_LIST="$FAILED_LIST\n- $1 (np: $2, elements: $NUMELEMS, mem1: $MEM1, mem2: $MEM2) [Opt2B]"
			fi
		    done
		done
	    done
}

let COUNTER=0
let SUCCESS=0
let FAILED=0
FAILED_LIST=""
