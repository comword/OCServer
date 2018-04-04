#!/bin/bash
ROOT=$(cd "${0%/*}" && echo $PWD)

#determine platform
UNAME=`uname`
if [ "$UNAME" == "Darwin" ]; then
   JAVA_HOME=$(/usr/libexec/java_home)
   # prepend our lib path to LD_LIBRARY_PATH
   export DYLD_LIBRARY_PATH=$ROOT/Core:$ROOT/External/sysroot/lib:$JAVA_HOME/jre/lib/amd64/server:$DYLD_LIBRARY_PATH
elif [ "$UNAME" == "Linux" ]; then
   JAVA_HOME=$(dirname $(dirname $(readlink -f $(which javac))))
   # prepend our lib path to LD_LIBRARY_PATH
   export LD_LIBRARY_PATH=$ROOT/Core:$ROOT/External/sysroot/lib:$JAVA_HOME/jre/lib/amd64/server:$LD_LIBRARY_PATH
fi
ulimit -n 2048
$ROOT/OCServer "$@"
