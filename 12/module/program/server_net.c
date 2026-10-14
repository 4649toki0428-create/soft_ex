/*****************************************************************
�ե�����̾	: server_net.c
��ǽ		: �����С��Υͥåȥ������
*****************************************************************/

#include"server_common.h"
#include"server_func.h"
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<unistd.h>

/* ���饤����Ȥ�ɽ����¤�� */
typedef struct{
	int		fd;
	char	name[MAX_NAME_SIZE];
}CLIENT;

static CLIENT	gClients[MAX_CLIENTS];	/* ���饤����� */
static int	gClientNum;					/* ���饤����ȿ� */

static fd_set	gMask;					/* select()�ѤΥޥ��� */
static int	gWidth;						/* gMask��Υ����å����٤��ӥåȿ� */

static int MultiAccept(int request_soc,int num);
static void Enter(int pos, int fd);
static void SetMask(int maxfd);
static void SendAllName(void);
static int RecvData(int pos,void *data,int dataSize);

/*****************************************************************
�ؿ�̾	: SetUpServer
��ǽ	: ���饤����ȤȤΥ��ͥ���������Ω����
		  �桼������̾������������Ԥ�
����	: int		num		  : ���饤����ȿ�
����	: ���ͥ������˼��Ԥ�����-1,����������0
*****************************************************************/
int SetUpServer(int num)
{
    struct sockaddr_in	server;
    int			request_soc;
    int                 maxfd;
    int			val = 1;
 
    /* �����������å� */
    assert(0<num && num<=MAX_CLIENTS);

    gClientNum = num;
    bzero((char*)&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    /* �����åȤ�������� */
    if((request_soc = socket(AF_INET,SOCK_STREAM,0)) < 0){
		fprintf(stderr,"Socket allocation failed\n");
		return -1;
    }
    setsockopt(request_soc,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));

    /* �����åȤ�̾����Ĥ��� */
    if(bind(request_soc,(struct sockaddr*)&server,sizeof(server))==-1){
		fprintf(stderr,"Cannot bind\n");
		close(request_soc);
		return -1;
    }
    fprintf(stderr,"Successfully bind!\n");
    
    /* ���饤����Ȥ������³�׵���Ԥ� */
    if(listen(request_soc, gClientNum) == -1){
		fprintf(stderr,"Cannot listen\n");
		close(request_soc);
		return -1;
    }
    fprintf(stderr,"Listen OK\n");

    /* ���饤����Ȥ���³���� */
    maxfd = MultiAccept(request_soc, gClientNum);
    close(request_soc);
    if(maxfd == -1)return -1;

    /* �����饤����Ȥ����桼����̾������ */
    SendAllName();

    /* select()�Τ���Υޥ����ͤ����ꤹ�� */
    SetMask(maxfd);

    return 0;
}

/*****************************************************************
�ؿ�̾	: SendRecvManager
��ǽ	: �����С����������Ƥ����ǡ������������
����	: �ʤ�
����	: �ץ�����ཪλ���ޥ�ɤ������Ƥ�����0���֤���
		  ����ʳ���1���֤�
*****************************************************************/
int SendRecvManager(void)
{
    char	command;
    fd_set	readOK;
    int		i;
    int		endFlag = 1;

    readOK = gMask;
    /* ���饤����Ȥ���ǡ������Ϥ��Ƥ��뤫Ĵ�٤� */
    if(select(gWidth,&readOK,NULL,NULL,NULL) < 0){
        /* ���顼�������ä� */
        return endFlag;
    }

    for(i=0;i<gClientNum;i++){
		if(FD_ISSET(gClients[i].fd,&readOK)){
	    	/* ���饤����Ȥ���ǡ������Ϥ��Ƥ��� */
	    	/* ���ޥ�ɤ��ɤ߹��� */
			RecvData(i,&command,sizeof(char));
	    	/* ���ޥ�ɤ��Ф��������Ԥ� */
	    	endFlag = ExecuteCommand(command,i);
	    	if(endFlag == 0)break;
		}
    }
    return endFlag;
}

/*****************************************************************
�ؿ�̾	: RecvIntData
��ǽ	: ���饤����Ȥ���int���Υǡ�����������
����	: int		pos	        : ���饤������ֹ�
		  int		*intData	: ���������ǡ���
����	: ������ä��Х��ȿ�
*****************************************************************/
int RecvIntData(int pos,int *intData)
{
    int n,tmp;
    
    /* �����������å� */
    assert(0<=pos && pos<gClientNum);
    assert(intData!=NULL);

    n = RecvData(pos,&tmp,sizeof(int));
    (*intData) = ntohl(tmp);
    
    return n;
}

/*****************************************************************
�ؿ�̾	: SendData
��ǽ	: ���饤����Ȥ˥ǡ���������
����	: int	   pos		: ���饤������ֹ�
							  ALL_CLIENTS�����ꤵ�줿���ˤ�����������
		  void	   *data	: ����ǡ���
		  int	   dataSize	: ����ǡ����Υ�����
����	: �ʤ�
*****************************************************************/
void SendData(int pos,void *data,int dataSize)
{
    int	i;
   
    /* �����������å� */
    assert(0<=pos && pos<gClientNum || pos==ALL_CLIENTS);
    assert(data!=NULL);
    assert(0<dataSize);

    if(pos == ALL_CLIENTS){
    	/* �����饤����Ȥ˥ǡ��������� */
		for(i=0;i<gClientNum;i++){
			write(gClients[i].fd,data,dataSize);
		}
    }
    else{
		write(gClients[pos].fd,data,dataSize);
    }
}

/*****************************************************************
�ؿ�̾	: Ending
��ǽ	: �����饤����ȤȤΥ��ͥ����������Ǥ���
����	: �ʤ�
����	: �ʤ�
*****************************************************************/
void Ending(void)
{
    int	i;

    printf("... Connection closed\n");
    for(i=0;i<gClientNum;i++)close(gClients[i].fd);
}

/*****
static
*****/
/*****************************************************************
�ؿ�̾	: MultiAccept
��ǽ	: ��³�׵�Τ��ä����饤����ȤȤΥ��ͥ���������Ω����
����	: int		request_soc	: �����å�
		  int		num     	: ���饤����ȿ�
����	: �����åȥǥ�������ץ�
*****************************************************************/
static int MultiAccept(int request_soc,int num)
{
    int	i,j;
    int	fd;
    
    for(i=0;i<num;i++){
		if((fd = accept(request_soc,NULL,NULL)) == -1){
			fprintf(stderr,"Accept error\n");
			for(j=i-1;j>=0;j--)close(gClients[j].fd);
			return -1;
		}
		Enter(i,fd);
    }
    return fd;
}

/*****************************************************************
�ؿ�̾	: Enter
��ǽ	: ���饤����ȤΥ桼����̾���������
����	: int		pos		: ���饤������ֹ�
		  int		fd		: �����åȥǥ�������ץ�
����	: �ʤ�
*****************************************************************/
static void Enter(int pos, int fd)
{
	/* ���饤����ȤΥ桼����̾��������� */
	read(fd,gClients[pos].name,MAX_NAME_SIZE);
#ifndef NDEBUG
	printf("%s is accepted\n",gClients[pos].name);
#endif
	gClients[pos].fd = fd;
}

/*****************************************************************
�ؿ�̾	: SetMask
��ǽ	: int		maxfd	: �����åȥǥ�������ץ��κ�����
����	: �ʤ�
����	: �ʤ�
*****************************************************************/
static void SetMask(int maxfd)
{
    int	i;

    gWidth = maxfd+1;

    FD_ZERO(&gMask);    
    for(i=0;i<gClientNum;i++)FD_SET(gClients[i].fd,&gMask);
}

/*****************************************************************
�ؿ�̾	: SendAllName
��ǽ	: �����饤����Ȥ����桼����̾������
����	: �ʤ�
����	: �ʤ�
*****************************************************************/
static void SendAllName(void)
{
  int	i,j,tmp1,tmp2;

    tmp2 = htonl(gClientNum);
    for(i=0;i<gClientNum;i++){
		tmp1 = htonl(i);
		SendData(i,&tmp1,sizeof(int));
		SendData(i,&tmp2,sizeof(int));
		for(j=0;j<gClientNum;j++){
			SendData(i,gClients[j].name,MAX_NAME_SIZE);
		}
	}
}

/*****************************************************************
�ؿ�̾	: RecvData
��ǽ	: ���饤����Ȥ���ǡ�����������
����	: int		pos	        : ���饤������ֹ�
		  void		*data		: ���������ǡ���
		  int		dataSize	: ��������ǡ����Υ�����
����	: ������ä��Х��ȿ�
*****************************************************************/
static int RecvData(int pos,void *data,int dataSize)
{
    int n;
    
    /* �����������å� */
    assert(0<=pos && pos<gClientNum);
    assert(data!=NULL);
    assert(0<dataSize);

    n = read(gClients[pos].fd,data,dataSize);
    
    return n;
}
