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
	char clientButton[4][6]={"0.jpg","1.jpg","2.jpg","3.jpg"};
	char endButton[]="END.jpg";
	char allButton[]="ALL.jpg";
	char *s,title[10];

    /* �����������å� */
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
			s=clientButton[i];
		}
		image = IMG_Load(s);
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

/*****************************************************************
�ؿ�̾	: DrawRectangle
��ǽ	: �ᥤ�󥦥���ɥ��˻ͳѤ�ɽ������
����	: int		x			: �ͳѤκ���� x ��ɸ
		  int		y			: �ͳѤκ���� y ��ɸ
		  int		width		: �ͳѤβ���
		  int		height		: �ͳѤι⤵
����	: �ʤ�
*****************************************************************/
void DrawRectangle(int x,int y,int width,int height)
{
#ifndef NDEBUG
    printf("#####\n");
    printf("DrawRectangle()\n");
    printf("x=%d,y=%d,width=%d,height=%d\n",x,y,width,height);
#endif


	rectangleColor(gMainRenderer,x,y,x+width,y+height,0xff0000ff);
	SDL_RenderPresent(gMainRenderer);

}

/*****************************************************************
�ؿ�̾	: DrawCircle
��ǽ	: �ᥤ�󥦥���ɥ��˱ߤ�ɽ������
����	: int		x		: �ߤ� x ��ɸ
		  int		y		: �ߤ� y ��ɸ
		  int		r		: �ߤ�Ⱦ��
����	: �ʤ�
*****************************************************************/
void DrawCircle(int x,int y,int r)
{
#ifndef NDEBUG
	printf("#####\n");
    printf("DrawCircle()\n");
    printf("x=%d,y=%d,tyokkei=%d\n",x,y,r);
#endif

     circleColor(gMainRenderer,x,y,r,0xff0000ff);
	SDL_RenderPresent(gMainRenderer);
}

/*****************************************************************
�ؿ�̾	: DrawDiamond
��ǽ	: �ᥤ�󥦥���ɥ���ɩ����ɽ������
����	: int		x		: ����� x ��ɸ
		  int		y		: ����� y ��ɸ
		  int		height		: �⤵
����	: �ʤ�
*****************************************************************/
void DrawDiamond(int x,int y,int height)
{
	Sint16	vx[5],vy[5];
	int	i;

#ifndef NDEBUG
    printf("#####\n");
    printf("DrawDiamond()\n");
    printf("x=%d,y=%d,height=%d\n",x,y,height);
#endif

    for(i=0;i<4;i++){
        vx[i] = x + height*((1-i)%2)/2;
        vy[i] = y + height*((2-i)%2);
    }
    vx[4]=vx[0];
    vy[4]=vy[0];
	
	polygonColor(gMainRenderer, vx, vy, 5 , 0xff0000ff);
	SDL_RenderPresent(gMainRenderer);

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