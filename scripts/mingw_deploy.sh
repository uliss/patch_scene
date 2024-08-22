#!/bin/bash

BIN_DIR="@CMAKE_INSTALL_PREFIX@/@CMAKE_INSTALL_BINDIR@"
APP_EXE="${BIN_DIR}/@PROJECT_NAME@.exe"
MINGW_LDD=$(which mingw-ldd)
MINGW_BIN_DIR=$(dirname ${MINGW_LDD})
windeployqt6 "${APP_EXE}"

dll_list() {
    ${MINGW_LDD} "$1" --dll-lookup-dirs "${BIN_DIR}" ${MINGW_BIN_DIR} \
        | grep -v 'not found' \
        | cut -d'>' -f2 \
        | grep -v 'Qt6' \
        | sed '/^$/d'
}

echo "- fix main app DLL deps ..."
dll_list "${APP_EXE}" | while read x
do
    if [ "$(dirname $x)" != "${BIN_DIR}" ]; then
        cp -v "$x" "${BIN_DIR}/"
    fi
done

echo "- fix plugins DLL deps ..."
ls -d ${BIN_DIR}/*/ | while read dir
do
    echo "- processing directory $(basename $dir) ..."
    find $dir -name '*.dll' | while read dll
    do
        dll_list $dll | while read dep_dll
        do
            if [ ! -f "${BIN_DIR}/$(basename $dep_dll)" ]; then
                cp -v "$dep_dll" "${BIN_DIR}/"
            fi
        done
    done
done

echo "- creating qt.conf ..."
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
