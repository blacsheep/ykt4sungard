/********************************************************************
	created:	2004/09/23
	created:	23:9:2004   16:35
	filename: 	e:\proj\goldmem\ksbcc\packbase\packb.h
	file path:	e:\proj\goldmem\ksbcc\packbase
	file base:	packb
	file ext:	h
	author:		ChenYH
	
	purpose:	Ϊ������CPACK�м���������ȱ�ݣ�����һ����ǿ��Լ��ST_PACK�ṹ
            �ܹ�֧�֣�
            �����ܹ����÷�ǿ�ƽṹ��ST_PACK��
            �����ܹ�֧�ְ�������ΪRAW-DATA-BLOCK(ST_SDPACK); ���㴫���ļ����Զ������ݿ飻
            ����ǿ����ST_CPACK����������
*********************************************************************/
#ifndef __PACKB_H_
#define __PACKB_H_

/* ������PARMBITS������С��ÿ�ֽڵ�BITS�� */
#define PARMBITS	   255   //(256) ���һ��λͼ��ר�����ڱ�־��CYH�Ĵ����ʽ�����汾��־��һ���ֽ�ΪPACK�ܹ��汾���ڶ��ֽ�ΪPACK���ݽṹ�汾
#define BITSPERBYTE	(8)
#define PARMBYTES 32       // PARMBITS/BITSPERBYTE -- Ӧ�ÿ������ϵͳ�����ٶ�

enum PDLTYPES
{
   PDL_CHAR=1,          // ��ͨ�ַ���ģʽ��Լ�����ȣ��������255
   PDL_VCHAR=2,         // �䳤�ַ����������255
   PDL_UCHAR=3,         // ���ڴ�Ŷ��������ݵ��ֶΣ�����Ȩ�޵�
   PDL_INT=4,           // �����ֶΣ���������64λ���У�long����64λ������32λ������Ϊ32λ������int���ǹ̶��ģ���˱�Э���в��ٲ���long��
   PDL_DOUBLE=5         // double�����ֶ�
};

#ifdef __cplusplus
extern "C" {
#endif

/* ���´����������ֽڶ���ʹ�õ� */
#ifdef WIN32
   #define __PACKED__ 
   #pragma pack(push,1)
#else
   #ifdef __GNUC__
      #define __PACKED__	__attribute__ ((packed))
      //#pragma pack(push,1)
   #else
      #ifdef HP_UX
         #define __PACKED__
         #pragma pack 1
      #else
         #define __PACKED__
         #pragma options align=packed
      #endif
   #endif
#endif

   // ���ڶ���ST_PACK�ṹ�����ݽṹ��
   typedef struct  _ST_PDATALIST {
      char szName[32];
      char wType;    // 0 - None; 1-char; 2-vs_char; 3-unsigned char; 4-int; 5-double
      int  nLength;  // 0 - None.
      unsigned int  dwOffset; // 
   } __PACKED__ ST_PDATALIST;

   
   /*
   ��ַ����
   */
   typedef struct _ST_ADDR
   {
      unsigned char addr[6];
   } __PACKED__ ST_ADDR;
   
   typedef struct _ST_HOOK
   {
      unsigned int	UserID;					/* �����ߵ�ID����*/
      char hostname[33];						/* ������ */
      char queuename[33];						/* ������ */
      unsigned int	queuetype;				/* �������� */
   } __PACKED__ ST_HOOK;
   
   /* 
   ��ͷ�ṹ
   */
   typedef struct _ST_PACKHEAD
   {
      unsigned int	RequestType;
      unsigned char	firstflag;				/* �Ƿ��һ�������װ�����*/
      unsigned char	nextflag;					/* �Ƿ����������*/
      unsigned int	recCount;					/* �����ļ�¼��*/
      unsigned int	retCode;					/* ���ش���*/
      
      ST_ADDR			addr;						/* �����ŵĵ�ַ��6���ӽڣ�*/
      ST_HOOK         hook;	                    /*                    */
      unsigned int    userdata;					/*					*/
      
      unsigned char ParmBits[PARMBYTES];
   } __PACKED__ ST_PACKHEAD;
   

   typedef struct _ST_SDPACK
   {
      unsigned short usDataLength;        /* �������Զ������ݳ��ȣ�������ֽ����ɱ�����ͽ����Զ���� */
      char data[1];              /* �Զ�����������ݣ�ע�⽻��˫�����������ֽ��� */
   } __PACKED__ ST_SDPACK;                
   /*
    *	�����Ľṹ��Ϊ��֧���Զ���������Ƶģ�����usDataLength��ָdata��ʵ�ʵ����ݳ��ȣ��û����ж��������
    * ��Ϊdata���֣��ⲿ���ڲ�ͬ�ֽ��������֮�佻�����ݣ������ݵ��ֽ�����Ҫ�Լ����ƣ����������ģ�鲻������
    */


   
#ifdef WIN32
   #pragma pack(pop)
#else
   #ifdef __GNUC__
   #else
      #ifdef HP_UX
         #pragma pack 0
      #else
         #pragma options align=reset
      #endif
   #endif
#endif
#undef __PACKED__
   
   #define MAXPACKROWS 100


   // ����ΪC/C++���ܵ��õ�һЩ����������
   
#ifdef __cplusplus
}
#endif

#endif
