
## convert_t16_to_mp4.sh

This script converts a raw ```.t16``` file stored in ```../captures``` to ```.mp4``` video file.

## Requirements

This ```.sh``` script uses the Node.js script in ```./convertRAWtoPNGs```, so Node.js needs to be installed (https://nodejs.org)

After that, run this command in the ```./convertRAWtoPNGs```folder  to install the package dependencies (just the PNG read/write library):
```npm install```

Also, ffmpeg needs to be installed.

## Usage

Usage: From the top folder of yombir repository, do the following command:
```./utils/convert_t16_to_mp4.sh <video_number> <rotation> <resolution multiplier>```

Where:
- ```<video_number>``` is the filename of the ```.t16``` input file under ```./captures```, without path and without extension, just the filename (a sequential number).
- <rotation> is:
	- a: Landscape, no rotation
	- b: Landscape, 180º rotation
	- c: Portrait, no rotation
	- d: Portrait, 180º rotation
- <resolution multiplier> is integer >= 1. The resolution multiplier makes the output video bigger by linearly interpolating pixel values. So for example with multiplier = 4, 256x192 becomes 1024x768.

The output mp4 video will be stored in ```./videos/<video_number>/Thermal_video_<video_number>.mp4```

Temporary PNG files under ```./videos/<video_number>/``` will be removed when done.

## Example

Convert file 1.t16, landscape, resolution x3:
```
./utils/convert_t16_to_mp4.sh 1 a 3
```
