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


/*
** Testcase for multi-process point-to-point
** communication operations
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"

#include <hip/hip_runtime.h>
#include "hip_mpitest_utils.h"
#include "hip_mpitest_buffer.h"

int elements=1024;
hip_mpitest_buffer *sendbuf=NULL;
hip_mpitest_buffer *recvbuf=NULL;

static void init_sendbuf (int *sendbuf, int count, int mynode)
{
    for (int i = 0; i < count; i++) {
        sendbuf[i] = mynode + 1;
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

    for (int i=0; i < count; i++) {
        if (recvbuf[i] != rank + 1) {
            res = false;
#ifdef VERBOSE
            printf("recvbuf[%d] = %d expected %d\n", i, recvbuf[i], rank+1);
#endif
            break;
        }
    }

    return res;
}

int type_p2p_nb_test (int *sendbuf, int *recvbuf, int count, MPI_Comm comm);

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
    ALLOCATE_SENDBUFFER(sendbuf, tmp_sendbuf, int, elements, sizeof(int), rank, MPI_COMM_WORLD, init_sendbuf);

    // Initialize recv buffer
    ALLOCATE_RECVBUFFER(recvbuf, tmp_recvbuf, int, elements, sizeof(int), rank, MPI_COMM_WORLD, init_recvbuf);

    //execute point-to-point operations
    int res = type_p2p_nb_test ((int *)sendbuf->get_buffer(), (int *)recvbuf->get_buffer(), elements, MPI_COMM_WORLD);
    if (MPI_SUCCESS != res) {
        printf("Error in type_p2p_nb_test. Aborting\n");
        FREE_BUFFER(sendbuf, tmp_sendbuf);
        FREE_BUFFER(recvbuf, tmp_recvbuf);
        delete (sendbuf);
        delete (recvbuf);

        MPI_Abort (MPI_COMM_WORLD, 1);
        return 1;
    }

    // verify results
    bool ret;
    if (recvbuf->NeedsStagingBuffer()) {
        HIP_CHECK(recvbuf->CopyFrom(tmp_recvbuf, elements*sizeof(int)));
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


int type_p2p_nb_test (int *sendbuf, int *recvbuf, int count, MPI_Comm comm)
{
    int size, rank, ret;
    int tag=251;
    MPI_Request *reqs=NULL;

    MPI_Comm_size (comm, &size);
    MPI_Comm_rank (comm, &rank);

    reqs = (MPI_Request*)malloc (2*sizeof(MPI_Request));
    if (NULL == reqs) {
        printf("4. Could not allocate memory. Aborting\n");
        MPI_Abort(comm, 1);
    }

    ret = MPI_Irecv (recvbuf, count, MPI_INT, rank, tag, comm, &reqs[0]);
    if (MPI_SUCCESS != ret) {
        return ret;
    }

    ret = MPI_Isend (sendbuf, count, MPI_INT, rank, tag, comm, &reqs[1]);
    if (MPI_SUCCESS != ret) {
        return ret;
    }

    ret = MPI_Waitall (2, reqs, MPI_STATUSES_IGNORE);
    if (MPI_SUCCESS != ret) {
        return ret;
    }
    free (reqs);

    return MPI_SUCCESS;
}
