/*****************************************************************
�ե�����̾	: server_command.c
��ǽ		: �����С��Υ��ޥ�ɽ���
*****************************************************************/

#include"server_common.h"
#include"server_func.h"
#include<arpa/inet.h>

static void SetIntData2DataBlock(void *data,int intData,int *dataSize);
static void SetCharData2DataBlock(void *data,char charData,int *dataSize);
static int hands[2] = {-1, -1};

/*****************************************************************
�ؿ�̾	: ExecuteCommand
��ǽ	: ���饤����Ȥ��������Ƥ������ޥ�ɤ򸵤ˡ�
		  ����������������¹Ԥ���
����	: char	command		: ���ޥ��
		  int	pos			: ���ޥ�ɤ����ä����饤������ֹ�
����	: �ץ�����ཪλ���ޥ�ɤ������Ƥ������ˤ�0���֤���
		  ����ʳ���1���֤�
*****************************************************************/
int ExecuteCommand(char command,int pos)
{
    unsigned char	data[MAX_DATA];
    int			dataSize,intData;
    int			endFlag = 1;

    /* �����������å� */
    assert(0<=pos && pos<MAX_CLIENTS);

#ifndef NDEBUG
    printf("#####\n");
    printf("ExecuteCommand()\n");
    printf("Get command %c\n",command);
#endif
    switch(command){
	    case END_COMMAND:
			dataSize = 0;
			/* ���ޥ�ɤΥ��å� */
			SetCharData2DataBlock(data,command,&dataSize);

			/* ���桼���������� */
			SendData(ALL_CLIENTS,data,dataSize);

			endFlag = 0;
			break;
	    case ROCK_COMMAND:
		case SCISSORS_COMMAND:
		case PAPER_COMMAND:
			if(command == ROCK_COMMAND) {
				hands[pos] = 0;
			} else if(command == SCISSORS_COMMAND) {
				hands[pos] = 1;
			} else if(command == PAPER_COMMAND) {
				hands[pos] = 2;
			}

			if(hands[0] != -1 && hands[1] != -1) {
				int result0, result1;

				if(hands[0] == hands[1]) {
					result0 = 0;
					result1 = 0;
				} else if((hands[0] == 0 && hands[1] == 1) || (hands[0] == 1 && hands[1] == 2) || (hands[0] == 2 && hands[1] == 0)) {
					result0 = 1;
					result1 = 2;
				} else {
					result0 = 2;
					result1 = 1;
				}
				dataSize = 0;
				SetCharData2DataBlock(data,RESULT_COMMAND,&dataSize);
				SetIntData2DataBlock(data,result0,&dataSize);
				SendData(0,data,dataSize);

				dataSize = 0;
				SetCharData2DataBlock(data,RESULT_COMMAND,&dataSize);
				SetIntData2DataBlock(data,result1,&dataSize);
				SendData(1,data,dataSize);

				hands[0] = -1;
				hands[1] = -1;
			}
			break;
	    default:
			/* ̤�ΤΥ��ޥ�ɤ������Ƥ��� */
			fprintf(stderr,"0x%02x is not command!\n",command);
    }

    return endFlag;
}


/*****
static
*****/
/*****************************************************************
�ؿ�̾	: SetIntData2DataBlock
��ǽ	: int ���Υǡ����������ѥǡ����κǸ�˥��åȤ���
����	: void		*data		: �����ѥǡ���
		  int		intData		: ���åȤ���ǡ���
		  int		*dataSize	: �����ѥǡ����θ��ߤΥ�����
����	: �ʤ�
*****************************************************************/
static void SetIntData2DataBlock(void *data,int intData,int *dataSize)
{
    int tmp;

    /* �����������å� */
    assert(data!=NULL);
    assert(0<=(*dataSize));

    tmp = htonl(intData);

    /* int ���Υǡ����������ѥǡ����κǸ�˥��ԡ����� */
    memcpy(data + (*dataSize),&tmp,sizeof(int));
    /* �ǡ��������������䤹 */
    (*dataSize) += sizeof(int);
}

/*****************************************************************
�ؿ�̾	: SetCharData2DataBlock
��ǽ	: char ���Υǡ����������ѥǡ����κǸ�˥��åȤ���
����	: void		*data		: �����ѥǡ���
		  int		intData		: ���åȤ���ǡ���
		  int		*dataSize	: �����ѥǡ����θ��ߤΥ�����
����	: �ʤ�
*****************************************************************/
static void SetCharData2DataBlock(void *data,char charData,int *dataSize)
{
    /* �����������å� */
    assert(data!=NULL);
    assert(0<=(*dataSize));

    /* int ���Υǡ����������ѥǡ����κǸ�˥��ԡ����� */
    *(char *)(data + (*dataSize)) = charData;
    /* �ǡ��������������䤹 */
    (*dataSize) += sizeof(char);
}
