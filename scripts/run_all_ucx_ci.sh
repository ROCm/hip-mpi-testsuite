#!/bin/bash
###############################################################################
# UCX CI Test List
###############################################################################

source infra_ucx.sh

# Currently using full list for CI, but this can be reduced to a smaller set.
if [ "1"  = "1" ] ; then
    ExecTest "hip_query_test"         "1" "1"          "D"
fi
ExecTest "hip_pt2pt_bl"             "2" "32 1048576" "D H M O R $ASYNC_TYPE"
ExecTest "hip_pt2pt_bsend"          "2" "32 1048576" "D H M O R $ASYNC_TYPE"
ExecTest "hip_pt2pt_ssend"          "2" "32 1048576" "D H M O R $ASYNC_TYPE"
ExecTest "hip_pt2pt_nb"             "2" "32 1048576" "D H M O R $ASYNC_TYPE"
ExecTest "hip_pt2pt_nb_testall"     "2" "32 1048576" "D H M O R $ASYNC_TYPE"
ExecTest "hip_pt2pt_persistent"     "2" "32 1048576" "D H M O R $ASYNC_TYPE"
ExecTest "hip_sendtoself"           "1" "32 1048576" "D H M O R $ASYNC_TYPE"
ExecTest "hip_pack"                 "1" "32"         "D H M O R $ASYNC_TYPE"
ExecTest "hip_unpack"               "1" "32"         "D H M O R $ASYNC_TYPE"
ExecTest "hip_type_resized_short"   "2" "32"         "D H M O R $ASYNC_TYPE"
ExecTest "hip_type_resized_long"    "2" "32"         "D H M O R $ASYNC_TYPE"
ExecTest "hip_type_struct_short"    "2" "32"         "D H M O R $ASYNC_TYPE"
ExecTest "hip_type_struct_long"     "2" "32"         "D H M O R $ASYNC_TYPE"
ExecTest "hip_osc_put_fence"        "2" "32 1048576" "D H"
ExecTest "hip_osc_get_fence"        "2" "32 1048576" "D H"
ExecTest "hip_osc_acc_fence"        "2" "32 1048576" "D H"
ExecTest "hip_osc_acc_lock"         "2" "32 1048576" "D H"
ExecTest "hip_osc_put_lock"         "2" "32 1048576" "D H"
ExecTest "hip_osc_get_lock"         "2" "32 1048576" "D H"
ExecTest "hip_osc_rput_lock"        "2" "32 1048576" "D H"
ExecTest "hip_osc_rget_lock"        "2" "32 1048576" "D H"
if [ "0" = "0" ] ; then
    ExecTest "hip_allreduce"        "4" "32" "D"
    ExecTest "hip_reduce"           "4" "32" "D"
    ExecTest "hip_gather"           "4" "1024"       "D H"
    ExecTest "hip_gatherv"          "4" "1024"       "D H"
    ExecTest "hip_scatter"          "4" "1024"       "D H"
    ExecTest "hip_scatterv"         "4" "1024"       "D H"
else
    ExecTest "hip_allreduce"        "4" "32 1048576" "D"
    ExecTest "hip_reduce"           "4" "32 1048576" "D"
    ExecTest "hip_iallreduce"       "4" "32 1048576" "D"
    ExecTest "hip_ireduce"          "4" "32 1048576" "D"
    ExecTest "hip_gather"           "4" "1024"       "D"
    ExecTest "hip_gatherv"          "4" "1024"       "D"
    ExecTest "hip_scatter"          "4" "1024"       "D"
    ExecTest "hip_scatterv"         "4" "1024"       "D"
fi
ExecTest "hip_alltoall"             "4" "1024"       "D H"
ExecTest "hip_alltoallv"            "4" "1024"       "D H"
ExecTest "hip_allgather"            "4" "1024"       "D H"
ExecTest "hip_allgatherv"           "4" "1024"       "D H"
ExecTest "hip_reduce_scatter"       "4" "1024"       "D H"
ExecTest "hip_reduce_scatter_block" "4" "1024"       "D H"
ExecTest "hip_scan"                 "4" "1024"       "D H"
ExecTest "hip_exscan"               "4" "1024"       "D H"
ExecTest "hip_pt2pt_nb_stress"      "2" "32 1048576" "D H M O R $ASYNC_TYPE"
ExecTest "hip_sendtoself_stress"    "1" "32 1048576" "D H M O R $ASYNC_TYPE"
ExecTestSingle "hip_osc_rget_stress"  "4" "1024" "D H"
ExecTestSingle "hip_osc_rput_stress"  "4" "1024" "D H"
ExecTest "hip_pt2pt_bl"             "2" "10 876 19680 980571" "D H"
ExecTest "hip_pt2pt_bl_mult"        "2" "1024" "D H"

if [ "0" = "1" ] ; then
    ExecTest "hip_pt2pt_bl"             "2" "32 1048576" "D H M O R V"
    ExecTest "hip_pt2pt_nb"             "2" "32 1048576" "D H M O R V"
    ExecTest "hip_sendtoself"           "1" "32 1048576" "D H M O R V"
    ExecTest "hip_pt2pt_bsend"          "2" "32 1048576" "V"
    ExecTest "hip_pt2pt_ssend"          "2" "32 1048576" "V"
    ExecTest "hip_pt2pt_nb_testall"     "2" "32 1048576" "V"
    ExecTest "hip_pt2pt_persistent"     "2" "32 1048576" "V"
    ExecTest "hip_alltoallv"            "4" "1024"       "D V"
    ExecTest "hip_allgatherv"           "4" "1024"       "D V"
    ExecTest "hip_pt2pt_nb_stress"      "2" "32 1048576" "V"
    ExecTest "hip_sendtoself_stress"    "1" "32 1048576" "V"
fi

printf "\n Executed %d Tests (%d passed %d failed)\n" $COUNTER $SUCCESS $FAILED

if [ "$FAILED" -ne 0 ]; then
    printf "\nCI Failure: %d tests failed.\n" "$FAILED"
    printf "Failed tests:\n%b\n" "$FAILED_LIST"
    exit 1
fi

printf "\nCI Success: All tests passed.\n"
exit 0
