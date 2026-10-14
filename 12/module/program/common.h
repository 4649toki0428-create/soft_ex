/*****************************************************************
�ե�����̾	: common.h
��ǽ		: �����С��ȥ��饤����Ȥǻ��Ѥ�������������Ԥ�
*****************************************************************/

#ifndef _COMMON_H_
#define _COMMON_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<assert.h>
#include<math.h>

#define PORT			(u_short)8888	/* �ݡ����ֹ� */

#define MAX_CLIENTS		4				/* ���饤����ȿ��κ����� */
#define MAX_NAME_SIZE	10 				/* �桼����̾�κ�����*/

#define MAX_DATA		200				/* ����������ǡ����κ����� */

#define END_COMMAND		'E'		  		/* �ץ�����ཪλ���ޥ�� */
#define CIRCLE_COMMAND	'C'				/* ��ɽ�����ޥ�� */
#define RECT_COMMAND	'R'				/* �ͳ�ɽ�����ޥ�� */
#define DIAMOND_COMMAND	'D'				/* ɩ��ɽ�����ޥ�� */

#define ROCK_COMMAND 'G'
#define SCISSORS_COMMAND 'S'
#define PAPER_COMMAND 'P'
#define RESULT_COMMAND 'T'

#endif
