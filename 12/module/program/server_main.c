/*****************************************************************
�ե�����̾	: server_main.c
��ǽ		: �����С��Υᥤ��롼����
*****************************************************************/

#include<SDL2/SDL.h>
#include"server_common.h"
#include"server_func.h"

int main(int argc,char *argv[])
{
	int	num;
	int	endFlag = 1;

	/* �����������å� */
	if(argc != 2){
		fprintf(stderr,"Usage: number of clients\n");
		exit(-1);
	}
	
	num = atoi(argv[1]);

	if(num != 2)
	{
		fprintf(stderr,"This program is for 2 clients\n");
		return -1;
	}
	/* SDL�ν���� */
	if(SDL_Init(SDL_INIT_TIMER) < 0) {
		printf("failed to initialize SDL.\n");
		exit(-1);
	}

	/* ���饤����ȤȤ���³ */
	if(SetUpServer(num) == -1){
		fprintf(stderr,"Cannot setup server\n");
		exit(-1);
	}
	
	
	/* �ᥤ�󥤥٥�ȥ롼�� */
	while(endFlag){
		endFlag = SendRecvManager();
	};

	/* ��λ���� */
	Ending();

	return 0;
}

