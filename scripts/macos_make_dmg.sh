#!/bin/bash

VERSION="@PROJECT_VERSION@"
SRC_APP="@PROJECT_NAME@.app"
SRC_DMG="PatchScene.dmg"
DEST_APP="PatchScene.app"
DEST_DMG="PatchScene-v@PROJECT_VERSION@-@CMAKE_SYSTEM_PROCESSOR@.dmg"
DIST_DIR="@PROJECT_BINARY_DIR@/dist"
cd "@PROJECT_BINARY_DIR@"

echo "- cleaning dist directory ..."
rm -rf "${DIST_DIR}/"
echo "- create dist directory ..."
mkdir -p "${DIST_DIR}/${DEST_APP}"

echo "- copy app to dist directory ..."
cp -R "${SRC_APP}/" "${DIST_DIR}/${DEST_APP}"

# fix dark theme
echo "- macos dark theme fix ..."
/usr/libexec/PlistBuddy -c "Add :NSRequiresAquaSystemAppearance bool true" "${DIST_DIR}/${DEST_APP}/Contents/Info.plist"

cd "${DIST_DIR}"

# deploy and create dmg
echo "- deploy and make DMG ..."
/opt/local/libexec/qt6/bin/macdeployqt "${DEST_APP}" -dmg -appstore-compliant -always-overwrite -codesign="-"

# check app
echo "- check app ..."
codesign --verify --verbose "${DEST_APP}"

# renaming
echo "- renaming DMG ..."
mv "${SRC_DMG}" "${DEST_DMG}"
