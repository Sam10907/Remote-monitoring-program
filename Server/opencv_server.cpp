
#include "stdafx.h"
#include "opencv_server.h"
using namespace cv;
int key = 0;
void CALLBACK f(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime) { //®É¶¡¨ì ±Nkey³]¬°'q'¨Ã¥BÂ÷¶}ºÊ±±µe­± 
    key = 'q';
}
int _tmain(int argc, _TCHAR* argv[])
{
	//³]©wwinsocketªº«e¸m§@·~ 
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
	/*ºÊ±±µøÀWªºÅÜ¼Æ³]¸m*/ 
        int num_of_recv_bytes;
        cv::VideoWriter outputVideo;
        cv::Size S = cv::Size((int)640, (int)480);
        outputVideo.open("D:\\receive.avi",CV_FOURCC('M','J','P','G'), 30, S, true);
        printf("%d\n",outputVideo.isOpened()); //±NºÊ±±µøÀW¦s¦¨ÀÉ®×(¥Ø«e³o­Ó¦³°ÝÃD)
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
                send(newfd, (char*)&len1, sizeof(int), 0); //¶Ç°e«ü¥Oªø«×
                send(newfd, command, strlen(command), 0); //¶Ç°e«ü¥O
                if (!strcmp(command, "exit")) break; //«ü¥Oexit¬°»P«È¤áºÝÂ_¶}³s½u
                if (!strcmp(command,"screen")) { //¿Ã¹õºI¹Ï
					int image_row,image_col;
					recv(newfd,(char*)&image_row,sizeof(image_row),MSG_WAITALL);
					recv(newfd,(char*)&image_col,sizeof(image_col),MSG_WAITALL);
					cv::Mat frame1 = cv::Mat::zeros(image_row, image_col, CV_8UC3);
					uchar* iptr1 = frame1.data;
					int FrameSize = image_col*image_row*3;

                   int time_id = SetTimer(NULL, 0, 20000, (TIMERPROC)&f); //20¬í«á°õ¦æ f() ¸õ¥X°j°é
                   while (key != 'q') {
					   if (num_of_recv_bytes = recv(newfd, (char*)iptr1, FrameSize, MSG_WAITALL) == -1) { //±µ¦¬µøÀW 
                            std::cerr << "recv failed, received bytes = " << num_of_recv_bytes << std::endl;
                        }
                        imshow("server", frame1); //Åã¥ÜµøÀW 
                        if (waitKey(100) == 13) break; 
                    }
                    //socket -> non blocking
                    u_long iMode = 1;
                    ioctlsocket(newfd, FIONBIO,&iMode); 
                    char monitor_end[50] = "monitor end"; //§i¶D«È¤áºÝ²×¤îºÊ±±
                    send(newfd, monitor_end, strlen(monitor_end), 0);
                    char* bbuf = new char[image_col * image_row * 3]; //¥Ñ©ó¬O¬ðµMÂ_¶}ºÊ±± ©Ò¥H·|¦³´Ý¯dªºµøÀWframe data¥²¶·±N³o­Ó¦¬°_¨Ó
                    Sleep(4000);   //µ¥«Ýºô¸ô±N¸ê®Æ¥þ¼Æ°e¹L¨Ó ®É¶¡µøºô¸ôªº±¡ªp½Õ¾ã
                    while ((num_of_recv_bytes = recv(newfd, bbuf, FrameSize, 0)) >= 0) {
                        if (num_of_recv_bytes >= image_col * image_row * 3) {
                            printf("%d\n", num_of_recv_bytes);
                            Sleep(4000);   //¦P¤W 
                        }
						else if(num_of_recv_bytes == 0) break;
                        else
                        {
                            printf("%d\n", num_of_recv_bytes);
							Sleep(4000);
                        }
                    }
                    delete bbuf;

                    destroyAllWindows(); //Ãö±¼µøÀWµøµ¡
                    bool iskill = KillTimer(NULL, time_id); //±N©w®É¾¹±þ±¼
                    key = 0;
                    //socket -> blocking
                    iMode = 0;
                    ioctlsocket(newfd, FIONBIO, &iMode);
                    continue;
                }
                else if(!strcmp(command,"monitor")){ //«ü¥Omonitor¬°ºÊ±±«È¤áºÝªº¬Û¾÷
                    int time_id = SetTimer(NULL, 0, 20000, (TIMERPROC)&f); //20¬í«á°õ¦æ f() ¸õ¥X°j°é
                    while (key != 'q') {
                        if (num_of_recv_bytes = recv(newfd, (char*)iptr, imgSize, MSG_WAITALL) == -1) { //±µ¦¬µøÀW »
                            std::cerr << "recv failed, received bytes = " << num_of_recv_bytes << std::endl;
                        }
                        outputVideo << frame;
                        char winname[50] = "server"; //µøµ¡¦W
                        imshow(winname, frame); //Åã¥ÜµøÀW
                        if (waitKey(100) == 13) break; //¥Ø«e¸Õ¹L ¨S¦³§@¥Î ©Ò¥H¤~¨Ï¥Î©w®É20¬í«á¸õ¥X°j°éªº¤èªk
                    }
                    //socket -> non blocking
                    u_long iMode = 1;
                    ioctlsocket(newfd, FIONBIO,&iMode);
                    
                    char monitor_end[50] = "monitor end"; //§i¶D«È¤áºÝ²×¤îºÊ±±
                    send(newfd, monitor_end, strlen(monitor_end), 0);
                    char* bbuf = new char[480 * 640 * 3]; //¥Ñ©ó¬O¬ðµMÂ_¶}ºÊ±± ©Ò¥H·|¦³´Ý¯dªºµøÀWframe data¥²¶·±N³o­Ó¦¬°_¨Ó
                    Sleep(4000);   //µ¥«Ýºô¸ô±N¸ê®Æ¥þ¼Æ°e¹L¨Ó ®É¶¡µøºô¸ôªº±¡ªp½Õ¾ã
                    while ((num_of_recv_bytes = recv(newfd, bbuf, imgSize, 0)) >= 0) {
                        if (num_of_recv_bytes >= 480 * 640 * 3) {
                            printf("%d\n", num_of_recv_bytes);
                            Sleep(4000);   //¦P¤W 
                        }
						else if(num_of_recv_bytes == 0) break;
                        else
                        {
                            printf("%d\n", num_of_recv_bytes);
                            Sleep(4000);
                        }
                    }
                    delete bbuf;

                    outputVideo.release(); //¥²¶·¥ý±NµøÀW½s½X³B²z¦n¤~¯à¹B§@ (ÄÀ¥XµøÀWÀÉ®×¦ý¥Ø«e¨S§@¥Î)
                    destroyAllWindows(); //Ãö±¼µøÀWµøµ¡
                    bool iskill = KillTimer(NULL, time_id); //±N©w®É¾¹±þ±¼
                    key = 0;
                    //socket -> blocking
                    iMode = 0;
                    ioctlsocket(newfd, FIONBIO, &iMode);
                    continue;
                }
				else{
					//±Nsocket³]¬°non blocking
                    u_long iMode = 1;
                    ioctlsocket(newfd, FIONBIO, &iMode);

                    Sleep(1000);  //µ¥«Ýkernelªºbuffer±N¸ê®Æ§¹¾ãªº¦¬¶i¨Ó(¦]¬°¬Onon blocking)
                    while (1) {
                        char buf[1024] = "";
                        readBytes = recv(newfd, buf, 1024, 0);
                        if (readBytes > 0) {
                            printf("%s", buf); //¦L¥X«È¤áºÝªº¿é¥X¸ê®Æ
                        }
                        if (readBytes < 1024) break;
                    }
                    std::cout << std::endl;
                    //±Nsocket«ì´_blocking
                    iMode = 0;
                    ioctlsocket(newfd, FIONBIO, &iMode);
                    continue;
				}
				//else if(«ü¥O){ ³B²zÀÉ®×¶Ç¿é }
				//else if(«ü¥O){ ³B²zÁä½L°¼¿ý,¥i¥H¥hgoogle§ä½d¨Òµ{¦¡½X }
            }
        }
        WSACleanup();
    }
	return 0;
}

