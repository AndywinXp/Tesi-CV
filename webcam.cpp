#include "webcam.hpp"

using namespace std;
using namespace cv;

int start_webcam()
{
    bool playVideo = true;
    VideoCapture cap(0);

    if(!cap.isOpened())
    {
        printf("Unable to open video!\n");
        return 0;
    }

    Mat frame;

    MessageBox(NULL, "ESC to quit\nSPACE to Play/Pause", "Attention", MB_OK);

    while(1)
    {
        if(playVideo)
            cap >> frame;

        if(frame.empty())
        {
            printf("Empty Frame\n");
            return 0;
        }

        imshow("My Webcam", frame);

        char key = waitKey(5);

        //Se premuta barra spaziatrice
        if(key == ' ')
            playVideo = !playVideo;
        //Se premuto tasto esc
        else if(GetAsyncKeyState(VK_ESCAPE))
        {
            //Rilascio Video
            cap.release();
            //Termino Applicazione
            exit(0);
        }
    }
    return 0;
}
