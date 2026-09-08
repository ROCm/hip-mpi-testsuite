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

#ifndef __HIP_MPITEST_BUFFER__
#define __HIP_MPITEST_BUFFER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hip/hip_runtime.h>


enum HIP_MPITEST_MEMTYPE {
      HIP_MPITEST_MEMTYPE_HOST=0,
      HIP_MPITEST_MEMTYPE_DEVICE,
      HIP_MPITEST_MEMTYPE_MANAGED,
      HIP_MPITEST_MEMTYPE_HOSTMALLOC,
      HIP_MPITEST_MEMTYPE_HOSTREGISTER,
      HIP_MPITEST_MEMTYPE_ASYNC,
      HIP_MPITEST_MEMTYPE_LAST
};

const char hip_mpitest_memtype_chars[HIP_MPITEST_MEMTYPE_LAST] = {'H','D','M','O','R','A'};

class hip_mpitest_buffer {
 protected:
    void                *buffer;
    HIP_MPITEST_MEMTYPE memtype;
    char                memchar;
    char            memname[32];

 public:
    void* get_buffer() {
	return buffer;
    }
    char get_memchar() {
	return memchar;
    }
    char *get_memname() {
	return memname;
    }

    virtual hipError_t  Allocate(size_t nBytes)=0;
    virtual hipError_t  CopyTo(void* src, size_t nBytes)=0;
    virtual hipError_t  CopyFrom(void* dst, size_t nBytes)=0;
    virtual hipError_t  Free ()=0;
    virtual bool        NeedsStagingBuffer()=0;
};


class hip_mpitest_buffer_host: public hip_mpitest_buffer {
 public:
    hip_mpitest_buffer_host () {
	memtype = HIP_MPITEST_MEMTYPE_HOST;
	memchar = 'H';
	strncpy (memname, "malloc", 32);
    }

    bool NeedsStagingBuffer() {
	return false;
    }

    hipError_t Allocate (size_t nBytes) {
	hipError_t err = hipErrorMemoryAllocation;
	char *tbuf = (char *) malloc (nBytes);
	if (NULL != tbuf) {
	    err = hipSuccess;
	    buffer = tbuf;
	}
	return err;
    }

    hipError_t Free () {
	free(buffer);
	buffer = NULL;
	return hipSuccess;
    }

    hipError_t CopyTo(void *src, size_t nBytes) {
	memcpy(buffer, src, nBytes);
	return hipSuccess;
    }

    hipError_t CopyFrom(void *dst, size_t nBytes) {
	memcpy(dst, buffer, nBytes);
	return hipSuccess;
    }
};

class hip_mpitest_buffer_device: public hip_mpitest_buffer {
 public:
    hip_mpitest_buffer_device () {
	memtype = HIP_MPITEST_MEMTYPE_DEVICE;
	memchar = 'D';
	strncpy (memname, "hipMalloc", 32);
    }

    bool NeedsStagingBuffer() {
	return true;
    }

    hipError_t Allocate (size_t nBytes) {
	return hipMalloc((void **)&buffer, nBytes);
    }

    hipError_t Free () {
	hipError_t err = hipFree(buffer);
	buffer = NULL;
	return err;
    }

    hipError_t CopyTo(void *src, size_t nBytes) {
	hipError_t err = hipMemcpy(buffer, src, nBytes, hipMemcpyDefault);
        if (err != hipSuccess) {
            return err;
        }
        return hipStreamSynchronize(0);
    }
    hipError_t CopyFrom(void *dst, size_t nBytes) {
	hipError_t err = hipMemcpy(dst, buffer, nBytes, hipMemcpyDefault);
        if (err != hipSuccess) {
            return err;
        }
        return hipStreamSynchronize(0);
    }

};


class hip_mpitest_buffer_managed: public hip_mpitest_buffer {
 public:
    hip_mpitest_buffer_managed () {
	memtype = HIP_MPITEST_MEMTYPE_MANAGED;
	memchar = 'M';
	strncpy (memname, "hipMallocManaged", 32);
    }

    bool NeedsStagingBuffer() {
	return false;
    }

    hipError_t Allocate(size_t nBytes) {
	return hipMallocManaged((void**) &buffer, nBytes);
    }

    hipError_t Free() {
	hipError_t err = hipFree(buffer);
	buffer = NULL;
	return err;
    }

    hipError_t CopyTo(void *src, size_t nBytes) {
	hipError_t err = hipMemcpy(buffer, src, nBytes, hipMemcpyDefault);
        if (err != hipSuccess) {
            return err;
        }
        return hipStreamSynchronize(0);
    }

    hipError_t CopyFrom(void *dst, size_t nBytes) {
	hipError_t err = hipMemcpy(dst, buffer, nBytes, hipMemcpyDefault);
        if (err != hipSuccess) {
            return err;
        }
        return hipStreamSynchronize(0);
    }
};

class hip_mpitest_buffer_hostmalloc: public hip_mpitest_buffer {
 public:
    hip_mpitest_buffer_hostmalloc () {
	memtype = HIP_MPITEST_MEMTYPE_HOSTMALLOC;
	memchar = 'O';
	strncpy (memname, "hipHostMalloc", 32);
    }

    bool NeedsStagingBuffer() {
	return false;
    }

    hipError_t Allocate(size_t nBytes) {
	return hipHostMalloc((void **)&buffer, nBytes);
    }

    hipError_t Free() {
	hipError_t err = hipFree(buffer);
	buffer = NULL;
	return err;
    }

    hipError_t CopyTo(void *src, size_t nBytes) {
	hipError_t err = hipMemcpy(buffer, src, nBytes, hipMemcpyDefault);
        if (err != hipSuccess) {
            return err;
        }
        return hipStreamSynchronize(0);
    }

    hipError_t CopyFrom(void *dst, size_t nBytes) {
	hipError_t err = hipMemcpy(dst, buffer, nBytes, hipMemcpyDefault);
        if (err != hipSuccess) {
            return err;
        }
        return hipStreamSynchronize(0);
    }
};

class hip_mpitest_buffer_hostregister: public hip_mpitest_buffer {
 public:
    hip_mpitest_buffer_hostregister () {
	memtype = HIP_MPITEST_MEMTYPE_HOSTREGISTER;
	memchar = 'R';
	strncpy (memname, "hipHostRegister", 32);
    }

    bool NeedsStagingBuffer() {
	return false;
    }

    hipError_t Allocate(size_t nBytes) {
	hipError_t err = hipErrorMemoryAllocation;
	char *tbuf = (char*) malloc (nBytes);
	if (NULL != tbuf) {
	    err = hipHostRegister(tbuf, nBytes, 0);
	    buffer = tbuf;
	}
	return err;
    }

    hipError_t Free() {
	hipError_t err = hipHostUnregister(buffer);
	free(buffer);
	buffer = NULL;
	return err;
    }

    hipError_t CopyTo(void *src, size_t nBytes) {
	memcpy(buffer, src, nBytes);
	return hipSuccess;
    }

    hipError_t CopyFrom(void *dst, size_t nBytes) {
	memcpy(dst, buffer, nBytes);
	return hipSuccess;
    }
};

class hip_mpitest_buffer_async: public hip_mpitest_buffer {
    hipStream_t stream;
 public:
    hip_mpitest_buffer_async () {
	memtype = HIP_MPITEST_MEMTYPE_ASYNC;
	memchar = 'A';
	strncpy (memname, "hipMallocAsync", 32);
	stream = nullptr;
    }

    bool NeedsStagingBuffer() {
	return true;
    }

    hipError_t Allocate (size_t nBytes) {
	hipError_t err = hipStreamCreate(&stream);
	if (err != hipSuccess) {
	    return err;
	}
	err = hipMallocAsync((void **)&buffer, nBytes, stream);
	if (err != hipSuccess) {
	    return err;
	}
	// Synchronize so the buffer is safe to hand to MPI / other streams
	return hipStreamSynchronize(stream);
    }

    hipError_t Free () {
	hipError_t err = hipFreeAsync(buffer, stream);
	if (err != hipSuccess) {
	    return err;
	}
	err = hipStreamSynchronize(stream);
	if (err != hipSuccess) {
	    return err;
	}
	buffer = NULL;
	hipError_t derr = hipStreamDestroy(stream);
	stream = nullptr;
	return derr;
    }

    hipError_t CopyTo(void *src, size_t nBytes) {
	hipError_t err = hipMemcpy(buffer, src, nBytes, hipMemcpyDefault);
        if (err != hipSuccess) {
            return err;
        }
        return hipStreamSynchronize(0);
    }

    hipError_t CopyFrom(void *dst, size_t nBytes) {
	hipError_t err = hipMemcpy(dst, buffer, nBytes, hipMemcpyDefault);
        if (err != hipSuccess) {
            return err;
        }
        return hipStreamSynchronize(0);
    }
};

// Some convinience macros
#define ALLOCATE_SENDBUFFER(_sendbuf, _tmp_sendbuf, _type, _elements, _extent, _rank, _comm, _init, _label) { \
     if (_sendbuf == nullptr) {                                                                       \
         ret = MPI_ERR_OTHER;                                                                         \
         goto _label;                                                                                    \
     } else {                                                                                         \
      if (_sendbuf->NeedsStagingBuffer() ) {                                                          \
        _tmp_sendbuf = (_type *) malloc (_elements * _extent);                                        \
        if (NULL == _tmp_sendbuf) {                                                                   \
            ret = MPI_ERR_OTHER;                                                                      \
            goto _label;                                                                                 \
        }                                                                                             \
        _init(_tmp_sendbuf, _elements, _rank);                                                        \
	if (_sendbuf->Allocate(_elements * _extent) != hipSuccess) {                                  \
            ret = MPI_ERR_OTHER;                                                                      \
            goto _label;                                                                                 \
        }                                                                                             \
        if (_sendbuf->CopyTo(_tmp_sendbuf, _elements * _extent) != hipSuccess) {                      \
            ret = MPI_ERR_OTHER;                                                                      \
            goto _label;                                                                                 \
        }                                                                                             \
      }                                                                                               \
      else {                                                                                          \
        if (_sendbuf->Allocate(_elements * _extent) != hipSuccess) {                                  \
            ret = MPI_ERR_OTHER;                                                                      \
            goto _label;                                                                                 \
        }                                                                                             \
        _init((_type *)_sendbuf->get_buffer(), _elements, _rank);                                     \
      }                                                                                               \
      report_buffertype(_comm, "Sendbuf", sendbuf);                                                   \
    }                                                                                                 \
}

#define ALLOCATE_RECVBUFFER(_recvbuf, _tmp_recvbuf, _type, _elements, _extent, _rank, _comm, _init, _label) { \
    if (_recvbuf == nullptr)  {                                                                       \
        ret = MPI_ERR_OTHER;                                                                          \
        goto _label;                                                                                     \
    } else {                                                                                          \
      if (_recvbuf->NeedsStagingBuffer() ) {                                                          \
        _tmp_recvbuf = (_type *) malloc (_elements * _extent);                                        \
        if (NULL == _tmp_recvbuf) {                                                                   \
            ret = MPI_ERR_OTHER;                                                                      \
            goto _label;                                                                                 \
        }                                                                                             \
        _init(_tmp_recvbuf, _elements);                                                               \
        if (_recvbuf->Allocate(_elements * _extent) != hipSuccess) {                                  \
            ret = MPI_ERR_OTHER;                                                                      \
            goto _label;                                                                                 \
        }                                                                                             \
        if (_recvbuf->CopyTo(_tmp_recvbuf, _elements * _extent) != hipSuccess) {                      \
            ret = MPI_ERR_OTHER;                                                                      \
            goto _label;                                                                                 \
        }                                                                                             \
      }                                                                                               \
      else {                                                                                          \
        if(_recvbuf->Allocate(_elements * _extent) != hipSuccess) {                                   \
            ret = MPI_ERR_OTHER;                                                                      \
            goto _label;                                                                                 \
        }                                                                                             \
        _init((_type*)_recvbuf->get_buffer(), _elements);	                                      \
      }                                                                                               \
      report_buffertype(_comm, "Recvbuf", _recvbuf);                                                  \
    }                                                                                                 \
}

#define FREE_BUFFER(_buf, _tmp_buf) { \
    if (_buf->NeedsStagingBuffer() ){ \
       free (_tmp_buf);               \
    }                                 \
    HIP_CHECK(_buf->Free());          \
}

#endif // __HIP_MPITEST_BUFFER__
