#!/bin/bash

# you might need these additonal packages:
# apt-get install  bison flex  libusb-1.0-0-dev libgudev-1.0-dev libxv-dev

# from http://lists.freedesktop.org/archives/gstreamer-openmax/2013-March/000724.html
# apt-get install build-essential autotools-dev automake autoconf libtool autopoint libxml2-dev zlib1g-dev libglib2.0-dev pkg-config bison flex python
# apt-get install libasound2-dev libgudev-1.0-dev libxt-dev libvorbis-dev libcdparanoia-dev libpango1.0-dev libtheora-dev libvisual-0.4-dev iso-codes libgtk-3-dev libraw1394-dev libiec61883-dev libavc1394-dev libv4l-dev libcairo2-dev libcaca-dev libspeex-dev libpng-dev libshout3-dev libjpeg-dev libaa1-dev libflac-dev libdv4-dev libtag1-dev libwavpack-dev libpulse-dev libsoup2.4-dev libbz2-dev libcdaudio-dev libdc1394-22-dev ladspa-sdk libass-dev libcurl4-gnutls-dev libdca-dev libdirac-dev libdvdnav-dev libexempi-dev libexif-dev libfaad-dev libgme-dev libgsm1-dev libiptcdata0-dev libkate-dev libmimic-dev libmms-dev libmodplug-dev libmpcdec-dev libofa0-dev libopus-dev librsvg2-dev librtmp-dev libschroedinger-dev libslv2-dev libsndfile1-dev libsoundtouch-dev libspandsp-dev libx11-dev libxvidcore-dev libzbar-dev libzvbi-dev liba52-0.7.4-dev libcdio-dev libdvdread-dev libmad0-dev libmp3lame-dev libmpeg2-4-dev libopencore-amrnb-dev libopencore-amrwb-dev libsidplay1-dev libtwolame-dev libx264-dev libvo-aacenc-dev
# source: http://gstreamer.freedesktop.org/wiki/HowToCompileForEmbedded
#
# to build with libav support
# apt-get install libavutil-dev
# apt-get install yasm
#
# and for our stream.sh script:
# apt-get install  inotify-tools
#
# For reference: there is also an ppa, but it currently seems to only offer gstreamer 1.0,
# which does not include uvch264src
# add-apt-repository ppa:gstreamer-developers/ppa
# apt-get update
# apt-get install gstreamer1.0*



SOURCE=$HOME/gstreamer_sources
BUILD=$HOME/gstreamer_build
GST_VERSION=1.16.0
if [ ! -d "$SOURCE" ]; then
	mkdir $SOURCE
fi
if [ ! -d "$BUILD" ]; then
	mkdir $BUILD
fi

cd $SOURCE
wget http://gstreamer.freedesktop.org/src/gstreamer/gstreamer-$GST_VERSION.tar.xz
tar -xJf gstreamer-$GST_VERSION.tar.xz
wget http://gstreamer.freedesktop.org/src/orc/orc-0.4.28.tar.xz
tar -xzf orc-0.4.28.tar.gz
wget http://gstreamer.freedesktop.org/src/gst-plugins-base/gst-plugins-base-$GST_VERSION.tar.xz
tar -xJf gst-plugins-base-$GST_VERSION.tar.xz
wget http://gstreamer.freedesktop.org/src/gst-plugins-good/gst-plugins-good-$GST_VERSION.tar.xz
tar -xJf gst-plugins-good-$GST_VERSION.tar.xz
wget http://gstreamer.freedesktop.org/src/gst-plugins-ugly/gst-plugins-ugly-$GST_VERSION.tar.xz
tar -xJf gst-plugins-ugly-$GST_VERSION.tar.xz
wget http://gstreamer.freedesktop.org/src/gst-plugins-bad/gst-plugins-bad-$GST_VERSION.tar.xz
tar -xJf gst-plugins-bad-$GST_VERSION.tar.xz
wget http://gstreamer.freedesktop.org/src/gst-libav/gst-libav-$GST_VERSION.tar.xz
tar -xJf gst-libav-$GST_VERSION.tar.xz
wget http://gstreamer.freedesktop.org/src/gst-rtsp-server/gst-rtsp-server-$GST_VERSION.tar.xz
tar -xJf gst-rtsp-server-$GST_VERSION.tar.xz

cd gstreamer-$GST_VERSION
./configure --prefix=$BUILD
make -j2
make install

cd ../orc-0.4.28
./configure --prefix=$BUILD
make -j2
make install

cd ../gst-plugins-base-$GST_VERSION
export PKG_CONFIG_PATH=$BUILD/lib/pkgconfig:$PKG_CONFIG_PATH; ./configure --prefix=$BUILD  --enable-orc --with-x
make -j2
make install

cd ../gst-plugins-good-$GST_VERSION
export PKG_CONFIG_PATH=$BUILD/lib/pkgconfig:$PKG_CONFIG_PATH; ./configure  --prefix=$BUILD  --enable-orc --with-libv4l2 --with-x
make -j2
make install

cd ../gst-plugins-ugly-$GST_VERSION
export PKG_CONFIG_PATH=$BUILD/lib/pkgconfig:$PKG_CONFIG_PATH; ./configure  --prefix=$BUILD  --enable-orc
make -j2
make install

cd ../gst-plugins-bad-$GST_VERSION
export PKG_CONFIG_PATH=$BUILD/lib/pkgconfig:$PKG_CONFIG_PATH; ./configure  --prefix=$BUILD  --enable-orc
make -j2
make install

cd ../gst-libav-$GST_VERSION
export PKG_CONFIG_PATH=$BUILD/lib/pkgconfig:$PKG_CONFIG_PATH; ./configure  --prefix=$BUILD  --enable-orc
make -j2
make install

cd ../gst-rtsp-server-$GST_VERSION
export PKG_CONFIG_PATH=$BUILD/lib/pkgconfig:$PKG_CONFIG_PATH; ./configure  --prefix=$BUILD  --enable-orc
make -j2
make install


echo
echo "GStreamer $GST_VERSION installed sucessfully!"
echo

# to completly remove gstreamer, simpely delete the two directories:
# rm -rf $SOURCE
# rm -rf $BUILD

# to use this version of gstreamer, add it to your PATH:
# echo -e "\n\n#Path for a local version of gstreamer\nPATH=\"$BUILD/bin:\$PATH\"" >> $HOME/.profile
