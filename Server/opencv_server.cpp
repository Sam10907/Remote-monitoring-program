/*監控端*/
#include "stdafx.h"
#include "opencv_server.h"
using namespace cv;
int key = 0;
void CALLBACK f(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime) { //時間到 將key設為'q'並且離開監控畫面 
    key = 'q';
}
int _tmain(int argc, _TCHAR* argv[])
{
	//設定winsocket的前置作業 
    WORD version;
    WSADATA wsaData;
    int err;
    version = MAKEWORD(2, 2);
    if ((err = WSAStartup(version, &wsaData))) {
        printf("WSAstart error.\n");
        exit(1);
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        return 1;
    }
    else {
        int fd,newfd;
        struct sockaddr_in addr,cli;
        int cli_size = sizeof(cli);
        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("[server] socket() failed");
            exit(1);
        }

        addr.sin_family = AF_INET;
        addr.sin_port = htons(1234);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if ((bind(fd, (struct sockaddr*) & addr, sizeof(addr))) < 0) {
            perror("[server] binding faild!");
            exit(2);
        }

        listen(fd, 5);
	/*-------------------------------------*/
	/*監控視頻的變數設置*/ 
        int num_of_recv_bytes;
        cv::VideoWriter outputVideo;
        cv::Size S = cv::Size((int)640, (int)480);
        outputVideo.open("D:\\receive.avi",CV_FOURCC('M','J','P','G'), 30, S, true);
        printf("%d\n",outputVideo.isOpened()); //將監控視頻存成檔案(目前這個有問題)
        int imgSize = 480 * 640 * 3;
        cv::Mat frame = cv::Mat::zeros(480, 640, CV_8UC3);
        uchar* iptr = frame.data;
	/*-------------------------------------*/
        int cnt = 0;
        while (1) {
            std::cout << ++cnt << std::endl;
            newfd = accept(fd, (struct sockaddr*)&cli, &cli_size);
            if (newfd < 0) {
                perror("[server] accept() faild!");
                exit(3);
            }
            while (1) {
                //start transmission
                
                char command[1000] = "";
                int readBytes;
                //printf("%d\n", count);
                printf("Command>");
                fgets(command, 1000, stdin);
                int len = strlen(command);
                command[len - 1] = '\0'; //remove '\n'
                int len1 = len - 1;
                send(newfd, (char*)&len1, sizeof(int), 0); //傳送指令長度
                send(newfd, command, strlen(command), 0); //傳送指令
                if (!strcmp(command, "exit")) break; //指令exit為與客戶端斷開連線
                if (!strcmp(command,"screen")) { //螢幕截圖
					int image_row,image_col;
					recv(newfd,(char*)&image_row,sizeof(image_row),MSG_WAITALL);
					recv(newfd,(char*)&image_col,sizeof(image_col),MSG_WAITALL);
					cv::Mat frame1 = cv::Mat::zeros(image_row, image_col, CV_8UC3);
					uchar* iptr1 = frame1.data;
					int FrameSize = image_col*image_row*3;

                   int time_id = SetTimer(NULL, 0, 20000, (TIMERPROC)&f); //20秒後執行 f() 跳出迴圈
                   while (key != 'q') {
					   if (num_of_recv_bytes = recv(newfd, (char*)iptr1, FrameSize, MSG_WAITALL) == -1) { //接收視頻 
                            std::cerr << "recv failed, received bytes = " << num_of_recv_bytes << std::endl;
                        }
                        imshow("server", frame1); //顯示視頻 
                        if (waitKey(100) == 13) break; 
                    }
                    //socket -> non blocking
                    u_long iMode = 1;
                    ioctlsocket(newfd, FIONBIO,&iMode); 
                    char monitor_end[50] = "monitor end"; //告訴客戶端終止監控
                    send(newfd, monitor_end, strlen(monitor_end), 0);
                    char* bbuf = new char[image_col * image_row * 3]; //由於是突然斷開監控 所以會有殘留的視頻frame data必須將這個收起來
                    Sleep(4000);   //等待網路將資料全數送過來 時間視網路的情況調整
                    while ((num_of_recv_bytes = recv(newfd, bbuf, FrameSize, 0)) >= 0) {
                        if (num_of_recv_bytes >= image_col * image_row * 3) {
                            printf("%d\n", num_of_recv_bytes);
                            Sleep(4000);   //同上 
                        }
						else if(num_of_recv_bytes == 0) break;
                        else
                        {
                            printf("%d\n", num_of_recv_bytes);
							Sleep(4000);
                        }
                    }
                    delete bbuf;

                    destroyAllWindows(); //關掉視頻視窗
                    bool iskill = KillTimer(NULL, time_id); //將定時器殺掉
                    key = 0;
                    //socket -> blocking
                    iMode = 0;
                    ioctlsocket(newfd, FIONBIO, &iMode);
                    continue;
                }
                else if(!strcmp(command,"monitor")){ //指令monitor為監控客戶端的相機
                    int time_id = SetTimer(NULL, 0, 20000, (TIMERPROC)&f); //20秒後執行 f() 跳出迴圈
                    while (key != 'q') {
                        if (num_of_recv_bytes = recv(newfd, (char*)iptr, imgSize, MSG_WAITALL) == -1) { //接收視頻
                            std::cerr << "recv failed, received bytes = " << num_of_recv_bytes << std::endl;
                        }
                        outputVideo << frame;
                        char winname[50] = "server"; //視窗名
                        imshow(winname, frame); //顯示視頻
                        if (waitKey(100) == 13) break; //目前試過 沒有作用 所以才使用定時20秒後跳出迴圈的方法
                    }
                    //socket -> non blocking
                    u_long iMode = 1;
                    ioctlsocket(newfd, FIONBIO,&iMode);
                    
                    char monitor_end[50] = "monitor end"; //告訴客戶端終止監控
                    send(newfd, monitor_end, strlen(monitor_end), 0);
                    char* bbuf = new char[480 * 640 * 3]; //由於是突然斷開監控 所以會有殘留的視頻frame data必須將這個收起來
                    Sleep(4000);   //等待網路將資料全數送過來 時間視網路的情況調整
                    while ((num_of_recv_bytes = recv(newfd, bbuf, imgSize, 0)) >= 0) {
                        if (num_of_recv_bytes >= 480 * 640 * 3) {
                            printf("%d\n", num_of_recv_bytes);
                            Sleep(4000);   //同上 
                        }
						else if(num_of_recv_bytes == 0) break;
                        else
                        {
                            printf("%d\n", num_of_recv_bytes);
                            Sleep(4000);
                        }
                    }
                    delete bbuf;

                    outputVideo.release(); //必須先將視頻編碼處理好才能運作 (釋出視頻檔案但目前沒作用)
                    destroyAllWindows(); //關掉視頻視窗
                    bool iskill = KillTimer(NULL, time_id); //將定時器殺掉
                    key = 0;
                    //socket -> blocking
                    iMode = 0;
                    ioctlsocket(newfd, FIONBIO, &iMode);
                    continue;
                }
				else{
					//將socket設為non blocking
                    u_long iMode = 1;
                    ioctlsocket(newfd, FIONBIO, &iMode);

                    Sleep(1000);  //等待kernel的buffer將資料完整的收進來(因為是non blocking)
                    while (1) {
                        char buf[1024] = "";
                        readBytes = recv(newfd, buf, 1024, 0);
                        if (readBytes > 0) {
                            printf("%s", buf); //印出客戶端的輸出資料
                        }
                        if (readBytes < 1024) break;
                    }
                    std::cout << std::endl;
                    //將socket恢復blocking
                    iMode = 0;
                    ioctlsocket(newfd, FIONBIO, &iMode);
                    continue;
				}
				//else if(指令){ 處理檔案傳輸 }
				//else if(指令){ 處理鍵盤側錄,可以去google找範例程式碼 }
            }
        }
        WSACleanup();
    }
	return 0;
}

