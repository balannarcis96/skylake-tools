#! /bin/sh
# vi:ts=4:et
set -e
echo "// Copyright (C) 1996-2006 Markus F.X.J. Oberhumer"
echo "//"
echo "//   Generic Posix/Unix system"
echo "//   Generic C compiler"

test "X${top_srcdir}" = X && top_srcdir=`echo "$0" | sed 's,[^/]*$,,'`../..

test "X${AR}" = X && AR="ar"
test "X${ARFLAGS}" = X && ARFLAGS="rcs"
test "X${STRIP}" = X && STRIP=":"
test "X${CC}" = X && CC="cc"
test "X${CFLAGS+set}" = Xset || CFLAGS="-O"
# CPPFLAGS, LDFLAGS, LIBS
# LZO_EXTRA_CPPFLAGS, LZO_EXTRA_CFLAGS, LZO_EXTRA_LDFLAGS
# LZO_EXTRA_SOURCES, LZO_EXTRA_OBJECTS, LZO_EXTRA_LIBS

CFI="-I${top_srcdir}/include -I${top_srcdir} -I${top_srcdir}/src"
BNAME=lzopro
BLIB=lib${BNAME}.a

CF="$CPPFLAGS $CFI $CFLAGS"
# info: we restrict ourselves to pure ANSI C library functions for the examples
CF="-DLZO_LIBC_ISOC90=1 $CF"
# info: we do not use _any_ external functions in freestanding mode
test -z "$LZO_CFG_FREESTANDING" || CF="-DLZO_CFG_FREESTANDING=1 $CF"
LF="$LDFLAGS $LZO_EXTRA_LDFLAGS"
LL="$BLIB $LIBS $LZO_EXTRA_LIBS"

. $top_srcdir/B/generic/clean.sh

for f in $top_srcdir/src/*.c $LZO_EXTRA_SOURCES; do
    echo $CC $CF $LZO_EXTRA_CPPFLAGS $LZO_EXTRA_CFLAGS -c $f
         $CC $CF $LZO_EXTRA_CPPFLAGS $LZO_EXTRA_CFLAGS -c $f
done

for f in *.o; do
    $STRIP $f
done
echo $AR $ARFLAGS $BLIB *.o $LZO_EXTRA_OBJECTS
     $AR $ARFLAGS $BLIB *.o $LZO_EXTRA_OBJECTS


if test -n "$LZO_CFG_FREESTANDING"; then

echo "//"
echo "// Building $BLIB in freestanding mode was successful. All done."
echo "// Now try 'nm --extern-only $BLIB'"

else

for f in lzopack precomp selftest simple; do
    echo $CC $CF $LF -o $f.out $top_srcdir/examples/$f.c $LL
         $CC $CF $LF -o $f.out $top_srcdir/examples/$f.c $LL
done
echo $CC $CF $LF -o lzotest.out $top_srcdir/lzotest/lzotest.c $LL
     $CC $CF $LF -o lzotest.out $top_srcdir/lzotest/lzotest.c $LL


echo "//"
echo "// Building LZO was successful. All done."

fi
true
