#!/bin/bash
ROOT=$(cd "${0%/*}" && echo $PWD)

#determine platform
UNAME=`uname`
if [ "$UNAME" == "Darwin" ]; then
   JAVA_HOME=$(/usr/libexec/java_home)
   JVM_PATH=$(dirname $(find $JAVA_HOME -name "libjvm*"))
   # prepend our lib path to LD_LIBRARY_PATH
   export DYLD_LIBRARY_PATH=$JVM_PATH:$DYLD_LIBRARY_PATH
elif [ "$UNAME" == "Linux" ]; then
   JAVA_HOME=$(dirname $(dirname $(readlink -f $(which javac))))
   JVM_PATH=$(dirname $(find $JAVA_HOME -name "libjvm*"))
   # prepend our lib path to LD_LIBRARY_PATH
   export LD_LIBRARY_PATH=$JVM_PATH:$LD_LIBRARY_PATH
else
  echo Unsupported platform.
fi
ulimit -n 2048
$DEBUGER $ROOT/OCServer_test "$@"
