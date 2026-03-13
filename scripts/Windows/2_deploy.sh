#!/bin/bash
# This is a script shell for deploying a meshlab-portable folder.
# Requires a properly built meshlab (see windows_build.sh).
#
# Without given arguments, the folder that will be deployed is meshlab/src/install.
#
# You can give as argument the path where meshlab is installed.

SCRIPTS_PATH="$(dirname "$(realpath "$0")")"
DISTRIB_PATH=$SCRIPTS_PATH/../../distrib
INSTALL_PATH=$SCRIPTS_PATH/../../src/install

#checking for parameters
for i in "$@"
do
case $i in
    -i=*|--install_path=*)
    INSTALL_PATH="${i#*=}"
    shift # past argument=value
    ;;
    *)
          # unknown option
    ;;
esac
done

windeployqt $INSTALL_PATH/meshlab.exe

# Deploy filter_sketchfab if it exists
if [ -f "$INSTALL_PATH/plugins/filter_sketchfab.dll" ]; then
    windeployqt $INSTALL_PATH/plugins/filter_sketchfab.dll --libdir $INSTALL_PATH/
fi

# Deploy LimeReport Qt dependencies if it exists
if [ -f "$INSTALL_PATH/limereport.dll" ]; then
    windeployqt $INSTALL_PATH/limereport.dll --libdir $INSTALL_PATH/
fi

# Move IFX files if they exist (from u3d library)
if ls $INSTALL_PATH/lib/meshlab/IFX* 1> /dev/null 2>&1; then
    mv $INSTALL_PATH/lib/meshlab/IFX* $INSTALL_PATH
fi

# Copy IFXCoreStatic.lib if it exists
if [ -f "$INSTALL_PATH/IFXCoreStatic.lib" ]; then
    cp $INSTALL_PATH/IFXCoreStatic.lib $INSTALL_PATH/lib/meshlab/
fi
cp $SCRIPTS_PATH/../../LICENSE.txt $INSTALL_PATH/
cp $SCRIPTS_PATH/../../docs/privacy.txt $INSTALL_PATH/

#at this point, distrib folder contains all the files necessary to execute meshlab
echo "$INSTALL_PATH is now a self contained meshlab application"
