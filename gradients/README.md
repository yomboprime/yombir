
# How to generate different gradient palettes

You can generate a different thermal gradient palette by changing the values in the ```colors``` array, at the top of the ```yombir/gradients/generateGradient.js``` file:

```
const colors = [
	createRGBT( 0, 0, 0,		10 ),
	createRGBT( 0, 0, 255,		15 ),
	createRGBT( 0, 255, 255,	20 ),
	createRGBT( 0, 255, 0,		25 ),
	createRGBT( 255, 255, 0,	30 ),
	createRGBT( 255, 128, 0,	35 ),
	createRGBT( 255, 0, 0,		40 ),
	createRGBT( 255, 0, 255,	100 ),
	createRGBT( 255, 255, 255,	150 )
];
```

The first three numbers of each line are R,G and B values from 0 to 255. The fourth number is the temperature in degrees Celsius for that color.

You can remove or add more colors, just keep in mind temperatures must be in increasing order and there must be a comma ```,``` at the end of the line except in the last color.

After changing the color you must regenerate the ```gradient.bin``` file (Node.js needs to be installed to do this, https://nodejs.org), do the following from this folder:

```node generateGradient.js```

The ```gradient.bin``` file in this directory is the one used in the viewer/recorder app, and in the mp4 conversion utility in ```yombir/utils/convertRAWtoPNGs.js```.

# Format of the gradient.bin file

1 row of 65536 pixels, 3 bytes per pixel (R, G and B)
