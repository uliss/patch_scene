#!/bin/bash

BIN_DIR="@CMAKE_INSTALL_PREFIX@/@CMAKE_INSTALL_BINDIR@"
APP_EXE="${BIN_DIR}/@PROJECT_NAME@.exe"
MINGW_LDD=$(which mingw-ldd)
MINGW_BIN_DIR=$(dirname ${MINGW_LDD})
windeployqt6 "${APP_EXE}"
${MINGW_LDD} "${APP_EXE}" --dll-lookup-dirs "${BIN_DIR}" ${MINGW_BIN_DIR} \
    | grep -v 'not found' \
    | cut -d'>' -f2 \
    | grep -v 'Qt6' \
    | sed '/^$/d' \
    | while read x
    do
        cp -v "$x" "${BIN_DIR}/"
    done

cat << 'EOF' > ${BIN_DIR}/qt.conf
[Paths]
Prefix = .
Documentation = .
Headers	= .
Libraries = .
LibraryExecutables = .
Binaries = .
Plugins	= .
QmlImports = .
ArchData = .
Data = .
Translations = .
EOF
