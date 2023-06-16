/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
** Copyright (c) 2022 Advanced Micro Devices, Inc. All rights reserved.
*/

#include <stdio.h>
#include "mpi.h"

#include <hip/hip_runtime.h>

#include "hip_mpitest_utils.h"
#include "hip_mpitest_buffer.h"

int elements = 1024;
hip_mpitest_buffer *sendbuf = NULL;
hip_mpitest_buffer *recvbuf = NULL;

static void init_sendbuf(int *sendbuf, int count, int mynode)
{
    // Rank 0 sends "1" and Rank 1 sends "2"
    for (int i = 0; i < count; i++) {
        sendbuf[i] = mynode + 1;
    }
}

static void init_recvbuf(int *recvbuf, int count)
{
    for (int i = 0; i < count; i++) {
        recvbuf[i] = 0;
    }
}

static bool check_recvbuf(int *recvbuf, int nProcs, int rank, int count)
{
    bool res = true;
    int result = 0;
    // Rank 0 receives "2" and Rank 1 receives "1"
    if (rank == 0) {
        result = 2;
    }
    else {
        result = 1;
    }

    for (int i = 0; i < count; i++) {
        if (recvbuf[i] != result) {
            res = false;
#ifdef VERBOSE
            printf("recvbuf[%d] = %d expected %d\n", i, recvbuf[i], result);
#endif
            break;
        }
    }
    return res;
}

int type_p2p_bl_test(int *sendbuf, int *recvbuf, int count, MPI_Comm comm);
int type_p2p_bsend_test(int *sendbuf, int *recvbuf, int count, MPI_Comm comm);
int type_p2p_ssend_test(int *sendbuf, int *recvbuf, int count, MPI_Comm comm);

int main(int argc, char *argv[])
{
    int rank, nProcs;
    int root = 0;

    bind_device();

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (nProcs != 2) {
        printf("This test requires exactly two processes!\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }
    parse_args(argc, argv, MPI_COMM_WORLD);

    int *tmp_sendbuf = NULL, *tmp_recvbuf = NULL;
    // Initialise send buffer
    ALLOCATE_SENDBUFFER(sendbuf, tmp_sendbuf, int, nProcs *elements, sizeof(int),
                        rank, MPI_COMM_WORLD, init_sendbuf);

    // Initialize recv buffer
    ALLOCATE_RECVBUFFER(recvbuf, tmp_recvbuf, int, nProcs *elements, sizeof(int),
                        rank, MPI_COMM_WORLD, init_recvbuf);

    // execute point-to-point operations
#if defined HIP_MPITEST_BSEND
    int res = type_p2p_bsend_test((int *)sendbuf->get_buffer(), (int *)recvbuf->get_buffer(),
                                elements, MPI_COMM_WORLD);
    if (MPI_SUCCESS != res) {
        printf("Error in type_p2p_bsend_test. Aborting\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }
#elif defined HIP_MPITEST_SSEND
    int res = type_p2p_ssend_test((int *)sendbuf->get_buffer(), (int *)recvbuf->get_buffer(),
                                elements, MPI_COMM_WORLD);
    if (MPI_SUCCESS != res) {
        printf("Error in type_p2p_ssend_test. Aborting\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }
#else
    int res = type_p2p_bl_test((int *)sendbuf->get_buffer(), (int *)recvbuf->get_buffer(),
                               elements, MPI_COMM_WORLD);
    if (MPI_SUCCESS != res) {
        printf("Error in type_p2p_bl_test. Aborting\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }
#endif

    // verify results
    bool ret;
    if (recvbuf->NeedsStagingBuffer()) {
        HIP_CHECK(recvbuf->CopyFrom(tmp_recvbuf, nProcs * elements * sizeof(int)));
        ret = check_recvbuf(tmp_recvbuf, nProcs, rank, elements);
    }
    else {
        ret = check_recvbuf((int *)recvbuf->get_buffer(), nProcs, rank, elements);
    }
    bool fret = report_testresult(argv[0], MPI_COMM_WORLD, sendbuf->get_memchar(), recvbuf->get_memchar(), ret);

    // Cleanup dynamic buffers
    FREE_BUFFER(sendbuf, tmp_sendbuf);
    FREE_BUFFER(recvbuf, tmp_recvbuf);

    delete (sendbuf);
    delete (recvbuf);

    MPI_Finalize();
    return fret ? 0 : 1;
}

int type_p2p_bl_test(int *sbuf, int *rbuf, int count, MPI_Comm comm)
{
    int size, rank, ret;
    int tag = 251;
    MPI_Status status;

    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);

        if (rank == 0) {
            ret = MPI_Send(sbuf, count, MPI_INT, 1, tag, comm);
            if (MPI_SUCCESS != ret) {
                return ret;
            }
            ret = MPI_Recv(rbuf, count, MPI_INT, 1, tag, comm, &status);
            if (MPI_SUCCESS != ret) {
                return ret;
            }
        }
        if (rank == 1) {
            ret = MPI_Recv(rbuf, count, MPI_INT, 0, tag, comm, &status);
            if (MPI_SUCCESS != ret) {
                return ret;
            }
            ret = MPI_Send(sbuf, count, MPI_INT, 0, tag, comm);
            if (MPI_SUCCESS != ret) {
                return ret;
            }
        }

    return MPI_SUCCESS;
}

int type_p2p_bsend_test(int *sbuf, int *rbuf, int count, MPI_Comm comm)
{
    int size, rank, ret;
    int tag = 251;
    MPI_Status status;
    int *buffer;
    int msg_size, buffersize;

    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);

    MPI_Pack_size(count, MPI_INT, comm, &msg_size);
    buffersize = MPI_BSEND_OVERHEAD + msg_size;
    buffer = (int *) malloc(buffersize);

    MPI_Buffer_attach(buffer, buffersize);

        if (rank == 0) {
            ret = MPI_Bsend(sbuf, count, MPI_INT, 1, tag, comm);
            if (MPI_SUCCESS != ret) {
                return ret;
            }
            ret = MPI_Recv(rbuf, count, MPI_INT, 1, tag, comm, &status);
            if (MPI_SUCCESS != ret) {
                return ret;
            }
        }
        if (rank == 1) {
            ret = MPI_Recv(rbuf, count, MPI_INT, 0, tag, comm, &status);
            if (MPI_SUCCESS != ret) {
                return ret;
            }
            ret = MPI_Bsend(sbuf, count, MPI_INT, 0, tag, comm);
            if (MPI_SUCCESS != ret) {
                return ret;
            }
        }

    MPI_Buffer_detach(&buffer, &buffersize);
    delete (buffer);

    return MPI_SUCCESS;
}

int type_p2p_ssend_test(int *sbuf, int *rbuf, int count, MPI_Comm comm) {
    int size, rank, ret;
    int tag = 251;

    MPI_Status status;

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    if (rank == 0) {
        ret = MPI_Ssend(sbuf, count, MPI_INT, 1, tag, comm);
        if (MPI_SUCCESS != ret) {
            return ret;
        }
        ret = MPI_Recv(rbuf, count, MPI_INT, 1, tag, comm, &status);
        if (MPI_SUCCESS != ret) {
            return ret;
        }
    } else if (rank == 1) {
        ret = MPI_Recv(rbuf, count, MPI_INT, 0, tag, comm, &status);
        if (MPI_SUCCESS != ret) {
            return ret;
        }
        ret = MPI_Ssend(sbuf, count, MPI_INT, 0, tag, comm);
        if (MPI_SUCCESS != ret) {
            return ret;
        }
    }

    return MPI_SUCCESS;
}