#! /bin/bash
#
#  Andrew Matusiewicz     andrew.matusiewicz@gmail.com      2013-09-20
#
#  Build script for Ubuntu 12.04.
#
#  Not fully tested, USE AT YOUR OWN RISK. The standard mechanism is
#  to manually do these processes, but if you find this script helpful,
#  by all means use it!
#
#  This script installs the prereqs for Qt5, VTK6, and OpenView and then
#  downloads and builds each of these three pieces of software in the current directory.
#  This takes a really long time, but the nice thing about it is that
#  once you enter your password for the first "sudo apt-get install" command, the
#  rest is completely automatic and you should be able to walk away from it and come
#  back to a clean build of OpenView and VTK.
#
#  This script is based on https://github.com/Kitware/openview/wiki/Build-instructions
#  and has been tested on Ubuntu 12.04 32 bit and 64 bit.

###### install dependencies, answering yes to all queries of "really install this?"
sudo apt-get install -y cmake cmake-curses-gui mesa-common-dev freeglut3-dev  g++ git libxcb1 libxcb1-dev libx11-xcb1 libx11-xcb-dev libxcb-keysyms1 libxcb-keysyms1-dev libxcb-image0 libxcb-image0-dev libxcb-shm0 libxcb-shm0-dev libxcb-icccm4 libxcb-icccm4-dev libxcb-sync0 libxcb-sync0-dev libxcb-xfixes0-dev libxrender-dev libxcb-shape0-dev libxcb-randr0-dev libxcb-render-util0 libxcb-render-util0-dev libxcb-glx0-dev


###### get Qt5
##build from source
if [ ! -d qt-everywhere-opensource-src-5.1.1 ] ;
then
  wget 'http://download.qt-project.org/official_releases/qt/5.1/5.1.1/single/qt-everywhere-opensource-src-5.1.1.tar.gz'
  tar xzf qt-everywhere-opensource-src-5.1.1.tar.gz
fi

Qt_dir="$PWD/Qt-5.1.1"
if [ ! -d  $Qt_dir ] ;
then
  cd qt-everywhere-opensource-src-5.1.1
  #pipe in the correct keyboard strokes to avoid having the installation stall here for user input
  printf "o\nyes\n" | ./configure -opengl desktop -prefix $Qt_dir
  make -j5
  make install
  cd ..
fi


##64 bit binary
#wget 'http://download.qt-project.org/official_releases/qt/5.1/5.1.1/qt-linux-opensource-5.1.1-x86_64-offline.run'
#chmod 700 qt-linux-opensource-5.1.1-x86_64-offline.run
#./qt-linux-opensource-5.1.1-x86_64-offline.run

###32 bit binary
#wget 'http://download.qt-project.org/official_releases/qt/5.1/5.1.1/qt-linux-opensource-5.1.1-x86-offline.run'
#chmod 700 qt-linux-opensource-5.1.1-x86-offline.run
#./qt-linux-opensource-5.1.1-x86-offline.run


###### Get and install VTK.  If you get "iostream.h not found" error, delete build directory and try again
if [ ! -d VTK-build ] ;
then
 wget 'http://www.vtk.org/files/release/6.0/vtk-6.0.0.tar.gz'
 tar xvf vtk-6.0.0.tar.gz
 mkdir VTK-build
 cd VTK-build
 cmake ../VTK6.0.0
 make -j5
 cd ..
fi

####### Now download and build openview
git clone git://github.com/Kitware/openview.git
export VTK_DIR="$PWD/VTK-build"
export Qt5Widgets_DIR="${Qt_dir}/lib/cmake/Qt5Widgets/"
export Qt5Quick_DIR="${Qt_dir}/lib/cmake/Qt5Quick/"

mkdir openview-build
cd openview-build
cmake ../openview
make -j5
cd ..

