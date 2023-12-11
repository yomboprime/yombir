

# NOTE: This project has moved to (Codeberg)[https://codeberg.org/yombo/yombir]


# Yombir

Native C++ app for the Topdon TC-001 and the Infiray P2 and similar thermal image USB cameras that use the Infiray Tiny1-B Micro LWIR Thermal Module.

Tested on:

- x86 PC
- Raspberry Pi 4B
- Pine64 Pinephone Pro

# License

Licensed under GPL2.0

# Building the app

## Linux

Requirements: In summary, OpenCV and libV4Linux.

```
libv4l2-dev libopencv-dev libopencv-core libopencv-imgproc libopencv-highgui libopencv-codecs libopencv-imgcodecs
```

To build:

```
mkdir build
cd build
cmake ..
make
```

## Other systems

TODO - Not tested.

# Configuration

Set the width and height of your screen in ```./config/screen_resolution.txt```. By default it is ```720 1440```.

The file ```./icons/yombir.desktop``` can be changed to suit your username, install location and camera device.

It launches ```./yombir.sh```, which makes use of ```gnome-session-inhibit``` to disable screen sleep during camera use.

# How to use

```./yombir [camera device path] [r[9]]```

By default the device is ```/dev/video0```

If 'r' is specified, screen is rotated 180º (only for realtime display, not on recorded files). If 'r9' is specified, a rotation of 90º CCW is done instead.

When running, the app will  display the thermal image in fullscreen. The first two seconds will display noise while it stabilizes.

Control keys:
- Press SPACEBAR to exit the app.
- Press ENTER to toggle recording of video.
- Press R to toggle the screen rotation (as in the 'r' command parameter)

When recording, a blinking red circle will appear. Recordings are saved to ```./captures``` folder with .t16 extension (from "Thermal 16 bpp")
Each file is just the captured raw frames at 16 bpp, one after the another at 24 FPS (about 2 MB/second). Resolution is 256x192.

# How to convert recordings to mp4 videos

See [./utils/README.md](./utils/README.md) on how to convert the recorded .t16 files to .mp4 videos.

# How to change the color palette

See [./gradients/README.md](./gradients/README.md).

# Known issues

I don't set any parameters via USB. Perhaps for this reason the camera sometimes readjusts the range or the optics by itself making an audible clic, and stopping transmitting for less than a second.

# Acknowledgements

People at eevblog.com forums for the reverse engineering of the camera's video format.

The libv4l2cpp library at Github (public domain)
