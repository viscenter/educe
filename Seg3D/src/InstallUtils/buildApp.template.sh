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

rm -rf $APP_NAME
"$PLATYPUS" -a 'SCIRun' -t 'Shell' -o 'None' -u 'SCI Institute' -p '/bin/sh' -V @SCIRUN_VERSION_STRING@ -s '????' -D -I 'org.SCI.SCIRun' -i @SCIRUN_ICON_FILE@ @SCIRUN_STARTUP_SCRIPT@ $APP_NAME

echo "Created app directory"

# Now copy the right bits into the script
RESOURCES=$APP_NAME/Contents/Resources

#CMAKE_INSTALL_PREFIX=/Users/bigler/cibc/SCIRun/install/test-shared
CMAKE_INSTALL_PREFIX=@CMAKE_INSTALL_PREFIX@
echo "Copying SCIRun binaries and libraries from $CMAKE_INSTALL_PREFIX"
cp -Rp $CMAKE_INSTALL_PREFIX/* $RESOURCES

# Copy thirdparty
SCIRUN_THIRDPARTY_DIR=@SCIRUN_THIRDPARTY_DIR@
echo "Copying Thirdparty from @SCIRUN_THIRDPARTY_DIR@"

mkdir $RESOURCES/thirdparty
for T in lib; do
    cp -Rp $SCIRUN_THIRDPARTY_DIR/$T $RESOURCES;
done;


OCD=`pwd`
cd ${RESOURCES}/thirdparty/lib

THIRDPARTY_LIBS=`ls -1 *.dylib`
for T in ${THIRDPARTY_LIBS}; do
COMMAND=`otool -L $T | sed -n 's:@SCIRUN_THIRDPARTY_DIR@/lib/:& :p' | awk -v lib=$T '{print "install_name_tool -change "$1$2" @executable_path/../lib/"$2" " lib }'`
echo "${COMMAND}"
sh -c "${COMMAND}"
COMMAND=`otool -L $T | sed -n 's:/usr/X11/:& :p' | awk -v lib=$T '{print "install_name_tool -change "$1$2" /usr/X11R6/"$2" " lib }'`
echo "${COMMAND}"
sh -c "${COMMAND}"

echo "install_name_tool -id @executable_path/../thirdparty/lib/${T} ${T}"
install_name_tool -id "@executable_path/../thirdparty/lib/${T}" ${T}
done
cd $OCD

cd ${RESOURCES}/lib

SCIRUN_LIBS=`ls -1 *.dylib`
for T in ${SCIRUN_LIBS}; do
COMMAND=`otool -L $T | sed -n 's:@SCIRUN_DIR@/lib/:& :p' | awk -v lib=$T '{print "install_name_tool -change "$1$2" "$2" " lib }'`
echo "${COMMAND}"
sh -c "${COMMAND}"
COMMAND=`otool -L $T | sed -n 's:[^/][^/]*lib[A-z0-9_]*.dylib:&:p' | awk -v lib=$T '{print "install_name_tool -change "$1" @executable_path/../lib/"$1" " lib }'`
echo "${COMMAND}"
sh -c "${COMMAND}"
COMMAND=`otool -L $T | sed -n 's:@SCIRUN_THIRDPARTY_DIR@/lib/:& :p' | awk -v lib=$T '{print "install_name_tool -change "$1$2" @executable_path/../lib/"$2" " lib }'`
echo "${COMMAND}"
sh -c "${COMMAND}"
COMMAND=`otool -L $T | sed -n 's:/usr/X11/:& :p' | awk -v lib=$T '{print "install_name_tool -change "$1$2" /usr/X11R6/"$2" " lib }'`
echo "${COMMAND}"
sh -c "${COMMAND}"

echo "install_name_tool -id @executable_path/../lib/${T} ${T}"
install_name_tool -id "@executable_path/../lib/${T}" ${T}
done
cd $OCD

cd ${RESOURCES}/bin
for T in scirun; do
COMMAND=`otool -L $T | sed -n 's:[^/][^/]*lib[A-z0-9_]*.dylib:&:p' | awk -v lib=$T '{print "install_name_tool -change "$1" @executable_path/../lib/"$1" " lib }'`
echo "${COMMAND}"
sh -c "${COMMAND}"
COMMAND=`otool -L $T | sed -n 's:@SCIRUN_THIRDPARTY_DIR@/lib/:& :p' | awk -v lib=$T '{print "install_name_tool -change "$1$2" @executable_path/../lib/"$2" " lib }'`
echo "${COMMAND}"
sh -c "${COMMAND}"
COMMAND=`otool -L $T | sed -n 's:/usr/X11/:& :p' | awk -v lib=$T '{print "install_name_tool -change "$1$2" /usr/X11R6/"$2" " lib }'`
echo "${COMMAND}"
sh -c "${COMMAND}"

done
cd $OCD



# Clean out some of the thirdparty cruft
find $RESOURCES/lib -name "*.a" -delete
find $RESOURCES/lib -name "*.la" -delete
find $RESOURCES/lib -name "*.so" -delete
find $RESOURCES/lib -name "*.sh" -delete
find $RESOURCES/lib -name "libair.*" -delete
find $RESOURCES/lib -name "libalan.*" -delete
find $RESOURCES/lib -name "libbane.*" -delete
find $RESOURCES/lib -name "libbiff.*" -delete
find $RESOURCES/lib -name "libcoil.*" -delete
find $RESOURCES/lib -name "libdye.*" -delete
find $RESOURCES/lib -name "libecho.*" -delete
find $RESOURCES/lib -name "libell.*" -delete
find $RESOURCES/lib -name "libgage.*" -delete
find $RESOURCES/lib -name "libhest.*" -delete
find $RESOURCES/lib -name "libhoover.*" -delete
find $RESOURCES/lib -name "liblimn.*" -delete
find $RESOURCES/lib -name "libamoss.*" -delete
find $RESOURCES/lib -name "libmite.*" -delete
find $RESOURCES/lib -name "liboush.*" -delete
find $RESOURCES/lib -name "libnrrd.*" -delete
find $RESOURCES/lib -name "libten.*" -delete
find $RESOURCES/lib -name "libunrrdu.*" -delete

SCIRUNDMG=`date "+@SCIRUN_BUILD_DIST@_%Y%m%d_@SCIRUN_BUILD_TYPE@.dmg"`
SCIRUNDIR=`date "+@SCIRUN_BUILD_DIST@_%Y%m%d_@SCIRUN_BUILD_TYPE@"`

rm -rf ${SCIRUNDIR}
rm -rf *.dmg
mkdir ${SCIRUNDIR}
cp -r ${APP_NAME} ${SCIRUNDIR}/${APP_NAME}
cp ${RESOURCES}/bin/LICENSE ${SCIRUNDIR}/LICENSE.txt

mkdir ${SCIRUNDIR}/example_nets
mkdir ${SCIRUNDIR}/example_nets/SCIRun
cp -r ${RESOURCES}/src/nets/* ${SCIRUNDIR}/example_nets/SCIRun

PACKAGES=`ls -1 ${RESOURCES}/src/Packages`
for T in ${PACKAGES}; do
  mkdir ${SCIRUNDIR}/example_nets/$T
  cp -r ${RESOURCES}/src/Packages/${T}/nets/* ${SCIRUNDIR}/example_nets/$T
done

hdiutil create -srcfolder ${SCIRUNDIR} ${SCIRUNDMG}
rm -rf ${SCIRUNDIR}

