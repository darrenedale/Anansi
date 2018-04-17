#! /bin/bash

MYDIR=$(dirname $(realpath "$0"))
BASEDIR=$(dirname "$MYDIR")
RESOURCEDIR="${BASEDIR}/resources"
ICONDIR="icons/mediatypes"
RESOURCEFILE="mediatypeicons.qrc"

cd "${RESOURCEDIR}"

echo "<RCC>"
echo "   <qresource prefix=\"/icons/mediatypes\">"

for ICONFILE in ${ICONDIR}/*.png; do
	echo "      <file alias=\"$(basename "${ICONFILE%\.*}")\">${ICONFILE}</file>"
done

echo "   </qresource>"
echo "</RCC>"
