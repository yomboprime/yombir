
const fs = require( 'fs' );
const pathJoin = require( 'path' ).join;
const PNG = require( 'pngjs' ).PNG;

const width = 256;
const height = 192;

if ( process.argv.length !== 5 ) {
	printUsageAndExit();
}

const resolutionMultiplier = parseInt( process.argv[ 2 ] );
console.log( "resolutionMultiplier: " + resolutionMultiplier );
if ( resolutionMultiplier < 1 ) {
	console.error( "Resolution multiplier must be an integer >= 1." );
	printUsageAndExit()
}

const outputWidth = width * resolutionMultiplier;
const outputHeight = height * resolutionMultiplier;
const frameSize = width * height * 2;
const buffer = Buffer.alloc( frameSize );

const inputPath = process.argv[ 3 ];
const outputDirPath = process.argv[ 4 ];

const inputFile = fs.openSync( inputPath );
if ( ! inputFile ) {
	console.error( "Could not open specified input file: '" + inputPath + "'." );
	printUsageAndExit();
}

const pngOut = new PNG( { colorType: 2, width: outputWidth, height: outputHeight } );
const data = pngOut.data;

const gradientPath = pathJoin( __dirname, "../../gradients/gradient.bin" );
let gradient = null;
try {
	gradient = fs.readFileSync( gradientPath );
} catch ( e ) {
	console.error( "Could not load '" + gradientPath + "'." );
	printUsageAndExit();
}

let numFrame = 1;
while ( frameSize === fs.readSync( inputFile, buffer, 0, frameSize ) ) {

	console.log( "Processing frame " + numFrame + "..." );

	for ( let j = 0; j < height; j ++ ) {
		for ( let i = 0; i < width; i ++ ) {

			for ( let rj = 0; rj < resolutionMultiplier; rj ++ ) {

				const v = rj / ( resolutionMultiplier > 1 ? resolutionMultiplier - 1 : 1 );

				for ( let ri = 0; ri < resolutionMultiplier; ri ++ ) {

					const u = ri / ( resolutionMultiplier > 1 ? resolutionMultiplier - 1 : 1 );

					const temperatureRawValue =
					(
						( 1 - u ) * ( 1 - v ) * readRAWValue( i, j ) +
						(     u ) * ( 1 - v ) * readRAWValue( i + 1, j ) +
						( 1 - u ) * (     v ) * readRAWValue( i, j + 1 ) +
						(     u ) * (     v ) * readRAWValue( i + 1, j + 1 )
					);

					const gradientP = 3 * Math.round( temperatureRawValue );
					const r = gradient[ gradientP ];
					const g = gradient[ gradientP + 1 ];
					const b = gradient[ gradientP + 2 ];

					const ii = i * resolutionMultiplier + ri;
					const jj = j * resolutionMultiplier + rj;

					const p = 4 * ( jj * outputWidth + ii );

					data[ p ] = r;
					data[ p + 1 ] = g;
					data[ p + 2 ] = b;
					data[ p + 3 ] = 255;

				}

			}

		}
	}

	let fileName = "" + numFrame;
	while ( fileName.length < 6 ) fileName = "0" + fileName;
	writePNG( pathJoin( outputDirPath, "frame_" + fileName + ".png" ), pngOut );
	numFrame ++;

}

fs.closeSync( inputFile );

console.log( "Done." );

function writePNG( path, png ) {

	var pngFileData = PNG.sync.write( png, { colorType: 6 } );
	fs.writeFileSync( path, pngFileData );

}

function readRAWValue( x, y ) {

	x = Math.max( 0, Math.min( width - 1, Math.round( x ) ) );
	y = Math.max( 0, Math.min( height - 1, Math.round( y ) ) );

	return buffer.readUInt16LE( ( y * width + x ) * 2 );

}

function printUsageAndExit() {
	console.log( "\nUsage: node convertRAWtoPNGs <resolution multiplier> <.t16 raw thermal input video file> <output directory for PNG images>" );
	process.exit( -1 );
}
