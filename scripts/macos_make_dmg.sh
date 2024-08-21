#!/bin/bash

VERSION="@PROJECT_VERSION@"
APP_NAME="PatchScene"
SRC_APP="@PROJECT_NAME@.app"
DEST_APP="PatchScene.app"
DEST_DMG="@DMG_FILE@"
DIST_DIR="@PROJECT_BINARY_DIR@/dist"
BLIST_BUDDY="@PLIST_BUDDY@"
FILE_ICNS_PATH="@FILE_ICNS@"
FILE_ICNS=$(basename ${FILE_ICNS_PATH})
FILE_DESC="'PatchScene project'"
FILE_ID="space.ceam.patch-scene.scheme"
DMGBUILD="@DMGBUILD@"
DMG_BACKGROUND="@PROJECT_SOURCE_DIR@/resources/mac_dmg_background.png"

cd "@PROJECT_BINARY_DIR@"

echo "- cleaning dist directory ..."
rm -rf "${DIST_DIR}/"
echo "- create dist directory ..."
mkdir -p "${DIST_DIR}/${DEST_APP}"

echo "- copy app to dist directory ..."
cp -R "${SRC_APP}/" "${DIST_DIR}/${DEST_APP}"

echo "- file icon ICNS ..."
cp "${FILE_ICNS_PATH}" "${DIST_DIR}/${DEST_APP}/Contents/Resources"

# fix dark theme
echo "- macos dark theme fix ..."
INFO_PLIST="${DIST_DIR}/${DEST_APP}/Contents/Info.plist"
${BLIST_BUDDY} -c "Add :NSRequiresAquaSystemAppearance bool true" ${INFO_PLIST}
echo "- adding psc extension ..."

CFBDT=":CFBundleDocumentTypes"
${BLIST_BUDDY} -c "Add ${CFBDT} array"                                      ${INFO_PLIST}
${BLIST_BUDDY} -c "Add ${CFBDT}:0:CFBundleTypeRole string Editor"           ${INFO_PLIST}
${BLIST_BUDDY} -c "Add ${CFBDT}:0:CFBundleTypeExtensions array"             ${INFO_PLIST}
${BLIST_BUDDY} -c "Add ${CFBDT}:0:CFBundleTypeExtensions:0 string psc"      ${INFO_PLIST}
${BLIST_BUDDY} -c "Add ${CFBDT}:0:CFBundleTypeIconFile string ${FILE_ICNS}" ${INFO_PLIST}
${BLIST_BUDDY} -c "Add ${CFBDT}:0:CFBundleTypeName string ${FILE_DESC}"     ${INFO_PLIST}

UTUTD=":UTExportedTypeDeclarations"
${BLIST_BUDDY} -c "Add ${UTUTD} array"                                      ${INFO_PLIST}
${BLIST_BUDDY} -c "Add ${UTUTD}:0:UTTypeIdentifier  string ${FILE_ID}"      ${INFO_PLIST}
#${BLIST_BUDDY} -c "Add ${UTUTD}:0:UTTypeConformsTo  string public.json"     ${INFO_PLIST}
${BLIST_BUDDY} -c "Add ${UTUTD}:0:UTTypeDescription string ${FILE_DESC}"    ${INFO_PLIST}
${BLIST_BUDDY} -c "Add ${UTUTD}:0:UTTypeIconFile    string ${FILE_ICNS}"    ${INFO_PLIST}

UTEXT="UTTypeTagSpecification:public.filename-extension"
${BLIST_BUDDY} -c "Add ${UTUTD}:0:${UTEXT}          array"                  ${INFO_PLIST}
${BLIST_BUDDY} -c "Add ${UTUTD}:0:${UTEXT}:0        string psc"             ${INFO_PLIST}

cd "${DIST_DIR}"

# deploy and create dmg
echo "- deploy and make App bundle ..."
/opt/local/libexec/qt6/bin/macdeployqt "${DEST_APP}" -appstore-compliant -always-overwrite -codesign="-"

# check app
echo "- check app ..."
codesign --verify --verbose "${DEST_APP}"


if [[ -x "${DMGBUILD}" ]]
then
    echo "- create DMG ..."
    cp "@PROJECT_BINARY_DIR@/dmg_settings.json" .
    ${DMGBUILD} --settings dmg_settings.json PatchScene ${DEST_DMG}
else
    echo "- no dmgbuild tool found found..."
fi



