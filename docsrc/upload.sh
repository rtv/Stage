#!/usr/bin/env bash

USAGE="USAGE: upload.sh <dest> <name>"

if [ $# -ne 2 ]; then
  echo $USAGE
  exit
fi

# Figure out the SF user name; define SFUSER in your startup scripts
# is this is different from your username.
if [ -n "$SFUSER" ]; then 
  _SFUSER=$SFUSER
else
  _SFUSER=$USER
fi
echo "assuming your SourceForge username is $_SFUSER..."

# Document destination and name
DEST=$1
NAME=$2

# Create a tarball with the correct name 
rm -f $DEST
ln -s $NAME $DEST
tar chzf $DEST.tgz $DEST

# Copy tarball to website
scp $DEST.tgz $SFUSER@web.sourceforge.net:/home/groups/p/pl/playerstage/htdocs/doc/

# Untar the file and re-jig the permissions
ssh $_SFUSER@web.sourceforge.net 'cd /home/groups/p/pl/playerstage/htdocs/doc/; tar xvzf '$DEST'.tgz --no-overwrite-dir; find '$DEST' -type d | xargs chmod 2775; find '$DEST' -type f | xargs chmod 664'

# clean up
rm $DEST $DEST.tgz
