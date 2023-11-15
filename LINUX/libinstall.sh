#!/bin/sh

################################################################
#
# A script to clone wdsp from github and to compile and 
# install it. This is a prerequisite for compiling pihpsdr
#
################################################################

################################################################
#
# a) determine the location of THIS script
#    (this is where the files should be located)
#    and assume this is in the pihpsdr directory
#
################################################################

SCRIPTFILE=`realpath $0`
THISDIR=`dirname $SCRIPTFILE`
TARGET=`dirname $THISDIR`

################################################################
#
# b) install lots of packages
# (many of them should already be there)
#
################################################################

# ------------------------------------
# Install standard tools and compilers
# ------------------------------------

sudo apt-get --yes install build-essential
sudo apt-get --yes install module-assistant
sudo apt-get --yes install vim
sudo apt-get --yes install make
sudo apt-get --yes install gcc
sudo apt-get --yes install g++
sudo apt-get --yes install gfortran
sudo apt-get --yes install git
sudo apt-get --yes install pkg-config
sudo apt-get --yes install cmake
sudo apt-get --yes install autoconf
sudo apt-get --yes install autopoint
sudo apt-get --yes install gettext
sudo apt-get --yes install automake
sudo apt-get --yes install libtool
sudo apt-get --yes install cppcheck
sudo apt-get --yes install dos2unix
sudo apt-get --yes install libzstd-dev

# ---------------------------------------
# Install libraries necessary for piHPSDR
# ---------------------------------------

sudo apt-get --yes install libfftw3-dev
sudo apt-get --yes install libgtk-3-dev
sudo apt-get --yes install libasound2-dev
sudo apt-get --yes install libcurl4-openssl-dev
sudo apt-get --yes install libusb-1.0-0-dev
sudo apt-get --yes install libi2c-dev
sudo apt-get --yes install libgpiod-dev
sudo apt-get --yes install libpulse-dev
sudo apt-get --yes install pulseaudio

# ----------------------------------------------
# Install standard libraries necessary for SOAPY
# ----------------------------------------------

sudo apt-get install --yes libaio-dev
sudo apt-get install --yes libavahi-client-dev
sudo apt-get install --yes libad9361-dev
sudo apt-get install --yes libiio-dev
sudo apt-get install --yes bison
sudo apt-get install --yes flex
sudo apt-get install --yes libxml2-dev
sudo apt-get install --yes librtlsdr-dev

################################################################
#
# c) download and install SoapySDR core
#
################################################################

cd $THISDIR
yes | rm -r SoapySDR
git clone https://github.com/pothosware/SoapySDR.git

cd $THISDIR/SoapySDR
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
make -j 4
sudo make install
sudo ldconfig

################################################################
#
# d) download and install libiio
#    NOTE: libiio has just changed the API and SoapyPlutoSDR
#          is not yet updated. So compile version 0.25, which
#          is the last one with the old API
#
# CURRENTLY DISABLED: apt get libiio-dev also works (give v0.23)
#
################################################################
#
#cd $THISDIR
#yes | rm -r libiio
#git clone https://github.com/analogdevicesinc/libiio.git
#git checkout v0.25
#
#cd $THISDIR
#mkdir build
#cd build
#cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
#make -j 4
#sudo make install
#sudo ldconfig

################################################################
#
# e) download and install Soapy for Adalm Pluto
#
################################################################

cd $THISDIR
yes | rm -rf SoapyPlutoSDR
git clone https://github.com/pothosware/SoapyPlutoSDR

cd $THISDIR/SoapyPlutoSDR
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
make -j 4
sudo make install
sudo ldconfig

################################################################
#
# f) download and install Soapy for RTL sticks
#
################################################################

cd $THISDIR
yes | rm -rf SoapyRTLSDR
git clone https://github.com/pothosware/SoapyRTLSDR

cd $THISDIR/SoapyRTLSDR
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
make -j 4
sudo make install
sudo ldconfig

################################################################
#
# g) create desktop icons, start scripts, etc.  for pihpsdr
#
################################################################

rm -f $HOME/Desktop/pihpsdr.desktop
rm -f $HOME/.local/share/applications/pihpsdr.desktop

cat <<EOT > $TARGET/pihpsdr.sh
cd $TARGET
$TARGET/pihpsdr >log 2>&1
EOT
chmod +x $TARGET/pihpsdr.sh

cat <<EOT > $TARGET/pihpsdr.desktop
#!/usr/bin/env xdg-open
[Desktop Entry]
Version=1.0
Type=Application
Terminal=false
Name[eb_GB]=piHPSDR
Exec=$TARGET/pihpsdr.sh
Icon=$TARGET/hpsdr_icon.png
Name=piHPSDR
EOT

cp $TARGET/pihpsdr.desktop $HOME/Desktop
mkdir -p $HOME/.local/share/applications
cp $TARGET/pihpsdr.desktop $HOME/.local/share/applications

cp $TARGET/release/pihpsdr/hpsdr.png $TARGET
cp $TARGET/release/pihpsdr/hpsdr_icon.png $TARGET

################################################################
#
# h) default GPIO lines to input + pullup
#
################################################################

if test -f "/boot/config.txt"; then
  if grep -q "gpio=4-13,16-27=ip,pu" /boot/config.txt; then
    echo "/boot/config.txt already contains gpio setup."
  else
    echo "/boot/config.txt does not contain gpio setup - adding it."
    echo "Please reboot system for this to take effect."
    cat <<EGPIO | sudo tee -a /boot/config.txt > /dev/null
[all]
# setup GPIO for pihpsdr controllers
gpio=4-13,16-27=ip,pu
EGPIO
  fi
fi
