#!/bin/bash

#PLATYPUS=/Users/bigler/Desktop/Download/Platypus/platypus\ command\ line\ tool/build/Development/platypus
PLATYPUS=/usr/local/bin/platypus

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

PROGRAM_NAME=SCIRun.app
"$PLATYPUS" -a 'SCIRun' -t 'Shell' -o 'None' -u 'SCI Institute' -p '/bin/sh' -V '3.1.0' -s '????' -I 'org.SCI.SCIRun' -i /Users/bigler/cibc/SCIRun/src/main/SCIRun-icon.icns '/Users/bigler/cibc/SCIRun/install/startup.sh' $PROGRAM_NAME

echo "Created app directory"

# Now copy the right bits into the script
RESOURCES=$PROGRAM_NAME/Contents/Resources

CMAKE_INSTALL_PREFIX=/Users/bigler/cibc/SCIRun/install/test-shared
echo "Copying SCIRun binaries and libraries from $INSTALL_DIR"
cp -Rp $INSTALL_DIR/* $RESOURCES

# Copy thirdparty
SCIRUN_THIRDPARTY_DIR=/Users/bigler/cibc/SCIRun/thirdparty.bin/3.1.0/Darwin-i386/gcc-4.0.1-32bit
echo "Copying Thirdparty from $SCIRUN_THIRDPARTY_DIR"

mkdir $RESOURCES/thirdparty
for T in SCIRUN_THIRDPARTY_VERSION bin include lib share; do
    cp -Rp $SCIRUN_THIRDPARTY_DIR/$T $RESOURCES/thirdparty;
done;

# Clean out some of the thirdparty cruft
find $RESOURCES/thirdparty -name "*.a" -delete



