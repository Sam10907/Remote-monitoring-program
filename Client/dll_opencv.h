#ifndef DLL_VIDEO_H
#define DLL_VIDEO_H

#include <winsock2.h>
#include <windows.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\video\video.hpp>
#include <iostream>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
using namespace cv;

class hwnd2Mat
{
public:
    hwnd2Mat(HWND hwindow, float scale = 1);
    virtual ~hwnd2Mat();
    virtual void read();
    Mat image;

private:
    HWND hwnd;
    HDC hwindowDC, hwindowCompatibleDC;
    int height, width, srcheight, srcwidth;
    HBITMAP hbwindow;
    BITMAPINFOHEADER  bi;
};
#endif
