# Usage: From the top folder of yombir repository:
# ./utils/convert_t16_to_mp4.sh <video_number> <rotation> <resolution multiplier>
# Where:
#	<video_number> is the filename of the .t16 input file under ./captures, without extension.
#	<rotation> is:
#		a: Landscape, no rotation
#		b: Landscape, 180º rotation
#		c: Portrait, no rotation
#		d: Portrait, 180º rotation
#	<resolution multiplier> is integer >= 1, resolution is multiplied by this number.
#
# The output mp4 video will be in ./videos/<video_number>/
# Temporary PNG files under ./videos/<video_number>/ will be removed when done.

rotParam=
case "$2" in
# Landscape, no rotation
a)
	rotParam=transpose=2,transpose=2
;;
# Landscape, 180º rotation
b)
	rotParam=
;;
# Portrait, no rotation
c)
	rotParam=transpose=2
;;
# Portrait, 180º rotation
d)
	rotParam=transpose=1
;;
*)
	echo "Third parameter must be a, b, c or d."
	exit
;;
esac

resMultiplier=$3

inputPath=./captures/${1}.t16
pngsPath=./videos/${1}
outputPath=./Thermal_video_${1}.mp4

mkdir ${pngsPath}

node ./utils/convertRAWtoPNGs/convertRAWtoPNGs ${resMultiplier} ${inputPath} ${pngsPath}

cd ${pngsPath}

if [ -z "$rotParam" ]; then
	ffmpeg -framerate 24 -pattern_type glob -i './*.png' -c:v libx264 -crf 23 -profile:v baseline -level 3.0 -pix_fmt yuv420p -movflags faststart ${outputPath}
else
	ffmpeg -framerate 24 -pattern_type glob -i './*.png' -c:v libx264 -crf 23 -profile:v baseline -level 3.0 -pix_fmt yuv420p -movflags faststart -vf "${rotParam}" ${outputPath}
fi

cd ../..

rm ${pngsPath}/*.png
