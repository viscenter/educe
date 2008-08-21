#
# parms INSTALL_DIR
#       OSNAME       [eg: Darwin]
#       SH_LIB_EXT   [eg: dylib]
#       BITS         [eg: 32|64]
#       TAR          [eg: gtar, a Gnu tar program]
#       SHARED_LIBS  [eg: DEFAULT|ENABLE|DISABLE]
#       MAKE_FLAGS   [eg: -j n]
#       

DIR=$1
OSNAME=$2
SH_LIB_EXT=$3
BITS=$4
TAR=$5
SHARED_LIBS=$6
MAKE_FLAGS=$7

if test "$SHARED_LIBS" = "DEFAULT"; then 
    SHARED_LIBS_FLAG=
elif test "$SHARED_LIBS" = "ENABLE"; then 
    SHARED_LIBS_FLAG="--enable-shared"
elif test "$SHARED_LIBS" = "DISABLE"; then 
    SHARED_LIBS_FLAG="--disable-shared"
else
    echo ""
    echo "ERROR: install-Teem.sh: SHARED_LIBS value ($SHARED_LIBS) is invalid."
    echo ""
    exit
fi

if test "$BITS" = "64"; then 
    BITS_FLAG="--enable-64bit"

    if test "$OSNAME" = "Linux"; then
        # Determine if this Itanium (ia64) or Athlon (amd - x86_64)
        the_type=`uname -m | grep ia64`
        if test -n "$the_type"; then
            the_type=ia
        else
            the_type=amd
        fi
    fi

else
    BITS_FLAG=
fi

echo
echo "Installing TEEM: $DIR $OSNAME $SH_LIB_EXT $BITS $TAR $SHARED_LIBS $MAKE_FLAGS"
echo

############
# Unpack:
	
install_source_dir=`pwd`
cd $DIR/src

if test -d $DIR/src/teem-1.9.0-src; then
    if test "$DO_NOT_UNTAR" != "yes"; then
        echo "It appears that Teem has already been untarred.  Do you wish to untar it again? (y/n)"
        read answer
        echo
        if test "$answer" = "y"; then
           $TAR zxf $install_source_dir/Tarballs/teem-1.9.0-src.tar.gz
        fi
    fi
else
    $TAR zxf $install_source_dir/Tarballs/teem-1.9.0-src.tar.gz
fi




################################
#    - Set up PNG/ZLIB variables for TEEM build.
#
# libpng/libz are usually built in the thirdparty.
# look for them there first.
#
PNG_INC_DIR=$DIR/include
ZLIB_INC_DIR=$DIR/include

# Seg3D doesn't use any of thirdparty on Linux except for teem, so
# /usr/include should be looked at if we didn't bother to build the
# heavyweight thirdparty.  There's probably a better way to let this
# script know we want a lightweight thirdparty build.
if test "$OSNAME" = "Linux"; then
    if test ! -f $DIR/include/png.h; then
        PNG_INC_DIR=/usr/include
    fi
    if test ! -f $DIR/include/zlib.h; then
        ZLIB_INC_DIR=/usr/include
    fi
fi

# on Darwin use the build in version
export TEEM_PNG=
export TEEM_PNG_IPATH=-I$PNG_INC_DIR
export TEEM_PNG_LPATH=-L$DIR/lib

export TEEM_ZLIB=
export TEEM_ZLIB_IPATH=-I$ZLIB_INC_DIR
export TEEM_ZLIB_LPATH=-L$DIR/lib


# Verify that png.h and zlib.h exist...
if test ! -f $PNG_INC_DIR/png.h; then
   echo 
   echo "ERROR: png.h not found in $PNG_INC_DIR"
   echo 
   exit
fi

if test ! -f $ZLIB_INC_DIR/zlib.h; then
   echo 
   echo "ERROR: zlib.h not found in $ZLIB_INC_DIR"
   echo 
   exit
fi

echo

############
# Patch:

echo
echo "#### PATCHING Teem FILES (if patches exist for this architecture) ####"
echo

PATCH_FILE=$install_source_dir/Patches/$OSNAME'_'$BITS/teem.patch

if test -f $PATCH_FILE; then
    echo "#### PATCHING $OSNAME SPECIFIC TEEM FILES ####"
    echo
    patch -p0 < $PATCH_FILE
    echo
fi

# One special case, there is a patch for Teem on intel macs
if test "$OSNAME" = "Darwin"; then

  mv $DIR/src/teem-1.9.0-src/src/air/endianAir.c $DIR/src/teem-1.9.0-src/src/air/endianAir.c.old
  sed 's:if TEEM_ENDIAN == 1234:ifdef __LITTLE_ENDIAN__:g' $DIR/src/teem-1.9.0-src/src/air/endianAir.c.old >$DIR/src/teem-1.9.0-src/src/air/endianAir.c
  mv $DIR/src/teem-1.9.0-src/src/air/privateAir.h $DIR/src/teem-1.9.0-src/src/air/privateAir.h.old
  sed 's:if TEEM_ENDIAN == 1234:ifdef __LITTLE_ENDIAN__:g' $DIR/src/teem-1.9.0-src/src/air/privateAir.h.old >$DIR/src/teem-1.9.0-src/src/air/privateAir.h

  if test "$BITS" = "64"; then
    mv $DIR/src/teem-1.9.0-src/src/make/darwin.mk $DIR/src/teem-1.9.0-src/src/make/darwin.mk.old
    sed 's:TEEM_32BIT = 1:TEEM_32BIT = 0:g' $DIR/src/teem-1.9.0-src/src/make/darwin.mk.old >$DIR/src/teem-1.9.0-src/src/make/darwin.mk
    mv $DIR/src/teem-1.9.0-src/src/make/darwin.mk $DIR/src/teem-1.9.0-src/src/make/darwin.mk.old
    sed 's:LD = gcc:LD = gcc -arch x86_64 -arch ppc64:g' $DIR/src/teem-1.9.0-src/src/make/darwin.mk.old >$DIR/src/teem-1.9.0-src/src/make/darwin.mk
    mv $DIR/src/teem-1.9.0-src/src/make/darwin.mk $DIR/src/teem-1.9.0-src/src/make/darwin.mk.old
    sed 's:OPT_CFLAG ?= -O2:OPT_CFLAG ?= -O2 -arch ppc64 -arch x86_64:g' $DIR/src/teem-1.9.0-src/src/make/darwin.mk.old >$DIR/src/teem-1.9.0-src/src/make/darwin.mk
  fi
  
  if test "$BITS" = "32"; then
    mv $DIR/src/teem-1.9.0-src/src/make/darwin.mk $DIR/src/teem-1.9.0-src/src/make/darwin.mk.old
    sed 's:TEEM_32BIT = 1:TEEM_32BIT = 1:g' $DIR/src/teem-1.9.0-src/src/make/darwin.mk.old >$DIR/src/teem-1.9.0-src/src/make/darwin.mk
    mv $DIR/src/teem-1.9.0-src/src/make/darwin.mk $DIR/src/teem-1.9.0-src/src/make/darwin.mk.old
    sed 's:LD = gcc:LD = gcc -arch i386 -arch ppc:g' $DIR/src/teem-1.9.0-src/src/make/darwin.mk.old >$DIR/src/teem-1.9.0-src/src/make/darwin.mk
    mv $DIR/src/teem-1.9.0-src/src/make/darwin.mk $DIR/src/teem-1.9.0-src/src/make/darwin.mk.old
    sed 's:OPT_CFLAG ?= -O2:OPT_CFLAG ?= -O2 -arch i386 -arch ppc:g' $DIR/src/teem-1.9.0-src/src/make/darwin.mk.old >$DIR/src/teem-1.9.0-src/src/make/darwin.mk
  fi

  mv $DIR/src/teem-1.9.0-src/src/GNUmakefile $DIR/src/teem-1.9.0-src/src/GNUmakefile.old
  sed 's:$(LD) -o $@:$(LD) -o $@ -install_name $@ :g' $DIR/src/teem-1.9.0-src/src/GNUmakefile.old >$DIR/src/teem-1.9.0-src/src/GNUmakefile
fi

############
# Fix Makefiles

cd $DIR/src/teem-1.9.0-src/src


# Change for capital letters to lower case letters:
lower_osname=`echo $OSNAME | awk '{ print tolower($1); }'`

export TEEM_ARCH=$lower_osname.$the_type$BITS
export TEEM_DEST=$DIR

# Teem now has different archtectures for Itanium and AMD64
if test "$MACHTYPE" = "ia64"; then
    export TEEM_ARCH=$lower_osname.ia64
fi

# IRIX and AIX are slightly different so hardcode them below (if on those platforms).
if test "$OSNAME" = "IRIX64"; then
    if test "$BITS" = "64"; then
        export TEEM_ARCH=irix6.64
    else
        export TEEM_ARCH=irix6.n32
    fi
elif test "$OSNAME" = "AIX"; then
    export TEEM_ARCH=aix
elif test "$OSNAME" = "Darwin"; then
    export TEEM_SHEXT=dylib
fi

### Fix the GNUmakefile...
if test "$OSNAME" = "Darwin"; then
  # Must use -single_modlue for linking... 
  cp GNUmakefile GNUmakefile.work
  sed "s,^LDFLAGS +=,LDFLAGS += -single_module,g" GNUmakefile.work > GNUmakefile
  rm -f GNUmakefile.work

fi

# AIX does not handle shared libraries
if test "$OSNAME" != "AIX"; then
    export TEEM_LINK_SHARED=true
fi

############
# Make

#    Can't use MAKE_FLAGS (specifically -j# as this can cause TEEM to build incorrectly.)
$MAKE


# Comment in when testing:
#echo "setenv TEEM_ARCH $TEEM_ARCH"
#echo "setenv TEEM_DEST $TEEM_DEST"
#echo "setenv TEEM_SHEXT $TEEM_SHEXT"
#echo "setenv TEEM_LINK_SHARED $TEEM_LINK_SHARED"
