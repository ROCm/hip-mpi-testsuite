#!/bin/bash
###############################################################################
# No-UCX CI Test List
###############################################################################

source infra_no_ucx.sh

# Currently using full list for CI, but this can be reduced to a smaller set.
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

printf "\n Running tests with btl/sm + smsc/accelerator and rocm_memcpy_async = true\n"
ExecTestSm1B "hip_pt2pt_bl"             "2" "32 1048576" "D H M O R"
ExecTestSm1B "hip_pt2pt_nb"             "2" "32 1048576" "D H M O R"
ExecTestSm1B "hip_pt2pt_nb_stress"      "2" "32 1048576" "D H"
ExecTestSm1B "hip_pt2pt_bl_mult"        "2" "1024" "D H"

printf "\n Running tests with btl/smcuda and use_sync_memops = true\n"
ExecTestSmCuda2A "hip_pt2pt_bl"             "2" "32 1048576" "D H M O R"
ExecTestSmCuda2A "hip_pt2pt_nb"             "2" "32 1048576" "D H M O R"
ExecTestSmCuda2A "hip_pt2pt_nb_stress"      "2" "32 1048576" "D H"
ExecTestSmCuda2A "hip_pt2pt_bl_mult"        "2" "1024" "D H"

printf "\n Running tests with btl/smcuda and use_sync_memops = false\n"
ExecTestSmCuda2B "hip_pt2pt_bl"             "2" "32 1048576" "D H M O R"
ExecTestSmCuda2B "hip_pt2pt_nb"             "2" "32 1048576" "D H M O R"
ExecTestSmCuda2B "hip_pt2pt_nb_stress"      "2" "32 1048576" "D H"
ExecTestSmCuda2B "hip_pt2pt_bl_mult"        "2" "1024" "D H"

printf "\n Executed %d Tests (%d passed %d failed)\n" $COUNTER $SUCCESS $FAILED

if [ "$FAILED" -ne 0 ]; then
    printf "\nCI Failure: %d tests failed.\n" "$FAILED"
    printf "Failed tests:\n%b\n" "$FAILED_LIST"
    exit 1
fi

printf "\nCI Success: All tests passed.\n"
exit 0
