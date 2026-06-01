#!/usr/bin/env bash

export APP_NAME="DigitalBloom"
export APP_VERSION=27.10
export QT_ROOT_DIR="/C/Qt/6.9.3/msvc2022_64"

echo "> $APP_NAME packager (Windows x86_64) [v$APP_VERSION]"

## CHECKS ######################################################################

if [ ${PWD##*/} != $APP_NAME ]; then
  echo "This script MUST be run from the $APP_NAME/ directory"
  exit 1
fi

## SETTINGS ####################################################################

use_contribs=false
make_install=false
create_package=true
upload_package=false



## APP INSTALL #################################################################

## APP DEPLOY ##################################################################

if [[ $create_package = true ]] ; then
  if [[ -v QT_ROOT_DIR ]]; then
    # cleanup undeployable Qt plugins (present, but missing their own dependencies)
    # only if we are on a GitHub Action server, because this remove the plugins from the Qt directory
    echo '---- Remove undeployable Qt plugins'
    #sudo rm $QT_ROOT_DIR/plugins/position/qtposition_nmea.dll
  fi
fi

#echo '---- Running windeployqt'
#windeployqt bin/ --qmldir qml/

#echo '---- Installation directory content recap (after windeployqt):'
#find bin/

#echo '---- Clean installation directory'
#rm bin/.gitkeep
#rm bin/qmltooling
#rm bin/generic

mv mybin $APP_NAME

## PACKAGE (zip) ###############################################################

if [[ $create_package = true ]] ; then
  echo '---- Compressing package'
  7z a $APP_NAME-$APP_VERSION-win64.zip $APP_NAME
fi

## PACKAGE (NSIS) ##############################################################

if [[ $create_package = true ]] ; then
  echo '---- Creating installer'
  mv $APP_NAME assets/windows/$APP_NAME
  makensis assets/windows/setup.nsi
  mv assets/windows/*.exe $APP_NAME-$APP_VERSION-win64.exe
fi

## UPLOAD ######################################################################


