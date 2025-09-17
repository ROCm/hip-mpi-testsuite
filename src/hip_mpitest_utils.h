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

#ifndef __HIP_MPITEST_UTILS__
#define __HIP_MPITEST_UTILS__

#include <cassert>
#include <cerrno>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <execinfo.h>
#include <getopt.h>

#include <hip/hip_runtime.h>
#include "hip_mpitest_config.h"
#include "hip_mpitest_buffer.h"
#include "mpi.h"

#define HIP_CHECK(cond) {                                                 \
        if(cond != hipSuccess){                                           \
            fprintf(stderr,"HIP error: %d line: %d\n", cond,  __LINE__);  \
            ret = cond;                                                   \
            goto out;                                                     \
        }                                                                 \
    }


#define SET_MEMBUF_TYPE(_bufchar, _membuf, _argc, _argv, _comm) {  \
   if (strncmp(_bufchar, "D", 1) == 0 ){                     \
       _membuf = new hip_mpitest_buffer_device;              \
   }                                                         \
   else if (strncmp(_bufchar, "H", 1) == 0) {                \
       _membuf = new hip_mpitest_buffer_host;                \
   }                                                         \
   else if (strncmp(_bufchar, "M", 1) == 0) {                \
       _membuf = new hip_mpitest_buffer_managed;             \
   }                                                         \
   else if (strncmp(_bufchar, "O", 1) == 0) {                \
       _membuf = new hip_mpitest_buffer_hostmalloc;          \
   }                                                         \
   else if (strncmp(_bufchar, "R", 1) == 0) {                \
       _membuf = new hip_mpitest_buffer_hostregister;        \
   }                                                         \
   else {                                                    \
       printf("Invalid input %s\n", _bufchar);               \
       print_help(_argc, _argv);                             \
       MPI_Abort (_comm, 1);                                 \
   }                                                         \
}

static void sig_handler(int signum){
  printf("\n [%d] Intercepted signal %d. Aborting test.\n", getpid(), signum);
  exit (1);
}

static void print_help (int argc, char **argv)
{
    int rank;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    if (0 == rank) {
        // print help message
        printf("Usage: %s -s <sendBufType> -r <recvBufType> -n <elements> -t <sleepTime>\n", argv[0]);
        printf("   with sendBufType and recvBufType being : \n"
               "         D      Device memory (i.e. hipMalloc) - default if not specified \n"
               "         H      Host memory (i.e. malloc)\n"
               "         M      Unified memory (i.e hipMallocManaged)\n"
               "         O      Device accessible page locked host memory (i.e. hipHostMalloc)\n"
               "         R      Registered host memory (i.e. hipHostRegister)\n"
	       "   elements:  number of elements to send/recv\n"
               "   sleepTime: time in seconds to sleep (optional)\n");
    }
}

extern hip_mpitest_buffer *sendbuf;
extern hip_mpitest_buffer *recvbuf;
extern int elements;

static void parse_args ( int argc, char **argv, MPI_Comm comm )
{
    static struct option longopts[] = {
        {"sendbuftype", required_argument, 0, 's'},
        {"recvbuftype", required_argument, 0, 'r'},
        {"elements",    required_argument, 0, 'n'},
        {"sleeptime",   required_argument, 0, 't'},
        {"help",        no_argument,       0, 'h'}
    };

    int longindex, stime=0;
    while (1) {
        int c;
        c = getopt_long(argc, argv, "s:r:n:t:h", longopts, &longindex);

        if (c == -1)
            break;

        switch(c) {
        case 'h' :
            print_help(argc, argv);
            MPI_Finalize();
            exit(0);
            break;
        case 's' :
            SET_MEMBUF_TYPE(optarg, sendbuf, argc, argv, comm);
            break;
        case 'r' :
            SET_MEMBUF_TYPE(optarg, recvbuf, argc, argv, comm);
            break;
        case 'n' :
            elements = atoi(optarg);
            break;
        case 't' :
            stime = atoi(optarg);
            if (stime > 0) {
                // give time to attach with a debugger
                sleep (stime);
            }
            break;
        default :
            print_help(argc, argv);
            MPI_Finalize();
            exit(0);
        }
    }

    if (sendbuf == NULL) {
        SET_MEMBUF_TYPE("D", sendbuf, argc, argv, comm);
    }
    if (recvbuf == NULL) {
        SET_MEMBUF_TYPE("D", recvbuf, argc, argv, comm);
    }

    signal(SIGABRT, sig_handler);
    signal(SIGILL,  sig_handler);
    signal(SIGBUS,  sig_handler);
    signal(SIGFPE,  sig_handler);
    signal(SIGSEGV, sig_handler);
    return;
}

static void bind_device()
{
    int num_devices;
    int ret;
    char *local_rank = NULL;

    HIP_CHECK(hipGetDeviceCount(&num_devices));

    local_rank = getenv("OMPI_COMM_WORLD_LOCAL_RANK");
    if (local_rank != NULL) {
        int lrank  = atoi(local_rank);
	int device = lrank % num_devices;
	HIP_CHECK(hipSetDevice(device));
    }

 out:
    return;
}

static void report_buffertype (MPI_Comm comm, const char *name, hip_mpitest_buffer *buf)
{
#ifdef VERBOSE
    int rank;
    MPI_Comm_rank (comm, &rank);

    if (rank == 0) {
        printf("%s is of type %c %s address %p\n",
               name, buf->get_memchar(), buf->get_memname(), buf->get_buffer());
    }
#endif
}


static void report_performance (char *exec, MPI_Comm comm, char sendtype, char recvtype,
                                int elements, long nBytes, int niter, double time)
{
#if HIP_MPITEST_PERFRESULTS
    int rank, size;
    double t1_sum=0.0;
    size_t nBytesKB = nBytes/1024;
    size_t nBytesMB = nBytes/(1024*1024);

    MPI_Comm_rank (comm, &rank);
    MPI_Comm_size (comm, &size);

    if (rank == 0) {
        if (nBytesKB == 0) {
            printf("%s %c %c: No. of elements: %d Msg length: %ld Bytes ",
                   basename(exec), sendtype, recvtype, elements, nBytes);
        }
        else if (nBytesMB < 10) {
            printf("%s %c %c: No. of elements: %d Msg length: %ld KBytes ",
                   basename(exec), sendtype, recvtype, elements, nBytesKB);
        }
        else {
            printf("%s %c %c: No. of elements: %d Msg length: %ld MBytes ",
                   basename(exec), sendtype, recvtype, elements, nBytesMB);
        }
    }

    if ( time != 0.0 ) {
        MPI_Reduce(&time, &t1_sum, 1, MPI_DOUBLE, MPI_SUM, 0, comm);
        if ( rank == 0) {
            printf("Avg. time %lf\n", t1_sum/(size*niter));
        }
    }
    else {
        if (rank == 0) {
            printf("\n");
        }
    }
#endif
}

static bool report_testresult (char *exec, MPI_Comm comm, char sendtype, char recvtype, bool ret)
{
    int gret=1, pret;
    int rank;
    MPI_Comm_rank (comm, &rank);
    char execname[32];

    pret = ret == true ? 1 : 0;
    snprintf(execname, 32, "%s %c %c :", basename(exec), sendtype, recvtype);
    MPI_Reduce(&pret, &gret, 1, MPI_INT, MPI_MIN, 0, comm);
    if (rank == 0 ) {
        printf ("%-32s \t [%s]\n", execname, gret != 0 ? "SUCCESS" : "FAILED");
    }
    return (bool)gret;
}

#endif
