
#include "stdafx.h"
#include "opencv_server.h"
using namespace cv;
int key = 0;
void CALLBACK f(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime) { //�ɶ��� �Nkey�]��'q'�åB���}�ʱ��e�� 
    key = 'q';
}
int _tmain(int argc, _TCHAR* argv[])
{
	//�]�wwinsocket���e�m�@�~ 
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
	/*�ʱ����W���ܼƳ]�m*/ 
        int num_of_recv_bytes;
        cv::VideoWriter outputVideo;
        cv::Size S = cv::Size((int)640, (int)480);
        outputVideo.open("D:\\receive.avi",CV_FOURCC('M','J','P','G'), 30, S, true);
        printf("%d\n",outputVideo.isOpened()); //�N�ʱ����W�s���ɮ�(�ثe�o�Ӧ����D)
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
                send(newfd, (char*)&len1, sizeof(int), 0); //�ǰe���O����
                send(newfd, command, strlen(command), 0); //�ǰe���O
                if (!strcmp(command, "exit")) break; //���Oexit���P�Ȥ���_�}�s�u
                if (!strcmp(command,"screen")) { //�ù��I��
					int image_row,image_col;
					recv(newfd,(char*)&image_row,sizeof(image_row),MSG_WAITALL);
					recv(newfd,(char*)&image_col,sizeof(image_col),MSG_WAITALL);
					cv::Mat frame1 = cv::Mat::zeros(image_row, image_col, CV_8UC3);
					uchar* iptr1 = frame1.data;
					int FrameSize = image_col*image_row*3;

                   int time_id = SetTimer(NULL, 0, 20000, (TIMERPROC)&f); //20������ f() ���X�j��
                   while (key != 'q') {
					   if (num_of_recv_bytes = recv(newfd, (char*)iptr1, FrameSize, MSG_WAITALL) == -1) { //�������W 
                            std::cerr << "recv failed, received bytes = " << num_of_recv_bytes << std::endl;
                        }
                        imshow("server", frame1); //��ܵ��W 
                        if (waitKey(100) == 13) break; 
                    }
                    //socket -> non blocking
                    u_long iMode = 1;
                    ioctlsocket(newfd, FIONBIO,&iMode); 
                    char monitor_end[50] = "monitor end"; //�i�D�Ȥ�ݲפ�ʱ�
                    send(newfd, monitor_end, strlen(monitor_end), 0);
                    char* bbuf = new char[image_col * image_row * 3]; //�ѩ�O��M�_�}�ʱ� �ҥH�|���ݯd�����Wframe data�����N�o�Ӧ��_��
                    Sleep(4000);   //���ݺ����N��ƥ��ưe�L�� �ɶ������������p�վ�
                    while ((num_of_recv_bytes = recv(newfd, bbuf, FrameSize, 0)) >= 0) {
                        if (num_of_recv_bytes >= image_col * image_row * 3) {
                            printf("%d\n", num_of_recv_bytes);
                            Sleep(4000);   //�P�W 
                        }
						else if(num_of_recv_bytes == 0) break;
                        else
                        {
                            printf("%d\n", num_of_recv_bytes);
							Sleep(4000);
                        }
                    }
                    delete bbuf;

                    destroyAllWindows(); //�������W����
                    bool iskill = KillTimer(NULL, time_id); //�N�w�ɾ�����
                    key = 0;
                    //socket -> blocking
                    iMode = 0;
                    ioctlsocket(newfd, FIONBIO, &iMode);
                    continue;
                }
                else if(!strcmp(command,"monitor")){ //���Omonitor���ʱ��Ȥ�ݪ��۾�
                    int time_id = SetTimer(NULL, 0, 20000, (TIMERPROC)&f); //20������ f() ���X�j��
                    while (key != 'q') {
                        if (num_of_recv_bytes = recv(newfd, (char*)iptr, imgSize, MSG_WAITALL) == -1) { //�������W��
                            std::cerr << "recv failed, received bytes = " << num_of_recv_bytes << std::endl;
                        }
                        outputVideo << frame;
                        char winname[50] = "server"; //�����W
                        imshow(winname, frame); //��ܵ��W
                        if (waitKey(100) == 13) break; //�ثe�չL �S���@�� �ҥH�~�ϥΩw��20�����X�j�骺��k
                    }
                    //socket -> non blocking
                    u_long iMode = 1;
                    ioctlsocket(newfd, FIONBIO,&iMode);
                    
                    char monitor_end[50] = "monitor end"; //�i�D�Ȥ�ݲפ�ʱ�
                    send(newfd, monitor_end, strlen(monitor_end), 0);
                    char* bbuf = new char[480 * 640 * 3]; //�ѩ�O��M�_�}�ʱ� �ҥH�|���ݯd�����Wframe data�����N�o�Ӧ��_��
                    Sleep(4000);   //���ݺ����N��ƥ��ưe�L�� �ɶ������������p�վ�
                    while ((num_of_recv_bytes = recv(newfd, bbuf, imgSize, 0)) >= 0) {
                        if (num_of_recv_bytes >= 480 * 640 * 3) {
                            printf("%d\n", num_of_recv_bytes);
                            Sleep(4000);   //�P�W 
                        }
						else if(num_of_recv_bytes == 0) break;
                        else
                        {
                            printf("%d\n", num_of_recv_bytes);
                            Sleep(4000);
                        }
                    }
                    delete bbuf;

                    outputVideo.release(); //�������N���W�s�X�B�z�n�~��B�@ (���X���W�ɮצ��ثe�S�@��)
                    destroyAllWindows(); //�������W����
                    bool iskill = KillTimer(NULL, time_id); //�N�w�ɾ�����
                    key = 0;
                    //socket -> blocking
                    iMode = 0;
                    ioctlsocket(newfd, FIONBIO, &iMode);
                    continue;
                }
				else{
					//�Nsocket�]��non blocking
                    u_long iMode = 1;
                    ioctlsocket(newfd, FIONBIO, &iMode);

                    Sleep(1000);  //����kernel��buffer�N��Ƨ��㪺���i��(�]���Onon blocking)
                    while (1) {
                        char buf[1024] = "";
                        readBytes = recv(newfd, buf, 1024, 0);
                        if (readBytes > 0) {
                            printf("%s", buf); //�L�X�Ȥ�ݪ���X���
                        }
                        if (readBytes < 1024) break;
                    }
                    std::cout << std::endl;
                    //�Nsocket��_blocking
                    iMode = 0;
                    ioctlsocket(newfd, FIONBIO, &iMode);
                    continue;
				}
				//else if(���O){ �B�z�ɮ׶ǿ� }
				//else if(���O){ �B�z��L����,�i�H�hgoogle��d�ҵ{���X }
            }
        }
        WSACleanup();
    }
	return 0;
}

