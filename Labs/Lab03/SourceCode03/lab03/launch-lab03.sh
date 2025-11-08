#!/bin/sh
bindir=$(pwd)
cd /Users/eythymisk/Desktop/VVR/VR24/lab03/lab03/
export 

if test "x$1" = "x--debugger"; then
	shift
	if test "x" = "xYES"; then
		echo "r  " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		GDB_COMMAND-NOTFOUND -batch -command=$bindir/gdbscript  /Users/eythymisk/Desktop/VVR/VR24/lab03/lab03/lab03 
	else
		"/Users/eythymisk/Desktop/VVR/VR24/lab03/lab03/lab03"  
	fi
else
	"/Users/eythymisk/Desktop/VVR/VR24/lab03/lab03/lab03"  
fi
