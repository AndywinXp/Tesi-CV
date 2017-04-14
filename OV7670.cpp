#include "OV7670.h"
// #pragma comment(lib, "ws2_32.lib")
// specifico per Visual Studio, ma essendo che qui usiamo MinGW qui non funzionerà mai

/* TODO (Andrea#9#04/14/17): Creare una funzione ibrida di ricezione pacchetto.
Il resto sembra funzionare bene (apertura e chiusura).
(Per ora) */


using namespace cv;

IplImage *image;
IplImage *image_luma;
IplImage *image_chroma;
IplImage *image_BGR;

IplImage *image_text;
IplImage *image_luma_text;
IplImage *image_chroma_text;
IplImage *image_BGR_text;

CvFont font;
CvScalar color;

void write_text_from_grayscale(char * msg, IplImage* image_source, IplImage* image_dest)
{
    cvCvtColor(image_source, image_dest, CV_GRAY2BGR);
	cvPutText(image_dest, msg, cvPoint(10,15), &font, color);
}

void write_text_from_color(char * msg, IplImage* image_source, IplImage* image_dest)
{
    cvCopy(image_source, image_dest, NULL);
	cvPutText(image_dest, msg, cvPoint(10,15), &font, color);
}

void init_my_opencv()
{
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1, 1, 0, 1 , 8);
	color = cvScalar(0,255,0,0);
	image = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 1);
	image_luma = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 1);
	image_chroma = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 1);
	image_BGR = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 3);

	image_text = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 3);
	image_luma_text = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 3);
	image_chroma_text = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 3);
	image_BGR_text = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 3);
}

int convert_from_yuv_to_bgr(IplImage *LUMA, IplImage *CHROMA, int color_code, IplImage *BGR)
{
   /// This function converts a LUMA + CHROMA stream encoded in YUV to a BGR image
   /// LUMA         :   8 bit LUMA input image
   /// CHROMA       :   8 bit CHROMA input image
   /// color_code   :   OpenCV color conversion code
   /// BGR          :   Three channels BGR output image

   /// Color conversion codes
   /// CV_YUV2BGR_UYNV
   /// CV_YUV2BGR_UYVY
   /// CV_YUV2BGR_Y422
   /// CV_YUV2BGR_YUNV
   /// CV_YUV2BGR_YUY2
   /// CV_YUV2BGR_YUYV
   /// CV_YUV2BGR_YVYU

   IplImage *YUV422;
   YUV422 = cvCreateImage(cvGetSize(LUMA),8,2);

   // Merge LUMA and CHROMA
   cvMerge(LUMA,CHROMA,NULL,NULL,YUV422);
   cvCvtColor(YUV422,BGR,color_code);

   cvReleaseImage(&YUV422);

   return 0;
}

MY_SOCKET init_hybrid_sock(long buffer)
{
    #ifdef _WIN32
    struct sockaddr_in clientaddr, servaddr;
	int len;

    WSADATA wsa;
	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	/* PREPARAZIONE INDIRIZZO CLIENT E SERVER ----------------------------- */
	memset((char *)&clientaddr, 0, sizeof(struct sockaddr_in));
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = INADDR_ANY;
	clientaddr.sin_port = htons(5555);

	printf("Client avviato\n");

	/* CREAZIONE SOCKET ---------------------------- */
	SOCKET sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd == INVALID_SOCKET) { perror("apertura socket"); exit(3); }

	printf("Creata la socket sd=%d\n", (int) sd);

	/* BIND SOCKET, a una porta scelta dal sistema --------------- */
	if (bind(sd, (struct sockaddr *) &clientaddr, sizeof(clientaddr)) == SOCKET_ERROR)
	{
		perror("bind socket ");
		exit(1);
	}
	printf("Client: bind socket ok, alla porta %i\n", ntohs(clientaddr.sin_port));

	if (setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (char*)&buffer, 8) == -1) {
		fprintf(stderr, "Error setting socket opts: %d\n", WSAGetLastError());
	}
    else printf("Settato buffer\n");
    #endif // _WIN32

    return (MY_SOCKET) sd;
}

void clean_socket_hybrid(MY_SOCKET sd)
{
    #ifdef _WIN32
    closesocket((SOCKET) sd); // SPECIFICO PER WINDOWS, funziona solo su socket
                     // e non su file descriptor come la close(fd) di unix
    WSACleanup();
    #endif // _WIN32
}

int start_OV7670()
{
	struct sockaddr_in clientaddr, servaddr;
	int len;
    long buffer = 65536 * 1024;


	//Initialise OpenCv
	init_my_opencv();

	//Initialise winsock
	MY_SOCKET sd = (MY_SOCKET) init_hybrid_sock(buffer);

	packet_data *tmp = (packet_data*)malloc(sizeof(packet_data));

	/*Overlay message variables*/
	char *msg;
	float p_frame_persi, byte_rate, frame_rate;
	struct timeval end, start;
	int frame_persi = 0, frame_ricevuti = 0;
	msg = (char*) malloc(sizeof(char)*100);
	gettimeofday(&start, NULL);
    /*end overlay variables*/

	BYTE *frame = (BYTE*)malloc(WIDTH*HEIGHT*sizeof(BYTE));
	BYTE *frame_luma = (BYTE*)malloc(WIDTH*HEIGHT*sizeof(BYTE));
	BYTE *frame_chroma = (BYTE*)malloc(WIDTH*HEIGHT*sizeof(BYTE));

	len = sizeof(servaddr);

	int expected_fragment = 0;
	int frame_wasted = 0;
	int last_counter = 0;

	printf("In attesa di frame...\n");
	MessageBox(NULL, "Q - Quit the application\nS - Save current frame to folder", "Command list", MB_ICONINFORMATION | MB_OK | MB_DEFBUTTON2);

	while (1)
	{
	    // anche questa da ibridizzare, mondo cane.
	    // ad esempio receive_packet_hybrid()

		if (recvfrom(sd, (char*)tmp, sizeof(packet_data), 0, (struct sockaddr *)&servaddr, &len) == SOCKET_ERROR)
		{
			perror("rcvfrom\n");
			printf("Errore nella ricezione di un pacchetto, ultimo arrivato: frame index -> %d, i -> %d\n", tmp->frame_index, tmp->fragment);
			exit(1);
		}
		//else printf("COUNT = %d\n", tmp->count);
		if (last_counter > tmp->count)
			printf("Pacchetto non ordinato %d\n", tmp->count);
		last_counter = tmp->count;

		if (tmp->fragment != expected_fragment)
		{
			if (expected_fragment != 0)
            {
                printf("Frame wasted %d\n", ++frame_wasted);
                frame_persi++;
            }

			expected_fragment = 0;
		}
		else
		{
		    memcpy((frame + (tmp->fragment*FRAME_I_SIZE)), tmp->data, FRAME_I_SIZE);
			memcpy((frame_luma + (tmp->fragment*FRAME_I_SIZE)), tmp->data_luma, FRAME_I_SIZE);
			memcpy((frame_chroma + (tmp->fragment*FRAME_I_SIZE)), tmp->data_chroma, FRAME_I_SIZE);

			if (expected_fragment != FRAME_SIZE / FRAME_I_SIZE - 1)
				expected_fragment++;
			else
			{
				expected_fragment = 0;

				/*Start create message to show over image*/
				frame_ricevuti++;
                gettimeofday(&end, NULL); // dovrebbe andare pure in unix, spero
                frame_rate = 1000000.0/(end.tv_usec - start.tv_usec);
                gettimeofday(&start, NULL);
                p_frame_persi = (100.0*frame_persi)/(frame_ricevuti);
                byte_rate = (FRAME_SIZE*frame_rate)/1024000.0;
                sprintf(msg, "| %.1f fps | Rec %d | Lost %d | Err %.2f %% | Network %.1f MB/s |", frame_rate, frame_ricevuti, frame_persi, p_frame_persi, byte_rate);
                /*End create message*/

                //Show Image NORMAL
                memcpy(image->imageData, frame, sizeof(BYTE)*WIDTH*HEIGHT);
                write_text_from_grayscale(msg, image, image_text);
				cvShowImage("OV7670", image_text);

                //Show Image LUMA
                memcpy(image_luma->imageData, frame_luma, sizeof(BYTE)*WIDTH*HEIGHT);
                write_text_from_grayscale(msg, image_luma, image_luma_text);
				cvShowImage("OV7670 Luma", image_luma_text);

				//Show Image CHROMA
				memcpy(image_chroma->imageData, frame_chroma, sizeof(BYTE)*WIDTH*HEIGHT);
				write_text_from_grayscale(msg, image_chroma, image_chroma_text);
				cvShowImage("OV7670 Chroma", image_chroma_text);

				//Show Image BGR
				convert_from_yuv_to_bgr(image_luma, image_chroma, COLOR_CODE, image_BGR);
				write_text_from_color(msg, image_BGR, image_BGR_text);
				cvShowImage("OV7670 BGR", image_BGR_text);


				//KEY PRESSED
				char key = cvWaitKey(1);

				if(key == 's')//Save Frame
                {
                    cvSaveImage("NORMAL.png", image);
                    cvSaveImage("LUMA.png", image_luma);
                    cvSaveImage("CHROMA.png", image_chroma);
                    cvSaveImage("BGR.png", image_BGR);

                    //Confirmation Dialog Box
                    MessageBox(NULL, "Images Correctly Saved!", "Success!", MB_ICONINFORMATION | MB_OK | MB_DEFBUTTON2);
                }
                else if (key == 'q')//Quit the application
                {
                    printf("Chiusura ricezione...\n");

                    //Release all memory
                    cvReleaseImage(&image);
                    cvReleaseImage(&image_text);
                    cvReleaseImage(&image_luma);
                    cvReleaseImage(&image_luma_text);
                    cvReleaseImage(&image_chroma);
                    cvReleaseImage(&image_chroma_text);
                    cvReleaseImage(&image_BGR);
                    cvReleaseImage(&image_BGR_text);

                    free(frame);
                    free(frame_chroma);
                    free(frame_luma);

                    free(tmp);
                    free(msg);

                    printf("Terminato...\n");

                    clean_socket_hybrid(sd);

                    exit(0);
                }
			}
		}
	}
}
