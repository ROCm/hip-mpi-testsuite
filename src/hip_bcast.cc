/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *****************************************************************************/

#include <stdio.h>
#include "mpi.h"

#include <hip/hip_runtime.h>
#include <chrono>

#include "hip_mpitest_utils.h"
#include "hip_mpitest_buffer.h"

#define NITER 1
int elements=100;
hip_mpitest_buffer *sendbuf=NULL;
hip_mpitest_buffer *recvbuf=NULL;

static void init_sendbuf (int *sendbuf, int count, int mynode)
{
    for (int i = 0; i < count; i++) {
        sendbuf[i] = i;
    }
}

static void init_recvbuf (int *recvbuf, int count)
{
    for (int i = 0; i < count; i++) {
        recvbuf[i] = 0;
    }
}

static bool check_recvbuf(int *recvbuf, int nprocs, int rank, int count)
{
    bool res=true;

    for (int i=0; i<count; i++) {
        if (recvbuf[i] != i) {
            res = false;
#ifdef VERBOSE
            printf("recvbuf[%d] = %d\n", i, recvbuf[l]);
#endif
            break;
        }
    }

    return res;
}

int bcast_test (void *buf, int count, MPI_Datatype datatype, MPI_Comm comm);

int main (int argc, char *argv[])
{
    int ret;
    int rank, size;
    int root = 0;
    double t1;
    std::chrono::high_resolution_clock::time_point t1s, t1e;

    bind_device();

    MPI_Init      (&argc, &argv);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);

    parse_args(argc, argv, MPI_COMM_WORLD);

    int *tmp_sendbuf=NULL;

    if (rank == 0) {
        // Initialize send buffer
        ALLOCATE_SENDBUFFER(sendbuf, tmp_sendbuf, int, size*elements, sizeof(int),
                            rank, MPI_COMM_WORLD, init_sendbuf, out);
    } else {
        // Initialize recv buffer
        ALLOCATE_RECVBUFFER(sendbuf, tmp_sendbuf, int, size*elements, sizeof(int),
                            rank, MPI_COMM_WORLD, init_recvbuf, out);
    }

    // execute the bcast test
    MPI_Barrier(MPI_COMM_WORLD);
    t1s = std::chrono::high_resolution_clock::now();
    ret = bcast_test (sendbuf->get_buffer(), elements, MPI_INT, MPI_COMM_WORLD);
    if (MPI_SUCCESS != ret) {
        fprintf(stderr, "Error in bcast_test. Aborting\n");
        goto out;
    }
    t1e = std::chrono::high_resolution_clock::now();
    t1 = std::chrono::duration<double>(t1e-t1s).count();

    // verify results
    bool res, fret;
    res = true;
    if (sendbuf->NeedsStagingBuffer()) {
        HIP_CHECK(sendbuf->CopyFrom(tmp_sendbuf, elements*size*sizeof(int)));
        res = check_recvbuf(tmp_sendbuf, size, rank, elements);
    }
    else {
        res = check_recvbuf((int*) sendbuf->get_buffer(), size, rank, elements);
    }

    fret = report_testresult(argv[0], MPI_COMM_WORLD, sendbuf->get_memchar(), '-', res);
    report_performance (argv[0], MPI_COMM_WORLD, sendbuf->get_memchar(), '-',
                        elements, (size_t)(elements * sizeof(int)), NITER, t1);

 out:
    //Free buffers
    FREE_BUFFER(sendbuf, tmp_sendbuf);
    delete (sendbuf);

    if (MPI_SUCCESS != ret) {
        MPI_Abort (MPI_COMM_WORLD, 1);
        return 1;
    }

    MPI_Finalize ();
    return fret ? 0 : 1;
}


int bcast_test (void *buf, int count, MPI_Datatype datatype, MPI_Comm comm)
{
    int ret;
#ifdef HIP_MPITEST_IBCAST
    MPI_Request req;

    ret = MPI_Ibcast (buf, count, datatype, 0, comm, &req);
    if (ret != MPI_SUCCESS) {
        return ret;
    }
    ret = MPI_Wait (&req, MPI_STATUS_IGNORE);
#else
    ret = MPI_Bcast (buf, count, datatype, 0, comm);
#endif

    return ret;
}
