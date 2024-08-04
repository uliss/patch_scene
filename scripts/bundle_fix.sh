#!/bin/bash

APP="@PROJECT_BINARY_DIR@/@PROJECT_NAME@.app"
cd "@PROJECT_BINARY_DIR@"

/usr/libexec/PlistBuddy -c "Set :CFBundleName PatchScene" ${APP}/Contents/Info.plist
/usr/libexec/PlistBuddy -c "Add :NSRequiresAquaSystemAppearance bool true" ${APP}/Contents/Info.plist

/opt/local/libexec/qt6/bin/macdeployqt ${APP} -dmg -appstore-compliant -always-overwrite -codesign="-"
codesign --verify --verbose ${APP}
