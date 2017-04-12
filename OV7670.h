#include <stdio.h>

#ifdef _WIN32
#include <WinSock2.h>
#include <direct.h> // alternativa unix unistd.h
#endif // _WIN32

#include <sys/time.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdlib.h>

#include <opencv2/core.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

//#include <opencv2/imgcodecs.hpp>
#include "opencv2/highgui.hpp"

//#define WIN32_LEAN_AND_MEAN
#define WIDTH 640
#define HEIGHT 480
#define FRAME_SIZE 307200
#define FRAME_I_SIZE 13520  //30720 //7680

#define COLOR_CODE CV_YUV2BGR_YVYU

typedef unsigned char BYTE;

typedef struct {
	int count;
	int fragment;
	int frame_index;
	BYTE data[FRAME_I_SIZE];
    BYTE data_luma[FRAME_I_SIZE];
	BYTE data_chroma[FRAME_I_SIZE];
} packet_data;

int start_OV7670();

