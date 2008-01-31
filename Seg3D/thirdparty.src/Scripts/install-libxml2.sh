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
    echo "ERROR: install-libxml2.sh: SHARED_LIBS value ($SHARED_LIBS) is invalid."
    echo ""
    exit
fi

if test "$BITS" = "64"; then 
    if test "$OSNAME" = "IRIX64"; then
        NO_THREADS="--without-threads"
        BITS_FLAG="-64"
    elif test "$OSNAME" = "AIX"; then
        BITS_FLAG="-q64"
    fi
else
    if test "$OSNAME" = "IRIX64"; then
        NO_THREADS="--without-threads"
        BITS_FLAG="-n32"
    elif test "$OSNAME" = "AIX"; then
        BITS_FLAG="-q32"
    fi
fi

echo
echo "Installing libxml2: $DIR $OSNAME $SH_LIB_EXT $BITS $TAR $SHARED_LIBS $MAKE_FLAGS"
echo

give_untar_time_warning()
{
    echo
    echo "Untarring libxml2 source... this may take a minute or so..."
    echo
}

##########
# Unpack:

install_source_dir=`pwd`
cd $DIR/src
if test -d $DIR/src/libxml2-2.6.22; then
    if test "$DO_NOT_UNTAR" != "yes"; then
        echo "It appears that libXML2 has already been untarred.  Do you wish to untar it again? (y/n)"
        read answer
        echo
        if test "$answer" = "y"; then
            give_untar_time_warning
            $TAR zxf $install_source_dir/Tarballs/libxml2-sources-2.6.22.tar.gz
        fi
    fi
else
    give_untar_time_warning
    $TAR zxf $install_source_dir/Tarballs/libxml2-sources-2.6.22.tar.gz
fi


##########
# Patch:


# Generic Patch
if test `uname -s` != "AIX"; then
    echo ""
    echo "Patching libxml2..."

    PATCH_FILE=$install_source_dir/Patches/Generic/libxml2.patch 
    cat $PATCH_FILE | patch -p0
fi

echo ""

##########
# Configure:

cd $DIR/src/libxml2-2.6.22

./configure $SHARED_LIBS_FLAG $NO_THREADS --prefix=$DIR CFLAGS="$BITS_FLAG" --without-iconv --without-zlib

##########
# Update Makefiles

if test "$BITS" = "64" && test "$OSNAME" = "Darwin"; then
  mv $DIR/src/libxml2-2.6.22/Makefile $DIR/src/libxml2-2.6.22/Makefile.old
  sed 's: gcc: gcc -m64:g' $DIR/src/libxml2-2.6.22/Makefile.old >$DIR/src/libxml2-2.6.22/Makefile 
  mv $DIR/src/libxml2-2.6.22/example/Makefile $DIR/src/libxml2-2.6.22/example/Makefile.old
  sed 's: gcc: gcc -m64:g' $DIR/src/libxml2-2.6.22/example/Makefile.old >$DIR/src/libxml2-2.6.22/example/Makefile 
  mv $DIR/src/libxml2-2.6.22/xstc/Makefile $DIR/src/libxml2-2.6.22/xstc/Makefile.old
  sed 's: gcc: gcc -m64:g' $DIR/src/libxml2-2.6.22/xstc/Makefile.old >$DIR/src/libxml2-2.6.22/xstc/Makefile 
  mv $DIR/src/libxml2-2.6.22/python/Makefile $DIR/src/libxml2-2.6.22/python/Makefile.old
  sed 's: gcc: gcc -m64:g' $DIR/src/libxml2-2.6.22/python/Makefile.old >$DIR/src/libxml2-2.6.22/python/Makefile 
fi

##########
# Make
$MAKE $MAKE_FLAGS
$MAKE install

