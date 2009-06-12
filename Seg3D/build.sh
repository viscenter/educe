#!/bin/bash
#  
#  For more information, please see: http://software.sci.utah.edu
#  
#  The MIT License
#  
#  Copyright (c) 2006 Scientific Computing and Imaging Institute,
#  University of Utah.
#  
#  License for the specific language governing rights and limitations under
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#  
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
#  
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#  
#    File   : build.sh
#    Author : McKay Davis
#    Date   : Tue Dec 19 15:35:50 2006


# SCIRun 3.0.0 build script
#
# This script will build SCIRun from scratch
#



SCIRUN_VERSION=3.2.0
INSIGHT_TOOLKIT=InsightToolkit-3.14.0
WXWIDGETS_TOOLKIT_MAC=wxMac-2.8.7
WXWIDGETS_TOOLKIT_ARG_MAC="--with-mac"
WXWIDGETS_TOOLKIT_GTK=wxGTK-2.8.7
WXWIDGETS_TOOLKIT_ARG_GTK="--with-gtk"
CMAKE=cmake-2.4.7

printhelp() {
    echo -e "--cmake-args=[string]\t\tCommand line arguments to CMake"
    echo -e "--debug\t\tBuilds SCIRun and ITK with debug symbols"
    echo -e "--release\t\tBuilds SCIRun and ITK without debug symbols"
    echo -e "--with-itk\t\tDownloads and builds $INSIGHT_TOOLKIT and the Insight SCIRun package"
    echo -e "--get-cmake\t\tDownloads and builds $CMAKE, regarless of the version installed on the system."
    echo -e "--static\t\tBuilds static libs"
    echo -e "--seg3d-only\t\tOnly builds static version of Seg3D PowerApp"
    echo -e "--packages=[string]\t\tComma seperated string specifies which SCIRun packages are built"
    echo -e "--no-ctest\t\tDo not run ctest"
    echo -e "--64bit\t\t Force a 64bit SCIRun version [OSX only]"
    echo -e "-j#\t\tRuns # parallel make processes when building"
    echo -e "-?\t\tThis help"
    exit 0
}

# will cause the script to bailout if the passed in command fails
try () {
  $*
  if [ $? != "0" ]; then
      echo -e "\n***ERROR in build script\nThe failed command was:\n$*\n"
      exit 1
  fi
}

trybuild () {
  $*
  if [ $? != "0" ]; then
      echo -e "Building SCIRun returned an error\n"
      echo -e "Either the code failed to build properly or\n"
      echo -e "the testing programs failed to complete without\n"
      echo -e "every single test program passing the test.\n"
      exit 1
  fi
}
# functionally equivalent to try(),
# but it prints a different error message
ensure () {
  $* >& /dev/null
  if [ $? != "0" ]; then
      echo -e "\n***ERROR, $* is required but not found on this system\n"
      exit 1
  fi
}


check_thirdparty_install() {
    if [ -e "$DIR/thirdparty.src/install_command.txt" ]; then
        export SCIRUN_THIRDPARTY_DIR=`grep SCIRUN_THIRDPARTY_DIR $DIR/thirdparty.src/install_command.txt | awk -F '\=' '{print $2}'`
    fi
    
    try echo "SCIRUN_THIRDPARTY_DIR=$SCIRUN_THIRDPARTY_DIR"
    
    if [ -e "$SCIRUN_THIRDPARTY_DIR" ]; then
        rebuild_thirdparty=0
    else
        rebuild_thirdparty=1
    fi 
}


# Try to find a version of cmake
find_cmake() {
    if [ "$getcmake" != "1" ]; then
        cmakebin=`which cmake`
        ctestbin=`which ctest`
    fi
    
    #if it is not found
    if [ ! -e "$cmakebin" ]; then
    # then look for our own copy made by this script previously
        cmakebin=$DIR/cmake/local/bin/cmake
        ctestbin=$DIR/cmake/local/bin/ctest
        try mkdir -p $DIR/cmake/
        try cd $DIR/cmake
        if [ ! -e "$cmakebin" ]; then
        # try to downlaod and build our own copy in local
            try $getcommand http://www.cmake.org/files/v2.4/${CMAKE}.tar.gz
            try tar xvzf ${CMAKE}.tar.gz
            try cd $CMAKE
            try ./configure --prefix=$DIR/cmake/local
            try make $makeflags
            try make install
        fi
    fi
    
    ctestbin="$ctestbin --ctest-config=${DIR}/src/CTestConfig.cmake"
    
    echo cmakebin=$cmakebin
    echo ctestbin=$ctestbin
    ensure $cmakebin --version
}


build_ITK() {
    if [ $builditk == "1" ]; then
        try cd $DIR
        if [ ! -e "$DIR/${INSIGHT_TOOLKIT}.tar.gz" ]; then 
            try echo "Downloading the Insight Toolkit (ITK)..."
            try $getcommand http://downloads.sourceforge.net/itk/${INSIGHT_TOOLKIT}.tar.gz
        fi
        
        if [ ! -e "$DIR/$INSIGHT_TOOLKIT" ]; then
            try tar xvzf ${INSIGHT_TOOLKIT}.tar.gz
        fi
        
        if [ "$osx" == "0" ]; then
          try cd $DIR/$INSIGHT_TOOLKIT
          try $cmakebin . -DBUILD_EXAMPLES=0 -DBUILD_TESTING=0 -DCMAKE_INSTALL_PREFIX=$SCIRUN_THIRDPARTY_DIR  -DBUILD_SHARED_LIBS=$buildsharedlibs -DCMAKE_BUILD_TYPE=$buildtype -DITK_USE_SYSTEM_PNG=ON -DITK_USE_SYSTEM_TIFF=ON -DITK_USE_SYSTEM_ZLIB=ON
          try make $makeflags
          try make install
          # Ensure that the insight installation suceeded.
          try cd $SCIRUN_THIRDPARTY_DIR/lib/InsightToolkit
          try mkdir -p $SCIRUN_THIRDPARTY_DIR/include/InsightToolkit/itkExtHdrs
          try cp $DIR/$INSIGHT_TOOLKIT/Utilities/itkExtHdrs/itkAnalyzeDbh.h $SCIRUN_THIRDPARTY_DIR/include/InsightToolkit/itkExtHdrs/itkAnalyzeDbh.h
        else
          # Cannot install programs with cmake under OSX as libraries
          # are improperly linked using cmake
          try cd $DIR/$INSIGHT_TOOLKIT
          try $cmakebin . -DBUILD_EXAMPLES=0 -DBUILD_TESTING=0 -DBUILD_SHARED_LIBS=$buildsharedlibs -DCMAKE_BUILD_TYPE=$buildtype
          try make $makeflags
          cmakeargs="${cmakeargs} -DITK_DIR=$DIR/$INSIGHT_TOOLKIT"          
        fi
    fi
}


build_wxWidgets() {
    if [ $buildwx == "1" ]; then
        # Set up which one to build based upon architecture.
        WXWIDGETS_TOOLKIT=$WXWIDGETS_TOOLKIT_GTK
        WXWIDGETS_TOOLKIT_ARG=$WXWIDGETS_TOOLKIT_ARG_GTK
        if [ "$osx" == "1" ]; then
            WXWIDGETS_TOOLKIT=$WXWIDGETS_TOOLKIT_MAC
            WXWIDGETS_TOOLKIT_ARG=$WXWIDGETS_TOOLKIT_ARG_MAC
        fi
        try cd $DIR
        if [ ! -e "$DIR/${WXWIDGETS_TOOLKIT}.tar.gz" ]; then 
            try echo "Downloading wxWidgets..."
            try $getcommand http://downloads.sourceforge.net/wxwindows/${WXWIDGETS_TOOLKIT}.tar.gz
        fi
        
        if [ ! -e "$DIR/$WXWIDGETS_TOOLKIT" ]; then
            try tar xvzf ${WXWIDGETS_TOOLKIT}.tar.gz
        fi
        
        try cd $DIR/$WXWIDGETS_TOOLKIT
        try ./configure $WXWIDGETS_TOOLKIT_ARG --with-opengl --disable-shared --prefix=$SCIRUN_THIRDPARTY_DIR
        try make $makeflags
        try make install

        # Ensure that the wxwidgets installation suceeded
        try cd $SCIRUN_THIRDPARTY_DIR/lib/wx
        cmakewxargs="-DWITH_WXWIDGETS=ON -DwxWidgets_CONFIG_EXECUTABLE=${SCIRUN_THIRDPARTY_DIR}/bin/wx-config"
    fi
}


find_svn() {
    # Try to find a version of svn
    svnbin=`which svn`
    echo svnbin=$svnbin
}

build_thirdparty() {
    try cd $DIR/thirdparty.src
    
    # If this script is running in a SVN tree, see if it needs to be updated
    if [ -e "$DIR/thirdparty.src/.svn" ] && [ -e "$svnbin" ]; then 
        tpurl=`$svnbin info . | grep URL | cut -d" " -f 2`
        local=`$svnbin info . | grep Revision | cut -d" " -f 2`
        remote=`$svnbin info $tpurl | grep Revision | cut -d" " -f 2`
        try echo "thirdparty.src URL = $tpurl"
        try echo "local thirdparty revision = $local"
        try echo "repository thirdparty revision = $remote"
        if test "$local" = "$local"; then
            check_thirdparty_install
        else
            echo "Thirdparty at $tpurl has changed, updating..."
            try $svnbin update
            rebuild_thirdparty=1
        fi
    else
        check_thirdparty_install
    fi
    
    if [ $rebuild_thirdparty == "1" ]; then
        echo "Rebuilding Thirdparty..."
        try rm -rf $DIR/thirdparty.bin/*
        try mkdir -p $DIR/thirdparty.bin
        if [ $darwin64 == "1" ]; then
          try ./install.sh --64bit $DIR/thirdparty.bin $makeflags        
        else
          try ./install.sh $DIR/thirdparty.bin $makeflags
        fi
    else
        echo "Thirdparty already built, skipping...";
    fi

    if [ -e "$DIR/thirdparty.src/install_command.txt" ]; then
        export SCIRUN_THIRDPARTY_DIR=`grep SCIRUN_THIRDPARTY_DIR $DIR/thirdparty.src/install_command.txt | awk -F '\=' '{print $2}'`
    fi

    if [ "$buildsharedlibs" == "0" ]; then
        echo "Removing dynamic libs from thirdparty..."
        try rm -rf $SCIRUN_THIRDPARTY_DIR/lib/*.so
        try rm -rf $SCIRUN_THIRDPARTY_DIR/lib/*.dylib
    fi
}  


update_scirun_src() {
    echo "Updating SCIRUN source..."

    # If the source tree is from SVN, update it too
    if [ -e "$DIR/src/.svn" ] && [ -e "$svnbin" ]; then 
        try svn update "$DIR/src"
    fi
}
    
configure_scirun_bin() {
    try cd $DIR/bin
    if [ "$darwin64" == "1" ]; then
      macflags="-DSCIRUN_ENABLE_64BIT=ON"
      echo -e "CONFIGURING SCIRUN TO BE 64BITS"
    else
      macflags=
    fi
    $cmakebin ../src -DBUILD_SHARED_LIBS=$buildsharedlibs -DBUILD_DATAFLOW=$builddataflow -DCMAKE_BUILD_TYPE=$buildtype -DLOAD_PACKAGE=$scirunpackages -DWITH_X11=$withx11 -DBUILD_TESTING=$testnetworks -DBUILD_UTILS=$buildutils $cmakewxargs $cmakeargs $macflags
}

build_scirun_bin() {
    if [ "$usectest" == "1" ] && [ -e "$ctestbin" ]; then
        echo "Building SCIRun using ctest..."
        trybuild $ctestbin -VV -D Experimental -A $DIR/bin/CMakeCache.txt
    else 
	echo "Building SCIRun using make..."
        trybuild make $makeflags
    fi
}


######### build.sh script execution starts here

export DIR=`pwd`
linux=0
osx=0

if test `uname` = "Darwin"; then
    getcommand="curl -OL"
    osx=1
elif test `uname` = "Linux"; then
    getcommand="wget"
    linux=1
else
    echo "Unsupported system.  Please run on OSX or Linux"
    exit 1
fi

buildtype="Release"
buildsharedlibs="1"
builddataflow="1"
makeflags=""
cmakeflags=""
builditk=0
buildwx=0
cmakewxargs=""
getcmake=0
withx11=1
scirunpackages="SCIRun,BioPSE,Teem,MatlabInterface"
cmakeargs=""
usectest="0"
testnetworks="0"
buildutils="1"
darwin64="0"

echo "Parsing arguments..."
while [ "$1" != "" ]; do
    case "$1" in
        --debug)
            buildtype="Debug";;
        --release)
            buildtype="Release";;
        --with-itk) 
            builditk="1";;
        --with-wxwidgets) 
            buildwx="1";;
        --get-cmake) 
            getcmake="1";;
        --no-ctest) 
            usectest="0";;
        --ctest) 
            usectest="1";;
        --test-networks)
            testnetworks="1";;
        --static) 
            buildsharedlibs="0"
            builddataflow="0";;
        --seg3d-only) 
            builddataflow="0"
            buildsharedlibs="0"
            builditk="1"
            buildutils="0"
            if [ "$osx" == "1" ]; then
                withx11="0"
            fi;;
       --64bit)
	    if [ "$osx" == "1" ]; then
		darwin64="1"
            else
		echo "Only OSX supporsts the selection between 32 and 64bit architectures."
		echo "On other platforms SCIRub uses the architecture of the operating system."
	    fi;;
       --packages=*)
            scirunpackages=`echo $1 | awk -F '\=' '{print $2}'`;;
        --cmake-args=*)
            cmakeargs=`echo $1 | cut -c 14-`;;
        -j*) 
            makeflags="${makeflags} $1";;
        -D*) 
            cmakeflags="${cmakeflags} $1";;
        -?|--?|-help|--help) 
            printhelp;;
        *) 
            echo \`$1\' parameter ignored;;
    esac
    shift 1
done

cmakeargs="${cmakeargs} ${cmakeflags}"

echo "CMake args: $cmakeargs"
echo "Build Type: $buildtype"
echo "Build SCIRun Dataflow: $builddataflow"
if [ "$builddataflow" != "0" ]; then
    echo "SCIRun Packages: $scirunpackages"
fi
echo "Build Insight ToolKit: $builditk"
echo "Build wxWidgets: $buildwx"
echo "Get CMake: $getcmake"
echo "Make Flags: $makeflags"
echo "Build Shared Libs: $buildsharedlibs"
echo "With X11: $withx11"

ensure make --version # ensure make is on the system

find_cmake

find_svn

build_thirdparty

build_ITK

build_wxWidgets

update_scirun_src

configure_scirun_bin

build_scirun_bin

