#!/bin/sh

echo "\$1 = "$1
#echo "\$2 = "$2
DIR=$1/Contents/Resources
VERSION=`uname -v|sed 's/\./ /g' | awk '{print $4}'`

#ls -l $1/../../shared/scirun
#/usr/X11R6/bin/xterm -e $1/../../shared/scirun

export SCIRUN_OBJDIR="${DIR}"
export SCIRUN_SRCDIR="${DIR}/src"
export SCIRUN_THIRDPARTY_DIR="${DIR}"
export SCIRUN_ITCL_WIDGETS="${SCIRUN_THIRDPARTY_DIR}/lib/iwidgets/scripts"
export TCL_LIBRARY="${SCIRUN_THIRDPARTY_DIR}/lib/tcl"

#export DYLD_LIBRARY_PATH="$SCIRUN_OBJDIR/lib:$SCIRUN_THIRDPARTY_DIR/lib"

if [ "$VERSION" != "9" ]; then
# TIGER WE NEED TO START X11
  export DISPLAY=:0
  if [ -d /Applications/Utilities/X11.app ]; then
      /usr/bin/open -a /Applications/Utilities/X11.app 
  elif [ -d /Applications/X11.app ]; then
      /usr/bin/open -a /Applications/X11.app 
  fi
fi
echo "Starting SCIRun"
/usr/X11R6/bin/xterm -geometry 60x20 -e "env;\"$SCIRUN_OBJDIR/bin/scirun\" $2 || sleep 60";
echo "Finished SCIRun"
