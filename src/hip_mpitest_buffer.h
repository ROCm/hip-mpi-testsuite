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
#include "hip_mpitest_config.h"


enum HIP_MPITEST_MEMTYPE {
      HIP_MPITEST_MEMTYPE_HOST=0,
      HIP_MPITEST_MEMTYPE_DEVICE,
      HIP_MPITEST_MEMTYPE_MANAGED,
      HIP_MPITEST_MEMTYPE_HOSTMALLOC,
      HIP_MPITEST_MEMTYPE_HOSTREGISTER,
      HIP_MPITEST_MEMTYPE_VMM,
      HIP_MPITEST_MEMTYPE_VMM_HOST,
      HIP_MPITEST_MEMTYPE_LAST
};

const char hip_mpitest_memtype_chars[HIP_MPITEST_MEMTYPE_LAST] = {'H','D','M','O','R','V','X'};

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

#if HIP_MPITEST_HAVE_VMM
class hip_mpitest_buffer_vmm : public hip_mpitest_buffer {
 protected:
    hipMemGenericAllocationHandle_t vmm_handle;
    size_t                          vmm_padded_size;

 public:
    hip_mpitest_buffer_vmm() {
        memtype         = HIP_MPITEST_MEMTYPE_VMM;
        memchar         = 'V';
        strncpy(memname, "hipMemCreate/Map", 32);
        vmm_padded_size = 0;
    }

    bool NeedsStagingBuffer() {
        return true;
    }

    hipError_t Allocate(size_t nBytes) {
        hipError_t err;
        int        deviceId = 0;

        err = hipGetDevice(&deviceId);
        if (err != hipSuccess) return err;

        int vmmSupported = 0;
        err = hipDeviceGetAttribute(&vmmSupported,
                  hipDeviceAttributeVirtualMemoryManagementSupported, deviceId);
        if (err != hipSuccess) return err;
        if (!vmmSupported) return hipErrorNotSupported;

        hipMemAllocationProp prop = {};
        prop.type                 = hipMemAllocationTypePinned;
        prop.location.type        = hipMemLocationTypeDevice;
        prop.location.id          = deviceId;
        prop.requestedHandleTypes = hipMemHandleTypePosixFileDescriptor;

        size_t granularity = 0;
        err = hipMemGetAllocationGranularity(&granularity, &prop,
                  hipMemAllocationGranularityMinimum);
        if (err != hipSuccess) return err;

        vmm_padded_size = ((nBytes + granularity - 1) / granularity) * granularity;
        printf("VMM Allocate: requested=%zu granularity=%zu padded=%zu device=%d\n",
               nBytes, granularity, vmm_padded_size, deviceId);

        err = hipMemCreate(&vmm_handle, vmm_padded_size, &prop, 0);
        if (err != hipSuccess) return err;
        printf("VMM Allocate: hipMemCreate succeeded\n");

        err = hipMemAddressReserve(&buffer, vmm_padded_size, 0, nullptr, 0);
        if (err != hipSuccess) {
            (void)hipMemRelease(vmm_handle);
            return err;
        }
        printf("VMM Allocate: hipMemAddressReserve succeeded ptr=%p\n", buffer);

        err = hipMemMap(buffer, vmm_padded_size, 0, vmm_handle, 0);
        if (err != hipSuccess) {
            (void)hipMemAddressFree(buffer, vmm_padded_size);
            (void)hipMemRelease(vmm_handle);
            buffer = nullptr;
            return err;
        }
        printf("VMM Allocate: hipMemMap succeeded\n");

        hipMemAccessDesc accessDesc = {};
        accessDesc.location.type    = hipMemLocationTypeDevice;
        accessDesc.location.id      = deviceId;
        accessDesc.flags            = hipMemAccessFlagsProtReadWrite;
        err = hipMemSetAccess(buffer, vmm_padded_size, &accessDesc, 1);
        if (err != hipSuccess) {
            (void)hipMemUnmap(buffer, vmm_padded_size);
            (void)hipMemAddressFree(buffer, vmm_padded_size);
            (void)hipMemRelease(vmm_handle);
            buffer = nullptr;
        } else {
            printf("VMM Allocate: hipMemSetAccess succeeded\n");
        }
        return err;
    }

    hipError_t Free() {
        printf("VMM Free: unmapping ptr=%p size=%zu\n", buffer, vmm_padded_size);
        hipError_t err1 = hipMemUnmap(buffer, vmm_padded_size);
        hipError_t err2 = hipMemRelease(vmm_handle);
        hipError_t err3 = hipMemAddressFree(buffer, vmm_padded_size);
        buffer          = nullptr;
        vmm_padded_size = 0;
        if (err1 != hipSuccess) return err1;
        if (err2 != hipSuccess) return err2;
        return err3;
    }

    hipError_t CopyTo(void *src, size_t nBytes) {
        printf("VMM CopyTo: src=%p dst=%p nBytes=%zu\n", src, buffer, nBytes);
        hipError_t err = hipMemcpy(buffer, src, nBytes, hipMemcpyDefault);
        if (err != hipSuccess) return err;
        return hipStreamSynchronize(0);
    }

    hipError_t CopyFrom(void *dst, size_t nBytes) {
        printf("VMM CopyFrom: src=%p dst=%p nBytes=%zu\n", buffer, dst, nBytes);
        hipError_t err = hipMemcpy(dst, buffer, nBytes, hipMemcpyDefault);
        if (err != hipSuccess) return err;
        return hipStreamSynchronize(0);
    }
};

/* VMM allocation with host (CPU) access in addition to device access.
 * hipMemSetAccess is called twice: once for the device (inherited from V)
 * and once for hipMemLocationTypeHost, making the buffer directly
 * CPU-dereferenceable without a staging copy. */
class hip_mpitest_buffer_vmm_host : public hip_mpitest_buffer_vmm {
 public:
    hip_mpitest_buffer_vmm_host() {
        memtype = HIP_MPITEST_MEMTYPE_VMM_HOST;
        memchar = 'X';
        strncpy(memname, "hipMemCreate/Map+HostAccess", 32);
    }

    bool NeedsStagingBuffer() {
        return false;
    }

    hipError_t Allocate(size_t nBytes) {
        hipError_t err = hip_mpitest_buffer_vmm::Allocate(nBytes);
        if (err != hipSuccess) return err;

        hipMemAccessDesc accessDesc = {};
        accessDesc.location.type    = hipMemLocationTypeHost;
        accessDesc.location.id      = 0;
        accessDesc.flags            = hipMemAccessFlagsProtReadWrite;
        err = hipMemSetAccess(buffer, vmm_padded_size, &accessDesc, 1);
        if (err != hipSuccess) {
            (void)hipMemUnmap(buffer, vmm_padded_size);
            (void)hipMemAddressFree(buffer, vmm_padded_size);
            (void)hipMemRelease(vmm_handle);
            buffer = nullptr;
        } else {
            printf("VMM Allocate: hipMemSetAccess (host) succeeded\n");
        }
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
#endif  /* HIP_MPITEST_HAVE_VMM */

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
