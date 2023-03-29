#
# Copyright © 2023 Juan Jose Luna Espinosa (https://github.com/yomboprime)
#
#    This source code is free software; you can redistribute it
#    and/or modify it in source code form under the terms of the GNU
#    General Public License as published by the Free Software
#    Foundation; either version 2 of the License, or (at your option)
#    any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
#
#
# REQUIREMENTS: libv4l2-dev libopencv-core libopencv-imgproc libopencv-highgui libopencv-codecs lopencv_imgcodecs
#

BUILD_TYPE = -g
#BUILD_TYPE = -o3

V4L2CAPTURE_SRC_FILES = ./lib/libv4l2cpp/src/logger.cpp ./lib/libv4l2cpp/src/V4l2Capture.cpp ./lib/libv4l2cpp/src/V4l2MmapDevice.cpp ./lib/libv4l2cpp/src/V4l2ReadWriteDevice.cpp ./lib/libv4l2cpp/src/V4l2Access.cpp ./lib/libv4l2cpp/src/V4l2Device.cpp ./lib/libv4l2cpp/src/V4l2Output.cpp

SRC_FILES = ${V4L2CAPTURE_SRC_FILES} ./src/main.cpp

LIB_INC_DIR = -I"/usr/include/opencv4/" -I"./lib/libv4l2cpp/inc/"
EXTRA_LIBS = -lv4l2 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs

#------

THE_PROGRAM = yombir

yombir: clean
	gcc ${BUILD_TYPE} ${LIB_INC_DIR} ${SRC_FILES} -o ${THE_PROGRAM} -lm ${EXTRA_LIBS} -lstdc++

clean:
	rm ${THE_PROGRAM} || "true"
