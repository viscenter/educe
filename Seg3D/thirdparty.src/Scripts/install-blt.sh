#
# parms INSTALL_DIR
#       OSNAME       [eg: Darwin]
#       SH_LIB_EXT   [eg: dylib]
#       BITS         [eg: 32|64]
#       TAR          [eg: gtar, a Gnu tar program]
#       SHARED_LIBS  [eg: --disable-shared]
#       MAKE_FLAGS   [eg: -j n]
#       

DIR=$1
OSNAME=$2
SH_LIB_EXT=$3
BITS=$4
TAR=$5
SHARED=$6
MAKE_FLAGS=$7

if test "$SHARED" = "YES"; then 
    SHARED_LIBS_FLAG="--enable-shared"
else
    SHARED_LIBS_FLAG=
fi

if test "$BITS" = "64"; then 
    BITS_FLAG="--enable-64bit"
else
    BITS_FLAG=
fi

echo
echo "Installing BLT: $DIR $OSNAME $SH_LIB_EXT $BITS $TAR $SHARED $MAKE_FLAGS"
echo

############
# Unpack:
	
install_source_dir=`pwd`
cd $DIR/src

if test -d $DIR/src/blt2.4z; then
    if test "$DO_NOT_UNTAR" != "yes"; then
        echo "It appears that BLT has already been untarred.  Do you wish to untar it again? (y/n)"
        read answer
        echo
        if test "$answer" = "y"; then
            $TAR zxf $install_source_dir/Tarballs/BLT2.4z.tar.gz
        fi
    fi
else
    $TAR zxf $install_source_dir/Tarballs/BLT2.4z.tar.gz
fi

if test ! -L blt; then
   ln -fs blt2.4z blt
fi

############
# Patch:

PATCH_FILE=$install_source_dir/Patches/$OSNAME'_'$BITS/blt-2.4z-1.patch

if test -f $PATCH_FILE; then
    echo "#### PATCHING $OSNAME SPECIFIC BLT FILES ####"
    echo
    sed 's,<PREFIX>, '$DIR' ,g' < $PATCH_FILE | patch -p0
    echo
    echo "#### DONE PATCHING FILES ####"
    echo
fi
   
############
# Configure: 

cd $DIR/src/blt2.4z

BLT_COMPILER=
WITH_CLFAGS=

if test "$OSNAME" = "Darwin"; then
    WITH_CFLAGS="--with-cflags='-O6 -fno-common'"
    BLT_COMPILER=
elif test "$OSNAME" = "IRIX64"; then
    BLT_COMPILER="--with-cc=cc"
elif test "$OSNAME" = "AIX"; then
    BLT_COMPILER="--with-cc=$CC"
fi

echo
echo Running: ./configure $SHARED_LIBS_FLAG --prefix $DIR $BITS_FLAG $WITH_CFLAGS $BLT_COMPILER
echo

# Need the 'eval' to pass the args through correctly as 'complete' args
# (ie: for those with spaces in them, even when they have quotes around
# them).
eval ./configure $SHARED_LIBS_FLAG --prefix=$DIR $BITS_FLAG $WITH_CFLAGS $BLT_COMPILER --with-tcl=$DIR/lib --with-tk=$DIR/lib

############
# Update Makefiles

if test "$BITS" = "64" && test "$OSNAME" = "IRIX64"; then 
    echo
    echo "#### FIXING MAKEFILE FOR 64bit SGI BUILD ####"
    echo
    cd src
    mv Makefile Makefile.old
    sed -e "s/-n32/-64/g" -e "s/CFLAGS =/CFLAGS = -64 /g" -e "s/-lnsl//g" Makefile.old > Makefile

    cd shared
    mv Makefile Makefile.old
    sed -e "s/-lnsl//g" -e "s/CFLAGS =/CFLAGS = -64 /g" -e "s/-n32/-64/g" Makefile.old > Makefile
    cd ../..
fi

if test "$BITS" = "64" && test "$OSNAME" = "Darwin"; then 
    echo
    echo "#### FIXING MAKEFILE FOR 64bit DARWIN BUILD ####"
    echo

    mv $DIR/src/blt/src/Makefile $DIR/src/blt/src/Makefile.old
    sed -e 's:cc:cc -m64 -arch ppc64 -arch x86_64:g' $DIR/src/blt/src/Makefile.old >$DIR/src/blt/src/Makefile
    mv $DIR/src/blt/src/Makefile $DIR/src/blt/src/Makefile.old
    sed -e 's:gcc -m64 -arch ppc64 -arch x86_64-:gcc-:g' $DIR/src/blt/src/Makefile.old >$DIR/src/blt/src/Makefile
    mv $DIR/src/blt/src/shared/Makefile $DIR/src/blt/src/shared/Makefile.old
    sed -e 's:cc:cc -m64 -arch ppc64 -arch x86_64:g' $DIR/src/blt/src/shared/Makefile.old >$DIR/src/blt/src/shared/Makefile
    mv $DIR/src/blt/src/shared/Makefile $DIR/src/blt/src/shared/Makefile.old
    sed -e 's:gcc -m64 -arch ppc64 -arch x86_64-:gcc-:g' $DIR/src/blt/src/shared/Makefile.old >$DIR/src/blt/src/shared/Makefile
fi

if test "$BITS" = "32" && test "$OSNAME" = "Darwin"; then 
    echo
    echo "#### FIXING MAKEFILE FOR 32bit DARWIN BUILD ####"
    echo

    mv $DIR/src/blt/src/Makefile $DIR/src/blt/src/Makefile.old
    sed -e 's:cc:cc -arch ppc -arch i386:g' $DIR/src/blt/src/Makefile.old >$DIR/src/blt/src/Makefile
    mv $DIR/src/blt/src/Makefile $DIR/src/blt/src/Makefile.old
    sed -e 's:gcc -arch ppc -arch i386-:gcc-:g' $DIR/src/blt/src/Makefile.old >$DIR/src/blt/src/Makefile
    mv $DIR/src/blt/src/shared/Makefile $DIR/src/blt/src/shared/Makefile.old
    sed -e 's:cc:cc -arch ppc -arch i386:g' $DIR/src/blt/src/shared/Makefile.old >$DIR/src/blt/src/shared/Makefile
    mv $DIR/src/blt/src/shared/Makefile $DIR/src/blt/src/shared/Makefile.old
    sed -e 's:gcc -arch ppc -arch i386-:gcc-:g' $DIR/src/blt/src/shared/Makefile.old >$DIR/src/blt/src/shared/Makefile
fi


if test "$OSNAME" = "Darwin"; then

    echo 
    echo "#### FIXING MAKEFILE FOR X11 ON LEOPARD ####"
    echo

    mv $DIR/src/blt/src/Makefile $DIR/src/blt/src/Makefile.old
    sed -e "s:-lX11:/usr/X11R6/lib/libX11.dylib:g" $DIR/src/blt/src/Makefile.old > $DIR/src/blt/src/Makefile
    mv $DIR/src/blt/src/shared/Makefile $DIR/src/blt/src/shared/Makefile.old
    sed -e "s:-lX11:/usr/X11R6/lib/libX11.dylib:g" $DIR/src/blt/src/shared/Makefile.old > $DIR/src/blt/src/shared/Makefile
fi

############
# Make

# Can't use MAKE_FLAGS (specifically -j# as this can cause BLT to build incorrectly.)
cd $DIR/src/blt/
$MAKE
( cd src/shared; $MAKE )
cd $DIR/src/blt/
$MAKE install
( cd src/shared; $MAKE install )

############
# Clean up includes and add necessary symbolic links

cd $DIR/lib
if test ! -L libBLT.$SH_LIB_EXT && test -e libBLT24.$SH_LIB_EXT; then
   ln -fs libBLT24.$SH_LIB_EXT libBLT.$SH_LIB_EXT
fi


    
