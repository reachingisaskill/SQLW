#! /bin/bash
#
# Adds lib and include directories to the CMAKE path to allow their inclusion in other builds.
# Temporary measure while I don't have an "make install" function.
#

# Get the working of the script
DIR="$( cd "$(dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
DEPLOY=deploy

# Update the local environment variables
CPATH=$CPATH:$DIR/$DEPLOY/include
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DIR/$DEPLOY/lib
LIBRARY_PATH=$LIBRARY_PATH:$DIR/$DEPLOY/lib
#PATH=$PATH:$DIR/$DEPLOY/bin

# Export to the environment
export CPATH
export LD_LIBRARY_PATH
export LIBRARY_PATH
#export PATH

