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
    echo "ERROR: install-freetype.sh: SHARED_LIBS value ($SHARED_LIBS) is invalid."
    echo ""
    exit
fi

if test "$BITS" = "64"; then 
    CFLAGS="-fPIC"
    CXXFLAGS=$CFLAGS
else
    CFLAGS="-fPIC"
    CXXFLAGS=$CFLAGS
fi

if test "$BITS" = "64" && test "$OSNAME" = "Darwin"; then 
  CFLAGS="'-m64  -arch ppc64 -arch x86_64 '"
  CXXFLAGS="'-m64  -arch ppc64 -arch x86_64 '"
  LDFLAGS=$CFLAGS
fi

if test "$BITS" = "32" && test "$OSNAME" = "Darwin"; then 
  CFLAGS="'-arch ppc -arch i386 '"
  CXXFLAGS="'-arch ppc -arch i386 '"
  LDFLAGS=$CFLAGS
fi

echo
echo "Installing freetype: $DIR $OSNAME $SH_LIB_EXT $BITS $TAR $SHARED_LIBS $MAKE_FLAGS"
echo

############
# Unpack:
	
install_source_dir=`pwd`
cd $DIR/src

if test -d $DIR/src/freetype-2.3.5; then
    if test "$DO_NOT_UNTAR" != "yes"; then
        echo "It appears that Freetype has already been untarred.  Do you wish to untar it again? (y/n)"
        read answer
        echo
        if test "$answer" = "y"; then
            $TAR zxf $install_source_dir/Tarballs/freetype-2.3.5.tar.gz
        fi
    fi
else
    $TAR zxf $install_source_dir/Tarballs/freetype-2.3.5.tar.gz
fi

############
# Patch:

# OS Patch
PATCH_FILE=$install_source_dir/Patches/$OSNAME'_'$BITS/freetype.patch

if test -f $PATCH_FILE; then
    echo "#### PATCHING $OSNAME SPECIFIC FREETYPE FILES ####"
    echo
    cat $PATCH_FILE | patch -p0
    echo
fi
   
############
# Configure: 

cd $DIR/src/freetype-2.3.5

# Specifying the GNUMAKE variable is necessary on SGI.
GNUMAKE=$MAKE ./configure $SHARED_LIBS_FLAG --prefix=$DIR $BITS_FLAG \
    CXXFLAGS="$CXXFLAGS" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS"

############
# Make


if test "$OSNAME" = "Darwin"; then
  if test "$BITS" = "64"; then
    mv $DIR/src/freetype-2.1.10/builds/unix/libtool $DIR/src/freetype-2.1.10/builds/unix/libtool.old
    sed 's:dynamiclib:dynamiclib -arch ppc64 -arch x86_64:g' $DIR/src/freetype-2.1.10/builds/unix/libtool.old >$DIR/src/freetype-2.1.10/builds/unix/libtool
    chmod 0777 $DIR/src/freetype-2.1.10/builds/unix/libtool
    mv $DIR/src/freetype-2.1.10/builds/unix/unix-cc.mk $DIR/src/freetype-2.1.10/builds/unix/unix-cc.mk.old
    sed 's:gcc:gcc -arch ppc64 -arch x86_64:g' $DIR/src/freetype-2.1.10/builds/unix/unix-cc.mk.old >$DIR/src/freetype-2.1.10/builds/unix/unix-cc.mk
  fi

  if test "$BITS" = "32"; then
    mv $DIR/src/freetype-2.1.10/builds/unix/libtool $DIR/src/freetype-2.1.10/builds/unix/libtool.old
    sed 's:dynamiclib:dynamiclib -arch ppc -arch i386:g' $DIR/src/freetype-2.1.10/builds/unix/libtool.old >$DIR/src/freetype-2.1.10/builds/unix/libtool
    chmod 0777 $DIR/src/freetype-2.1.10/builds/unix/libtool
    mv $DIR/src/freetype-2.1.10/builds/unix/unix-cc.mk $DIR/src/freetype-2.1.10/builds/unix/unix-cc.mk.old
    sed 's:gcc:gcc -arch ppc -arch i386:g' $DIR/src/freetype-2.1.10/builds/unix/unix-cc.mk.old >$DIR/src/freetype-2.1.10/builds/unix/unix-cc.mk
  fi
fi  

$MAKE $MAKE_FLAGS
$MAKE install

