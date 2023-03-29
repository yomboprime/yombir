/*
 * Copyright © 2023 Juan Jose Luna Espinosa (https://github.com/yomboprime)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */


// Includes

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <inttypes.h>

#include <logger.h>
#include <V4l2Capture.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/utils/logger.hpp>

// Types

enum PixelFormat {
	RAW,
	MJPG,
	YUYV422,
	THERMAL_GRAY16LE
};

struct VideoCaptureParams {

	VideoCaptureParams() {
		devicePath = std::string( "" );
		xResolution = 0;
		yResolution = 0;
		fps = 0.0;
		pixelFormat = PixelFormat::MJPG;
	}

	VideoCaptureParams( std::string path, PixelFormat pixelFormat, int xRes, int yRes, float fpsParam ) {
		devicePath = path;
		xResolution = xRes;
		yResolution = yRes;
		fps = fpsParam;
		this->pixelFormat = pixelFormat;
	}

	std::string devicePath;
	int xResolution;
	int yResolution;
	float fps;
	PixelFormat pixelFormat;

};


// Global variables

bool terminate = false;

VideoCaptureParams captureParams;
V4l2Capture *videoCapture;

int thermalImage16leSizeBytes;
uint16_t *captureBuffer;
uint16_t *thermalImage16le;
uint8_t *thermalImageRGB;

bool rotate180 = true;

int displayWidth;
int displayHeight;
int displaySizeBytes;
uint8_t *frameBufferRGB;

const int gradientWidth = 1 << 16;
uint8_t *gradientRGB;

cv::Mat iconPause;
cv::Mat iconRecord;

bool isRecording;
FILE *recordFile;
uint64_t numFrame;
uint64_t numFrameRecorded;


// Funtion prototypes

void signalHandler( int );
void printErrorAndExit( std::string error );

bool initVideoCapture( std::string devicePath, std::string &error );
bool captureFrame( std::string &error );
void closeVideoCapture();

bool processFrame( std::string &error );
void convertThermalGray16LEToRGB( int width, int height, const uint16_t *frameContents, uint8_t *dest, uint8_t *gradientRGB, bool rotate );

bool initGraphics( std::string &error );
void repaint();
void processKey( int key );
void finishGraphics();
void paintMatInMat( cv::Mat *sprite, int x, int y, cv::Mat *screen );

void initRecording();
void stopRecording();
void toggleRecording();
void recordFrame();


// Functions

int main( int argc, char** argv ) {

	signal( SIGTERM, signalHandler );
	signal( SIGINT, signalHandler );

	// Parse parameters
	if ( argc > 3 ) printErrorAndExit( std::string( "" ) );
	std::string devicePath = std::string( "/dev/video0" );
	if ( argc >= 2 ) devicePath = std::string( argv[ 1 ] );
	if ( argc >= 3 && argv[ 2 ][ 0 ] == 'r' && argv[ 2 ][ 1 ] == 0 )  rotate180 = false;

	std::string error;

	if ( ! initVideoCapture( devicePath, error ) ) printErrorAndExit( error );
	if ( ! initGraphics( error ) ) printErrorAndExit( error );

	const int waitMilliseconds = floor( 1000.0f / 60.0 );

	while ( ! terminate ) {

		int key = cv::waitKey( waitMilliseconds ) & 0x000000FF;
		processKey( key );

		if ( ! captureFrame( error ) ) printErrorAndExit( error );

		if ( isRecording ) recordFrame();

		if ( ! processFrame( error ) ) printErrorAndExit( error );

		repaint();

		numFrame++;

	}
	finishGraphics();
	closeVideoCapture();

	return 0;
}

void signalHandler( int ) {

	terminate = true;

}

void printErrorAndExit( std::string error ) {
	printf( "\n%s\n", error.c_str() );
	printf( "\nUsage: ./yombir [video device path] [r]\n    (Default device: /dev/video0)\n    r = rotate screen 180º (only for realtime display, not\n    on recorded files) \n\n");
	exit( -1 );
}

bool initVideoCapture( std::string devicePath, std::string &error ) {

	captureParams = VideoCaptureParams( devicePath, PixelFormat::THERMAL_GRAY16LE, 256, 384, 24 );

	const char *fcc = NULL;
	int imageYRes = captureParams.yResolution;
	switch ( captureParams.pixelFormat ) {
		case PixelFormat::MJPG:
			//fcc = "MJPG";
			error = std::string( "MJPG format is not supported." );
			return false;
		case PixelFormat::THERMAL_GRAY16LE:
			imageYRes /= 2;
			fcc = "YUYV";
			break;
		case PixelFormat::YUYV422:
		case PixelFormat::RAW:
			//fcc = "YUYV";
			error = std::string( "Pixel format is not supported." );
			return false;
		default:
			error = std::string( "Internal error: Unknown pixel format." );
			return false;
	}

	// Set v4l log silent
	initLogger( -1 );

	// Open video device
	V4L2DeviceParameters videoCaptureParams(
		captureParams.devicePath.c_str(),
		V4l2Device::fourcc( fcc ),
		captureParams.xResolution,
		captureParams.yResolution,
		captureParams.fps,
		IOTYPE_MMAP
	);

	videoCapture = V4l2Capture::create( videoCaptureParams );
	if ( ! videoCapture ) {
		error = std::string( "Could not open video capture device '" ) + captureParams.devicePath + std::string( "'." );
		return false;
	}

	captureParams.yResolution = imageYRes;

	// * 2 bytes / word
	thermalImage16leSizeBytes = captureParams.xResolution * captureParams.yResolution * 2;

	// * 2: Capture buffer is 2 images one of top of each other
	// / 2: 1 word / 2 bytes
	captureBuffer = new uint16_t[ thermalImage16leSizeBytes * 2 / 2 ];
	// / 2: 1 word / 2 bytes
	thermalImage16le = captureBuffer + thermalImage16leSizeBytes / 2;
	memset( thermalImage16le, 0, thermalImage16leSizeBytes );

	thermalImageRGB = new uint8_t[ 3 * ( thermalImage16leSizeBytes / 2 ) ];

	isRecording = false;
	recordFile = NULL;
	numFrame = 0;
	numFrameRecorded = 0;

	return true;

}

bool captureFrame( std::string &error ) {

	timeval tv;
	tv.tv_sec=1;
	tv.tv_usec=0;
	int cameraReady = videoCapture->isReadable( &tv );
	bool thereIsError = false;
	if ( cameraReady == -1 ) {
		thereIsError = true;
	}
	else if ( cameraReady == 1 ) {

		unsigned int bufferSize = videoCapture->getBufferSize();
		if ( bufferSize > thermalImage16leSizeBytes * 2 ) {
			error = "Error: video buffer too small for incoming image.";
			return false;
		}
		int readSize = videoCapture->read( (char *)captureBuffer, bufferSize );
		if ( readSize == -1 ) {
			thereIsError = true;
		}

	}

	if ( thereIsError ) {

		error = std::string( "Camera stream '" ) + captureParams.devicePath + std::string( "' ended unexpectedly." );
		return false;

	}

	return true;

}

void closeVideoCapture() {

	if ( isRecording ) stopRecording();

	if ( videoCapture ) delete videoCapture;

	delete [] captureBuffer;

}

bool processFrame( std::string &error ) {

	// Apply the color gradient
	convertThermalGray16LEToRGB( captureParams.xResolution, captureParams.yResolution, thermalImage16le, thermalImageRGB, gradientRGB, rotate180 );

	return true;

}

void convertThermalGray16LEToRGB( int width, int height, const uint16_t *frameContents, uint8_t *dest, uint8_t *gradientRGB, bool rotate ) {

	int n = width * height;

	for ( int i = 0; i < n; i ++ ) {

		int ir = rotate ? n - 1 - i : i;
		uint16_t gray = frameContents[ ir ];

		int p = ((int)gray) * 3;
		*dest ++ = gradientRGB[ p + 2 ];
		*dest ++ = gradientRGB[ p + 1 ];
		*dest ++ = gradientRGB[ p + 0 ];

	}

}

bool initGraphics( std::string &error ) {

	displayWidth = 400;
	displayHeight = 300;

	displaySizeBytes = displayWidth * displayHeight * 3;
	frameBufferRGB = new uint8_t[ displaySizeBytes ];
	memset( frameBufferRGB, 0x85, displaySizeBytes );

	gradientRGB = new uint8_t[ gradientWidth * 3 ];
	FILE *f = fopen( "./gradients/gradient.bin", "r+" );
	if ( ! f || fread( gradientRGB, gradientWidth * 3, 1, f ) != 1 ) {
		if ( f ) fclose( f );
		error = "Couldn't load './gradients/gradient.bin.'";
		return false;
	}
	fclose( f );

	cv::utils::logging::setLogLevel( cv::utils::logging::LOG_LEVEL_SILENT );

	iconPause = cv::imread( "./icons/pause.png" );
	iconRecord = cv::imread( "./icons/record.png" );

	cv::namedWindow( "yombir", cv::WINDOW_NORMAL );

	return true;

}

void finishGraphics() {

	delete [] gradientRGB;
	delete [] frameBufferRGB;

}

void repaint() {

	// Create cv::Mat from thermal image buffer
	cv::Mat thermalImageMat( captureParams.yResolution, captureParams.xResolution, CV_8UC3, thermalImageRGB );

	// Create cv:Mat from frameBuffer
	cv::Mat frameBufferMat( displayHeight, displayWidth, CV_8UC3, frameBufferRGB );

	// Paint thermal image in framebuffer
	cv::resize( thermalImageMat, frameBufferMat, frameBufferMat.size() );

	// Paint icons
	if ( isRecording ) {

		if ( int( floor( 2 * numFrame / captureParams.fps ) ) & 1 ) paintMatInMat( &iconRecord, 10, 10, &frameBufferMat );

	}
	else {

		paintMatInMat( &iconPause, 10, 10, &frameBufferMat );

	}

	// Show framebuffer window in fullscreen
	imshow( "yombir", frameBufferMat );
	cv::setWindowProperty( "yombir", cv::WND_PROP_ASPECT_RATIO, cv::WINDOW_KEEPRATIO );
	cv::setWindowProperty( "yombir", cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN );

}

void paintMatInMat( cv::Mat *sprite, int x, int y, cv::Mat *screen ) {

	sprite->copyTo( screen->rowRange( x, x + sprite->rows ).colRange( y, y + sprite->cols ) );

}

void initRecording() {

	// Read sequence number from file
	int64_t sequence = 1;
	const char *sequenceFilePath = "./captures/sequence.txt";
	FILE *seqFile = fopen( sequenceFilePath, "r+" );
	if ( seqFile ) {
		size_t n = 200;
		char *line = (char *)malloc( n );
		line[ 0 ] = 0;
		if ( getline( &line, &n, seqFile ) >= 1 ) {
			if ( sscanf( line, "%lu", &sequence ) != 1 ) sequence = 1;
		}
		free( line );
		line = NULL;
		fclose( seqFile );

		// Increment sequence number
		sequence++;

	}

	// Write sequence number to file
	seqFile = fopen( sequenceFilePath, "w+" );
	if ( seqFile ) {

		size_t n = 200;
		char *line = (char *)malloc( n );
		line[ 0 ] = 0;
		snprintf( line, n, "%lu", sequence );
		size_t l = strlen( line );
		if ( fwrite( line, l, 1, seqFile ) != 1 ) {
			printErrorAndExit( std::string( "Could not write sequence file." ) );
		}
		free( line );
		line = NULL;
		fclose( seqFile );

	}
	else printErrorAndExit( "Could not open for write sequence file." );

	// Open video file for write
	std::string recordFilePath = std::string( "./captures/" ) + std::to_string( sequence ) + std::string( ".t16" );
	recordFile = fopen( recordFilePath.c_str(), "w+" );
	if ( ! recordFile ) {
		printErrorAndExit( std::string( "Could not open record file for writing." ) );
	}

	isRecording = true;
}

void stopRecording() {

	fclose( recordFile );
	isRecording = false;

}

void toggleRecording() {

	if ( isRecording ) {
		stopRecording();
	}
	else {
		initRecording();
	}
}

void recordFrame() {

	if ( fwrite( thermalImage16le, thermalImage16leSizeBytes, 1, recordFile ) != 1 ) {
		printErrorAndExit( std::string( "Could not store thermal image in record file." ) );
	}

	numFrameRecorded ++;

}

void processKey( int key ) {

	switch ( key ) {

		case 10:
		case 13:
			toggleRecording();
			break;

		case ' ':
			terminate = true;
			break;

		case 'v':
			// TODO: Cycle gradient color?
			break;

		case 255:
			// Nothing to  do
			break;

		default:
			//println( std::string( "Unrecognized key: " ) + std::to_string( key ) );
			break;
	}

}
