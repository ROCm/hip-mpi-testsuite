/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
** Copyright (c) 2022 Advanced Micro Devices, Inc. All rights reserved.
*/

#include <stdio.h>
#include "mpi.h"

#include <hip/hip_runtime.h>

#include "hip_mpitest_utils.h"
#include "hip_mpitest_buffer.h"
#define NUM_NB_ITERATIONS 98
int elements=1024;
hip_mpitest_buffer *sendbuf=NULL;
hip_mpitest_buffer *recvbuf=NULL;

static void init_sendbuf (int *sendbuf, int count, int mynode)
{
    int  l=0;
    count = count / NUM_NB_ITERATIONS;
    int nProcs = count / elements;
    for (int iteration=0; iteration < NUM_NB_ITERATIONS; iteration++) {
        for (int i = 0; i < count; i++, l++) {
            sendbuf[l] = mynode + 1 + iteration * nProcs;
        }
    }
}

static void init_recvbuf (int *recvbuf, int count )
{
    for (int i = 0; i < count; i++) {
        recvbuf[i] = 0;
    }
}

static bool check_recvbuf (int *recvbuf, int nProcs, int rank, int count)
{
    bool res=true;
    int  l=0;
    for (int iteration=0; iteration < NUM_NB_ITERATIONS; iteration++) {
        for (int recvrank=0; recvrank < nProcs; recvrank++) {
            if (recvrank == rank) {
                l += count;
                continue; //No send-to-self for right now
            }
            for (int i=0; i < count; i++, l++) {
                if (recvbuf[l] != recvrank + 1 + iteration * nProcs) {
                    res = false;
    #ifdef VERBOSE
                    printf("recvbuf[%d] = %d expected %d\n", i, recvbuf[l], recvrank+1);
    #endif
                    break;
                }
            }
        }
    }
    return res;
}

int type_p2p_nb_stress_test (int *sendbuf, int *recvbuf, int count, MPI_Comm comm);

int main (int argc, char *argv[])
{
    int rank, nProcs;
    int root = 0;

    bind_device();

    MPI_Init      (&argc, &argv);
    MPI_Comm_size (MPI_COMM_WORLD, &nProcs);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);

    parse_args(argc, argv, MPI_COMM_WORLD);

    int *tmp_sendbuf=NULL, *tmp_recvbuf=NULL;
    // Initialise send buffer
    ALLOCATE_SENDBUFFER(sendbuf, tmp_sendbuf, int, nProcs*elements*NUM_NB_ITERATIONS, sizeof(int),
                        rank, MPI_COMM_WORLD, init_sendbuf);

    // Initialize recv buffer
    ALLOCATE_RECVBUFFER(recvbuf, tmp_recvbuf, int, nProcs*elements*NUM_NB_ITERATIONS, sizeof(int),
                        rank, MPI_COMM_WORLD, init_recvbuf);

    //execute point-to-point operations
    int res = type_p2p_nb_stress_test ((int *)sendbuf->get_buffer(), (int *)recvbuf->get_buffer(),
                                elements, MPI_COMM_WORLD);
    if (MPI_SUCCESS != res) {
        printf("Error in type_p2p_nb_stress_test. Aborting\n");
        MPI_Abort (MPI_COMM_WORLD, 1);
        return 1;
    }
    
    // verify results
    bool ret;
    if (recvbuf->NeedsStagingBuffer()) {
        HIP_CHECK(recvbuf->CopyFrom(tmp_recvbuf, nProcs*elements*NUM_NB_ITERATIONS*sizeof(int)));
        ret = check_recvbuf(tmp_recvbuf, nProcs, rank, elements);
    }
    else {
        ret = check_recvbuf((int*) recvbuf->get_buffer(), nProcs, rank, elements);
    }
    bool fret = report_testresult(argv[0], MPI_COMM_WORLD, sendbuf->get_memchar(), recvbuf->get_memchar(), ret);
    report_performance (argv[0], MPI_COMM_WORLD, sendbuf->get_memchar(), recvbuf->get_memchar(), elements,
                        (size_t)(elements *sizeof(int)), 0, 0.0);

    //Cleanup dynamic buffers
    FREE_BUFFER(sendbuf, tmp_sendbuf);
    FREE_BUFFER(recvbuf, tmp_recvbuf);

    delete (sendbuf);
    delete (recvbuf);

    MPI_Finalize ();
    return fret ? 0 : 1;
}


int type_p2p_nb_stress_test (int *sbuf, int *rbuf, int count, MPI_Comm comm)
{
    int size, rank, ret;
    int tag=251;
    MPI_Request *reqs;
    int *sendbuf;
    int *recvbuf;

    MPI_Comm_size (comm, &size);
    MPI_Comm_rank (comm, &rank);

    reqs = (MPI_Request*)malloc (2*size*NUM_NB_ITERATIONS*sizeof(MPI_Request));
    if (NULL == reqs) {
        printf("4. Could not allocate memory. Aborting\n");
        MPI_Abort(comm, 1);
    }
    for (int j=0; j<NUM_NB_ITERATIONS; j++) {
        for (int i=0; i<size; i++) {
            if (i == rank) {
                // No send-to-self for the moment
                reqs[2*i+2*size*j]   = MPI_REQUEST_NULL;
                reqs[2*i+2*size*j+1] = MPI_REQUEST_NULL;
                continue;
            }
            recvbuf = &rbuf[i*count+j*count*size];
            ret = MPI_Irecv (recvbuf, count, MPI_INT, i, tag, comm, &reqs[2*i+2*size*j]);
            if (MPI_SUCCESS != ret) {
                return ret;
            }
            sendbuf = &sbuf[i*count+j*count*size];
            ret = MPI_Isend (sendbuf, count, MPI_INT, i, tag, comm, &reqs[2*i+2*size*j+1]);
            if (MPI_SUCCESS != ret) {
                return ret;
            }
        }
    }
    ret = MPI_Waitall (2*size*NUM_NB_ITERATIONS, reqs, MPI_STATUSES_IGNORE);
    if (MPI_SUCCESS != ret) {
        return ret;
    }
    free (reqs);

    return MPI_SUCCESS;
}

