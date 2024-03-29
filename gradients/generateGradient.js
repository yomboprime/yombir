
const fs = require( "fs" );
const pathJoin = require( 'path' ).join;

const outFilePath = "gradient.bin";

console.log( "Writing " + outFilePath + "..." );

const width = 65536;

// Animal range:
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


// Ice range
/*
const colors = [
	createRGBT( 0, 0, 0,		- 10 ),
	createRGBT( 0, 0, 255,		- 7 ),
	createRGBT( 0, 255, 255,	- 4 ),
	createRGBT( 0, 255, 0,		- 1 ),
	createRGBT( 255, 255, 0,	- 0 ),
	createRGBT( 255, 128, 0,	10 ),
	createRGBT( 255, 0, 0,		20 ),
	createRGBT( 255, 0, 255,	50 ),
	createRGBT( 255, 255, 255,	150 )
];
*/

// These colors are more suitable to analize a FDM 3d printer or similar temperatures (not tested):
/*
const colors = [
	createRGBT( 0, 0, 0,		10 ),
	createRGBT( 0, 0, 255,		15 ),
	createRGBT( 0, 255, 255,	20 ),
	createRGBT( 0, 255, 0,		25 ),
	createRGBT( 255, 255, 0,	30 ),
	createRGBT( 255, 128, 0,	35 ),
	createRGBT( 255, 0, 0,		150 ),
	createRGBT( 255, 0, 255,	220 ),
	createRGBT( 255, 255, 255,	240 )
];
*/

const rgbBuffer = Buffer.alloc( 3 * width );

let p = 0;
for ( let i = 0; i < width; i ++ ) {

	const t = i / ( width - 1 );

	const thermal16le = ( Math.floor( t * 16384 ) & 0x0FFFC ) << 2;

	const temperatureCelsius = thermal16le / 64 - 273.15;

	let iColor = colors.length - 1;
	for ( let c = 0; c < colors.length; c ++ ) {
		if ( temperatureCelsius < colors[ c ].t ) {
			iColor = c;
			break;
		}
	}
	iColor = Math.max( 0, iColor - 1 );

	//console.log( "T: " + temperatureCelsius + ", iColor: " + iColor );

	const iColor0 = iColor;
	const iColor1 = Math.min( colors.length - 1, iColor + 1 );
	const color0 = colors[ iColor0 ];
	const color1 = colors[ iColor1 ];

	let tCol = 0;
	if ( iColor0 !== iColor1 ) tCol = ( temperatureCelsius - color0.t ) / ( color1.t - color0.t );
	tCol = Math.min( 1.0, Math.max( 0.0, tCol ) );


	rgbBuffer[ p++ ] = interpolateColor( color0, color1, tCol, "r" );
	rgbBuffer[ p++ ] = interpolateColor( color0, color1, tCol, "g" );
	rgbBuffer[ p++ ] = interpolateColor( color0, color1, tCol, "b" );

}

fs.writeFile( outFilePath, rgbBuffer, function( err ) {
	if ( err ) {
		console.log( "Error writing buffer: " + err );
		return;
	}
	console.log( "Done." );
} );

function createRGBT( r, g, b, t ) {
	return {
		r: r,
		g: g,
		b: b,
		t: t
	};
}

function interpolateColor( color0, color1, t, channel ) {

	const f = color0[ channel ] * ( 1.0 - t ) + color1[ channel ] * ( t );

	const scaled = Math.min( 255, Math.max( 0, Math.round( f ) ) );

	return scaled;

}
