/*****************************************************************
�ե�����̾	: client_win.c
��ǽ		: ���饤����ȤΥ桼�������󥿡��ե���������
*****************************************************************/

#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<SDL2/SDL2_gfxPrimitives.h>
#include"common.h"
#include"client_func.h"

static SDL_Window *gMainWindow;
static SDL_Renderer *gMainRenderer;
static SDL_Rect gButtonRect[MAX_CLIENTS+2];

static int CheckButtonNO(int x,int y,int num);
static int waitingResult = 0;

/*****************************************************************
�ؿ�̾	: InitWindows
��ǽ	: �</tool_call>󥦥ɥɽԤ
	: int	clientID		: 饤ֹ
		  int	num				: �����饤����ȿ�
����	: ���������Ǥ����Ȥ�0�����Ԥ����Ȥ�-1
*****************************************************************/
int InitWindows(int clientID,int num,char name[][MAX_NAME_SIZE])
{
	int i;
	SDL_Texture *texture;
	SDL_Surface *image;
	SDL_Rect src_rect;
	SDL_Rect dest_rect;
	const char *handImages[3]={"janken_gu.png","janken_choki.png","janken_pa.png"};
	char endButton[]="END.jpg";
	const char *s;
	char title[10];

    /*å */
    assert(num==2);
	
	/* SDL�ν���� */
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("failed to initialize SDL.\n");
		return -1;
	}
	
	/* �ᥤ��Υ�����ɥ���������� */
	if((gMainWindow = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 300, 0)) == NULL) {
		printf("failed to initialize videomode.\n");
		return -1;
	}

	gMainRenderer = SDL_CreateRenderer(gMainWindow, -1, SDL_RENDERER_SOFTWARE);//SDL_RENDERER_ACCELERATED |SDL_RENDERER_PRESENTVSYNC);//0);

	/* ������ɥ��Υ����ȥ�򥻥å� */
	sprintf(title,"%d",clientID);
	SDL_SetWindowTitle(gMainWindow, title);
	
	/* �طʤ���ˤ��� */
	SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 255);
  	SDL_RenderClear(gMainRenderer);

	/* �ܥ���κ��� */
	for(i=0;i<4;i++){
		gButtonRect[i].x = 20+80*i;
		gButtonRect[i].y=10;
		gButtonRect[i].w=70;
		gButtonRect[i].h=20;
      
		if(i==3){
			s=endButton;
		}
		
		else{
			s=handImages[i];
		}
		image = IMG_Load(s);
		if (image == NULL) {
    		fprintf(stderr, "画像を読み込めません [%s]: %s\n",
            s, IMG_GetError());
    	return -1;
		}
		texture = SDL_CreateTextureFromSurface(gMainRenderer, image);
		src_rect = (SDL_Rect){0, 0, image->w, image->h};
		SDL_RenderCopy(gMainRenderer, texture, &src_rect, (&gButtonRect[i]));
		SDL_FreeSurface(image);
	}
	SDL_RenderPresent(gMainRenderer);
	
	return 0;
}

/*****************************************************************
�ؿ�̾	: DestroyWindow
��ǽ	: SDL��λ����
����	: �ʤ�
����	: �ʤ�
********************mmand(void);
これで client_win.c のクリック処理から呼び出せます。結果待ち中の送信制限は、ボタンと送信をつないだ後に追加しましょう。

￼
￼
￼
*********************************************/
void DestroyWindow(void)
{
	SDL_Quit();
}

/*****************************************************************
�ؿ�̾	: WindowEvent
��ǽ	: �ᥤ�󥦥���ɥ����Ф��륤�٥�Ƚ�����Ԥ�
����	: int		num		: �����饤����ȿ�
����	: �ʤ�
*****************************************************************/
void WindowEvent(int num)
{
	SDL_Event event;
	SDL_MouseButtonEvent *mouse;
	int buttonNO;

    /* �����������å� */
    assert(0<num && num<=MAX_CLIENTS);

	if(SDL_PollEvent(&event)){

		switch(event.type){
			case SDL_QUIT:
				SendEndCommand();
				break;
			case SDL_MOUSEBUTTONUP:
				mouse = (SDL_MouseButtonEvent*)&event;
				if(mouse->button == SDL_BUTTON_LEFT){
					buttonNO = CheckButtonNO(mouse->x,mouse->y,num);
#ifndef NDEBUG
					printf("#####\n");
					printf("WindowEvent()\n");
					printf("Button %d is pressed\n",buttonNO);
#endif
					switch(buttonNO){
						case 0:
							if(waitingResult == 0) {
								SendRockCommand();
								waitingResult = 1;
							}
							break;
						case 1:
							if(waitingResult == 0) {
								SendScissorsCommand();
								waitingResult = 1;
							}
							break;
						case 2:
							if(waitingResult == 0) {
								SendPaperCommand();
								waitingResult = 1;
							}
							break;
						case 3:
							SendEndCommand();
							break;
					}
				}
				break;
		}
	}
}




/*****
static
*****/
/*****************************************************************
�ؿ�̾	: CheckButtonNO
��ǽ	: ����å����줿�ܥ�����ֹ���֤�
����	: int	   x		: �ޥ����β����줿 x ��ɸ
		  int	   y		: �ޥ����β����줿 y ��ɸ
		  char	   num		: �����饤����ȿ�
����	: �����줿�ܥ�����ֹ���֤�
		  �ܥ��󤬲�����Ƥ��ʤ�����-1���֤�
*****************************************************************/
static int CheckButtonNO(int x,int y,int num)
{
	int i;

 	for(i=0;i<4;i++){
		if(gButtonRect[i].x < x &&
			gButtonRect[i].y < y &&
      		gButtonRect[i].x + gButtonRect[i].w > x &&
			gButtonRect[i].y + gButtonRect[i].h > y){
			return i;
		}
	}
 	return -1;
}

void EnableHandButtons(void)
{
	waitingResult = 0;
}

void DrawResult(int result)
{
    const char *message;

    switch (result) {
        case 0:
            message = "DRAW";
            break;
        case 1:
            message = "WIN";
            break;
        case 2:
            message = "LOSE";
            break;
        default:
            return;
    }

    /* 前回の結果を白で消す。上部のボタンは残す */
    SDL_Rect area = {0, 60, 500, 240};
    SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 255);
    SDL_RenderFillRect(gMainRenderer, &area);

    /* 黒い文字で結果を描画する */
    stringRGBA(gMainRenderer, 200, 120, message, 0, 0, 0, 255);

    SDL_RenderPresent(gMainRenderer);
}