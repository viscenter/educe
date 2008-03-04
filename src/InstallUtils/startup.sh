#!/bin/sh

echo "Starting scirun from "$PWD
echo "pwd returns" `pwd`
echo "\$1 = "$1
DIR=$1/Contents/Resources
export DISPLAY=:0
#ls -l $1/../../shared/scirun
#/usr/X11R6/bin/xterm -e $1/../../shared/scirun

export SCIRUN_OBJDIR=$DIR
export SCIRUN_SRCDIR=$DIR/src
export SCIRUN_THIRDPARTY_DIR=$DIR/thirdparty
export SCIRUN_ITCL_WIDGETS=${SCIRUN_THIRDPARTY_DIR}/lib/iwidgets/scripts
export TCL_LIBRARY=${SCIRUN_THIRDPARTY_DIR}/lib/tcl

export DYLD_LIBRARY_PATH="$SCIRUN_OBJDIR/lib:$SCIRUN_THIRDPARTY_DIR/lib"

if [ -d /Applications/Utilities/X11.app ]; then
    /usr/bin/open -a /Applications/Utilities/X11.app 
elif [ -d /Applications/X11.app ]; then
    /usr/bin/open -a /Applications/X11.app 
fi

echo "Starting SCIRun"
/usr/X11R6/bin/xterm -geometry 200x50 -e "env;$SCIRUN_OBJDIR/bin/scirun || sleep 10";
echo "Finished SCIRun"
