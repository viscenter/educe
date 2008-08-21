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

if test "$OSNAME" != "Darwin"; then
  cd $DIR/src/libxml2-2.6.22
  ./configure $SHARED_LIBS_FLAG $NO_THREADS --prefix=$DIR CFLAGS="$BITS_FLAG" --without-iconv --without-zlib

  $MAKE $MAKE_FLAGS
  $MAKE install
else
# BUILD INCLUDE FILES ETC  
  cd $DIR/src/
  mkdir $DIR/tmp
  mkdir $DIR/tmp/ppc
  mkdir $DIR/tmp/ppc/src
  mkdir $DIR/tmp/i386
  mkdir $DIR/tmp/i386/src
  
  cp -r libxml2-2.6.22 $DIR/tmp/ppc/src/libxml2-2.6.22
  cp -r libxml2-2.6.22 $DIR/tmp/i386/src/libxml2-2.6.22
  
  cd $DIR/src/libxml2-2.6.22
  ./configure $SHARED_LIBS_FLAG $NO_THREADS --prefix=$DIR CFLAGS="$BITS_FLAG" --without-iconv --without-zlib
  
# BUILD PPC VERSION
  
##########
# Update Makefiles

  cd $DIR/tmp/ppc/src/libxml2-2.6.22
  ./configure $SHARED_LIBS_FLAG $NO_THREADS --prefix=$DIR/tmp/ppc CFLAGS="$BITS_FLAG" --without-iconv --without-zlib
  
  cd $DIR/tmp/i386/src/libxml2-2.6.22
  ./configure $SHARED_LIBS_FLAG $NO_THREADS --prefix=$DIR/tmp/i386 CFLAGS="$BITS_FLAG" --without-iconv --without-zlib  
  
  if test "$BITS" = "64"; then
	cd $DIR/tmp/ppc/src/libxml2-2.6.22
    mv $DIR/tmp/ppc/src/libxml2-2.6.22/libtool $DIR/tmp/ppc/src/libxml2-2.6.22/libtool.old
    sed 's: -dynamiclib: -dynamiclib -arch ppc64 :g' $DIR/tmp/ppc/src/libxml2-2.6.22/libtool.old >$DIR/tmp/ppc/src/libxml2-2.6.22/libtool

    mv $DIR/tmp/ppc/src/libxml2-2.6.22/Makefile $DIR/tmp/ppc/src/libxml2-2.6.22/Makefile.old
    sed 's: gcc: gcc -arch ppc64 :g' $DIR/tmp/ppc/src/libxml2-2.6.22/Makefile.old >$DIR/tmp/ppc/src/libxml2-2.6.22/Makefile 

    mv $DIR/tmp/ppc/src/libxml2-2.6.22/example/Makefile $DIR/tmp/ppc/src/libxml2-2.6.22/example/Makefile.old
    sed 's: gcc: gcc -arch ppc64 :g' $DIR/tmp/ppc/src/libxml2-2.6.22/example/Makefile.old >$DIR/tmp/ppc/src/libxml2-2.6.22/example/Makefile 

    mv $DIR/tmp/ppc/src/libxml2-2.6.22/doc/examples/Makefile $DIR/tmp/ppc/src/libxml2-2.6.22/doc/examples/Makefile.old
    sed 's: gcc: gcc -arch ppc64 :g' $DIR/tmp/ppc/src/libxml2-2.6.22/doc/examples/Makefile.old >$DIR/tmp/ppc/src/libxml2-2.6.22/doc/examples/Makefile 

    mv $DIR/tmp/ppc/src/libxml2-2.6.22/doc/Makefile $DIR/tmp/ppc/src/libxml2-2.6.22/doc/Makefile.old
    sed 's: gcc: gcc -arch ppc64 :g' $DIR/tmp/ppc/src/libxml2-2.6.22/doc/Makefile.old >$DIR/tmp/ppc/src/libxml2-2.6.22/doc/Makefile 

    mv $DIR/tmp/ppc/src/libxml2-2.6.22/xstc/Makefile $DIR/tmp/ppc/src/libxml2-2.6.22/xstc/Makefile.old
    sed 's: gcc: gcc -arch ppc64 :g' $DIR/tmp/ppc/src/libxml2-2.6.22/xstc/Makefile.old >$DIR/tmp/ppc/src/libxml2-2.6.22/xstc/Makefile 
    
    mv $DIR/tmp/ppc/src/libxml2-2.6.22/python/Makefile $DIR/tmp/ppc/src/libxml2-2.6.22/python/Makefile.old
    sed 's: gcc: gcc -arch ppc64 :g' $DIR/tmp/ppc/src/libxml2-2.6.22/python/Makefile.old >$DIR/tmp/ppc/src/libxml2-2.6.22/python/Makefile 

	cd $DIR/tmp/i386/src/libxml2-2.6.22
    mv $DIR/tmp/i386/src/libxml2-2.6.22/libtool $DIR/tmp/i386/src/libxml2-2.6.22/libtool.old
    sed 's: -dynamiclib: -dynamiclib -arch x86_64 :g' $DIR/tmp/i386/src/libxml2-2.6.22/libtool.old >$DIR/tmp/i386/src/libxml2-2.6.22/libtool

    mv $DIR/tmp/i386/src/libxml2-2.6.22/Makefile $DIR/tmp/i386/src/libxml2-2.6.22/Makefile.old
    sed 's: gcc: gcc -arch x86_64 :g' $DIR/tmp/i386/src/libxml2-2.6.22/Makefile.old >$DIR/tmp/i386/src/libxml2-2.6.22/Makefile 

    mv $DIR/tmp/i386/src/libxml2-2.6.22/example/Makefile $DIR/tmp/i386/src/libxml2-2.6.22/example/Makefile.old
    sed 's: gcc: gcc -arch x86_64 :g' $DIR/tmp/i386/src/libxml2-2.6.22/example/Makefile.old >$DIR/tmp/i386/src/libxml2-2.6.22/example/Makefile 

    mv $DIR/tmp/i386/src/libxml2-2.6.22/doc/examples/Makefile $DIR/tmp/i386/src/libxml2-2.6.22/doc/examples/Makefile.old
    sed 's: gcc: gcc -arch x86_64 :g' $DIR/tmp/i386/src/libxml2-2.6.22/doc/examples/Makefile.old >$DIR/tmp/i386/src/libxml2-2.6.22/doc/examples/Makefile 

    mv $DIR/tmp/i386/src/libxml2-2.6.22/doc/Makefile $DIR/tmp/i386/src/libxml2-2.6.22/doc/Makefile.old
    sed 's: gcc: gcc -arch x86_64 :g' $DIR/tmp/i386/src/libxml2-2.6.22/doc/Makefile.old >$DIR/tmp/i386/src/libxml2-2.6.22/doc/Makefile 

    mv $DIR/tmp/i386/src/libxml2-2.6.22/xstc/Makefile $DIR/tmp/i386/src/libxml2-2.6.22/xstc/Makefile.old
    sed 's: gcc: gcc -arch x86_64 :g' $DIR/tmp/i386/src/libxml2-2.6.22/xstc/Makefile.old >$DIR/tmp/i386/src/libxml2-2.6.22/xstc/Makefile 
    
    mv $DIR/tmp/i386/src/libxml2-2.6.22/python/Makefile $DIR/tmp/i386/src/libxml2-2.6.22/python/Makefile.old
    sed 's: gcc: gcc -arch x86_64 :g' $DIR/tmp/i386/src/libxml2-2.6.22/python/Makefile.old >$DIR/tmp/i386/src/libxml2-2.6.22/python/Makefile   
  else
	cd $DIR/tmp/ppc/src/libxml2-2.6.22
    mv $DIR/tmp/ppc/src/libxml2-2.6.22/libtool $DIR/tmp/ppc/src/libxml2-2.6.22/libtool.old
    sed 's: -dynamiclib: -dynamiclib -arch ppc :g' $DIR/tmp/ppc/src/libxml2-2.6.22/libtool.old >$DIR/tmp/ppc/src/libxml2-2.6.22/libtool

    mv $DIR/tmp/ppc/src/libxml2-2.6.22/Makefile $DIR/tmp/ppc/src/libxml2-2.6.22/Makefile.old
    sed 's: gcc: gcc -arch ppc :g' $DIR/tmp/ppc/src/libxml2-2.6.22/Makefile.old >$DIR/tmp/ppc/src/libxml2-2.6.22/Makefile 

    mv $DIR/tmp/ppc/src/libxml2-2.6.22/example/Makefile $DIR/tmp/ppc/src/libxml2-2.6.22/example/Makefile.old
    sed 's: gcc: gcc -arch ppc :g' $DIR/tmp/ppc/src/libxml2-2.6.22/example/Makefile.old >$DIR/tmp/ppc/src/libxml2-2.6.22/example/Makefile 

    mv $DIR/tmp/ppc/src/libxml2-2.6.22/doc/examples/Makefile $DIR/tmp/ppc/src/libxml2-2.6.22/doc/examples/Makefile.old
    sed 's: gcc: gcc -arch ppc :g' $DIR/tmp/ppc/src/libxml2-2.6.22/doc/examples/Makefile.old >$DIR/tmp/ppc/src/libxml2-2.6.22/doc/examples/Makefile 

    mv $DIR/tmp/ppc/src/libxml2-2.6.22/doc/Makefile $DIR/tmp/ppc/src/libxml2-2.6.22/doc/Makefile.old
    sed 's: gcc: gcc -arch ppc :g' $DIR/tmp/ppc/src/libxml2-2.6.22/doc/Makefile.old >$DIR/tmp/ppc/src/libxml2-2.6.22/doc/Makefile 

    mv $DIR/tmp/ppc/src/libxml2-2.6.22/xstc/Makefile $DIR/tmp/ppc/src/libxml2-2.6.22/xstc/Makefile.old
    sed 's: gcc: gcc -arch ppc :g' $DIR/tmp/ppc/src/libxml2-2.6.22/xstc/Makefile.old >$DIR/tmp/ppc/src/libxml2-2.6.22/xstc/Makefile 
    
    mv $DIR/tmp/ppc/src/libxml2-2.6.22/python/Makefile $DIR/tmp/ppc/src/libxml2-2.6.22/python/Makefile.old
    sed 's: gcc: gcc -arch ppc :g' $DIR/tmp/ppc/src/libxml2-2.6.22/python/Makefile.old >$DIR/tmp/ppc/src/libxml2-2.6.22/python/Makefile 

	cd $DIR/tmp/i386/src/libxml2-2.6.22
    mv $DIR/tmp/i386/src/libxml2-2.6.22/libtool $DIR/tmp/i386/src/libxml2-2.6.22/libtool.old
    sed 's: -dynamiclib: -dynamiclib -arch i386 :g' $DIR/tmp/i386/src/libxml2-2.6.22/libtool.old >$DIR/tmp/i386/src/libxml2-2.6.22/libtool

    mv $DIR/tmp/i386/src/libxml2-2.6.22/Makefile $DIR/tmp/i386/src/libxml2-2.6.22/Makefile.old
    sed 's: gcc: gcc -arch i386 :g' $DIR/tmp/i386/src/libxml2-2.6.22/Makefile.old >$DIR/tmp/i386/src/libxml2-2.6.22/Makefile 

    mv $DIR/tmp/i386/src/libxml2-2.6.22/example/Makefile $DIR/tmp/i386/src/libxml2-2.6.22/example/Makefile.old
    sed 's: gcc: gcc -arch i386 :g' $DIR/tmp/i386/src/libxml2-2.6.22/example/Makefile.old >$DIR/tmp/i386/src/libxml2-2.6.22/example/Makefile 

    mv $DIR/tmp/i386/src/libxml2-2.6.22/doc/examples/Makefile $DIR/tmp/i386/src/libxml2-2.6.22/doc/examples/Makefile.old
    sed 's: gcc: gcc -arch i386 :g' $DIR/tmp/i386/src/libxml2-2.6.22/doc/examples/Makefile.old >$DIR/tmp/i386/src/libxml2-2.6.22/doc/examples/Makefile 

    mv $DIR/tmp/i386/src/libxml2-2.6.22/doc/Makefile $DIR/tmp/i386/src/libxml2-2.6.22/doc/Makefile.old
    sed 's: gcc: gcc -arch i386 :g' $DIR/tmp/i386/src/libxml2-2.6.22/doc/Makefile.old >$DIR/tmp/i386/src/libxml2-2.6.22/doc/Makefile 

    mv $DIR/tmp/i386/src/libxml2-2.6.22/xstc/Makefile $DIR/tmp/i386/src/libxml2-2.6.22/xstc/Makefile.old
    sed 's: gcc: gcc -arch i386 :g' $DIR/tmp/i386/src/libxml2-2.6.22/xstc/Makefile.old >$DIR/tmp/i386/src/libxml2-2.6.22/xstc/Makefile 
    
    mv $DIR/tmp/i386/src/libxml2-2.6.22/python/Makefile $DIR/tmp/i386/src/libxml2-2.6.22/python/Makefile.old
    sed 's: gcc: gcc -arch i386 :g' $DIR/tmp/i386/src/libxml2-2.6.22/python/Makefile.old >$DIR/tmp/i386/src/libxml2-2.6.22/python/Makefile 

  fi

  cd $DIR/tmp/ppc/src/libxml2-2.6.22

  $MAKE $MAKE_FLAGS
  $MAKE install

  cd $DIR/tmp/i386/src/libxml2-2.6.22

  $MAKE $MAKE_FLAGS
  $MAKE install


  cd $DIR/src/libxml2-2.6.22
  $MAKE $MAKE_FLAGS
  $MAKE install
  
  cd $DIR
  rm $DIR/lib/libxml2.2.6.22.dylib
  lipo -create $DIR/tmp/i386/lib/libxml2.2.6.22.dylib $DIR/tmp/ppc/lib/libxml2.2.6.22.dylib -output $DIR/lib/libxml2.2.6.22.dylib

  install_name_tool -id $DIR/lib/libxml2.2.6.22.dylib $DIR/lib/libxml2.2.6.22.dylib

  ln -s $DIR/lib/libxml2.2.6.22.dylib $DIR/lib/libxml2.dylib
  rm $DIR/lib/libxml2.a
  lipo -create $DIR/tmp/i386/lib/libxml2.a $DIR/tmp/ppc/lib/libxml2.a -output $DIR/lib/libxml2.a
fi
##########
# Make


