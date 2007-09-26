#!/bin/sh
PWD=`pwd`
DSTROOT=$PWD/Pkgroot
FRAMEWORK=$PWD/../../Framework
LOADER=$PWD/../../Loaders/OSX
DISPLAYSERVER=$PWD/../../Experiments/NewDisplayServer
MODULES=$PWD/../../Modules
OVIMSPACECHEWING=$PWD/../../Modules/OVIMSpaceChewing
OVIMUIM=$PWD/../../Modules/OVIMUIM

sudo rm -rf $DSTROOT/*

cd $FRAMEWORK
xcodebuild clean
xcodebuild
xcodebuild DSTROOT=$DSTROOT install

cd $LOADER
xcodebuild clean
xcodebuild
xcodebuild DSTROOT=$DSTROOT install

cd $DISPLAYSERVER
xcodebuild clean
xcodebuild
xcodebuild DSTROOT=$DSTROOT install

cd $MODULES
make clean
make
make DSTROOT=$DSTROOT install

rm -rf $DSTROOT/Library/OpenVanilla/0.8/Modules/OVIMSpaceChewing*

cd $OVIMSPACECHEWING
make clean
make
make DSTROOT=$DSTROOT install

cd $OVIMUIM
make clean
make
make DSTROOT=$DSTROOT install