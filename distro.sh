#!/bin/bash
# RTV 5Dec2000 from BPG's original
# makes a distribution
# e.g. if player is a top level directory for some great software
# ./distro.sh player 1.1 
# creates the distribution file Player-1.1.tgz 

# first argument is the software directory
SOFTWARE=$1

#second argument is the version number for the distro
VERSION=$2

#these are combined to form the distro name
DISTRO=$SOFTWARE-$VERSION

#force the first character of the distro name to upper case
DISTRO=$(echo ${DISTRO:0:1} | tr [:lower:] [:upper:])${DISTRO:1}

echo Creating distribution $DISTRO from directory $SOFTWARE
echo Removing old version link $DISTRO
rm -f $DISTRO
echo Creating new version link $DISTRO
ln -s stage $DISTRO
echo "Making clean"
cd $DISTRO && make clean && cd ..
echo "Removing any exisiting tarball $DISTRO.tgz"
/bin/rm -f $DISTRO.tgz
echo "Creating tarball $DISTRO.tgz, excluding CVS directories"
/bin/tar hcvzf $DISTRO.tgz $DISTRO --exclude "*CVS"
echo "File $DISTRO.tgz contains $SOFTWARE v.$VERSION"
echo "Done." 