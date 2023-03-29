
# Yombir

Native C++ app for the Topdon TC-001 and the Infiray P2 and similar thermal image cameras.

# License

Licensed under GPL2.0

# Building the app

## Linux

Requirements: libv4l2-dev libopencv-core libopencv-imgproc libopencv-highgui libopencv-codecs lopencv_imgcodecs

To build:

```
make
```

## Other systems

TODO

# How to use

```./yombir [camera device path] [r]```

By default the device is ```/dev/video0```

If r is specified, screen is rotated 180º (only for realtime display, not on recorded files)

When running, the app will  display the thermal image in fullscreen. The first two seconds will display noise while it stabilizes.

Control keys:
- Press SPACEBAR to exit the app.
- Press ENTER to toggle recording of video.

When recording, a blinking red circle will appear. Recordings are saved to ```./captures``` folder with .t16 extension (from "Thermal 16 bpp")
Each file is just the captured raw frames at 16 bpp, one after the another at 24 FPS (about 2 MB/second). Resolution is 256x192.

# How to convert recordings to mp4 videos

See [./utils/README.md] on how to convert the recorded .t16 files to .mp4 videos.

# How to change the color palette

See [./gradients/README.md].

# Known issues

I don't set any parameters via USB. Perhaps for this reason the camera sometimes readjusts the range or the optics by itself making an audible clic, and stopping transmitting for less than a second.

# Acknowledgements

People at eevblog.com forums for the reverse engineering of the camera's video format.

The libv4l2cpp library at Github (public domain)
