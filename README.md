# hip-mpi-testsuite

The hip-mpi-testsuite contains a collection of MPI tests capable of utilizing arbitray combinations of different memory types.
The code requires a ROCm enabled implementation of Open MPI, and UCX compiled with ROCm support.

All tests take the same set of optional arguments:

```
Usage: executable_name -s <sendBufType> -r <recvBufType> -n <elements> -t <sleepTime>
       with sendBufType and recvBufType being :
                  D      Device memory (i.e. hipMalloc) - default if not specified
                  H      Host memory (i.e. malloc)
                  M      Unified memory (i.e hipMallocManaged)
                  O      Device accessible page locked host memory (i.e. hipHostMalloc)
                  R      Registered host memory (i.e. hipHostRegister)
                  A      Memory allocation using hipMallocAsync
                  V      Memory allocation using hipMemCreate/Map (Virtual Memory Management)
            elements:  number of elements to send/recv
            sleepTime: time in seconds to sleep
```

The `V` (Virtual Memory Management) type is only available when the testsuite is
built against a ROCm/HIP version that provides the VMM driver APIs, version 7.1.0
or newer (`configure` reports `enable the VMM ('V') memory type ... 1`). On older
versions it is compiled out and requesting `-s V`/`-r V` is rejected.

To compile and run all tests in the testsuite 

```
./configure CXX=mpiCC --with-rocm=/opt/rocm
make
cd scripts/
./run_all.sh
```

Compiling and running a benchmark can be done for example using the following commands:

```
make bench
mpirun --mca pml ucx -x UCX_RNDV_THRESH=128 -np 16 ./benchmarks/hip_allreduce_bench -s D -r D -n 1048576
```
Note: performance tuning might be necessary depending on the operation executed, message length, and platform. This can include selecting components used for the operation (e.g. ucc, tuned, han, etc.) as well as setting parameters of the component, and environment variable for tuning UCX performance.
