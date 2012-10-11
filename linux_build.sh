#! /usr/bin/env bash
#
# build the server
SRCDIR=`pwd`
BUILDDIR=build/linux/`uname -p`
TARGET=equitwebserver
ERR_COMPILE=1
ERR_MOC=2
ERR_LINK=3

mkdir -p ${BUILDDIR}
cd ${BUILDDIR}

SOURCES=
HEADERS=`ls ${SRCDIR}/*.h`
OBJECTS=

echo "Building in ${BUILDDIR}"
echo -n "Searching for sources ... "

for CPP in `ls ${SRCDIR}/*.cpp`; do
	SOURCES="${SOURCES} ${CPP}"
done

echo "done"

echo "Running moc ... "

for MOCABLE in ${HEADERS}; do
	echo -n "   `basename ${MOCABLE}` "
	MOCABLE=`basename ${MOCABLE} | sed 's/\\.h//'`
	rm -f ${MOCABLE}.moc.cpp
	moc-qt4 ${SRCDIR}/${MOCABLE}.h >${MOCABLE}.moc.cpp 2>/dev/null

	if [ ! 0 -eq $? ]; then
		echo "[failed]"
		exit ${ERR_MOC}
	fi

	echo "[ok]"
	SOURCES="${SOURCES} ${MOCABLE}.moc.cpp"
done

echo "Building sources ..."

for SRCFILE in ${SOURCES}; do
	echo -n "   `basename ${SRCFILE}` "
	OBJFILE=`basename ${SRCFILE} | sed 's/\\.cpp/\\.o/'`
	OBJECTS="${OBJECTS} ${OBJFILE}"
	gcc -c -I /usr/include/qt4/QtNetwork/ -I /usr/include/qt4/QtCore/ -I /usr/include/qt4/QtXml/ -I /usr/include/qt4/QtGui/ -I /usr/include/qt4/Qt -I /usr/include/qt4/ -o ${OBJFILE} ${SRCFILE}

	if [ ! 0 -eq $? ]; then
		echo "[failed]"
		exit ${ERR_COMPILE}
	fi

	echo "[ok]"
done

echo -n "Linking ... "
gcc -L /usr/lib/qt4/ -lstdc++ -lQtCore -lQtNetwork -lQtGui -lQtXml -o ${TARGET} ${OBJECTS}

if [ ! 0 -eq $? ]; then
	echo "[failed]"
	exit ${ERR_LINK}
fi

echo "[ok]"
cd ${SRCDIR}
