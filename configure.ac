#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([hip-mpi-testsuite], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_MACRO_DIR(m4)
m4_include(config/compiler_checks.m4)
m4_include(config/opal_check_rocm.m4)
m4_include(config/oac_check_package.m4)
m4_include(config/pmix_functions.m4)

# Checks for programs.
AC_PROG_CXX
AC_PROG_MAKE_SET

AC_MSG_CHECKING([if want developer-level debugging flags])
AC_ARG_ENABLE(debug,
  AC_HELP_STRING([Enable debug flags for compilation (default=yes)]))
if test "$enable_debug" = yes ; then
   SUBCONFIGURE_FLAGS="$SUBCONFIGURE_FLAGS --enable-debug"
   AC_MSG_RESULT([yes])
   CXXFLAGS="-g -O0"
   CFLAGS="-g -O0"
elif test "$enable_debug" = no ; then   
   SUBCONFIGURE_FLAGS="$SUBCONFIGURE_FLAGS --disable-debug"
   AC_MSG_RESULT([no]) 
   CXXFLAGS="-O3"
   CFLAGS="-O3"
else
   AC_MSG_RESULT([not set (implicit no)]) 
   CXXFLAGS="-O3"
   CFLAGS="-O3"
fi 


AC_LANG_PUSH([C++])
GET_CXX_COMPILER_VENDOR(compiler_vendor)
if test "$compiler_vendor" = "gnu" ; then
  CXXFLAGS="$CXXFLAGS -Wno-delete-non-virtual-dtor -std=gnu++17"
elif test "$compiler_vendor" = "intel" ; then
  CXXFLAGS="$CXXFLAGS -std=c++17"
fi

c_xx="$(basename "${CXX}")"
AC_PATH_PROG([MPICC], [$c_xx], [notfound], [$PATH])
if [ test "$MPICC" = "notfound" ] ; then
   AC_MSG_WARN([$c_xx not found.])
   AC_MSG_ERROR([Aborting.])
fi
mpi_dir="$(dirname "${MPICC}")"

PROG_CXX_MPI_HELPER([],[have_mpi_support])
if [ test "x$have_mpi_support" = "xno" ] ; then
   AC_MSG_WARN([hip-mpi-testsuite requires an MPI installation.])
   AC_MSG_ERROR([Aborting.])
fi
AC_SUBST(mpi_dir)


OPAL_CHECK_ROCM([hip_rocm],
                [rocm_happy="yes"],
                [rocm_happy="no"])

if [ test "$rocm_happy" = "no" ] ; then
   AC_MSG_WARN([hip-mpi-testsuite requires a functional ROCm installation.])
   AC_MSG_ERROR([Aborting.])
fi

AC_PATH_PROG([HIPCC], [hipcc], [notfound], [$with_rocm/bin])
if [ test "$HIPCC" = "notfound" ] ; then
   AC_MSG_WARN([hipcc not found.])
   AC_MSG_ERROR([Aborting.])
fi
AC_SUBST(HIPCC)

# Checks for library functions.
AC_LANG_POP([C++])
AC_LANG_PUSH([C])
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC

# Checks for header files.
AC_CHECK_HEADER([signal.h],
[],
[AC_MSG_ERROR(Couldn't find signal.h)])

AC_CHECK_HEADER([getopt.h],
[],
[AC_MSG_ERROR(Couldn't find getopt.h)])

AC_LANG_POP([C])
AC_LANG_PUSH([C++])

perfresults=no
AC_ARG_ENABLE(perf-timing,
[  --enable-perf-timing    enable measuring performance of operation (default=no)],
[perfresults=yes])

AC_MSG_CHECKING([whether to enable performance measurements of operations])
AC_MSG_RESULT($perfresults)

let hip_mpitest_perfresults=0
if [ test "$perfresults" = "yes" ] ; then
    hip_mpitest_perfresults=1
fi	
AC_SUBST(hip_mpitest_perfresults)


AC_CHECK_DECL([MPIX_Query_rocm_support], [HAVE_MPIX_QUERY_ROCM=1], [HAVE_MPIX_QUERY_ROCM=0],
   [ #include "mpi.h"
     #include "mpi-ext.h"],
   [] )

HIP_QUERY_TEST=""
if [ test "x$HAVE_MPIX_QUERY_ROCM" = "x1" ] ; then
   HIP_QUERY_TEST="hip_query_test"
fi
AC_SUBST(HIP_QUERY_TEST)
AC_SUBST(HAVE_MPIX_QUERY_ROCM)

AC_LANG_PUSH([C++])
vmm_save_LDFLAGS="$LDFLAGS"
vmm_save_LIBS="$LIBS"
LDFLAGS="$hip_rocm_LDFLAGS $LDFLAGS"
LIBS="$hip_rocm_LIBS $LIBS"
CXX_HELPER([if HIP VMM APIs are available],
           [HIP_MPITEST_HAVE_VMM_INT],
           [#include <hip/hip_runtime.h>],
           [hipMemGenericAllocationHandle_t h;
            hipMemCreate(&h, 4096, nullptr, 0);
            hipMemRelease(h);
            (void)hipDeviceAttributeVirtualMemoryManagementSupported;])
LDFLAGS="$vmm_save_LDFLAGS"
LIBS="$vmm_save_LIBS"
AC_LANG_POP([C++])

let hip_mpitest_have_vmm=0
if test "$HIP_MPITEST_HAVE_VMM_INT" = "1" ; then
    hip_mpitest_have_vmm=1
fi
AC_SUBST(hip_mpitest_have_vmm)

ucc_support=no;
HIP_UCC_SUPPORT=`ompi_info --parsable | grep coll | grep ucc | wc -l`
  if [ test  "$HIP_UCC_SUPPORT" != "0" ] ; then
    ucc_support=yes;
  fi
AC_MSG_CHECKING([for ucc support in Open MPI])
AC_MSG_RESULT($ucc_support)
AC_SUBST(HIP_UCC_SUPPORT)


echo " "
AC_MSG_NOTICE(creating final output files)

#AC_CONFIG_FILES([Makefile.defs])
AC_SUBST(CXX)
AC_SUBST(CC)
AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_OUTPUT(src/hip_mpitest_config.h src/Makefile benchmarks/Makefile Makefile.defs scripts/run_all_ucx_impl.sh scripts/run_all_no_ucx.sh)
