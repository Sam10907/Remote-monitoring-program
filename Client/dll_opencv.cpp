// dll_opencv.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "dll_opencv.h"
extern "C" __declspec(dllexport) void video_monitor(SOCKET socket);
extern "C" __declspec(dllexport) void screen_monitor(SOCKET socket);
hwnd2Mat::hwnd2Mat(HWND hwindow, float scale)
{
    hwnd = hwindow;
    hwindowDC = GetDC(hwnd);
    hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

    RECT windowsize;    // get the height and width of the screen
    GetClientRect(hwnd, &windowsize);

    srcheight = windowsize.bottom;
    srcwidth = windowsize.right;
    height = (int)(windowsize.bottom * scale);
    width = (int)(windowsize.right * scale);

    image.create(height, width, CV_8UC4);

    // create a bitmap
    hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
    bi.biWidth = width;
    bi.biHeight = -height;  //this is the line that makes it draw upside down or not
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // use the previously created device context with the bitmap
    SelectObject(hwindowCompatibleDC, hbwindow);
    read();
};

void hwnd2Mat::read()
{
    // copy from the window device context to the bitmap device context
    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, image.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow
};

hwnd2Mat::~hwnd2Mat()
{
    // avoid memory leak
    DeleteObject(hbwindow);
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hwnd, hwindowDC);
};

void video_monitor(SOCKET socket) {
    int bbytee;
    cout << "before open the cam" << endl;

    VideoCapture cap(0); //抓取相機設備

    if (!cap.isOpened()) {
        cout << "Could not open the camera" << endl;
        system("pause");
    }

    Mat frame;
    frame = Mat::zeros(480, 640, CV_8UC3);
    int imgSize = frame.cols * frame.rows * 3;

    int cnt = 0;
    //Mat frame;
    while (1) {
        int rbytes;
        char message[50] = ""; //接收終止訊息
        cap >> frame;
        if (frame.empty()) {
            cerr << "[client] VideoCapture(0) error!" << endl;
        }

        cout << ++cnt << ":" << frame.isContinuous() << "," << frame.size() << endl;

        if ((rbytes = recv(socket, message, sizeof(message), 0)) == 0) {}
        else
        { //收到終止訊息後終止
            if (!strcmp(message, "monitor end")) {
                printf("out\n");
                break;
            }
        }

        while (1) { //由於是non blocking 所以必須等send()成功後才跳出迴圈
            bbytee = send(socket, (char*)frame.data, imgSize, 0);
            if (bbytee < 0){
				continue; // errno == EINPROGRESS
			}
            else{
				break;
			}
        }
		
        //cv::imshow("client", frame);
        if (waitKey(200) == 13) {
            break;
        }
    }
}
void screen_monitor(SOCKET socket){
	Mat ui(40, 400, CV_8UC3, Scalar(0, 130, 0));
    putText(ui, "Press Esc to stop capturing", Point(30, 30), FONT_HERSHEY_COMPLEX, 0.7,
        Scalar(0, 0, 255), 1);
    Mat bgrImg;
    hwnd2Mat capDesktop(GetDesktopWindow());
	int cnt = 0;
	int bbytee;
	int count = 0, count1 = 0;
	while (true)
    {
        capDesktop.read();
        cvtColor(capDesktop.image, bgrImg, COLOR_BGRA2BGR);
		int rbytes;
        char message[50] = ""; //接收終止訊息
		int frame_size = bgrImg.total()*bgrImg.elemSize();
		int image_row = bgrImg.rows;
		int image_col = bgrImg.cols;
		
        if (bgrImg.empty()) {
            cerr << "[client] ScreenVideo error!" << endl;
        }
        cout << ++cnt << ":" << bgrImg.isContinuous() << "," << bgrImg.size() << endl;
        if ((rbytes = recv(socket, message, sizeof(message), 0)) == 0) {}
        else
        { //收到終止訊息後終止
            if (!strcmp(message, "monitor end")) {
                printf("out\n");
                break;
            }
        }

		while(count < 1){
			bbytee = send(socket,(char*)&image_row,sizeof(image_row),0);
			if (bbytee < 0) continue;
            else{
				count++;
				cout<<"enter"<<endl;
				break;
			}
		}
		while(count1 < 1){
			bbytee = send(socket,(char*)&image_col,sizeof(image_col),0);
			if (bbytee < 0) continue;
            else{
				count1++;
				cout<<"enter1"<<endl;
				break;
			}
		}
		
        while (1) { //由於是non blocking 所以必須等send()成功後才跳出迴圈
			bbytee = send(socket, (char*)bgrImg.data, frame_size, 0);
            if (bbytee < 0) continue; //errno == EINPROGRESS
            else break;
        }
		//cout<<bbytee<<endl;
        //imshow("desktop capture", bgrImg);
        int key = waitKey(200);

        if (key == 27)
            break;
    }
}


