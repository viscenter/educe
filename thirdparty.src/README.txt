
Usage: install.sh [options] <install-dir> [make-options]

[options]: The following options can be specified in the following order.

[--uintah-only]:

   This optional argument will build only the packages needed by the
   Uintah PSE.

[--seg3d-only]:

   Build only the packages needed by Seg3D.

[--static-only]:

   Build only the static versions of the thirdparty libraries.  In
   such cases where the build system cannot be forced to build only
   static libraries, the shared libraries are removed after
   compilation.

[--verify-install]:

   Perform a validation of the install and exit.

<install-dir>:

   You must specify the complete path to the top of the installation
   directory.  For example, you might want something like this:

   /usr/local/SCIRun/Thirdparty

   (NOTE: This directory must already exist.)

   Based on the SCIRun version you are installing, the compiler, and
   the number of bits, the final install will go into:

   /usr/local/SCIRun/Thirdparty/3.1.0/Darwin/gcc-3.3-32bit/

   (In this case '3.1.0' corresponds to the version of SCIRun,
   'Darwin' to the OS, and 'gcc-3.3-32bit' to the compiler-version-#_of_bits.)

[make-options]:  (Usually: -j#)

   Usually this is used to specify a parallel make.  If you use '-j3',
   then the make system will use 3 processes (processors if you have
   them) when compiling the Thirdparty.


* IMPORTANT NOTES:

          1) You must use an ABSOLUTE path for the installation
             directory (Not, for example: ~/Thirdparty or
             ../Thirdparty).  Remember to create (mkdir) the directory
             before you run the install.

          2) 64 bit installation is not supported on the Macintosh at
             this time.  Please use 32 bits.

          3) The SCIRun Thirdparty install script previously let you
             specify whether you wished to build a 32 or 64 bit
             thirdparty.  However, the script now just automatically
             chooses the default number of bits on your system.
             (While there is no way for the user to override this, it
             doesn't matter as the Thirdparty installation does not
             currently even know how to build non-native bit
             libraries.)

