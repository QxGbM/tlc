#! /bin/sh

TLC=../tlc
CC=gcc
CFLAGS=-m32
TESTDIR=./
TMP=tmp

if [ $# -ne 1 ]; then
    echo "One target parameter (WIN, LIN or MAC) must be specified."
    exit
fi

target=$1
if [ ! -d $target ]; then
    echo "The result directory \"$target\" does not exist."
    exit
fi

if [ ! -d $TMP ]; then
    mkdir $TMP
fi

cd $TMP
for f in ../${TESTDIR}/*.c
do
    base=`basename ${f} .c`
    log=${base}.c.log
    asm=${base}.s
    ../$TLC $f > ${log} 2>&1
    $CC $CFLAGS ${asm} -o ${base}
    diff ../${target}/$log $log > ${log}.diff 2>&1
    diff ../${target}/$asm $asm > ${asm}.diff 2>&1
    if [ -s ${log}.diff ]; then
	echo "The log of ${base}.c is something wrong."
    else
	rm -f ${log}.diff
    fi
    if [ -s ${asm}.diff ]; then
	echo "The asm-file of ${base}.c is something wrong."
    else
	rm -rf ${asm}.diff
    fi
done
