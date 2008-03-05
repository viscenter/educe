#!/bin/bash

cd @CMAKE_BINARY_DIR@
echo "Running make install"
make install

#PLATYPUS=/usr/local/bin/platypus
PLATYPUS=@PLATYPUS_EXECUTABLE@

#         COMMAND LINE OPTIONS
        
#         // *required*
#         -a      Application Name
#         -t      Script Type
#         -o      Output Type: None, TextWindow
        
#         // *advanced options*
        
#         -i      Icon file
#         -p      Interpreter
#         -V      Version
#         -s      Signature (4 character string)
#         -I      Identifier
#         -f  Bundled file argument

#         -A      Requires Administrator privileges
#         -S      Secure bundled script
#         -D      Is Droppable
#         -B      Runs in background
#         -R  remainRunningAfterCompletion
        
#         -X  Suffixes supported
#         -T  File Type Codes supported
        
#         -b  Verbose output
#         -v      Print version string
#         -h      Print help

APP_NAME=@SCIRUN_APP_NAME@
"$PLATYPUS" -a 'SCIRun' -t 'Shell' -o 'None' -u 'SCI Institute' -p '/bin/sh' -V @SCIRUN_VERSION_STRING@ -s '????' -I 'org.SCI.SCIRun' -i @SCIRUN_ICON_FILE@ @SCIRUN_STARTUP_SCRIPT@ $APP_NAME

echo "Created app directory"

# Now copy the right bits into the script
RESOURCES=$APP_NAME/Contents/Resources

#CMAKE_INSTALL_PREFIX=/Users/bigler/cibc/SCIRun/install/test-shared
CMAKE_INSTALL_PREFIX=@CMAKE_INSTALL_PREFIX@
echo "Copying SCIRun binaries and libraries from $CMAKE_INSTALL_PREFIX"
cp -Rp $CMAKE_INSTALL_PREFIX/* $RESOURCES

# Copy thirdparty
#SCIRUN_THIRDPARTY_DIR=/Users/bigler/cibc/SCIRun/thirdparty.bin/3.1.0/Darwin-i386/gcc-4.0.1-32bit
SCIRUN_THIRDPARTY_DIR=@SCIRUN_THIRDPARTY_DIR@
echo "Copying Thirdparty from @SCIRUN_THIRDPARTY_DIR@"

mkdir $RESOURCES/thirdparty
for T in SCIRUN_THIRDPARTY_VERSION bin include lib share; do
    cp -Rp $SCIRUN_THIRDPARTY_DIR/$T $RESOURCES/thirdparty;
done;

# Clean out some of the thirdparty cruft
find $RESOURCES/thirdparty -name "*.a" -delete



