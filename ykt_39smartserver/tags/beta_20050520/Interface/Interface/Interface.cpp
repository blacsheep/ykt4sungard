 // Interface.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "export.h"
#include "Interface.h"
#include "LocalBlack.h"
#include "ConnectPool.h"
#include "SystemInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib,"..\\lib\\DRTPApi.lib") 
#pragma message("Automatically linking with DRTPApi.dll")

int  iMServerNo;
int  iMFunc;
long iSmartKey;
char sSmartKey[48];
int  iServerNo;
int  iFunc;
TSSmartDoc *gpSmartDoc = NULL;
long gnDocCount = 0 ;
int  gTick = 600000;
TSSystemParam  GlobalParam;
bool bSmartSign = false;
CLock  OutLock;
CLock  InLock;
CTestData  TestData;
char  szLogPath[256];
char  szINIFile[256];
int nReload;
/////////////////////////////////////////////////////////////////////////////
// CInterfaceApp

BEGIN_MESSAGE_MAP(CInterfaceApp, CWinApp)
	//{{AFX_MSG_MAP(CInterfaceApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInterfaceApp construction

CInterfaceApp::CInterfaceApp()
{
	gpSmartDoc = NULL;
	gnDocCount = 0 ;
	iServerNo = 0 ;
	iFunc = 9991;
	iSmartKey = 0 ;
	gTick = 600000;

	iMServerNo = 0;
	iMFunc = 9990;


	ZeroMemory(sSmartKey, sizeof(sSmartKey));
	ZeroMemory(&GlobalParam, sizeof(GlobalParam));

	TCHAR szPath[MAX_PATH];

	GetCurPath(szPath);
	if( szPath[lstrlen(szPath)-1] == '\\' )
	{
		sprintf(szLogPath, "%接口日志", szPath);
		sprintf(szINIFile, "%sSmartServer.ini", szPath);
	}
	else
	{
		sprintf(szLogPath, "%s\\接口日志", szPath);
		sprintf(szINIFile, "%s\\SmartServer.ini", szPath);
	}

	CreateDirectory(szLogPath, NULL);
	nReload = 0;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CInterfaceApp object

CInterfaceApp theApp;
/////////////////////////////////////////////////////////////////////////////
// CInterfaceApp initialization
BOOL CInterfaceApp::InitInstance()
{
	return TRUE;
}

//签出
long CInterfaceApp::SignOutDoc(TSSmartDoc *pDoc)
{
	//return RET_OK;
	ST_CPACK rpack;

	memset(&rpack, 0, sizeof(ST_CPACK));

	rpack.head.RequestType = 930055; 
	rpack.head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	rpack.head.nextflag = 0;    /* 是否后续包请求*/
	rpack.head.recCount = 1;    /* 本包的记录数*/
	rpack.head.retCode = 0;     /* 返回代码*/
	rpack.head.userdata = 0;

	SetValue(&rpack.head,&rpack.pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
	SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT2, sSmartKey);		//动态密钥
	SetValue(&rpack.head,&rpack.pack, F_SDATE0, pDoc->m_szDeviceID);	//终端设备ID

	char buffer[10960];
	int  nLen = sizeof(buffer);
	char omsg[256];

	HANDLE hHandle = ConnectPool.Alloc();

	if( EncodeBuf(&rpack, (unsigned char*)buffer, &nLen, omsg) ) 
	{
		//printf("SignOutDoc\n");
		if( SendData(hHandle, iServerNo, iFunc, (char*)buffer, nLen, 1, FALSE) == RET_OK ) 
		{
			memset(buffer, 0, sizeof(buffer));
			nLen = sizeof(buffer);
			int nlen = RecvData(hHandle, buffer, nLen, 1500);
			if( nlen > 0 )
			{
				ST_CPACK apack;
				if( DecodeBuf((unsigned char*)buffer,nlen,&apack,omsg) )
				{
					if( !apack.head.retCode )
					{
						ConnectPool.Free(hHandle);
						WriteLog("设备:%s签出成功!\n", pDoc->m_szDeviceID);
						return RET_OK;
					}
					else
					{
						printf("%s\n", apack.pack.vsmess);
					}
				}
			}
		}
	}

	ConnectPool.Free(hHandle);

	WriteLog("设备:%s签出失败!\n", pDoc->m_szDeviceID);

	return RET_SYSERROR;
}

//签到
long CInterfaceApp::SignInDoc(TSSmartDoc *pDoc)
{
//	DownloadBlackList(pDoc, 0, "D:\\ZW.LOG");

	ST_CPACK rpack;

	memset(&rpack, 0, sizeof(ST_CPACK));

	rpack.head.RequestType = 930054; 
	rpack.head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	rpack.head.nextflag = 0;    /* 是否后续包请求*/
	rpack.head.recCount = 1;    /* 本包的记录数*/
	rpack.head.retCode = 0;     /* 返回代码*/
	rpack.head.userdata = 0;

	SetValue(&rpack.head,&rpack.pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
	SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
	SetValue(&rpack.head,&rpack.pack, F_SDATE1, pDoc->m_szDeviceID);		//终端设备ID

	char buffer[10960];
	int  nLen = sizeof(buffer);
	char omsg[256];
	HANDLE hHandle = ConnectPool.Alloc();

	if( EncodeBuf(&rpack, (unsigned char*)buffer, &nLen, omsg) ) 
	{
//		printf("SignInDoc\n");
		if( SendData(hHandle, iServerNo, iFunc, (char*)buffer, nLen, 1, FALSE) == RET_OK ) 
		{
			nLen = sizeof(buffer);
			memset(buffer, 0, sizeof(buffer));
			int nlen = RecvData(hHandle, buffer, nLen, 0);
			if( nlen > 0 )
			{
				ST_CPACK apack;
				if( DecodeBuf((unsigned char*)buffer,nlen,&apack,omsg) )
				{
					if( !apack.head.retCode )
					{
						strcpy(pDoc->m_szTableName, apack.pack.scust_limit);
						pDoc->m_nStartuse = 1;
						ConnectPool.Free(hHandle);
						WriteLog("后台允许设备:%s签到!\n", pDoc->m_szDeviceID);
						return RET_OK;
					}
					else
					{
						printf("%s\n", apack.pack.vsmess);
					}
				}
			}
		}
	}

	ConnectPool.Free(hHandle);

	WriteLog("设备:%s签到失败!\n", pDoc->m_szDeviceID);

	return RET_SYSERROR;
}

long CInterfaceApp::SmartSignOut()
{
	ST_CPACK rpack;

	memset(&rpack, 0, sizeof(ST_CPACK));

	rpack.head.RequestType = 930053; 
	rpack.head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	rpack.head.nextflag = 0;    /* 是否后续包请求*/
	rpack.head.recCount = 1;    /* 本包的记录数*/
	rpack.head.retCode = 0;     /* 返回代码*/
	rpack.head.userdata = 0;

	SetValue(&rpack.head,&rpack.pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
	SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥

	char buffer[10960];
	int  nLen = sizeof(buffer);
	char omsg[256];

	HANDLE hHandle = ConnectPool.Alloc();

	if( EncodeBuf(&rpack, (unsigned char*)buffer, &nLen, omsg) ) 
	{
//		printf("SmartSignOut\n");
		if( SendData(hHandle, iServerNo, iFunc, (char*)buffer, nLen, 1, FALSE) == RET_OK ) 
		{
			memset(buffer, 0, sizeof(buffer));
			int nlen = RecvData(hHandle, buffer, nLen, 1500);
			if( nlen > 0 )
			{
				ST_CPACK apack;
				if( DecodeBuf((unsigned char*)buffer,nlen,&apack,omsg) )
				{
					if( !apack.head.retCode )
					{
						WriteLog("三九前置与金仕达签出成功!\n");
						ConnectPool.Free(hHandle);
						return RET_OK;
					}
					else
					{
						WriteLog("三九前置与金仕达签出失败,金仕达返回代码:%s\n", apack.pack.vsmess);
						printf("%s\n", apack.pack.vsmess);
					}
				}
			}
		}
	}

	ConnectPool.Free(hHandle);

	return RET_SYSERROR;
}

long CInterfaceApp::SmartSignIn()
{
	ST_CPACK rpack;
	CSystemInfo info;

	memset(&rpack, 0, sizeof(ST_CPACK));

	rpack.head.RequestType = 930052; 
	rpack.head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	rpack.head.nextflag = 0;    /* 是否后续包请求*/
	rpack.head.recCount = 1;    /* 本包的记录数*/
	rpack.head.retCode = 0;     /* 返回代码*/
	rpack.head.userdata = 0;

	SetValue(&rpack.head,&rpack.pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
	SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT, sSmartKey);			//动态密钥

	WriteLog("三九前置机向金仕达前置申请签到! IP:%s, MAC=%s, 注册号=%d, 动态密钥=%s\n", info.szIP, info.szMAC, iSmartKey, sSmartKey);

	char buffer[10960];
	int  nLen = sizeof(buffer);
	char omsg[256];

	HANDLE hHandle = ConnectPool.Alloc();
	
	if( EncodeBuf(&rpack, (unsigned char*)buffer, &nLen, omsg) ) 
	{
//		printf("SmartSignIn\n");
		if( SendData(hHandle, iServerNo, iFunc, (char*)buffer, nLen, 1, FALSE) == RET_OK ) 
		{
			nLen = sizeof(buffer);
			memset(buffer, 0, sizeof(buffer));
			int nlen = RecvData(hHandle, buffer, nLen, 0);
			if( nlen > 0 )
			{
				ST_CPACK apack;
				memset(&apack, 0, sizeof(apack));

				if( DecodeBuf((unsigned char*)buffer,nlen,&apack,omsg) )
				{
					if( !apack.head.retCode )
					{
						strncpy(sSmartKey, apack.pack.scust_limit2, sizeof(sSmartKey));
						strcpy(GlobalParam.szParameter1, apack.pack.snote);
						strcpy(GlobalParam.szParameter2, apack.pack.scert_addr);
						strcpy(GlobalParam.szParameter3, apack.pack.semail2);
						strcpy(GlobalParam.szParameter4, apack.pack.semail);
						strcpy(GlobalParam.szParameter5, apack.pack.saddr);
						ConnectPool.Free(hHandle);

						printf("前置采集服务器签到: 设备授权密钥%s.\n", GlobalParam.szParameter2);
						printf("前置采集服务器签到: 黑名单版本%s.\n", GlobalParam.szParameter3);
						printf("前置采集服务器签到: 黑名单有效期%s.\n", GlobalParam.szParameter4);
						printf("前置采集服务器签到: 扎帐时间%s.\n", GlobalParam.szParameter1);
						
						WriteLog("三九前置与金仕达签到成功: 设备授权密钥%s.\n", GlobalParam.szParameter2);
						WriteLog("三九前置与金仕达签到成功: 黑名单版本%s.\n", GlobalParam.szParameter3);
						WriteLog("三九前置与金仕达签到成功: 黑名单有效期%s.\n", GlobalParam.szParameter4);
						WriteLog("三九前置与金仕达签到成功: 扎帐时间%s.\n", GlobalParam.szParameter1);
						
						return RET_OK;
					}
					else
					{
						WriteLog("前置机与金仕达签到失败! 金仕达返回错误代码:%s\n", apack.pack.vsmess);
					}
				}
			}
		}
	}

	ConnectPool.Free(hHandle);

	return RET_SYSERROR;
}

//----------------------------------------------------------------------------
//
//  Function:   CheckSmartRegister
//
//  Synopsis:   前置机注册
//
//  Arguments:  无
//				
//				
//  History:    ZhangWei   Created
//
//  Notes:
//----------------------------------------------------------------------------
bool CInterfaceApp::CheckSmartRegister()
{
	char szValue[256];

	memset(szValue, 0, sizeof(szValue));
	int nRegister = GetPrivateProfileString("REGISTER", "REGISTER", "0", szValue, 256, szINIFile);
	if( nRegister <= 0 || strcmp(szValue, "1") )
	{
		iSmartKey = 0 ;
		memset(sSmartKey, 0, sizeof(sSmartKey));
		return false;
	}
	else
	{
		GetPrivateProfileString("REGISTER", "KEY", "", sSmartKey, 64, szINIFile);
		iSmartKey = GetPrivateProfileInt("REGISTER", "REGISTERNO", 0, szINIFile);
		if( iSmartKey <= 0 || strlen(sSmartKey) <= 0 ) 
			return false;
		return true;
	}
}

//----------------------------------------------------------------------------
//
//  Function:   SmartRegister
//
//  Synopsis:   前置机注册
//
//  Arguments:  无
//				
//				
//  History:    ZhangWei   Created
//
//  Notes:
//----------------------------------------------------------------------------
long CInterfaceApp::SmartRegister()
{
	if( CheckSmartRegister() )
	{
		return RET_OK;
	}

	CSystemInfo info;

	ST_CPACK rpack;

	memset(&rpack, 0, sizeof(ST_CPACK));

	rpack.head.RequestType = 930051; 
	rpack.head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	rpack.head.nextflag = 0;    /* 是否后续包请求*/
	rpack.head.recCount = 1;    /* 本包的记录数*/
	rpack.head.retCode = 0;     /* 返回代码*/
	rpack.head.userdata = 0;

	SetValue(&rpack.head,&rpack.pack, F_SCUST_AUTH2, "SMART999");	//前置机注册号
//	debug info SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT, "10.83.28.21");//info.szIP
	SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT, info.szIP);//info.szIP
	SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT2, info.szMAC);	//mac address
	// modified by lina 20050305 WriteLog("前置机注册! IP:%s, MAC=%s.\n", info.szIP, info.szMAC);
	WriteLog("三九前置机与金仕达申请注册! 申请参数IP:%s, MAC=%s.\n", info.szIP, info.szMAC);

//	debug info SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT, "10.83.28.11");//info.szIP
//	debug info SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT2, "00-0D-61-9B-40-8C");	//mac address

//	debug info SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT, "10.83.28.17");//info.szIP
//	debug info SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT2, "00-0D-61-4A-7F-3C");	//mac address

	char buffer[10960];
	int  nLen = sizeof(buffer);
	char omsg[256];

	HANDLE hHandle = ConnectPool.Alloc();
//	printf("SmartRegister\n");
	if( EncodeBuf(&rpack, (unsigned char*)buffer, &nLen, omsg) ) 
	{
		if( SendData(hHandle, iServerNo, iFunc, (char*)buffer, nLen, 1, FALSE) == RET_OK ) 
		{
			nLen = sizeof(buffer);			
			memset(buffer, 0, sizeof(buffer));
			int nlen = RecvData(hHandle, buffer, nLen, 1500);
			if( nlen > 0 )
			{
				ST_CPACK apack;
				if( DecodeBuf((unsigned char*)buffer,nlen,&apack,omsg) )
				{
					if( !apack.head.retCode )
					{
						WritePrivateProfileString("REGISTER", "REGISTERNO", GetValue(apack.pack.lcert_code), szINIFile);
						WritePrivateProfileString("REGISTER", "KEY", apack.pack.scust_limit, szINIFile);
						WritePrivateProfileString("REGISTER", "REGISTER", "1", szINIFile);

						iSmartKey = apack.pack.lcert_code;
						strncpy(sSmartKey, apack.pack.scust_limit, sizeof(sSmartKey));
						WriteLog("三九前置机与金仕达注册成功! 注册号:%d, 密钥:%s\n", apack.pack.lcert_code, apack.pack.scust_limit);
						printf("三九前置机与金仕达注册成功! 注册号:%d, 密钥:%s\n", apack.pack.lcert_code, apack.pack.scust_limit);
						ConnectPool.Free(hHandle);
						return RET_OK;
					}
					else
					{
						printf("%s\n", apack.pack.vsmess);
						WriteLog("三九前置机与金仕达注册失败 接收金仕达信息:%s! \n", apack.pack.vsmess);
					}
				}
			}
		}
	}

	WriteLog("三九前置机与金仕达注册失败! \n");

	ConnectPool.Free(hHandle);

	return RET_SYSERROR;
}

//----------------------------------------------------------------------------
//
//  Function:   MakeFirstTaskPlan
//
//  Synopsis:   根据请求包, 转换成内部用的任务包
//
//  Arguments:  pTaskPlan   -- 返回内部用的任务包
//				pPacket     -- 金仕达请求包
//  History:    2004-09-21  ZhangWei   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
bool CInterfaceApp::MakeFirstTaskPlan(TSSmartTaskPlan *pTaskPlan, ST_CPACK *pPacket, ST_PACK *pArray, int nRow)
{
	bool bResult = true ;
	int iCount = 0 ;
	
	//WriteLog("-------MakeFirstTaskPlan----------\n");

	if( pPacket->pack.lvol2 == 930021 )
	{
		if( FillSmartDoc(&pPacket->pack) )
		{
			WriteLog("金仕达更新设备资料: 功能号:%d...\n", pPacket->pack.lvol2);
			printf("金仕达更新设备资料: 功能号:%d...\n", pPacket->pack.lvol2);
			return AnswerSmartDocSuccess(pPacket->pack.lvol2, pPacket->pack.lvol1, 0, "")==RET_OK?true:false;
		}
		else
		{
			return AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "参数不合法!")==RET_OK?true:false;
		}
	}
	else if( pPacket->pack.lvol2 == 930003 || pPacket->pack.lvol2 == 930004 )
	{
		TSSmartDoc *pDoc = CheckDeviceID(pPacket->pack.sdate0);
		if( pDoc == NULL )
		{
			WriteLog("金仕达下达任务:  设备%s, 功能号:%d.  .....没有此设备!\n", pPacket->pack.sdate0, pPacket->pack.lvol2);
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "没有此设备!");
			return false;
		}

		long nAuthID = pDoc->m_nAuthID;
		WriteLog("金仕达下达任务: 设备%s, 功能号:%d, 黑名单条数:%d.......\n", 
			                      pDoc->m_szDeviceID, 
								  pPacket->pack.lvol2, 
								  nRow);

		pTaskPlan->nTaskPlanID = GetTickCount()-5;
		sprintf(pTaskPlan->szTaskName,"功能号:%d", pPacket->pack.lvol2); //任务计划名称
		pTaskPlan->nTaskCycle=0;                  //任务计划执行周期
		GetCurDateTime(pTaskPlan->szBeginTime);   //开始日期时间
		pTaskPlan->nRepeatTime  =  0;              //持续多长时间（分钟）
		pTaskPlan->nRepeatTimes = 1;            //重复次数
		GetCurDateTime(pTaskPlan->szTaskTime);    //任务计划的时间

		pTaskPlan->nTask = 1 ;
		pTaskPlan->pTask[0].nTaskID = GetTickCount();
		pTaskPlan->pTask[0].nTaskPlanID=pTaskPlan->nTaskPlanID; 
		pTaskPlan->pTask[0].nAuthID = nAuthID;                  
		strcpy(pTaskPlan->pTask[0].szDeviceID, pDoc->m_szDeviceID);      
		strcpy(pTaskPlan->pTask[0].szTaskCode,"70");
		pTaskPlan->pTask[0].nPriority = 6;
		pTaskPlan->pTask[0].nFunc = pPacket->pack.lvol2; 
		sprintf(pTaskPlan->pTask[0].szSystemNO, "%d", pPacket->pack.lvol1);
		sprintf(pTaskPlan->pTask[0].szMemo, "");
		
		//printf("分配TSAttachData 内容\n");
		TSAttachData *pData = new TSAttachData;
		if( pData == NULL )
		{
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "系统错误!");
			return false;
		}
		//printf("分配TSBlackCard 内容\n");
		TSBlackCard *pNo = new TSBlackCard[nRow];
		if( pNo == NULL )
		{
			delete pData;
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "系统错误!");
			return false;
		}

		ZeroMemory(pData, sizeof(TSAttachData));
		ZeroMemory(pNo, sizeof(TSBlackCard)*nRow);

		if( pPacket->pack.lvol2 == 930003 )
			pData->nValue = 0 ;  //下传黑名单
		else
			pData->nValue = 1 ;  //删除黑名单

		pData->pData = (void*)pNo;
		pTaskPlan->pTask[0].pData = (char*)pData;
		pData->nCount = nRow;

		for(int i=0; i< nRow; i++)
		{
			if( !i )
			{
				strncpy(pNo[i].sVersion, pPacket->pack.sserial0, 12);
				strncpy(pNo[i].sValid, pPacket->pack.sserial1, 12);
				pNo[i].nCardID = pPacket->pack.lvol0;
				pNo[i].nFlag = pData->nValue;
				if(pNo[i].nFlag  == 0)//add by lina 20050305
				{
					WriteLog("金仕达要求增加黑名单 %ld,版本号%s\n",pNo[i].nCardID,pNo[i].sVersion);
				}
				else
					WriteLog("金仕达要求删除黑名单 %ld,版本号%s\n",pNo[i].nCardID,pNo[i].sVersion);

			}
			else
			{
				strncpy(pNo[i].sVersion, pArray[i-1].sserial0, 12);
				strncpy(pNo[i].sValid, pArray[i-1].sserial1, 12);
				pNo[i].nCardID = pArray[i-1].lvol0;
				pNo[i].nFlag = pData->nValue;
				if(pNo[i].nFlag  == 0)//add by lina 20050305
				{
					WriteLog("金仕达要求增加黑名单 %ld,版本号%s\n",pNo[i].nCardID,pNo[i].sVersion);
				}
				else
					WriteLog("金仕达要求删除黑名单 %ld,版本号%s\n",pNo[i].nCardID,pNo[i].sVersion);
			}
		}
		iCount = 1 ;
	}
	//下传增量黑名单----广播黑名单
	else if( pPacket->pack.lvol2 == 930005 )
	{
		WriteLog("金仕达下达广播黑名单任务: 设备%s, 功能号:%d, 卡号:%d.......\n", "00000000", pPacket->pack.lvol2, pPacket->pack.lvol0);
		AnswerResult(pPacket->pack.lvol2, pPacket->pack.lvol1, 0, "收到广播信息!");

		pTaskPlan->nTaskPlanID = GetTickCount()-5;
		sprintf(pTaskPlan->szTaskName,"功能号:%d", pPacket->head.RequestType); //任务计划名称
		pTaskPlan->nTaskCycle=0;                  //任务计划执行周期
		GetCurDateTime(pTaskPlan->szBeginTime);   //开始日期时间
		pTaskPlan->nRepeatTime = 0 ;              //持续多长时间（分钟）
		pTaskPlan->nRepeatTimes = 1;             //重复次数
		GetCurDateTime(pTaskPlan->szTaskTime);    //任务计划的时间
		pTaskPlan->nTask = 0 ;

		int p=0;
		for(int i=0; i< gnDocCount; i++)
		{
			if( !gpSmartDoc[i].m_nParentID )
			{
				pTaskPlan->pTask[p].nTaskID = GetTickCount();
				pTaskPlan->pTask[p].nTaskPlanID=pTaskPlan->nTaskPlanID; 
				pTaskPlan->pTask[p].nAuthID = gpSmartDoc[i].m_nAuthID ;
				strcpy(pTaskPlan->pTask[p].szDeviceID, gpSmartDoc[i].m_szDeviceID);
				strcpy(pTaskPlan->pTask[p].szTaskCode,"70");
				pTaskPlan->pTask[p].nPriority = 9;
				pTaskPlan->pTask[p].nFunc = 930005;
				
			    if( pPacket->pack.lvol4 ) 
					sprintf(pTaskPlan->pTask[p].szMemo, "-%d", pPacket->pack.lvol0);
				else
					sprintf(pTaskPlan->pTask[p].szMemo, "%d", pPacket->pack.lvol0);
				p++;
				pTaskPlan->nTask++;
				//WriteLog("!!!广播黑名单....设备:%s.....卡号:%d....\n", gpSmartDoc[i].m_szDeviceID, pPacket->pack.lvol0);
			}
			Sleep(3);
		}

		if( pTaskPlan->nTask > 0 )
			iCount = 1 ;
	}
	//设置消费编号及版本
	else if( pPacket->pack.lvol2 == 930011 )
	{
		TSSmartDoc *pDoc = CheckDeviceID(pPacket->pack.sdate0);
		if( pDoc == NULL )
		{
			WriteLog("金仕达下达任务:  设备%s, 功能号:%d.  .....没有此设备!\n", pPacket->pack.sdate0, pPacket->head.RequestType);
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "没有此设备!");
			return false;
		}

		if( strlen(pPacket->pack.sserial2) != 12 )
		{
			WriteLog("金仕达下达任务:  设备%s, 功能号:%d.  .....参数不合法!\n", pDoc->m_szDeviceID, pPacket->head.RequestType);
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "参数不合法!");
			return false;
		}

		long nAuthID = pDoc->m_nAuthID;
		WriteLog("金仕达下达任务: 设备%s, 功能号:%d...\n", pDoc->m_szDeviceID, pPacket->head.RequestType);

		pTaskPlan->nTaskPlanID = GetTickCount()-5;
		sprintf(pTaskPlan->szTaskName,"功能号:%d", pPacket->head.RequestType); //任务计划名称
		pTaskPlan->nTaskCycle=0;                  //任务计划执行周期
		GetCurDateTime(pTaskPlan->szBeginTime);   //开始日期时间
		pTaskPlan->nRepeatTime = 0 ;              //持续多长时间（分钟）
		pTaskPlan->nRepeatTimes = 1 ;             //重复次数
		GetCurDateTime(pTaskPlan->szTaskTime);    //任务计划的时间

		pTaskPlan->nTask = 1 ;
		pTaskPlan->pTask[0].nTaskID = GetTickCount();
		pTaskPlan->pTask[0].nTaskPlanID=pTaskPlan->nTaskPlanID; 
		pTaskPlan->pTask[0].nAuthID = nAuthID;                  
		strcpy(pTaskPlan->pTask[0].szDeviceID, pDoc->m_szDeviceID);      
		strcpy(pTaskPlan->pTask[0].szTaskCode,"104");
		pTaskPlan->pTask[0].nPriority = 6;
		pTaskPlan->pTask[0].nFunc = 930011; 
		sprintf(pTaskPlan->pTask[0].szSystemNO, "%d", pPacket->pack.lvol1);

		TSAttachData *pData = new TSAttachData;
		if( pData == NULL )
		{
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "系统错误!");
			return false;
		}

		int *pNo = new int[nRow];
		if( pNo == NULL )
		{
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "系统错误!");
			delete pData;
			return false;
		}

		ZeroMemory(pData, sizeof(TSAttachData));
		ZeroMemory(pNo, sizeof(int)*nRow);

		pData->pData = (void*)pNo;
		pTaskPlan->pTask[0].pData = (char*)pData;

		pData->nCount = nRow;
		strncpy(pData->szValue, pPacket->pack.sserial2,12);

		for(int i=0; i< nRow; i++)
		{
			if( !i )
			{
				pNo[i] = pPacket->pack.lvol4;
			}
			else
			{
				pNo[i] = pArray[i-1].lvol4;
			}
		}
		iCount = 1 ;
	}
	//设置消费快捷编号
	else if( pPacket->pack.lvol2 == 930012 )
	{
		TSSmartDoc *pDoc = CheckDeviceID(pPacket->pack.sdate0);
		if( pDoc == NULL )
		{
			WriteLog("金仕达下达任务:  设备%s, 功能号:%d.  .....没有此设备!\n", pPacket->pack.sdate0, pPacket->head.RequestType);
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "没有此设备!");
			return false;
		}

		long nAuthID = pDoc->m_nAuthID;
		WriteLog("金仕达下达任务: 设备%s, 功能号:%d...\n", pDoc->m_szDeviceID, pPacket->head.RequestType);

		pTaskPlan->nTaskPlanID = GetTickCount()-5;
		sprintf(pTaskPlan->szTaskName,"功能号:%d", pPacket->head.RequestType); //任务计划名称
		pTaskPlan->nTaskCycle=0;                  //任务计划执行周期
		GetCurDateTime(pTaskPlan->szBeginTime);   //开始日期时间
		pTaskPlan->nRepeatTime = 0 ;              //持续多长时间（分钟）
		pTaskPlan->nRepeatTimes = 1 ;             //重复次数
		GetCurDateTime(pTaskPlan->szTaskTime);    //任务计划的时间

		pTaskPlan->nTask = 1 ;
		pTaskPlan->pTask[0].nTaskID = GetTickCount();
		pTaskPlan->pTask[0].nTaskPlanID=pTaskPlan->nTaskPlanID; 
		pTaskPlan->pTask[0].nAuthID = nAuthID;                  
		strcpy(pTaskPlan->pTask[0].szDeviceID, pDoc->m_szDeviceID);      
		strcpy(pTaskPlan->pTask[0].szTaskCode,"105");
		pTaskPlan->pTask[0].nPriority = 6;
		pTaskPlan->pTask[0].nFunc = 930012; 
		sprintf(pTaskPlan->pTask[0].szSystemNO, "%d", pPacket->pack.lvol1);
//		printf("pPacket->pack.saddr=%s\n",pPacket->pack.saddr);
		strcpy(pTaskPlan->pTask[0].szMemo, 	pPacket->pack.saddr);//modified by lina 20050318
		
		/*modified by lina 20050318 TSAttachData *pData = new TSAttachData;
		if( pData == NULL )
		{
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "系统错误!");
			return false;
		}

		int *pNo = new int[nRow];
		if( pNo == NULL )
		{
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "系统错误!");
			delete pData;
			return false;
		}

		ZeroMemory(pData, sizeof(TSAttachData));
		ZeroMemory(pNo, sizeof(int)*nRow);

		pData->pData = (void*)pNo;
		pTaskPlan->pTask[0].pData = (char*)pData;
		pData->nCount = nRow;

		for(int i=0; i< nRow; i++)
		{
			if( !i )
			{
				pNo[i] = pPacket->pack.lvol4;
			}
			else
			{
				pNo[i] = pArray[i-1].lvol4;
			}
		}*/
		iCount = 1 ;
	}
	//设置消费时间段参数
	else if( pPacket->pack.lvol2 == 930013 )
	{
		TSSmartDoc *pDoc = CheckDeviceID(pPacket->pack.sdate0);
		if( pDoc == NULL )
		{
			WriteLog("金仕达下达任务:  设备%s, 功能号:%d.  .....没有此设备!\n", pPacket->pack.sdate0, pPacket->head.RequestType);
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "没有此设备!");
			return false;
		}

		long nAuthID = pDoc->m_nAuthID;
		WriteLog("金仕达下达任务: 设备%s, 功能号:%d...\n", pDoc->m_szDeviceID, pPacket->head.RequestType);

		pTaskPlan->nTaskPlanID = GetTickCount()-5;
		sprintf(pTaskPlan->szTaskName,"功能号:%d", pPacket->head.RequestType); //任务计划名称
		pTaskPlan->nTaskCycle=0;                  //任务计划执行周期
		GetCurDateTime(pTaskPlan->szBeginTime);   //开始日期时间
		pTaskPlan->nRepeatTime = 0 ;              //持续多长时间（分钟）
		pTaskPlan->nRepeatTimes = 1 ;             //重复次数
		GetCurDateTime(pTaskPlan->szTaskTime);    //任务计划的时间

		pTaskPlan->nTask = 1 ;
		pTaskPlan->pTask[0].nTaskID = GetTickCount();
		pTaskPlan->pTask[0].nTaskPlanID=pTaskPlan->nTaskPlanID; 
		pTaskPlan->pTask[0].nAuthID = nAuthID;                  
		strcpy(pTaskPlan->pTask[0].szDeviceID, pDoc->m_szDeviceID);      
		strcpy(pTaskPlan->pTask[0].szTaskCode,"107");
		pTaskPlan->pTask[0].nPriority = 6;
		pTaskPlan->pTask[0].nFunc = 930013; 
		sprintf(pTaskPlan->pTask[0].szSystemNO, "%d", pPacket->pack.lvol1);

		TSAttachData *pData = new TSAttachData;
		if( pData == NULL )
		{
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "系统错误!");
			return false;
		}

		TSXFTimePara *pNo = new TSXFTimePara[nRow];
		if( pNo == NULL )
		{
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "系统错误!");
			delete pData;
			return false;
		}

		ZeroMemory(pData, sizeof(TSAttachData));
		ZeroMemory(pNo, sizeof(TSXFTimePara)*nRow);

		pData->pData = (void*)pNo;
		pTaskPlan->pTask[0].pData = (char*)pData;
		pData->nCount = nRow;

		for(int i=0; i< nRow; i++)
		{
			if( !i )
			{
				strncpy(pNo[i].szBeginTime, pPacket->pack.sopen_emp, 5);
				strncpy(pNo[i].szEndTime, pPacket->pack.sclose_emp, 5);
				strncpy(pNo[i].szClass, pPacket->pack.snote2, 64);
				pNo[i].nTimes = pPacket->pack.lvol5;
				pNo[i].nMoney = pPacket->pack.lvol4;
			}
			else
			{
				strncpy(pNo[i].szBeginTime, pArray[i-1].sopen_emp, 5);
				strncpy(pNo[i].szEndTime, pArray[i-1].sclose_emp, 5);
				strncpy(pNo[i].szClass, pArray[i-1].snote2, 64);
				pNo[i].nTimes = pArray[i-1].lvol5;
				pNo[i].nMoney = pArray[i-1].lvol4;
			}

			if( strlen(pNo[i].szBeginTime) != 5 || strlen(pNo[i].szEndTime) != 5 || 
				pNo[i].nTimes < 0 )
			{
				delete [] pNo;
				pData->pData = NULL;

				delete pData;
				pTaskPlan->pTask[0].pData = NULL;

				WriteLog("金仕达下达任务:  设备%s, 功能号:%d.  .....参数不合法!\n", pDoc->m_szDeviceID, pPacket->head.RequestType);
				AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "参数不合法!");
				return false;
			}
		}
		iCount = 1 ;
	}
	//下传补助发放名单
	else if(  pPacket->pack.lvol2 == 930020 )
	{
		TSSmartDoc *pDoc = CheckDeviceID(pPacket->pack.sdate0);
		if( pDoc == NULL )
		{
			WriteLog("金仕达下达任务:  设备%s, 功能号:%d.  .....没有此设备!\n", pPacket->pack.sdate0, pPacket->head.RequestType);
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "没有此设备!");
			return false;
		}

		if( strcmp(pDoc->m_szMacCode, "5301") )
		{
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "只有LPort设备才有此功能!");
			return false;
		}

		long nAuthID = pDoc->m_nAuthID;
		WriteLog("金仕达下达任务: 设备%s, 功能号:%d...\n", pDoc->m_szDeviceID, pPacket->head.RequestType);

		pTaskPlan->nTaskPlanID = GetTickCount()-5;
		sprintf(pTaskPlan->szTaskName,"功能号:%d", pPacket->head.RequestType); //任务计划名称
		pTaskPlan->nTaskCycle=0;                  //任务计划执行周期
		GetCurDateTime(pTaskPlan->szBeginTime);   //开始日期时间
		pTaskPlan->nRepeatTime = 0 ;              //持续多长时间（分钟）
		pTaskPlan->nRepeatTimes = 1 ;             //重复次数
		GetCurDateTime(pTaskPlan->szTaskTime);    //任务计划的时间

		pTaskPlan->nTask = 1 ;
		pTaskPlan->pTask[0].nTaskID = GetTickCount();
		pTaskPlan->pTask[0].nTaskPlanID=pTaskPlan->nTaskPlanID; 
		pTaskPlan->pTask[0].nAuthID = nAuthID;                  
		strcpy(pTaskPlan->pTask[0].szDeviceID, pDoc->m_szDeviceID);      
		strcpy(pTaskPlan->pTask[0].szTaskCode,"60");
		pTaskPlan->pTask[0].nPriority = 6;
		pTaskPlan->pTask[0].nFunc = 930020; 
		sprintf(pTaskPlan->pTask[0].szSystemNO, "%d", pPacket->pack.lvol1);

		TSAttachData *pData = new TSAttachData;
		if( pData == NULL )
		{
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "系统错误!");
			return false;
		}

		TSBZDataList *pNo = new TSBZDataList[nRow];
		if( pNo == NULL )
		{
			AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "系统错误!");
			delete pData;
			return false;
		}

		ZeroMemory(pData, sizeof(TSAttachData));
		ZeroMemory(pNo, sizeof(TSBZDataList)*nRow);

		pData->pData = (void*)pNo;
		pTaskPlan->pTask[0].pData = (char*)pData;
		pData->nCount = nRow;

		for(int i=0; i< nRow; i++)
		{
			if( !i )
			{
				pNo[i].nCardID = pPacket->pack.lvol0;
				pNo[i].nBatch = pPacket->pack.lvol6;
				pNo[i].nMoney = pPacket->pack.lvol5;
			}
			else
			{
				pNo[i].nCardID = pArray[i-1].lvol0;
				pNo[i].nBatch = pArray[i-1].lvol6;
				pNo[i].nMoney = pArray[i-1].lvol5;
			}
			printf("i=%d\n",i);
			printf("补助名单卡号=%d\n",pNo[i].nCardID);
			printf("补助名单批号=%d\n",pNo[i].nBatch);
			printf("补助名单金额=%d\n",pNo[i].nMoney);
			if( pNo[i].nBatch >= 65535 )
			{
				delete [] pNo;
				pData->pData = NULL;
				delete pData;
				pTaskPlan->pTask[0].pData = NULL;

				WriteLog("金仕达下达任务:  设备%s, 功能号:%d.  .....参数不合法!\n", pDoc->m_szDeviceID, pPacket->head.RequestType);
				AnswerResultError(pPacket->pack.lvol2, pPacket->pack.lvol1, -1, "参数不合法!");
				return false;
			}
		}
		iCount = 1 ;
	}
	//其它功能包
	else
	{
		pTaskPlan->nTaskPlanID = GetTickCount()-5;
		sprintf(pTaskPlan->szTaskName,"功能号:%d", pPacket->head.RequestType); //任务计划名称
		pTaskPlan->nTaskCycle=0;                  //任务计划执行周期
		GetCurDateTime(pTaskPlan->szBeginTime);   //开始日期时间
		pTaskPlan->nRepeatTime = 0 ;              //持续多长时间（分钟）
		pTaskPlan->nRepeatTimes = 1 ;             //重复次数
		GetCurDateTime(pTaskPlan->szTaskTime);    //任务计划的时间
		pTaskPlan->nTask = 0 ;

		WriteLog("金仕达下达任务: 设备%s, 功能号:%d...\n", pPacket->pack.sdate0, pPacket->pack.lvol2);

		if( !strcmp(pPacket->pack.sdate0, ALLDEVICE) )
		{
			for(int i=0; i< gnDocCount; i++)
			{
				if( FillTask(gpSmartDoc[i].m_szDeviceID, pTaskPlan, pPacket->pack.lvol2, &pPacket->pack) )
					iCount++;
				Sleep(2);
			}
		}
		else
		{
			for(int i=0; i< nRow; i++)
			{
				if( !i )
				{
					if( FillTask(pPacket->pack.sdate0, pTaskPlan, pPacket->pack.lvol2, &pPacket->pack) )
						iCount ++;
				}
				else
				{
					if( FillTask(pArray[i-1].sdate0, pTaskPlan, pPacket->pack.lvol2, &pArray[i-1]) )
						iCount ++;
				}
				Sleep(2);
			}
		}
	}
	
	//WriteLog("------End Make First Task Plan----- iCount = %d\n",iCount);
	return (iCount<=0?false:true);
}

//----------------------------------------------------------------------------
//
//  Function:   MakeNextTaskPlan
//
//  Synopsis:   根据请求包, 转换成内部用的任务包
//
//  Arguments:  pTaskPlan   -- 返回内部用的任务包
//				pPacket     -- 金仕达请求包
//  History:    2004-09-21  ZhangWei   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
bool CInterfaceApp::MakeNextTaskPlan(TSSmartTaskPlan *pTaskPlan, ST_CPACK *pPacket, ST_PACK *pArray, int nRow)
{
	return true;
}

//----------------------------------------------------------------------------
//
//  Function:   ConvertError
//
//  Synopsis:   没有数据或硬件出错
//
//  Arguments:  pPacket   -- 返回金仕达包
//				ucRawData -- 收上的数据
//				nDataLen  -- 收上的数据长度
//  History:    1-11-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
long CInterfaceApp::ConvertError(int nType, TSSmartTask *pTask, TSSmartDoc *pDoc, ST_CPACK *pPacket, unsigned char *ucRawData, int nDataLen)
{
	memset(pPacket, 0, sizeof(ST_CPACK));
	pPacket->head.RequestType = 930098; 
	pPacket->head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	pPacket->head.nextflag = 0;    /* 是否后续包请求*/
	pPacket->head.recCount = 1;    /* 本包的记录数*/
	pPacket->head.userdata = 0;
	pPacket->head.retCode = 0;     /* 返回代码*/

	if( nType == 0x09 )
	{
		SetValue(&pPacket->head,&pPacket->pack, F_VSMESS, "无记录或数据");	//返回码信息
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL4, "10");	//返回
	}
	else
	{
		SetValue(&pPacket->head,&pPacket->pack, F_VSMESS, "终端无反应");	//返回码信息
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL4, "-6");	//返回
	}

	pPacket->head.RequestType = 930098; 
	SetValue(&pPacket->head,&pPacket->pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
	SetValue(&pPacket->head,&pPacket->pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL1, pTask->szSystemNO);		//消息ID

	return RET_OK;
}


//----------------------------------------------------------------------------
//
//  Function:   ConvertDealData
//
//  Synopsis:   将硬件数据流转换为交易数据
//
//  Arguments:  pPacket   -- 返回金仕达包
//				ucRawData -- 收上的数据
//				nDataLen  -- 收上的数据长度
//  History:    1-11-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
long CInterfaceApp::ConvertDealData(int nType, TSSmartTask *pTask, TSSmartDoc *pDoc, ST_CPACK *pPacket, unsigned char *ucRawData, int nDataLen)
{
	TSSFTable   table;
	ZeroMemory(&table, sizeof(table));

	printf("===========交易数据================================\n");

	sprintf(table.sAuthData,"%.2X%.2X%.2X%.2X", ucRawData[0],ucRawData[1],ucRawData[2],ucRawData[3]);
	//strcpy(table.sAuthData, pDoc->m_szDeviceID);
	printf("注册号:%s\n", table.sAuthData);

	sprintf(table.sCommVer,"%d",ucRawData[4]);
	//strcpy(table.sCommVer, "19");
	printf("通信版本:%s\n",table.sCommVer);

	sprintf(table.sWalletcode , "%d", ucRawData[5]>>4); 
	//strcpy(table.sWalletcode, "0");
	printf("钱包号:%s\n",table.sWalletcode);

	sprintf(table.sCommStart, "%d",ucRawData[5]&0x0F);
	//strcpy(table.sCommStart, "2");
	printf("启动原因:%s\n",table.sCommStart);

	table.nCardID = ucRawData[8]*65536+ucRawData[9]*256+ucRawData[10];
	//table.nCardID = pDoc->m_nCardID;
	printf("交易卡号:%d\n", table.nCardID);

    table.nInvoiceNo = ucRawData[6]*256+ucRawData[7];
	//table.nInvoiceNo = pDoc->m_wFlowNo;
	printf("交易流水号:%.5d\n",ucRawData[6]*256+ucRawData[7]);

	sprintf(table.sCRCData,"%.2X%.2X",ucRawData[26],ucRawData[27]);
	printf("CRC:%s\n",table.sCRCData);

	if( ucRawData[12] >= 100 || ucRawData[13] >= 100 || 
		ucRawData[14] >= 100 || ucRawData[15] >= 100  || ucRawData[16] >= 100 )
	{
		sprintf(table.sDealDateTime, "%02d%02d%02d %02d%02d%02d", 00, 1, 1, 1,1,1);
		printf("交易时间:%02d%02d%02d %02d%02d%02d\n", 00, 1, 1, 1,1,1);
	}
	else
	{
		sprintf(table.sDealDateTime,"%02d%02d%02d %02d%02d%02d",ucRawData[11],ucRawData[12],ucRawData[13],ucRawData[14],ucRawData[15],ucRawData[16]); 
		printf("交易时间:%02d%02d%02d %02d%02d%02d\n",ucRawData[11],ucRawData[12],ucRawData[13],ucRawData[14],ucRawData[15],ucRawData[16]);
	}

	//GetCurDateTimeEx(table.sDealDateTime);

	table.nTimes = ucRawData[17]*256+ucRawData[18]; 
	//table.nTimes = pDoc->m_nDealCount ;
	printf("卡片累计使用次数:%d\n",table.nTimes); 

	table.nInMoney = ucRawData[19]+ucRawData[20]*256+ucRawData[21]*65536;
	//table.nInMoney = pDoc->m_nInMoney;
	printf("入卡金额(分):%d\n", table.nInMoney); 

	table.nOutMoney = ucRawData[22]+ucRawData[23]*256+ucRawData[24]*65536;
	//table.nOutMoney = pDoc->m_nOutMoney;
	printf("出卡金额(分):%d\n", table.nOutMoney);

	sprintf(table.sDealCode1, "%d",ucRawData[25]);
	printf("交易标记:%s\n",table.sDealCode1); 

	memset(pPacket, 0, sizeof(ST_CPACK));
	pPacket->head.RequestType = 930031; 
	pPacket->head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	pPacket->head.nextflag = 0;    /* 是否后续包请求*/
	pPacket->head.recCount = 1;    /* 本包的记录数*/
	pPacket->head.retCode = 0;     /* 返回代码*/
	pPacket->head.userdata = 0;

	if( nType == 0x21 )
	{
		pPacket->head.RequestType = 930098; 
		SetValue(&pPacket->head,&pPacket->pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
		SetValue(&pPacket->head,&pPacket->pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL1, pTask->szSystemNO);			//消息ID
		SetValue(&pPacket->head,&pPacket->pack, F_VSMESS, "收数成功");	//返回码信息
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL4, "0");	//返回
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL11, GetValue(table.nInvoiceNo));	//交易流水号
	}
	else
	{
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL4, GetValue(table.nInvoiceNo));	//交易流水号
	}

	char *p = GetValue(table.nInvoiceNo);

	CString strTime = table.sDealDateTime;

	SetValue(&pPacket->head,&pPacket->pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
	SetValue(&pPacket->head,&pPacket->pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
	SetValue(&pPacket->head,&pPacket->pack, F_SDATE1, table.sAuthData);			//终端设备ID
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL11, GetValue(table.nInvoiceNo));	//交易流水号
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL5, GetValue(table.nCardID));		//交易卡号
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL6, table.sWalletcode);			//钱包号
	SetValue(&pPacket->head,&pPacket->pack, F_SPOST_CODE, strTime.Left(6).GetBuffer(0));		//交易日期
	SetValue(&pPacket->head,&pPacket->pack, F_SPOST_CODE2, strTime.Right(6).GetBuffer(0));		//交易时间
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL7, GetValue(table.nTimes));							//累计使用次数
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL8, GetValue(table.nInMoney-table.nOutMoney));			//本次消费金额
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL9, GetValue(table.nInMoney));							//入卡金额
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL10, GetValue(table.nOutMoney));						//出卡金额
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL12, table.sDealCode1);						//交易标记

	SetValue(&pPacket->head,&pPacket->pack, F_LBANK_ACC_TYPE, table.sCommVer);						//通信版本
	SetValue(&pPacket->head,&pPacket->pack, F_LBANK_ACC_TYPE2, table.sCommStart);					//启动原因
	SetValue(&pPacket->head,&pPacket->pack, F_SBANK_CODE2, table.sCRCData);				//CRC

	return RET_OK;
}

//----------------------------------------------------------------------------
//
//  Function:   ConvertDealData1
//
//  Synopsis:   将硬件数据流转换为现金充值数据
//
//  Arguments:  pPacket   -- 返回金仕达包
//				ucRawData -- 收上的数据
//				nDataLen  -- 收上的数据长度
//  History:    1-11-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
long CInterfaceApp::ConvertDealData1(int nType, TSSmartTask *pTask, TSSmartDoc *pDoc, ST_CPACK *pPacket, unsigned char *ucRawData, int nDataLen)
{
	if( ++(pDoc->m_wFlowNo) > 65535 )
		pDoc->m_wFlowNo=0 ;

	pDoc->m_nDealCount++;
	pDoc->m_nInMoney += 100 ;
	pDoc->m_nOutMoney = pDoc->m_nInMoney + 100 ;

	TSSFTable   table;
	ZeroMemory(&table, sizeof(table));

	printf("==========充值数据==================================\n");

	sprintf(table.sAuthData,"%.2X%.2X%.2X%.2X", ucRawData[0],ucRawData[1],ucRawData[2],ucRawData[3]);
	printf("注册号:%s\n", table.sAuthData);

	sprintf(table.sCommVer,"%d",ucRawData[4]);
	printf("通信版本:%s\n",table.sCommVer);

	//sprintf(table.sWalletcode , "%d", ucRawData[5]);//ucRawData[5]>>4); 
	sprintf(table.sWalletcode , "%d", ucRawData[5]>>4); 
	printf("钱包号:%s\n",table.sWalletcode);

	sprintf(table.sCommStart, "%d",ucRawData[5]&0x0F);
	printf("启动原因:%s\n",table.sCommStart);

	table.nCardID = ucRawData[8]*65536+ucRawData[9]*256+ucRawData[10];
	printf("交易卡号:%d\n", table.nCardID);

    table.nInvoiceNo = ucRawData[6]*256+ucRawData[7];
	printf("交易流水号:%.5d\n", table.nInvoiceNo);

	if( ucRawData[12] >= 100 || ucRawData[13] >= 100 || 
		ucRawData[14] >= 100 || ucRawData[15] >= 100  || ucRawData[16] >= 100 )
	{
		sprintf(table.sDealDateTime, "%02d%02d%02d %02d%02d%02d", 00, 1, 1, 1,1,1);
		printf("交易时间:%02d%02d%02d %02d%02d%02d\n", 00, 1, 1, 1,1,1);
	}
	else
	{
		sprintf(table.sDealDateTime,"%02d%02d%02d %02d%02d%02d",ucRawData[11],ucRawData[12],ucRawData[13],ucRawData[14],ucRawData[15],ucRawData[16]); 
		printf("交易时间:%02d%02d%02d %02d%02d%02d\n",ucRawData[11],ucRawData[12],ucRawData[13],ucRawData[14],ucRawData[15],ucRawData[16]);
	}

	table.nTimes = ucRawData[17]*256+ucRawData[18]; 
	printf("卡片累计使用次数:%d\n",table.nTimes); 

	table.nInMoney = ucRawData[19]+ucRawData[20]*256+ucRawData[21]*65536;
	printf("入卡金额(分):%d\n", table.nInMoney); 

	table.nOutMoney = ucRawData[22]+ucRawData[23]*256+ucRawData[24]*65536;
	printf("出卡金额(分):%d\n",table.nOutMoney);

	sprintf(table.sDealCode1, "%d",ucRawData[25]);
	printf("交易标记:%s\n",table.sDealCode1); 

	sprintf(table.sCRCData,"%.2X%.2X",ucRawData[26],ucRawData[27]);
	printf("CRC:%s\n",table.sCRCData);

	memset(pPacket, 0, sizeof(ST_CPACK));
	pPacket->head.RequestType = 930034; 
	pPacket->head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	pPacket->head.nextflag = 0;    /* 是否后续包请求*/
	pPacket->head.recCount = 1;    /* 本包的记录数*/
	pPacket->head.retCode = 0;     /* 返回代码*/
	pPacket->head.userdata = 0;

	if( nType == 0x26 )
	{
		pPacket->head.RequestType = 930098; 
		SetValue(&pPacket->head,&pPacket->pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
		SetValue(&pPacket->head,&pPacket->pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL1, pTask->szSystemNO);			//消息ID
		SetValue(&pPacket->head,&pPacket->pack, F_VSMESS, "收数成功");	//返回码信息
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL4, "0");	//返回
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL11, GetValue(table.nInvoiceNo));	//交易流水号
	}
	else
	{
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL4, GetValue(table.nInvoiceNo));	//交易流水号
	}

	CString strTime = table.sDealDateTime;
	SetValue(&pPacket->head,&pPacket->pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
	SetValue(&pPacket->head,&pPacket->pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
	SetValue(&pPacket->head,&pPacket->pack, F_SDATE1, table.sAuthData);			//终端设备ID
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL11, GetValue(table.nInvoiceNo));	//交易流水号
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL5, GetValue(table.nCardID));	//交易卡号
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL6, table.sWalletcode);			//钱包号
	SetValue(&pPacket->head,&pPacket->pack, F_SPOST_CODE, strTime.Left(6).GetBuffer(0));		//交易日期
	SetValue(&pPacket->head,&pPacket->pack, F_SPOST_CODE2, strTime.Right(6).GetBuffer(0));		//交易时间
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL7, GetValue(table.nTimes));								//累计使用次数

	char  szSub[256];
	sprintf(szSub, "%d", table.nOutMoney-table.nInMoney);
	printf("本次消费金额:%s....\n", szSub);

	SetValue(&pPacket->head,&pPacket->pack, F_LVOL8, szSub);			//本次消费金额

	SetValue(&pPacket->head,&pPacket->pack, F_LVOL9, GetValue(table.nInMoney));							//入卡金额
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL10, GetValue(table.nOutMoney));							//出卡金额
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL12, table.sDealCode1);						//交易标记

	SetValue(&pPacket->head,&pPacket->pack, F_LBANK_ACC_TYPE, table.sCommVer);						//通信版本
	SetValue(&pPacket->head,&pPacket->pack, F_LBANK_ACC_TYPE2, table.sCommStart);					//启动原因
	SetValue(&pPacket->head,&pPacket->pack, F_SBANK_CODE2, table.sCRCData);				//CRC

	return RET_OK;
}


//----------------------------------------------------------------------------
//
//  Function:   ConvertDealData1
//
//  Synopsis:   将硬件数据流转换为现金充值管理费数据
//
//  Arguments:  pPacket   -- 返回金仕达包
//				ucRawData -- 收上的数据
//				nDataLen  -- 收上的数据长度
//  History:    1-11-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
long CInterfaceApp::ConvertDealData2(int nType, TSSmartTask *pTask, TSSmartDoc *pDoc, ST_CPACK *pPacket, unsigned char *ucRawData, int nDataLen)
{
	TSSFTable   table;
	ZeroMemory(&table, sizeof(table));

	printf("==========充值管理费数据==================================\n");

	sprintf(table.sAuthData,"%.2X%.2X%.2X%.2X", ucRawData[0],ucRawData[1],ucRawData[2],ucRawData[3]);
	printf("注册号:%s\n", table.sAuthData);

	sprintf(table.sCommVer,"%d",ucRawData[4]);
	printf("通信版本:%s\n",table.sCommVer);

	sprintf(table.sCommStart, "%d",ucRawData[5]&0x0F);
	printf("启动原因:%.2X\n",table.sCommStart);

	table.nCardID = ucRawData[8]*65536+ucRawData[9]*256+ucRawData[10];
	printf("交易卡号:%d\n", table.nCardID);

    table.nInvoiceNo = ucRawData[6]*256+ucRawData[7];
	printf("交易流水号:%.5d\n", table.nInvoiceNo);

	if( ucRawData[12] >= 100 || ucRawData[16] >= 100 || 
		ucRawData[13] >= 100 || ucRawData[14] >= 100  || ucRawData[15] >= 100 )
	{
		sprintf(table.sDealDateTime, "%02d%02d%02d %02d%02d%02d", 00, 1, 1, 1,1,1);
		printf("交易时间:%02d%02d%02d %02d%02d%02d \n", 00, 1, 1, 1,1,1);
	}
	else
	{
		sprintf(table.sDealDateTime,"%02d%02d%02d %02d%02d%02d",ucRawData[11],ucRawData[12],ucRawData[13],ucRawData[14],ucRawData[15],ucRawData[16]); 
		printf("交易时间:%02d%02d%02d %02d%02d%02d\n",ucRawData[11],ucRawData[12],ucRawData[13],ucRawData[14],ucRawData[15],ucRawData[16]);
	}

	table.nTimes = ucRawData[17]*256+ucRawData[18]; 
	printf("卡片累计使用次数:%d\n",table.nTimes); 

	table.nInMoney = ucRawData[19]+ucRawData[20]*256+ucRawData[21]*65536;
	printf("入卡金额(分):%d\n",table.nInMoney); 

	table.nOutMoney = ucRawData[22]+ucRawData[23]*256+ucRawData[24]*65536;
	printf("管理费金额(分):%d\n",table.nOutMoney);

	sprintf(table.sDealCode1, "%d",ucRawData[25]);
	printf("交易标记:%s\n",table.sDealCode1); 

	sprintf(table.sCRCData,"%.2X%.2X",ucRawData[26],ucRawData[27]);
	printf("CRC:%s\n",table.sCRCData);

	memset(pPacket, 0, sizeof(ST_CPACK));
	pPacket->head.RequestType = 930036; 
	pPacket->head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	pPacket->head.nextflag = 0;    /* 是否后续包请求*/
	pPacket->head.recCount = 1;    /* 本包的记录数*/
	pPacket->head.retCode = 0;     /* 返回代码*/
	pPacket->head.userdata = 0;

	if( nType == 0x27 )
	{
		pPacket->head.RequestType = 930098; 
		SetValue(&pPacket->head,&pPacket->pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
		SetValue(&pPacket->head,&pPacket->pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL1, pTask->szSystemNO);			//消息ID
		SetValue(&pPacket->head,&pPacket->pack, F_VSMESS, "收数成功");	//返回码信息
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL4, "0");	//返回
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL11, GetValue(table.nInvoiceNo));	//交易流水号
	}
	else
	{
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL4, GetValue(table.nInvoiceNo));	//交易流水号
	}

	CString strTime = table.sDealDateTime;
	SetValue(&pPacket->head,&pPacket->pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
	SetValue(&pPacket->head,&pPacket->pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
	SetValue(&pPacket->head,&pPacket->pack, F_SDATE1, table.sAuthData);			//终端设备ID
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL11, GetValue(table.nInvoiceNo));	//交易流水号
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL5, GetValue(table.nCardID));	//交易卡号
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL6, table.sWalletcode);			//钱包号
	SetValue(&pPacket->head,&pPacket->pack, F_SPOST_CODE, strTime.Left(6).GetBuffer(0));		//交易日期
	SetValue(&pPacket->head,&pPacket->pack, F_SPOST_CODE2, strTime.Right(6).GetBuffer(0));		//交易时间
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL7, GetValue(table.nTimes));					//累计使用次数
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL8, GetValue(table.nOutMoney));			//本次消费金额
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL9, GetValue(table.nInMoney));				//入卡金额
	//delete by lina 20050404SetValue(&pPacket->head,&pPacket->pack, F_LVOL10, GetValue(0));							//出卡金额
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL10, GetValue(table.nInMoney));							//出卡金额

	SetValue(&pPacket->head,&pPacket->pack, F_LVOL12, table.sDealCode1);				//交易标记

	SetValue(&pPacket->head,&pPacket->pack, F_LBANK_ACC_TYPE, table.sCommVer);						//通信版本
	SetValue(&pPacket->head,&pPacket->pack, F_LBANK_ACC_TYPE2, table.sCommStart);					//启动原因
	SetValue(&pPacket->head,&pPacket->pack, F_SBANK_CODE2, table.sCRCData);				//CRC

	return RET_OK;
}


//----------------------------------------------------------------------------
//
//  Function:   ConvertIdentifyData
//
//  Synopsis:   将硬件数据流转换为计时宝数据
//
//  Arguments:  pPacket   -- 返回金仕达包
//				ucRawData -- 收上的数据
//				nDataLen  -- 收上的数据长度
//  History:    1-11-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
long CInterfaceApp::ConvertIdentifyData(int nType, TSSmartTask *pTask, TSSmartDoc *pDoc, ST_CPACK *pPacket, unsigned char *ucRawData, int nDataLen)
{
	TSJSBTable   table;
	ZeroMemory(&table, sizeof(table));

	printf("===========计时宝数据================================\n");

	sprintf(table.sAuthData,"%.2X%.2X%.2X%.2X",ucRawData[0],ucRawData[1],ucRawData[2],ucRawData[3]); 
	printf("注册号:%.2X%.2X%.2X%.2X\n",ucRawData[0],ucRawData[1],ucRawData[2],ucRawData[3]); 

	sprintf(table.sCommVer,"%.2X",ucRawData[4]); 
	printf("通信版本:%.2X\n",ucRawData[4]); 

	sprintf(table.sCommStart,"%.2X",ucRawData[5]); 
	printf("启动原因:%.2X\n",ucRawData[5]); 

	table.nInvoiceNo = ucRawData[6]*256+ucRawData[7];	 
	printf("交易流水号:%.5d\n",ucRawData[6]*256+ucRawData[7]);	 

	table.nICCardCode = ucRawData[22];
	printf("卡型代码:%d\n",ucRawData[22]);

	table.nWorkMode = ucRawData[20];
	printf("工作模式代码:%d\n",ucRawData[20]);

	if( table.nICCardCode == 0 )
	{
		table.nCardID = ucRawData[9]*256*256*256+ucRawData[10]*256*256+ucRawData[11]*256+ucRawData[12];
	}
	else if( table.nICCardCode == 100 )
	{
		if( table.nWorkMode == 4 )
			table.nCardID = 0 ;
		else
			table.nCardID = ucRawData[10]*256*256+ucRawData[11]*256+ucRawData[12];
	}
	else if( table.nICCardCode == 101 )
	{
		table.nCardID = ucRawData[11]*256+ucRawData[12];	 
	}
	else
	{
		table.nCardID = 0 ;
	}

	printf("卡号:%ld\n",table.nCardID);

	if( table.nWorkMode == 4 && (table.nICCardCode == 100 || table.nICCardCode == 101 ) )
	{
		sprintf(table.sShowCardNo,"%.2X%.2X%.2X%.2X%.2X",ucRawData[8],ucRawData[9],ucRawData[10],ucRawData[11],ucRawData[12]); 
		printf("显示卡号:%.2X%.2X%.2X%.2X%.2X\n",ucRawData[8],ucRawData[9],ucRawData[10],ucRawData[11],ucRawData[12]); 
	}

	sprintf(table.sDealDateTime,"%02d%02d%02d %02d%02d%02d",ucRawData[13]+2000,ucRawData[14],ucRawData[15],ucRawData[16],ucRawData[17],ucRawData[18]); 
	printf("交易时间:%02d%02d%02d %02d%02d%02d\n",ucRawData[13]+2000,ucRawData[14],ucRawData[15],ucRawData[16],ucRawData[17],ucRawData[18]); 

	table.nTimerType = ucRawData[21];
	printf("考勤类型:%d\n",ucRawData[21]); 

	table.nICCardCode = ucRawData[22]; 
	printf("卡型代码:%d\n",table.nICCardCode); 

	table.nInductorNo = ucRawData[23]; 
	printf("感应头编号:%d\n",ucRawData[23]); 

	table.nDutyFlag = ucRawData[24]; 
	printf("上下班标记:%d\n",ucRawData[24]); 

	if( table.nDutyFlag > 9 )
		table.nDutyFlag = 0 ;

	sprintf(table.sDealCode1,"%d",ucRawData[25]); 
	printf("交易标记:%s\n",ucRawData[25]); 

	return RET_OK;
}

//----------------------------------------------------------------------------
//
//  Function:   ConvertStateData
//
//  Synopsis:   将硬件数据流转换为扎帐数据
//
//  Arguments:  pPacket   -- 返回金仕达包
//				ucRawData -- 收上的数据
//				nDataLen  -- 收上的数据长度
//  History:    1-11-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
long CInterfaceApp::ConvertStateData(int nType, TSSmartTask *pTask, TSSmartDoc *pDoc, ST_CPACK *pPacket, unsigned char *ucRawData, int nDataLen)
{
	if( ++(pDoc->m_nBeginInvoice) > 65535 )
		pDoc->m_nBeginInvoice=0 ;

	TSSetlment   m_SRC;
	ZeroMemory(&m_SRC, sizeof(m_SRC));

	printf("==============扎帐数据===================================\n");

	sprintf(m_SRC.sMachineID, "%.2X%.2X%.2X%.2X",ucRawData[0],ucRawData[1],ucRawData[2],ucRawData[3]); 
	printf("注册号:%s\n",m_SRC.sMachineID); 

	m_SRC.nSettleInvoice = ucRawData[6]*256+ucRawData[7]; 
	printf("扎帐流水:%d\n",  m_SRC.nSettleInvoice); 

	if( !IsValidDateTime(&ucRawData[8])  )
	{
		GetCurDateTimeEx(m_SRC.sSettleTime);
	}
	else
	{
		sprintf(m_SRC.sSettleTime, "%02d%02d%02d %02d%02d%02d",ucRawData[8],ucRawData[9],ucRawData[10],ucRawData[11],ucRawData[12],ucRawData[13]);  
	}

	printf("扎帐时间:%s\n", m_SRC.sSettleTime);

	m_SRC.nBeginInvoice = ucRawData[14]*256+ucRawData[15]; 
	printf("起始流水号:%d\n",m_SRC.nBeginInvoice); 

	m_SRC.nEndInvoice = ucRawData[16]*256+ucRawData[17]; 
	printf("结束流水号:%d\n",m_SRC.nEndInvoice); 

	m_SRC.nDealCount = ucRawData[18]*256+ucRawData[19]; 
	printf("正常消费总笔数:%d\n",m_SRC.nDealCount); 

	m_SRC.nDealAmount = ucRawData[20]+ucRawData[21]*256+ucRawData[22]*65536; 
	printf("正常消费总金额:%d\n",m_SRC.nDealAmount); 

	m_SRC.nCancelCount = ucRawData[23]*256+ucRawData[24];
	printf("冲正消费总笔数:%d\n",m_SRC.nCancelCount);  

	m_SRC.nCancelAmount = ucRawData[25]+ucRawData[26]*256+ucRawData[27]*65536 ;
	printf("冲正消费总金额:%d\n",m_SRC.nCancelAmount); 

	m_SRC.nExcepCount = ucRawData[28]*256+ucRawData[29] ;
	printf("异常消费总笔数%d\n",m_SRC.nExcepCount); 

	m_SRC.nExcepACount = ucRawData[30]+ucRawData[31]*256+ucRawData[32]*65536 ;
	printf("异常消费总金额%d\n",m_SRC.nExcepACount); 

	m_SRC.nOtherCount = ucRawData[33]*256+ucRawData[34] ;
	printf("其他交易总笔数:%d\n",m_SRC.nOtherCount); 

	m_SRC.nOuterkeeper = ucRawData[35];
	printf("扎帐标记:%.2X\n",m_SRC.nOuterkeeper);

	sprintf(m_SRC.sCRCData,"%.2X%.2X",ucRawData[36],ucRawData[37]);
	printf("CRC:%s\n",m_SRC.sCRCData);

	memset(pPacket, 0, sizeof(ST_CPACK));
	pPacket->head.RequestType = 930035; 
	pPacket->head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	pPacket->head.nextflag = 0;    /* 是否后续包请求*/
	pPacket->head.recCount = 1;    /* 本包的记录数*/
	pPacket->head.retCode = 0;     /* 返回代码*/
	pPacket->head.userdata = 0;

	if( nType == 0x23 )
	{
		pPacket->head.RequestType = 930098; 
		SetValue(&pPacket->head,&pPacket->pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
		SetValue(&pPacket->head,&pPacket->pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL1, pTask->szSystemNO);			//消息ID
		SetValue(&pPacket->head,&pPacket->pack, F_VSMESS, "收数成功");	//返回码信息
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL4, GetValue(RET_OK));	//返回
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL3, GetValue(m_SRC.nDealAmount));	//正常消费总金额

		SetValue(&pPacket->head,&pPacket->pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
		SetValue(&pPacket->head,&pPacket->pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
		SetValue(&pPacket->head,&pPacket->pack, F_SDATE1, m_SRC.sMachineID);			//终端设备ID
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL0, GetValue(m_SRC.nSettleInvoice));	//结帐流水号
		SetValue(&pPacket->head,&pPacket->pack, F_SSERIAL0, m_SRC.sSettleTime);			//结帐终止时间
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL11, GetValue(m_SRC.nBeginInvoice));	//起始终端交易流水号
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL12, GetValue(m_SRC.nEndInvoice));	//结束终端交易流水号
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL2, GetValue(m_SRC.nDealCount));		//正常消费总笔数
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL5, GetValue(m_SRC.nCancelCount));	//冲正消费总笔数
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL6, GetValue(m_SRC.nCancelAmount));	//冲正消费总金额
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL7, GetValue(m_SRC.nExcepCount));	//异常消费总笔数
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL8, GetValue(m_SRC.nExcepACount));	//异常消费总金额
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL9, GetValue(m_SRC.nOtherCount));	//其他记录总笔数
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL10, GetValue(m_SRC.nOuterkeeper));	//结帐标志
	//	SetValue(&pPacket->head,&pPacket->pack, F_LVOL12, GetValue(ucRawData[35]));			//交易标记
	}
	else
	{
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL4, GetValue(m_SRC.nDealAmount));	//正常消费总金额
		SetValue(&pPacket->head,&pPacket->pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
		SetValue(&pPacket->head,&pPacket->pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
		SetValue(&pPacket->head,&pPacket->pack, F_SDATE1, m_SRC.sMachineID);			//终端设备ID
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL0, GetValue(m_SRC.nSettleInvoice));	//结帐流水号
		SetValue(&pPacket->head,&pPacket->pack, F_SSERIAL0, m_SRC.sSettleTime);			//结帐终止时间
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL1, GetValue(m_SRC.nBeginInvoice));	//起始终端交易流水号
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL2, GetValue(m_SRC.nEndInvoice));	//结束终端交易流水号
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL3, GetValue(m_SRC.nDealCount));		//正常消费总笔数
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL5, GetValue(m_SRC.nCancelCount));	//冲正消费总笔数
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL6, GetValue(m_SRC.nCancelAmount));	//冲正消费总金额
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL7, GetValue(m_SRC.nExcepCount));	//异常消费总笔数
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL8, GetValue(m_SRC.nExcepACount));	//异常消费总金额
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL9, GetValue(m_SRC.nOtherCount));	//其他记录总笔数
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL10, GetValue(m_SRC.nOuterkeeper));	//结帐标志
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL12, GetValue(ucRawData[35]));			//交易标记
	}

	SetValue(&pPacket->head,&pPacket->pack, F_LBANK_ACC_TYPE, "0");						//通信版本
	SetValue(&pPacket->head,&pPacket->pack, F_LBANK_ACC_TYPE2, "0");					//启动原因
	SetValue(&pPacket->head,&pPacket->pack, F_SBANK_CODE2, m_SRC.sCRCData);				//CRC

	return RET_OK;
}

//----------------------------------------------------------------------------
//
//  Function:   ConvertAssisData
//
//  Synopsis:   将硬件数据流转换为补助数据
//
//  Arguments:  pPacket   -- 返回金仕达包
//				ucRawData -- 收上的数据
//				nDataLen  -- 收上的数据长度
//  History:    1-11-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
long CInterfaceApp::ConvertAssisData(int nType, TSSmartTask *pTask, TSSmartDoc *pDoc, ST_CPACK *pPacket, unsigned char *ucRawData, int nDataLen)
{
	if( ++(pDoc->m_wStateNo) > 65535 )
		pDoc->m_wStateNo=0 ;

	pDoc->m_nDealCount++;
	pDoc->m_nInMoney -= 1 ;
	pDoc->m_nOutMoney  = pDoc->m_nInMoney+1;

	TSBZTable  table;
	ZeroMemory(&table, sizeof(table));

	printf("============补助数据=====================================\n");

	sprintf(table.sAuthData,"%.2X%.2X%.2X%.2X",ucRawData[0],ucRawData[1],ucRawData[2],ucRawData[3]);
	printf("注册号:%s", table.sAuthData);

	sprintf(table.sCommVer, "%d",ucRawData[4]);
	printf("通信版本:%s\n", table.sCommVer);

	sprintf(table.sCommStart, "%d",ucRawData[5]);
	printf("启动原因:%s\n", table.sCommStart);

	sprintf(table.sMachineCode, "%.2X%.2X",ucRawData[6],ucRawData[7]);
	printf("机型代码:%s\n", table.sMachineCode);

	table.nInvoiceNo = ucRawData[8]*256+ucRawData[9];
	printf("补助流水号:%d\n",table.nInvoiceNo);

	sprintf(table.sDealDateTime, "%02d%02d%02d %02d%02d%02d",ucRawData[10]+2000,ucRawData[11],ucRawData[12],ucRawData[13],ucRawData[14],ucRawData[15]);
	printf("补助时间:%s\n", table.sDealDateTime);

	sprintf(table.sUseCard, "%.2X",ucRawData[16]);
	printf("卡型代码:%s\n",table.sUseCard);

	table.nICCardCode = ucRawData[17];
	printf("卡类:%d\n", table.nICCardCode);

	sprintf(table.sShowCardNo, "%.2X%.2X%.2X%.2X%.2X",ucRawData[18],ucRawData[19],ucRawData[20],ucRawData[21],ucRawData[22]);
	printf("显示卡号:%s\n", table.sShowCardNo);

	table.nCardID = ucRawData[23]*256*256+ucRawData[24]*256+ucRawData[25];
	printf("交易卡号:%d\n",table.nCardID);

	table.nTimes = ucRawData[26]*256+ucRawData[27];
	printf("卡片使用流水:%d\n", table.nTimes);

	sprintf(table.sWalletCode, "%d",ucRawData[28] & 0x0F);
	printf("钱包属性代码:%s\n", table.sWalletCode);

	int flag = ucRawData[28] & 0xF0;

	table.nInMoney = ucRawData[29]+ucRawData[30]*256;

	table.nOutMoney = ucRawData[31]+ucRawData[32]*256+ucRawData[33]*65536;
	printf("钱包余额:%d\n", table.nOutMoney);

	if( !flag )
	{
		table.nInMoney = table.nInMoney + table.nOutMoney;
	}
	else
	{
		table.nInMoney = table.nOutMoney - table.nInMoney;
	}

	printf("补助金额:%d\n",table.nInMoney);

	table.nBatch = ucRawData[34]*256+ucRawData[35];
	printf("补助批次号:%d\n", table.nBatch);

	sprintf(table.sDealCode1, "%d",ucRawData[36]);
	printf("交易标记:%s\n",table.sDealCode1);

	sprintf(table.sCRCData,"%.2X%.2X",ucRawData[37],ucRawData[38]);
	printf("CRC:%s\n",table.sCRCData);

	memset(pPacket, 0, sizeof(ST_CPACK));
	pPacket->head.RequestType = 930033; 
	pPacket->head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	pPacket->head.nextflag = 0;    /* 是否后续包请求*/
	pPacket->head.recCount = 1;    /* 本包的记录数*/
	pPacket->head.retCode = 0;     /* 返回代码*/
	pPacket->head.userdata = 0;

	CString strTime = table.sDealDateTime;
	SetValue(&pPacket->head,&pPacket->pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
	SetValue(&pPacket->head,&pPacket->pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
	SetValue(&pPacket->head,&pPacket->pack, F_SDATE1, table.sAuthData);			//终端设备ID
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL11, GetValue(table.nInvoiceNo));	//交易流水号
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL5, GetValue(table.nCardID));	//交易卡号
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL6, GetValue(table.nBatch));			//钱包号
	SetValue(&pPacket->head,&pPacket->pack, F_SPOST_CODE, strTime.Left(6).GetBuffer(0));		//交易日期
	SetValue(&pPacket->head,&pPacket->pack, F_SPOST_CODE2, strTime.Right(6).GetBuffer(0));		//交易时间
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL7, GetValue(table.nTimes));								//累计使用次数
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL8, GetValue(table.nOutMoney-table.nInMoney));			//补助金额
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL9, GetValue(table.nInMoney));							//入卡金额
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL10, GetValue(table.nOutMoney));							//出卡金额
	SetValue(&pPacket->head,&pPacket->pack, F_LVOL12, table.sDealCode1);						//交易标记

	SetValue(&pPacket->head,&pPacket->pack, F_LBANK_ACC_TYPE, table.sCommVer);						//通信版本
	SetValue(&pPacket->head,&pPacket->pack, F_LBANK_ACC_TYPE2, table.sCommStart);					//启动原因
	SetValue(&pPacket->head,&pPacket->pack, F_SBANK_CODE2, table.sCRCData);				//CRC

	if( nType == 0x24 )
	{
		pPacket->head.RequestType = 930098; 
		SetValue(&pPacket->head,&pPacket->pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
		SetValue(&pPacket->head,&pPacket->pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL1, pTask->szSystemNO);			//消息ID
		SetValue(&pPacket->head,&pPacket->pack, F_VSMESS, "收数成功");	//返回码信息
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL4, "0");	//返回
	}

	return RET_OK;
}

//----------------------------------------------------------------------------
//
//  Function:   ConvertGSJData
//
//  Synopsis:   将硬件数据流转换为挂失机数据
//
//  Arguments:  pPacket   -- 返回金仕达包
//				ucRawData -- 收上的数据
//				nDataLen  -- 收上的数据长度
//  History:    1-11-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
long CInterfaceApp::ConvertGSJData(int nType, TSSmartTask *pTask, TSSmartDoc *pDoc, ST_CPACK *pPacket, unsigned char *ucRawData, int nDataLen)
{
	TSGSJRecord *pRecord = (TSGSJRecord*)ucRawData;
	

	

	memset(pPacket, 0, sizeof(ST_CPACK));

	pPacket->head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	pPacket->head.nextflag = 0;    /* 是否后续包请求*/
	pPacket->head.recCount = 1;    /* 本包的记录数*/
	pPacket->head.retCode = 0;     /* 返回代码*/
	pPacket->head.userdata = 0;

	SetValue(&pPacket->head,&pPacket->pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
	SetValue(&pPacket->head,&pPacket->pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
	SetValue(&pPacket->head,&pPacket->pack, F_SDATE1, pDoc->m_szDeviceID);		//终端设备ID

	if( nType == 0x81 )
	{
		WriteLog("申请挂失");
		WriteLog("卡号%s",pRecord->szShowID);
		WriteLog("密码%s",pRecord->szPassword);
		
		pPacket->head.RequestType = 930040; 
		SetValue(&pPacket->head,&pPacket->pack, F_SSTATION0, pRecord->szShowID);			//学工号
		SetValue(&pPacket->head,&pPacket->pack, F_SSTOCK_CODE, pRecord->szPassword);		//卡密码
		SetValue(&pPacket->head,&pPacket->pack, F_SBANK_CODE2, pRecord->szCRC);				//CRC
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL4, GetValue(pRecord->nCardID));					//交易流水号
		SetValue(&pPacket->head,&pPacket->pack, F_LBANK_ACC_TYPE, pRecord->szSerial);		//通信版本号
	}
	else if( nType == 0x82 )
	{
		pPacket->head.RequestType = 930041; 
		SetValue(&pPacket->head,&pPacket->pack, F_SSTATION0, pRecord->szShowID);			//学工号
		SetValue(&pPacket->head,&pPacket->pack, F_SSTOCK_CODE, pRecord->szPassword);		//卡密码
		SetValue(&pPacket->head,&pPacket->pack, F_SBANK_CODE2, pRecord->szCRC);				//CRC
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL4, GetValue(pRecord->nCardID));					//交易流水号
		SetValue(&pPacket->head,&pPacket->pack, F_LBANK_ACC_TYPE, pRecord->szSerial);		//通信版本号
	}
	else 
	{
		WriteLog("修改密码----------");
		WriteLog("显示卡号%s",pRecord->szShowID);
		WriteLog("交易卡号%d",pRecord->nCardID); 
		WriteLog("密码%s",pRecord->szPassword);
		pPacket->head.RequestType = 930045; 

		SetValue(&pPacket->head,&pPacket->pack, F_SSTOCK_CODE2, pRecord->szPassword);
		SetValue(&pPacket->head,&pPacket->pack, F_LBANK_ACC_TYPE, "00");			//通信版本
		SetValue(&pPacket->head,&pPacket->pack, F_LBANK_ACC_TYPE2, "00");			//启动原因
		SetValue(&pPacket->head,&pPacket->pack, F_SBANK_CODE2, pRecord->szCRC);		//CRC
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL12, "153");					//交易标记

		SetValue(&pPacket->head,&pPacket->pack, F_LVOL4, "0");							//交易流水号
		SetValue(&pPacket->head,&pPacket->pack, F_LVOL5, GetValue(pRecord->nCardID));	//交易卡号
		SetValue(&pPacket->head,&pPacket->pack, F_SPOST_CODE, "");						//交易日期
		SetValue(&pPacket->head,&pPacket->pack, F_SPOST_CODE2, "");						//交易时间
		SetValue(&pPacket->head,&pPacket->pack, F_SSTATION0, pRecord->szShowID);		//显示卡号
	}

	return RET_OK;
}

//将long型转为字符串, 适用于多线程环境
char* GetValue(long iValue)
{
	static CLock lock;
	static char strString[64];

	lock.Lock();
	sprintf(strString,"%d", iValue);
	lock.Unlock();

	return strString;
}

//设置CPacket包的值, 源文件来自于金仕达提供的DEMO
void SetValue(ST_PACKHEAD *head, ST_PACK *pack,int ids,char *data)
{
	long lv;
	double dv;
#define PC(a)	{ SetParmBit(head,ids);strncpy((char *)pack->a,data,sizeof(pack->a));pack->a[sizeof(pack->a)-1]=0;}//CHAR[]
#define PI(a)	{ SetParmBit(head,ids);lv=atol(data);memcpy(&pack->a,&lv,sizeof(pack->a));}//INT
#define PD(a)	{ SetParmBit(head,ids);dv=atof(data);memcpy(&pack->a,&dv,sizeof(pack->a));}//LONG

	switch(ids)
	{                                
	case F_SCUST_NO:                     
		PC(scust_no);                
		break;                       
	case F_SCUST_NO2:                    
		PC(scust_no2);               
		break;                       
	case F_SHOLDER_AC_NO:                
		PC(sholder_ac_no);           
		break;                       
	case F_SHOLDER_AC_NO2:               
		PC(sholder_ac_no2);          
		break;                       
	case F_SHOLDER_TYPE:                 
		PC(sholder_type);            
		break;                       
	case F_SHOLDER_TYPE2:                
		PC(sholder_type2);           
		break;                       
	case F_SNAME:                        
		PC(sname);                   
		break;                       
	case F_SNAME2:                       
		PC(sname2);                  
		break;                       
	case F_SALL_NAME:                    
		PC(sall_name);               
		break;                       
	case F_SMARKET_CODE:                 
		PC(smarket_code);            
		break;                       
	case F_SMARKET_CODE2:                
		PC(smarket_code2);           
		break;                       
	case F_SDATE0:                       
		PC(sdate0);                  
		break;                       
	case F_SDATE1:                       
		PC(sdate1);                  
		break;                       
	case F_SDATE2:                       
		PC(sdate2);                  
		break;                       
	case F_SDATE3:                       
		PC(sdate3);                  
		break;                       
	case F_STIME0:                       
		PC(stime0);                  
		break;                       
	case F_STIME1:                       
		PC(stime1);                  
		break;                       
	case F_STIME2:                       
		PC(stime2);                  
		break;                       
	case F_STIME3:                       
		PC(stime3);                  
		break;                       
	case F_LVOL0:                        
		PI(lvol0);                   
		break;                       
	case F_LVOL1:                        
		PI(lvol1);                   
		break;                       
	case F_LVOL2:                        
		PI(lvol2);                   
		break;                       
	case F_LVOL3:                        
		PI(lvol3);                   
		break;                       
	case F_LVOL4:                        
		PI(lvol4);                   
		break;                       
	case F_LVOL5:                        
		PI(lvol5);                   
		break;                       
	case F_LVOL6:                        
		PI(lvol6);                   
		break;                       
	case F_LVOL7:                        
		PI(lvol7);                   
		break;                       
	case F_LVOL8:                        
		PI(lvol8);                   
		break;                       
	case F_LVOL9:                        
		PI(lvol9);                   
		break;                       
	case F_LVOL10:                       
		PI(lvol10);                  
		break;                       
	case F_LVOL11:                       
		PI(lvol11);                  
		break;                       
	case F_LVOL12:                       
		PI(lvol12);                  
		break;                       
	case F_DAMT0:                        
		PD(damt0);                   
		break;                       
	case F_DAMT1:                        
		PD(damt1);                   
		break;                       
	case F_DAMT2:                        
		PD(damt2);                   
		break;                       
	case F_DAMT3:                        
		PD(damt3);                   
		break;                       
	case F_DAMT4:                        
		PD(damt4);                   
		break;                       
	case F_DAMT5:                        
		PD(damt5);                   
		break;                       
	case F_DAMT6:                        
		PD(damt6);                   
		break;                       
	case F_DAMT7:                        
		PD(damt7);                   
		break;                       
	case F_DAMT8:                        
		PD(damt8);                   
		break;                       
	case F_DAMT9:                        
		PD(damt9);                   
		break;                       
	case F_DAMT10:                       
		PD(damt10);                  
		break;                       
	case F_DAMT11:                       
		PD(damt11);                  
		break;                       
	case F_DAMT12:                       
		PD(damt12);                  
		break;                       
	case F_DAMT13:                       
		PD(damt13);                  
		break;                       
	case F_DAMT14:                       
		PD(damt14);                  
		break;                       
	case F_DAMT15:                       
		PD(damt15);                  
		break;                       
	case F_DAMT16:                       
		PD(damt16);                  
		break;                       
	case F_DAMT17:                       
		PD(damt17);                  
		break;                       
	case F_DAMT18:                       
		PD(damt18);                  
		break;                       
	case F_DAMT19:                       
		PD(damt19);                  
		break;                       
	case F_DAMT20:                       
		PD(damt20);                  
		break;                       
	case F_DAMT21:                       
		PD(damt21);                  
		break;                       
	case F_DAMT22:                       
		PD(damt22);                  
		break;                       
	case F_DAMT23:                       
		PD(damt23);                  
		break;                       
	case F_DAMT24:                       
		PD(damt24);                  
		break;                       
	case F_DAMT25:                       
		PD(damt25);                  
		break;                       
	case F_DAMT26:                       
		PD(damt26);                  
		break;                       
	case F_DAMT27:                       
		PD(damt27);                  
		break;                       
	case F_DAMT28:                       
		PD(damt28);                  
		break;                       
	case F_DAMT29:                       
		PD(damt29);                  
		break;                       
	case F_DAMT30:                       
		PD(damt30);                  
		break;                       
	case F_DAMT31:                       
		PD(damt31);                  
		break;                       
	case F_DAMT32:                       
		PD(damt32);                  
		break;                       
	case F_DAMT33:                       
		PD(damt33);                  
		break;                       
	case F_SSTOCK_CODE:                  
		PC(sstock_code);             
		break;                       
	case F_SSTOCK_CODE2:                 
		PC(sstock_code2);            
		break;                       
	case F_SCUST_TYPE:                   
		PC(scust_type);              
		break;                       
	case F_SCUST_TYPE2:                  
		PC(scust_type2);             
		break;                       
	case F_SSTAT_TYPE:                   
		PC(sstat_type);              
		break;                       
	case F_SSTAT_TYPE2:                  
		PC(sstat_type2);             
		break;                       
	case F_SROOM_NO:                     
		PC(sroom_no);                
		break;                       
	case F_SROOM_NO2:                    
		PC(sroom_no2);               
		break;                       
	case F_SOPEN_EMP:                    
		PC(sopen_emp);               
		break;                       
	case F_SCLOSE_EMP:                   
		PC(sclose_emp);              
		break;                       
	case F_SCHANGE_EMP:                  
		PC(schange_emp);             
		break;                       
	case F_SCHECK_EMP:                   
		PC(scheck_emp);              
		break;                       
	case F_SEMP:                         
		PC(semp);                    
		break;                       
	case F_SNATION_CODE:                 
		PC(snation_code);            
		break;                       
	case F_LCERT_CODE:                   
		PI(lcert_code);              
		break;                       
	case F_STX_PWD:                      
		PC(stx_pwd);                 
		break;                       
	case F_STX_PWD2:                     
		PC(stx_pwd2);                
		break;                       
	case F_SWITHDRAW_PWD:                
		PC(swithdraw_pwd);           
		break;                       
	case F_SWITHDRAW_PWD2:               
		PC(swithdraw_pwd2);          
		break;                       
	case F_SEMP_PWD:                     
		PC(semp_pwd);                
		break;                       
	case F_SEMP_PWD2:                    
		PC(semp_pwd2);               
		break;                       
	case F_SBANK_PWD:                    
		PC(sbank_pwd);               
		break;                       
	case F_SBANK_PWD2:                   
		PC(sbank_pwd2);              
		break;                       
	case F_SCUST_AUTH:                   
		PC(scust_auth);              
		break;                       
	case F_SCUST_AUTH2:                  
		PC(scust_auth2);             
		break;                       
	case F_SCUST_LIMIT:                  
		PC(scust_limit);             
		break;                       
	case F_SCUST_LIMIT2:                 
		PC(scust_limit2);            
		break;                       
	case F_LSAFE_LEVEL:                  
		PI(lsafe_level);             
		break;                       
	case F_LSAFE_LEVEL2:                 
		PI(lsafe_level2);            
		break;                       
	case F_SPOST_CODE:                   
		PC(spost_code);              
		break;                       
	case F_SPOST_CODE2:                  
		PC(spost_code2);             
		break;                       
	case F_SPHONE:                       
		PC(sphone);                  
		break;                       
	case F_SPHONE2:                      
		PC(sphone2);                 
		break;                       
	case F_SPHONE3:                      
		PC(sphone3);                 
		break;                       
	case F_SPAGER:                       
		PC(spager);                  
		break;                       
	case F_SEMAIL:                       
		PC(semail);                  
		break;                       
	case F_SEMAIL2:                      
		PC(semail2);                 
		break;                       
	case F_SNOTE:                        
		PC(snote);                   
		break;                       
	case F_SNOTE2:                       
		PC(snote2);                  
		break;                       
	case F_SCERT_NO:                     
		PC(scert_no);                
		break;                       
	case F_SCERT_NO2:                    
		PC(scert_no2);               
		break;                       
	case F_SCERT_ADDR:                   
		PC(scert_addr);              
		break;                       
	case F_SSTATUS0:                     
		PC(sstatus0);                
		break;                       
	case F_SSTATUS1:                     
		PC(sstatus1);                
		break;                       
	case F_SSTATUS2:                     
		PC(sstatus2);                
		break;                       
	case F_SSTATUS3:                     
		PC(sstatus3);                
		break;                       
	case F_SSTATUS4:                     
		PC(sstatus4);                
		break;                       
	case F_LWITHDRAW_FLAG:               
		PI(lwithdraw_flag);          
		break;                       
	case F_SADDR:                        
		PC(saddr);                   
		break;                       
	case F_SADDR2:                       
		PC(saddr2);                  
		break;                       
	case F_SSERIAL0:                     
		PC(sserial0);                
		break;                       
	case F_SSERIAL1:                     
		PC(sserial1);                
		break;                       
	case F_SSERIAL2:                     
		PC(sserial2);                
		break;                       
	case F_SSERIAL3:                     
		PC(sserial3);                
		break;                       
	case F_SSERIAL4:                     
		PC(sserial4);                
		break;                       
	case F_SCURRENCY_TYPE:               
		PC(scurrency_type);          
		break;                       
	case F_SCURRENCY_TYPE2:              
		PC(scurrency_type2);         
		break;                       
	case F_SBRANCH_CODE0:                
		PC(sbranch_code0);           
		break;                       
	case F_SBRANCH_CODE1:                
		PC(sbranch_code1);           
		break;                       
	case F_SBRANCH_CODE2:                
		PC(sbranch_code2);           
		break;                       
	case F_USSET0:                       
		PC(usset0);                  
		break;                       
	case F_USSET1:                       
		PC(usset1);                  
		break;                       
	case F_USSET2:                       
		PC(usset2);                  
		break;                       
	case F_USSET3:                       
		PC(usset3);                  
		break;                       
	case F_USSET4:                       
		PC(usset4);                  
		break;                       
	case F_USSET5:                       
		PC(usset5);                  
		break;                       
	case F_USSET6:                       
		PC(usset6);                  
		break;                       
	case F_SSTATION0:                    
		PC(sstation0);               
		break;                       
	case F_SSTATION1:                    
		PC(sstation1);               
		break;                       
	case F_SBANK_ACC:                    
		PC(sbank_acc);               
		break;                       
	case F_SBANK_ACC2:                   
		PC(sbank_acc2);              
		break;                       
	case F_LBANK_ACC_TYPE:               
		PI(lbank_acc_type);          
		break;                       
	case F_LBANK_ACC_TYPE2:              
		PI(lbank_acc_type2);         
		break;                       
	case F_SMAIN_FLAG:                   
		PC(smain_flag);              
		break;                       
	case F_SMAIN_FLAG2:                  
		PC(smain_flag2);             
		break;                       
	case F_SBANK_CODE:                   
		PC(sbank_code);              
		break;                       
	case F_SBANK_CODE2:                  
		PC(sbank_code2);             
		break;                       
	case F_SEMP_NO:                      
		PC(semp_no);                 
		break;                       
	case F_SEMP_NO2:                     
		PC(semp_no2);                
		break;                       
	case F_DRATE0:                       
		PD(drate0);                  
		break;                       
	case F_DRATE1:                       
		PD(drate1);                  
		break;                       
	case F_LSERIAL0:                     
		PI(lserial0);                
		break;                       
	case F_LSERIAL1:                     
		PI(lserial1);                
		break;                       
	case F_SBANKNAME:                    
		PC(sbankname);               
		break;                       
	case F_SBANKNAME2:                   
		PC(sbankname2);              
		break;                       
	case F_SCARD0:                       
		PC(scard0);                  
		break;                       
	case F_SCARD1:                       
		PC(scard1);                  
		break;                       
	case F_SORDER0:                      
		PC(sorder0);                 
		break;                       
	case F_SORDER1:                      
		PC(sorder1);                 
		break;                       
	case F_SORDER2:                      
		PC(sorder2);                 
		break;                       
	case F_VSMESS:                       
		PC(vsmess);                  
		break;       
	case F_SCUSTTYPES:
		PC(scusttypes);
		break;
	case F_SSECTYPES:
		PC(ssectypes);
		break;
	case F_VSVARSTR0:                    
		PC(vsvarstr0);               
		break;                       
	case F_VSVARSTR1:                    
		PC(vsvarstr1);               
		break;                       
	case F_VSVARSTR2:                    
		PC(vsvarstr2);               
		break;                       
	case F_VSVARSTR3:                    
		PC(vsvarstr3);               
		break;                       
	default:
		printf("not existed parameter=%d....\n",ids);
		break;
	}                                
}  

//是否为非法的日期时间(仅简单判断)
bool IsValidDateTime(unsigned char *pDateTime)
{
	if( pDateTime[0] > 100 || 
	   pDateTime[1] >= 13 || pDateTime[1] == 0 || 
	   pDateTime[2] > 31 || pDateTime[2] == 0 ||
	   pDateTime[3] > 24 || 
	   pDateTime[4] > 60 ||  
	   pDateTime[5] > 60 )
	{
		return false;
	}
	return true;
}

//得到当前时间YYYY-MM-DD HH:MI24:SS
void GetCurDateTime(char *pszDateTime)
{
	SYSTEMTIME  SysTime;

	GetLocalTime(&SysTime);

	sprintf(pszDateTime, "%04d-%02d-%02d %02d:%02d:%02d", 
		SysTime.wYear,  SysTime.wMonth, SysTime.wDay,
		SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
}

//得到当前时间
void GetCurDateTimeEx(char *pszDateTime)
{
	SYSTEMTIME  SysTime;

	GetLocalTime(&SysTime);

	sprintf(pszDateTime, "%02d%02d%02d %02d%02d%02d", 
		SysTime.wYear-2000,  SysTime.wMonth, SysTime.wDay,
		SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
}

//得到16进制的数值
long GetHexNumber(char cChar)
{
	if( cChar >= '0' && cChar <= '9' )
		return (cChar - '0');
	else if( cChar >= 'a' && cChar <= 'f' )
		return (cChar-'a') + 10 ;
	else if( cChar >= 'A' && cChar <= 'F' )
		return (cChar-'A') + 10 ;
	else
		return 0;
}

//将16进制的数值转成一个10进制的long
long ConvertID(char *pHexID)
{
	long nValue = 0 ;
	long len = strlen(pHexID);
	for(int i=0; i<len; i++)
	{
		nValue = nValue + (GetHexNumber(pHexID[i]) * (long)pow(16, len-i-1));
	}
	return nValue;
}

//调试打印语句(类似于printf)
void DebugPrint(const char *format, ...)
{
	TCHAR sBuffer[1024];
	va_list ptr;

	va_start(ptr, format);
	wvsprintf(sBuffer, format, ptr);
	va_end(ptr);

	printf("%s", sBuffer);
}

//写心跳日志
void WriteTickLog(const char *format, ...)
{
	TCHAR  szFile[MAX_PATH];
	TCHAR sBuffer[1024],szTemp[MAX_PATH];
	va_list ptr;
	SYSTEMTIME  SysTime;

	va_start(ptr, format);
	wvsprintf(sBuffer, format, ptr);
	va_end(ptr);

	GetLocalTime(&SysTime);
	wsprintf(szTemp,"%s\\%04d%02d%2d",szLogPath,SysTime.wYear,SysTime.wMonth,
		SysTime.wDay);
	CreateDirectory(szTemp,NULL);
	
	sprintf(szFile, "%s\\心跳日志.log", szTemp);

	FILE *fp = NULL ;
	if( (fp=fopen(szFile, "a+")) != NULL )
	{
		TCHAR  szLog[512];

		wsprintf(szLog, "%04d-%02d-%02d %02d:%02d:%02d:%04d %s", 
			SysTime.wYear,  SysTime.wMonth, SysTime.wDay,
			SysTime.wHour, SysTime.wMinute, SysTime.wSecond, SysTime.wMilliseconds, 
			sBuffer);

		fwrite(szLog, lstrlen(szLog), sizeof(TCHAR), fp);
		fclose(fp);
	}
}

//写日志(每天一个日志文件)
void WriteLog(const char *format, ...)
{
    TCHAR  szFile[MAX_PATH];
	TCHAR sBuffer[1024],szTemp[MAX_PATH];
	va_list ptr;
	SYSTEMTIME  SysTime;

	va_start(ptr, format);
	wvsprintf(sBuffer, format, ptr);
	va_end(ptr);

	if( strlen(sBuffer) < 128 )
		printf("%s", sBuffer);

	GetLocalTime(&SysTime);
	wsprintf(szTemp,"%s\\%04d%02d%2d",szLogPath,SysTime.wYear,SysTime.wMonth,
		SysTime.wDay);
	CreateDirectory(szTemp,NULL);
	sprintf(szFile, "%s\\接口日志.log", szTemp);

	FILE *fp = NULL ;
	if( (fp=fopen(szFile, "a+")) != NULL )
	{
		TCHAR  szLog[512];

		wsprintf(szLog, "%04d-%02d-%02d %02d:%02d:%02d %s", 
			SysTime.wYear,  SysTime.wMonth, SysTime.wDay,
			SysTime.wHour, SysTime.wMinute, SysTime.wSecond,sBuffer);

		fwrite(szLog, lstrlen(szLog), sizeof(TCHAR), fp);
		fclose(fp);
	}
}

//退出CWinApp
int CInterfaceApp::ExitInstance() 
{
	//删除内存中保存的设备信息
	if( gpSmartDoc != NULL )
	{
		delete [] gpSmartDoc;
		gpSmartDoc = NULL ;
	}
	gnDocCount = 0 ;

	return CWinApp::ExitInstance();
}

bool CInterfaceApp::FillSmartDoc(ST_PACK *pPacket)
{
	TSSmartDoc  doc;
	
	ZeroMemory(&doc, sizeof(doc));
	printf("------接收设备ID=%s--------\n",pPacket->sdate0);
	printf("------接收设备机号=%d------\n",pPacket->lvol4);
	printf("------接收地址=%s----------\n",pPacket->sstation0);
	printf("------接收机型代码=%s------\n",pPacket->semp);
	printf("------接收设备注册号=%s-----\n",pPacket->sdate2);
	strcpy(doc.m_szMacCode, pPacket->semp);			//终端机型号代码
	strcpy(doc.m_szMacCard, pPacket->sholder_type);//可用IC卡卡型
	strcpy(doc.m_szMacModle, pPacket->semp);		//终端机型号
	strcpy(doc.m_szMacType, pPacket->semp);			//机型名称
	strcpy(doc.m_szDeviceID, pPacket->sdate0);		//设备ID
	strcpy(doc.m_szRegister, pPacket->sdate2);		//注册号
	doc.m_nAuthID=ConvertID(doc.m_szDeviceID);//授权号
	doc.m_nMachineNo=pPacket->lvol4;				//机号	
	strcpy(doc.m_szVersion, pPacket->sdate3);		//设备版本号
	strcpy(doc.m_szPassword, "000000");				//系统员密码
	strcpy(doc.m_szOprPasswd, "000000");			//管理员密码
	doc.m_nPwdswitch=-1;							//密码开关

	if( strlen(pPacket->stime0) > 0 && strlen(pPacket->stime0) != 8 )
	{
		printf("非法父设备!\n");
		strcpy(doc.m_sClockVer, "");
	}
	else
		strcpy(doc.m_sClockVer, pPacket->stime0);

	strcpy(doc.m_szOrgid, "001");					//所属的组织代码
	doc.m_nPortCount=pPacket->lvol5;				//服务器端口总数
	printf("------服务器端口总数%d-----\n",pPacket->lvol5);
	doc.m_nSMTPort=pPacket->lvol6;				  //服务器端口号
	printf("------服务器端口号%d-----\n",pPacket->lvol6);
	strcpy(doc.m_szPort, pPacket->sbank_code);	  //通讯端口
	printf("------通讯端口%s-----\n",pPacket->sbank_code);
	doc.m_nBaudRate=pPacket->lvol9;			  //波特率
	doc.m_nCommMode=pPacket->lvol8;			  //链路模式
	printf("------链路模式%d-----\n",pPacket->lvol8);

	strcpy(doc.m_szAddr, pPacket->sstation0);    //通讯地址
	doc.m_nStatus=pPacket->lvol7;                //设备状态, 下面定义为:
	printf("------设备状态%d-----\n",pPacket->lvol7);
	doc.m_nConnType = pPacket->lvol10;			  //通讯方式
	strcpy(doc.m_szClass, pPacket->snote2);	  //卡类				  //

	doc.m_wFlowNo=pPacket->lvol11+1;			  //期望流水号 modified
	doc.m_wLastInvoiceNo=pPacket->lvol11;        //终端交易流水号期末值
	doc.m_wStateNo=pPacket->lvol12;
	doc.m_nFlow=1;									//序号
	doc.m_nBeginInvoice=0;							//扎帐的开始流水
	doc.m_wSettleinvoice=0;							//扎帐流水号期末值
	doc.m_wEndInvoice=0;							//扎帐的结束流水号
	strcpy(doc.m_sBlackExpire, pPacket->sserial0);  //黑名单的有效期
	strcpy(doc.m_sDownBlackTime, pPacket->sserial1); //黑名单下传时间期末值
	doc.m_nStartuse=0;								 //签到结果

	if( !CheckSmartDocValid(&doc) )
	{
		printf("CheckSmartDocValid !!!!!\n");
		return false;
	}
	WriteLog("终端机型号代码=%s,可用IC卡卡型=%s, 终端机型号=%s, 机型名称=%s, "
		"设备ID=%s, 注册号=%s, 机号=%d, 设备版本号=%s, 系统员密码=%s, 管理员密码=%s,"
		"密码开关=%d, 服务器端口总数=%d, 服务器端口号=%d, 通讯端口=%s, 波特率=%d,"
		"链路模式=%d,通讯地址=%s, 设备状态=%d, 交易流水号期末值=%d, 黑名单的有效期=%s, 黑名单版本=%s, 通讯方式=%d,卡类=%s, 父设备:%s\n",
		pPacket->semp, pPacket->sholder_type, pPacket->semp, pPacket->semp,
		pPacket->sdate0, pPacket->sdate2, pPacket->lvol4, pPacket->sdate3, "000000", "000000", 
		doc.m_nPwdswitch, pPacket->lvol5, pPacket->lvol6, pPacket->sbank_code, pPacket->lvol9, 
		pPacket->lvol8, pPacket->sstation0, pPacket->lvol7, pPacket->lvol11, pPacket->sserial0, pPacket->sserial1, 
		pPacket->lvol10, pPacket->snote2, pPacket->stime0);

	printf("更新设备(ID=%s, 注册号:%s, 机号=%d, 网络类型:%d..父设备:%s...\n", 
		doc.m_szDeviceID, doc.m_szRegister, 
		doc.m_nMachineNo, doc.m_nCommMode, pPacket->stime0);
//	ReadSmartDocList(TSSmartDoc *pSmartDoc,long *nRecount);
	nReload = 1;
	return true;
}

TSSmartDoc* CInterfaceApp::CheckDeviceID(char *pDeviceID)
{
	for(long i=0; i< gnDocCount; i++)
	{
		if( !strcmp(gpSmartDoc[i].m_szDeviceID, pDeviceID) )
			return &gpSmartDoc[i];
	}
	return NULL;
}

bool CInterfaceApp::FillTask(char *pDeviceID, TSSmartTaskPlan *pTaskPlan, unsigned int nRequest, ST_PACK *pPacket)
{
	WriteLog("-------FillTask----------\n");

	TSSmartDoc *pDoc = CheckDeviceID(pDeviceID);
	if( pDoc == NULL )
	{
		char szText[256];
		sprintf(szText, "没有此设备:%s!", pDeviceID);
		AnswerResultError(nRequest, pPacket->lvol1, -1, szText);
		WriteLog("金仕达下达任务:  设备%s, 功能号:%d.  .....没有此设备!\n", pDeviceID, nRequest);
		printf("------无此设备------\n");
		return false;
	}

	long nAuthID = pDoc->m_nAuthID;

	WriteLog("金仕达下达任务:  设备%s, 功能号:%d.\n", pDeviceID, nRequest);

	bool bResult = true;
	int i = pTaskPlan->nTask;

	sprintf(pTaskPlan->pTask[i].szSystemNO, "%d", pPacket->lvol1);
	pTaskPlan->pTask[i].nFunc = nRequest;

	switch(nRequest)
	{
	case 930001: //下传设备时钟
		if( strlen(pPacket->sserial0) == 12 )
		{
			pTaskPlan->pTask[i].nTaskID = GetTickCount();           
			pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
			pTaskPlan->pTask[i].nAuthID = nAuthID;                  
			strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
			strcpy(pTaskPlan->pTask[i].szTaskCode,"101");           
			pTaskPlan->pTask[i].nPriority = 5;                      
			strcpy(pTaskPlan->pTask[i].szMemo, pPacket->sserial0);  
			pTaskPlan->nTask++;
			WriteLog("下传设备时钟(%d). 终端设备ID=%s.  时钟=%s.\n", nRequest, pDeviceID, pPacket->sserial0);
		}
		else
		{
			bResult = false;
			WriteLog("下传设备时钟(%d). 终端设备ID=%s.  时钟=%s......参数不合法!\n", nRequest, pDeviceID, pPacket->sserial0);
			AnswerResultError(nRequest, pPacket->lvol1, -1, "参数不合法!");
		}
		break;
	case 930002: //上传设备时钟
		{
		pTaskPlan->pTask[i].nTaskID = GetTickCount();
		pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
		pTaskPlan->pTask[i].nAuthID = nAuthID;                  
		strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
		strcpy(pTaskPlan->pTask[i].szTaskCode,"03");           
		pTaskPlan->pTask[i].nPriority = 5;
		pTaskPlan->nTask++;
		WriteLog("上传设备时钟(%d). 终端设备ID=%s.  \n", nRequest, pDeviceID);
		}
		break;
	case 930003: //下传黑名单（指定设备）
		{
		pTaskPlan->pTask[i].nTaskID = GetTickCount();
		pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
		pTaskPlan->pTask[i].nAuthID = nAuthID;                  
		strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
		strcpy(pTaskPlan->pTask[i].szTaskCode,"71");           
		pTaskPlan->pTask[i].nPriority = 5;
		sprintf(pTaskPlan->pTask[i].szMemo, "%d %s %s", 
					pPacket->lvol0, pPacket->sserial0, pPacket->sserial1);
		pTaskPlan->nTask++;

		WriteLog("下传黑名单（指定设备）(%d). 终端设备ID=%s. [%d]  [%s]  [%s]\n", 
			nRequest, pDeviceID, pPacket->lvol0, pPacket->sserial0, pPacket->sserial1);
		}
		break;
	case 930004: //删除黑名单（指定设备）
		{
		pTaskPlan->pTask[i].nTaskID = GetTickCount();
		pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
		pTaskPlan->pTask[i].nAuthID = nAuthID;                  
		strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
		strcpy(pTaskPlan->pTask[i].szTaskCode,"71");           
		pTaskPlan->pTask[i].nPriority = 5;
		sprintf(pTaskPlan->pTask[i].szMemo, "%d %s", 
					pPacket->lvol0, pPacket->sserial0);
		pTaskPlan->nTask++;
		WriteLog("删除黑名单（指定设备）(%d). 终端设备ID=%s. [%d]  [%s]\n", 
			nRequest, pDeviceID, pPacket->lvol0, pPacket->sserial0);
		}
		break;
	case 930005: //下传增量黑名单
		//add by lina 
		pTaskPlan->pTask[i].nTaskID = GetTickCount();
		pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
		pTaskPlan->pTask[i].nAuthID = nAuthID;                  
		strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
		strcpy(pTaskPlan->pTask[i].szTaskCode,"71");           
		pTaskPlan->pTask[i].nPriority = 5;
		break;
	case 930006: //下传搭伙费比率
		{
		pTaskPlan->pTask[i].nTaskID = GetTickCount();
		pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
		pTaskPlan->pTask[i].nAuthID = nAuthID;                  
		strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
		strcpy(pTaskPlan->pTask[i].szTaskCode,"120");           
		pTaskPlan->pTask[i].nPriority = 5;
		sprintf(pTaskPlan->pTask[i].szMemo, "%d %d", 
					pPacket->lvol6, pPacket->lvol5);
		pTaskPlan->nTask++;
		WriteLog("下传搭伙费比率(%d). 终端设备ID=%s. [%d]  [%d]\n", 
			nRequest, pDeviceID, pPacket->lvol6, pPacket->lvol5);
		TSAttachData *pHead = new TSAttachData;
		pHead->nValue = pPacket->lvol6 ;
		pHead->nCount = pPacket->lvol5 ;
		pTaskPlan->pTask[i].pData = (char*)pHead;
		}
		break;
	case 930007: //下传设备主参数
		{
		if( atoi(pPacket->sbank_pwd) != 0x65 && 
			atoi(pPacket->sbank_pwd) != 0x64 && 
			atoi(pPacket->sbank_pwd) != 0x00 )
		{
			AnswerResultError(nRequest, pPacket->lvol1, -1, "参数不合法!");
			return false;
		}

		if( strlen(pPacket->sbankname) < 4 )
		{
			AnswerResultError(nRequest, pPacket->lvol1, -1, "卡类参数不合法!");
			return false;
		}

		pTaskPlan->pTask[i].nTaskID = GetTickCount();
		pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
		pTaskPlan->pTask[i].nAuthID = nAuthID;                  
		strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
		strcpy(pTaskPlan->pTask[i].szTaskCode,"80");
		pTaskPlan->pTask[i].nPriority = 3;

		TSAttachData *pHead = new TSAttachData;
		if( pHead == NULL )
		{
			AnswerResultError(nRequest, pPacket->lvol1, -1, "前置采集服务器系统错误!");
			WriteLog("下传设备主参数(%d). 终端设备ID=%s. 内存不够, 不能分配新的任务!\n", nRequest, pDeviceID);
			return false;
		}

		TSResultData  *pData = new TSResultData;
		if( pData == NULL )
		{
			AnswerResultError(nRequest, pPacket->lvol1, -1, "前置采集服务器系统错误!");
			WriteLog("下传设备主参数(%d). 终端设备ID=%s. 内存不够, 不能分配新的任务!\n", nRequest, pDeviceID);
			delete pHead;
			return false;
		}

		ZeroMemory(pHead, sizeof(TSAttachData));
		ZeroMemory(pData, sizeof(TSResultData));

		pHead->pData = (void*)pData;
		pTaskPlan->pTask[0].pData = (char*)pHead;
		pHead->nCount = 1;

		strncpy(pData->sValue1, pPacket->sdate0, 8);		//终端设备ID
		pData->nValue1 = pPacket->lvol3;					//机号
		strncpy(pData->sValue2, pPacket->sdate2, 8);			//注册号
		pData->nValue2 = pPacket->lvol5;			//lvol5  波特率 
		strcpy(pData->sValue3, pPacket->semp_pwd);	//sopen_emp 系统员密码 
		strcpy(pData->sValue4, pPacket->semp_pwd2);	//sclose_emp 管理员密码
		pData->nValue3 = pPacket->lvol6;					//lvol6 密码开关
		strcpy(pData->sValue5, pPacket->sbank_pwd);	//scurrency_type 卡片结构 
		pData->nValue4 = pPacket->lvol7;					//lvol7 卡的最大使用次数
		pData->nValue5 = pPacket->lvol8;					//lvol8 钱包最高存款限额
		pData->nValue6 = pPacket->lvol9;					//lvol9 钱包最低剩余款限额
		pData->nValue7 = pPacket->lvol10;					//lvol10 定值收费方式使用的定值额
		strcpy(pData->sValue6, pPacket->sbranch_code0);	    //sbranch_code0 钱包代码 
		pData->nValue8 = pPacket->lvol11;					//lvol11 每次交易最高额
		strncpy(pData->sValue7, pPacket->sbankname,64);		//sbankname 终端机适用用户卡类别
		strncpy(pData->sValue8, pPacket->scurrency_type2,2);  //scurrency_type2 收费机增强功能开关
		pData->nValue9 = pPacket->lvol12;					  //lvol11 每次交易最高额

		pTaskPlan->nTask++;

		WriteLog("下传设备主参数(%d). 终端设备ID=%s\n", nRequest, pPacket->sdate0);
		WriteLog("机号=%d\n", pPacket->lvol3);
		WriteLog("注册号=%s\n", pPacket->sdate2);
		WriteLog("波特率=%d\n", pPacket->lvol5);
		WriteLog("系统员密码=%s\n", pPacket->semp_pwd);
		WriteLog("管理员密码=%s\n", pPacket->semp_pwd2);
		WriteLog("密码开关=%d\n", pPacket->lvol6);
		WriteLog("卡片结构=%s\n", pPacket->sbank_pwd);
		WriteLog("卡的最大使用次数=%d\n", pPacket->lvol7);
		WriteLog("钱包最高存款限额=%d\n", pPacket->lvol8);
		WriteLog("钱包最低剩余款限额=%d\n", pPacket->lvol9);
		WriteLog("定值收费方式使用的定值额=%d\n", pPacket->lvol10);
		WriteLog("钱包代码=%s\n", pPacket->sbranch_code0);
		WriteLog("每次交易最高额=%d\n", pPacket->lvol11);
		WriteLog("终端机适用用户卡类别=%s\n", pPacket->sbankname);
		WriteLog("收费机增强功能开关=%s\n", pPacket->scurrency_type2);
		WriteLog("收费方式=%d\n\n", pPacket->lvol12);
		}
		break;
	case 930008: //上传设备主参数
		pTaskPlan->pTask[i].nTaskID = GetTickCount();
		pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
		pTaskPlan->pTask[i].nAuthID = nAuthID;                  
		strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
		strcpy(pTaskPlan->pTask[i].szTaskCode,"81");           
		pTaskPlan->pTask[i].nPriority = 4;
		pTaskPlan->nTask++;
		WriteLog("上传设备主参数(%d). 终端设备ID=%s \n", nRequest, pDeviceID);
		break;
	case 930009: //设置补助开关
		if( strcmp(pDoc->m_szMacCode, "5301") )
		{
			AnswerResultError(nRequest, pPacket->lvol1, -1, "只有LPort设备才有此功能!");
			bResult = false ;
		}
		else
		{
			pTaskPlan->pTask[i].nTaskID = GetTickCount();
			pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
			pTaskPlan->pTask[i].nAuthID = nAuthID;                  
			strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
			strcpy(pTaskPlan->pTask[i].szTaskCode,"62");           
			pTaskPlan->pTask[i].nPriority = 5;
			sprintf(pTaskPlan->pTask[i].szMemo, "%d", pPacket->lvol4);
			pTaskPlan->nTask++;
			WriteLog("设置补助开关(%d). 终端设备ID=%s. [%d]\n", nRequest, pDeviceID, pPacket->lvol4);
		}
		break;
	case 930010: //下传大额消费限额
		pTaskPlan->pTask[i].nTaskID = GetTickCount();
		pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
		pTaskPlan->pTask[i].nAuthID = nAuthID;                  
		strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
		strcpy(pTaskPlan->pTask[i].szTaskCode,"102");           
		pTaskPlan->pTask[i].nPriority = 5;
		sprintf(pTaskPlan->pTask[i].szMemo, "%d", pPacket->lvol4);
		pTaskPlan->nTask++;
		WriteLog("下传大额消费限额(%d). 终端设备ID=%s. [%d]\n", nRequest, pDeviceID, pPacket->lvol4);
		break;
	case 930011: //设置消费编号及版本
		break;
	case 930012: //设置消费快捷编号
		break;
	case 930013: //设置消费时间段参数
		break;
	case 930014: //防火状态设置\防盗状态设置\取消防火防盗恢复正常
		{
		WriteLog("防火状态设置...(%d). 终端设备ID=%s. [%d]\n", 
			nRequest, pDeviceID, pPacket->lvol4);
		if( pPacket->lvol4 == 0 )
		{
			pTaskPlan->pTask[i].nTaskID = GetTickCount();
			pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
			pTaskPlan->pTask[i].nAuthID = nAuthID;                  
			strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
			strcpy(pTaskPlan->pTask[i].szTaskCode,"110");           
			pTaskPlan->pTask[i].nPriority = 8;
			pTaskPlan->nTask++;
		}
		else if( pPacket->lvol4 == 1 )
		{
			pTaskPlan->pTask[i].nTaskID = GetTickCount();
			pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
			pTaskPlan->pTask[i].nAuthID = nAuthID;                  
			strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
			strcpy(pTaskPlan->pTask[i].szTaskCode,"111");           
			pTaskPlan->pTask[i].nPriority = 9;
			pTaskPlan->nTask++;
		}
		else if( pPacket->lvol4 == 2 )
		{
			pTaskPlan->pTask[i].nTaskID = GetTickCount();
			pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
			pTaskPlan->pTask[i].nAuthID = nAuthID;                  
			strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
			strcpy(pTaskPlan->pTask[i].szTaskCode,"112");           
			pTaskPlan->pTask[i].nPriority = 4;
			pTaskPlan->nTask++;
		}
		else
		{
			printf("功能号%d的参数不合法!\n", 930014);
			AnswerResultError(nRequest, pPacket->lvol1, -1, "参数不合法!");
			bResult = false ;
		}
		}
		break;
	case 930015: //设备控制
		break;
	case 930017: //采集补助发放流水（历史）
	case 930016: //采集消费流水（历史）
	case 930018: //采集现金充值流水（历史）
	case 930022: //采集现金充值管理费流水（历史）
		if( nRequest == 930017 )	//采集补助发放流水（历史）
		{
			if( strcmp(pDoc->m_szMacCode, "5301") )
			{
				AnswerResultError(nRequest, pPacket->lvol1, -1, "只有LPort设备才有此功能!");
				bResult = false ;
				return bResult;
			}
		}
		else
		{
			if( !strcmp(pDoc->m_szMacCode, "5301") )
			{
				AnswerResultError(nRequest, pPacket->lvol1, -1, "LPort设备无此功能!");
				bResult = false ;
				return bResult;
			}
		}
		pTaskPlan->nRepeatTimes = 0 ;
		pTaskPlan->pTask[i].nTaskID = GetTickCount();
		pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
		pTaskPlan->pTask[i].nAuthID = nAuthID;                  
		strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
		strcpy(pTaskPlan->pTask[i].szTaskCode,"09");           
		pTaskPlan->pTask[i].nPriority = 6;
		sprintf(pTaskPlan->pTask[i].szMemo, "%d:%d", pPacket->lvol5, pPacket->lvol6);
		pTaskPlan->nTask++;
		WriteLog("采集流水（历史）...(%d). 终端设备ID=%s. [%d--%d]....MSGID=%d..FUNCTION=%d.....\n", nRequest, pDeviceID, pPacket->lvol5, pPacket->lvol6, pPacket->lvol1, pPacket->lvol4);
		break;
	case 930019: //采集设备结账流水（历史）
		pTaskPlan->nRepeatTimes = 0 ;
		pTaskPlan->pTask[i].nTaskID = GetTickCount();
		pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
		pTaskPlan->pTask[i].nAuthID = nAuthID;                  
		strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
		strcpy(pTaskPlan->pTask[i].szTaskCode,"16");           
		pTaskPlan->pTask[i].nPriority = 6;
		sprintf(pTaskPlan->pTask[i].szMemo, "%d:%d", pPacket->lvol5, pPacket->lvol6);
		pTaskPlan->nTask++;
		WriteLog("采集设备结账流水（历史）...(%d). 终端设备ID=%s. [%d--%d]\n", nRequest, pDeviceID, pPacket->lvol5, pPacket->lvol6);
		break;
	case 930020: //下传补助发放名单
		break;
	case 930038: //修改设备注册号
		pTaskPlan->pTask[i].nTaskID = GetTickCount();
		pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
		pTaskPlan->pTask[i].nAuthID = nAuthID;                  
		strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
		strcpy(pTaskPlan->pTask[i].szTaskCode,"38");           
		pTaskPlan->pTask[i].nPriority = 6;
		sprintf(pTaskPlan->pTask[i].szMemo, "%s", pPacket->sdate2);
		pTaskPlan->nTask++;
		WriteLog("修改设备注册号...(%d). 终端设备ID=%s. [%s]\n", nRequest, pDeviceID, pPacket->sdate2);
		break;
	case 930056: //下传设备监控参数
		gTick = pPacket->lvol4*1000;
		WriteLog("下传设备监控心跳:%d\n", gTick/1000);
		printf("下传设备监控心跳:%d\n", gTick/1000);
		bResult = false ;
		break;
	case 930060: //设备签到签退
		if( !strcmp(pDoc->m_szMacCode, "5301") )
		{
			AnswerResultError(nRequest, pPacket->lvol1, -1, "LPort设备不能签到签退!");
			bResult = false ;
		}
		else
		{
			pTaskPlan->pTask[i].nTaskID = GetTickCount();
			pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
			pTaskPlan->pTask[i].nAuthID = nAuthID;                  
			strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
			strcpy(pTaskPlan->pTask[i].szTaskCode,"130");           
			pTaskPlan->pTask[i].nPriority = 3;//5;
			sprintf(pTaskPlan->pTask[i].szMemo, "%d", pPacket->lvol4);
			pTaskPlan->nTask++;
			WriteLog("设备签到签退(%d). 终端设备ID=%s. [%d]\n", nRequest, pDeviceID, pPacket->lvol4);
		}
		break;
	case 930061://设置每日最大消费金额
			pTaskPlan->pTask[i].nTaskID = GetTickCount();
			pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
			pTaskPlan->pTask[i].nAuthID = nAuthID;                  
			strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
			strcpy(pTaskPlan->pTask[i].szTaskCode,"131");           
			pTaskPlan->pTask[i].nPriority = 5;
			sprintf(pTaskPlan->pTask[i].szMemo, "%d", pPacket->lvol4);
			pTaskPlan->nTask++;
			WriteLog("设置每日累计消费限额(%d). 终端设备ID=%s. 金额=[%d]\n", nRequest, pDeviceID, pPacket->lvol4);
			break;
	case 930062://1.24.	初始化LPORT端口参数
			pTaskPlan->pTask[i].nTaskID = GetTickCount();
			pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
			pTaskPlan->pTask[i].nAuthID = nAuthID;                  
			strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
			strcpy(pTaskPlan->pTask[i].szTaskCode,"132");           
			pTaskPlan->pTask[i].nPriority = 5;
			sprintf(pTaskPlan->pTask[i].szMemo, "%d", pPacket->lvol4);
			pTaskPlan->nTask++;
			WriteLog("初始化LPORT端口参数(%d). 终端设备ID=%s. 端口号=[%d]\n", nRequest, pDeviceID, pPacket->lvol4);
			break;
	case 930063://1.25.	设置终端管理员密码
			pTaskPlan->pTask[i].nTaskID = GetTickCount();
			pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
			pTaskPlan->pTask[i].nAuthID = nAuthID;                  
			strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
			strcpy(pTaskPlan->pTask[i].szTaskCode,"133");           
			pTaskPlan->pTask[i].nPriority = 5;
			sprintf(pTaskPlan->pTask[i].szMemo, "%s", pPacket->semp_pwd);
			pTaskPlan->nTask++;
			WriteLog("设置终端管理员密码(%d). 终端设备ID=%s. 密码=[%s]\n", nRequest, pDeviceID, pPacket->semp_pwd);
			break;
	case 930064://获取lport指定端口的设备参数
			pTaskPlan->pTask[i].nTaskID = GetTickCount();
			pTaskPlan->pTask[i].nTaskPlanID=pTaskPlan->nTaskPlanID; 
			pTaskPlan->pTask[i].nAuthID = nAuthID;                  
			strcpy(pTaskPlan->pTask[i].szDeviceID, pDeviceID);      
			strcpy(pTaskPlan->pTask[i].szTaskCode,"134");           
			pTaskPlan->pTask[i].nPriority = 5;
			sprintf(pTaskPlan->pTask[i].szMemo, "%d", pPacket->lvol4);
			pTaskPlan->nTask++;
			WriteLog("获取lport指定端口的设备参数(%d). 终端设备ID=%s. 端口号=[%d]\n", nRequest, pDeviceID, pPacket->lvol4);

		break;
	case 930099: //设备监控心跳
		//gTick = pPacket->lvol4;
		printf("-------设备监控心跳\n");
		bResult = false ;
		break;
	default:
		WriteLog("不支持的功能号.......(%d). 终端设备ID=%s....\n", nRequest, pDeviceID);
		printf("不支持的功能号.......(%d). 终端设备ID=%s....\n", nRequest, pDeviceID);

		AnswerResultError(nRequest, pPacket->lvol1, -1, "不支持的功能号");
		bResult = false ;
		
		break;
	}
	return bResult;
}

long CInterfaceApp::AnswerSmartDocSuccess(int iFunction, int iMsg, int iResult, char *pszDeviceID)
{
	ST_CPACK rpack;

	memset(&rpack, 0, sizeof(rpack));

	rpack.head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	rpack.head.nextflag = 0;    /* 是否后续包请求*/
	rpack.head.recCount = 1;    /* 本包的记录数*/	
	rpack.head.retCode = 0;     /* 返回代码*/
	rpack.head.userdata = 0;

	rpack.head.RequestType = 930098; 
	SetValue(&rpack.head,&rpack.pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
	SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
	SetValue(&rpack.head,&rpack.pack, F_LVOL1, GetValue(iMsg));				//消息ID
	SetValue(&rpack.head,&rpack.pack, F_LVOL2, GetValue(iFunction));		//功能号
	SetValue(&rpack.head,&rpack.pack, F_LVOL4, GetValue(iResult));			//消息ID
	SetValue(&rpack.head,&rpack.pack, F_VSMESS, "成功");					//返回码信息
	SetValue(&rpack.head,&rpack.pack, F_SDATE0, pszDeviceID);				//设备ID

	char buffer[8192];
	int  nLen = sizeof(buffer);
	char omsg[256];

	memset(buffer, 0, sizeof(buffer));

	HANDLE hHandle = ConnectPool.Alloc();

	if( EncodeBuf(&rpack, (unsigned char*)buffer, &nLen, omsg) ) 
	{
		printf("AnswerSmartDocSuccess\n");
		if( SendData(hHandle, iServerNo, iFunc, (char*)buffer, nLen, 1, FALSE) == RET_OK ) 
		{
			nLen = sizeof(buffer);
			memset(buffer, 0, sizeof(buffer));
			// modified by lina 2005042 int nlen = RecvData(hHandle, buffer, nLen, 0);
			int nlen = RecvData(hHandle, buffer, nLen, 1000);
			if( nlen > 0 )
			{
				ST_CPACK apack;
				if( DecodeBuf((unsigned char*)buffer,nlen,&apack,omsg) )
				{
					if( !apack.head.retCode )
					{
						ConnectPool.Free(hHandle);
						return RET_OK;
					}
				}
			}
		}
	}

	ConnectPool.Free(hHandle);

	return RET_SYSERROR;
}


long CInterfaceApp::AnswerResult(int iFunction, int iMsg, int iResult, char *pszMsg)
{
	ST_CPACK rpack;

	memset(&rpack, 0, sizeof(rpack));

	rpack.head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	rpack.head.nextflag = 0;    /* 是否后续包请求*/
	rpack.head.recCount = 1;    /* 本包的记录数*/
	rpack.head.retCode = 0;     /* 返回代码*/
	rpack.head.userdata = 0;

	rpack.head.RequestType = 930098; 
	SetValue(&rpack.head,&rpack.pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
	SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
	SetValue(&rpack.head,&rpack.pack, F_LVOL1, GetValue(iMsg));				//消息ID
	SetValue(&rpack.head,&rpack.pack, F_LVOL2, GetValue(iFunction));		//功能号
	SetValue(&rpack.head,&rpack.pack, F_LVOL4, GetValue(iResult));			//消息ID
	SetValue(&rpack.head,&rpack.pack, F_VSMESS, pszMsg);					//返回码信息

	char buffer[8192];
	int  nLen = sizeof(buffer);
	char omsg[256];

	memset(buffer, 0, sizeof(buffer));

	HANDLE hHandle = ConnectPool.Alloc();

	if( EncodeBuf(&rpack, (unsigned char*)buffer, &nLen, omsg) ) 
	{
		printf("AnswerResult\n");
		SendData(hHandle, iServerNo, iFunc, (char*)buffer, nLen, 1, FALSE);
	}
	ConnectPool.Free(hHandle);
	return RET_OK;
}

long CInterfaceApp::AnswerResultError(int iFunction, int iMsg, int iResult, char *pszMsg)
{
	ST_CPACK rpack;

	memset(&rpack, 0, sizeof(rpack));

	rpack.head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	rpack.head.nextflag = 0;    /* 是否后续包请求*/
	rpack.head.recCount = 1;    /* 本包的记录数*/
	rpack.head.retCode = 0;     /* 返回代码*/
	rpack.head.userdata = 0;

	rpack.head.RequestType = 930098; 
	SetValue(&rpack.head,&rpack.pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
	SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
	SetValue(&rpack.head,&rpack.pack, F_LVOL1, GetValue(iMsg));				//消息ID
	SetValue(&rpack.head,&rpack.pack, F_LVOL2, GetValue(iFunction));			//功能号
	SetValue(&rpack.head,&rpack.pack, F_LVOL4, GetValue(iResult));			//消息ID
	SetValue(&rpack.head,&rpack.pack, F_VSMESS, pszMsg);					//返回码信息

	char buffer[8192];
	int  nLen = sizeof(buffer);
	char omsg[256];

	memset(buffer, 0, sizeof(buffer));

	HANDLE hHandle = ConnectPool.Alloc();

	if( EncodeBuf(&rpack, (unsigned char*)buffer, &nLen, omsg) ) 
	{
		printf("AnswerResultError\n");
		if( SendData(hHandle, iServerNo, iFunc, (char*)buffer, nLen, 1, FALSE) == RET_OK ) 
		{
			nLen = sizeof(buffer);
			memset(buffer, 0, sizeof(buffer));
//modified by lina 20050402			int nlen = RecvData(hHandle, buffer, nLen, 0);
			int nlen = RecvData(hHandle, buffer, nLen, 1000);
			if( nlen > 0 )
			{
				ST_CPACK apack;
				if( DecodeBuf((unsigned char*)buffer,nlen,&apack,omsg) )
				{
					if( !apack.head.retCode )
					{
						ConnectPool.Free(hHandle);
						return RET_OK;
					}
					else
					{
						printf("%s\n", apack.pack.vsmess);
					}
				}
			}
		}
	}

	ConnectPool.Free(hHandle);

	return RET_SYSERROR;
}

long CInterfaceApp::ReportResult(TSSmartTask *pTask, long iResult, TSResultData *pData)
{
	ST_CPACK rpack;

	memset(&rpack, 0, sizeof(rpack));

	rpack.head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	rpack.head.nextflag = 0;    /* 是否后续包请求*/
	rpack.head.recCount = 1;    /* 本包的记录数*/
	rpack.head.retCode = 0;     /* 返回代码*/
	rpack.head.userdata = 0;

	rpack.head.RequestType = 930098; 
	SetValue(&rpack.head,&rpack.pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
	SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
	SetValue(&rpack.head,&rpack.pack, F_LVOL1, pTask->szSystemNO);			//消息ID
	SetValue(&rpack.head,&rpack.pack, F_LVOL4, GetValue(iResult));			//消息ID
	SetValue(&rpack.head,&rpack.pack, F_VSMESS, pData->sMsg);				//返回码信息

	switch(pTask->nFunc)
	{
	case 930001://下传设备时钟
		WriteLog("设备:%s, 返回包(930098),下传设备时钟.....返回码:%d, 消息:%s..\n",pTask->szDeviceID,  iResult, pData->sMsg);
		break;
	case 930002://上传设备时钟
		WriteLog("设备:%s, 返回包(930098),上传设备时钟.....返回码:%d, 消息:%s..\n",pTask->szDeviceID,  iResult, pData->sMsg);
		SetValue(&rpack.head,&rpack.pack, F_SSERIAL0, pData->sValue1);
		break;
	case 930003://下传黑名单（指定设备）
		{	
		TSAttachData *pHead = (TSAttachData*)pTask->pData;
		if( pHead == NULL )
			return RET_OK;

		TSBlackCard *pArray = (TSBlackCard*)pHead->pData;
		if( pArray == NULL )
			return RET_OK;

		SetValue(&rpack.head,&rpack.pack, F_SDATE0, pTask->szDeviceID);
		SetValue(&rpack.head,&rpack.pack, F_LVOL0, GetValue(pArray[pHead->nCount-1].nCardID));
		SetValue(&rpack.head,&rpack.pack, F_SSERIAL0, pArray[pHead->nCount-1].sVersion);

		WriteLog("设备:%s, 返回包(930098),下传黑名单（指定设备）.....返回码:%d, 消息:%s..\n",pTask->szDeviceID,  iResult, pData->sMsg);

		if( pArray != NULL )
		{
//			printf("删除TSBlackCard内容\n");
			delete [] pArray;
		}
		if( pHead != NULL )	
		{
			delete pHead;
//			printf("删除TSAttachData内容\n");
		}
		pTask->pData = NULL;

	}
		break;
	case 930004://删除黑名单（指定设备）
		{
		TSAttachData *pHead = (TSAttachData*)pTask->pData;
		if( pHead == NULL )
			return RET_OK;

		TSBlackCard *pArray = (TSBlackCard*)pHead->pData;
		if( pArray == NULL )
			return RET_OK;

		SetValue(&rpack.head,&rpack.pack, F_SDATE0, pTask->szDeviceID);
		SetValue(&rpack.head,&rpack.pack, F_LVOL0, GetValue(pArray[pHead->nCount-1].nCardID));
		SetValue(&rpack.head,&rpack.pack, F_SSERIAL0, pArray[pHead->nCount-1].sVersion);

		WriteLog("设备:%s, 返回包(930098),删除黑名单（指定设备）.....返回码:%d, 消息:%s..\n", pTask->szDeviceID, iResult, pData->sMsg);

		if( pArray != NULL )	delete pArray;
		if( pHead != NULL )	delete pHead;
		pTask->pData = NULL;

		}
		break;
	case 930005://下传增量黑名单
		SetValue(&rpack.head,&rpack.pack, F_LVOL0, GetValue(pData->nValue1));
		SetValue(&rpack.head,&rpack.pack, F_SSERIAL0, pData->sValue1);
		WriteLog("设备:%s, 返回包(930098),下传增量黑名单.....返回码:%d, 消息:%s..\n", pTask->szDeviceID, iResult, pData->sMsg);
		break;
	case 930006://下传搭伙费比率
		{
		TSAttachData *pHead = (TSAttachData*)pTask->pData;
		if( pHead == NULL )
			return RET_OK;

		if( pHead != NULL )	delete pHead;
		pTask->pData = NULL;

		SetValue(&rpack.head,&rpack.pack, F_SDATE0, pTask->szDeviceID);
		SetValue(&rpack.head,&rpack.pack, F_LVOL6, GetValue(pData->nValue1));
		WriteLog("设备:%s, 返回包(930098),下传搭伙费比率.....返回码:%d, 消息:%s..\n", pTask->szDeviceID, iResult, pData->sMsg);
		}
		break;
	case 930007://下传设备主参数
		{
		SetValue(&rpack.head,&rpack.pack, F_SDATE0, pTask->szDeviceID);
		WriteLog("设备:%s, 返回包(930098),下传设备主参数.....返回码:%d, 消息:%s..\n", pTask->szDeviceID, iResult, pData->sMsg);
		TSAttachData *pHead = (TSAttachData*)pTask->pData;
		if( pHead )
		{
			TSResultData *pArray = (TSResultData*)pHead->pData;
			if( pArray != NULL )	delete pArray;
			if( pHead != NULL )	delete pHead;
			pTask->pData = NULL;
		}
		}	
		break;
	case 930008://上传设备主参数
		{
		SetValue(&rpack.head,&rpack.pack, F_SDATE0, pData->sValue1);	//终端设备ID
		SetValue(&rpack.head,&rpack.pack, F_LVOL3, GetValue(pData->nValue1));	//机号	
		SetValue(&rpack.head,&rpack.pack, F_SDATE2, pData->sValue2);	//注册号
		SetValue(&rpack.head,&rpack.pack, F_LVOL5, GetValue(pData->nValue2));	//lvol5  波特率 
		SetValue(&rpack.head,&rpack.pack, F_SEMP_PWD, pData->sValue3);	//sopen_emp 系统员密码 
		SetValue(&rpack.head,&rpack.pack, F_SEMP_PWD2, pData->sValue4);	//sclose_emp 管理员密码
		SetValue(&rpack.head,&rpack.pack, F_LVOL6, GetValue(pData->nValue3));	//lvol6 密码开关
		SetValue(&rpack.head,&rpack.pack, F_SBANK_PWD, pData->sValue5);	//scurrency_type 卡片结构 
		SetValue(&rpack.head,&rpack.pack, F_LVOL7, GetValue(pData->nValue4));	//lvol7 卡的最大使用次数
		SetValue(&rpack.head,&rpack.pack, F_LVOL8, GetValue(pData->nValue5));	//lvol8 钱包最高存款限额
		SetValue(&rpack.head,&rpack.pack, F_LVOL9, GetValue(pData->nValue6));	//lvol9 钱包最低剩余款限额
		SetValue(&rpack.head,&rpack.pack, F_LVOL10, GetValue(pData->nValue7));	//lvol10 定值收费方式使用的定值额
		SetValue(&rpack.head,&rpack.pack, F_SBRANCH_CODE0, pData->sValue6);	//sbranch_code0 钱包代码 
		SetValue(&rpack.head,&rpack.pack, F_LVOL11, GetValue(pData->nValue8));	//lvol11 每次交易最高额
		SetValue(&rpack.head,&rpack.pack, F_SBANKNAME, pData->sValue7);	//sbankname 终端机适用用户卡类别
		SetValue(&rpack.head,&rpack.pack, F_SCURRENCY_TYPE2, pData->sValue8);	//scurrency_type2 收费机增强功能开关
		SetValue(&rpack.head,&rpack.pack, F_LVOL12, GetValue(pData->nValue9));	//收费方式

		WriteLog("上传设备主参数. 终端设备ID=%s\n", pData->sValue1);
		WriteLog("机号=%d\n", pData->nValue1);
		WriteLog("注册号=%s\n", pData->sValue2);
		WriteLog("波特率=%d\n", pData->nValue2);
		WriteLog("系统员密码=%s\n", pData->sValue3);
		WriteLog("管理员密码=%s\n", pData->sValue4);
		WriteLog("密码开关=%d\n", pData->nValue3);
		WriteLog("卡片结构=%s\n", pData->sValue5);
		WriteLog("卡的最大使用次数=%d\n", pData->nValue4);
		WriteLog("钱包最高存款限额=%d\n", pData->nValue5);
		WriteLog("钱包最低剩余款限额=%d\n", pData->nValue6);
		WriteLog("定值收费方式使用的定值额=%d\n", pData->nValue7);
		WriteLog("钱包代码=%s\n", pData->sValue6);
		WriteLog("每次交易最高额=%d\n", pData->nValue8);
		WriteLog("终端机适用用户卡类别=%s\n", pData->sValue7);
		WriteLog("收费机增强功能开关=%s\n\n", pData->sValue8);
		WriteLog("收费方式=%d\n\n", pData->nValue9);

		WriteLog("设备:%s, 返回包(930098),上传设备主参数.....返回码:%d, 消息:%s..\n", pTask->szDeviceID, iResult, pData->sMsg);
		}
		break;
	case 930009://设置补助开关
		WriteLog("设备:%s, 返回包(930098),设置补助开关.....返回码:%d, 消息:%s..\n",pTask->szDeviceID,  iResult, pData->sMsg);
		break;
	case 930010://下传大额消费限额
		WriteLog("设备:%s, 返回包(930098),下传大额消费限额.....返回码:%d, 消息:%s..\n", pTask->szDeviceID, iResult, pData->sMsg);
		break;
	case 930011://设置消费编号及版本
		{
		WriteLog("设备:%s, 返回包(930098),设置消费编号及版本....返回码:%d, 消息:%s..\n",pTask->szDeviceID,  iResult, pData->sMsg);
		
		TSAttachData *pHead = (TSAttachData*)pTask->pData;
		if( pHead )
		{
			int *pArray = (int*)pHead->pData;
			delete [] pArray;
			delete pHead;
			pTask->pData = NULL;
		}
		}
		break;
	case 930012://设置消费快捷编号
		{
		WriteLog("设备:%s, 返回包(930098),设置消费快捷编号.....返回码:%d, 消息:%s..\n", pTask->szDeviceID, iResult, pData->sMsg);
		printf("设备:%s, 返回包(930098),设置消费快捷编号.....返回码:%d, 消息:%s..\n", pTask->szDeviceID, iResult, pData->sMsg);
		TSAttachData *pHead = (TSAttachData*)pTask->pData;
		if( pHead )
		{
			int *pArray = (int*)pHead->pData;
			if( pArray != NULL )  delete [] pArray; 
			if( pHead != NULL )	delete pHead;
			pTask->pData = NULL;
		}
		}
		break;
	case 930013://设置消费时间段参数
		{
		WriteLog("设备:%s, 返回包(930098),设置消费时间段参数.....返回码:%d, 消息:%s..\n", pTask->szDeviceID, iResult, pData->sMsg);
		TSAttachData *pHead = (TSAttachData*)pTask->pData;
		if( pHead )
		{
			TSXFTimePara *pArray = (TSXFTimePara*)pHead->pData;
			if( pArray != NULL )	delete [] pArray;
			if( pHead != NULL )	delete pHead;
			pTask->pData = NULL;

		}
		}
		break;
	case 930014://防火状态设置\防盗状态设置\取消防火防盗恢复正常
		WriteLog("设备:%s, 返回包(930098),防火状态设置.....返回码:%d, 消息:%s..\n", pTask->szDeviceID, iResult, pData->sMsg);
		break;
	case 930015://设备控制
		WriteLog("设备:%s, 返回包(930098),设备控制.....返回码:%d, 消息:%s..\n", pTask->szDeviceID, iResult, pData->sMsg);
		break;
	case 930020://下传补助发放名单
		{
		WriteLog("设备:%s, 返回包(930098),下传补助发放名单.....返回码:%d, 消息:%s..\n", pTask->szDeviceID, iResult, pData->sMsg);

		TSAttachData *pHead = (TSAttachData*)pTask->pData;
		if( pHead == NULL )
			return RET_OK;

		TSBZDataList *pArray = (TSBZDataList*)pHead->pData;
	///	SetValue(&rpack.head,&rpack.pack, F_SDATE0, pTask->szDeviceID);	//终端设备ID
	///	SetValue(&rpack.head,&rpack.pack, F_LVOL0, GetValue(pArray[pHead->nCount-1].nCardID));	//机号	
		if( pArray != NULL )
			delete [] pArray;

		if( pArray != NULL )
			delete pHead;

		pTask->pData = NULL;

		}
		break;
	case 930056://下传设备监控参数
		WriteLog("设备:%s, 返回包(930056),下传设备监控参数.....返回码:%d, 消息:%s..\n", pTask->szDeviceID, iResult, pData->sMsg);
		break;
	case 930060://设备签到签退
		WriteLog("设备:%s, 返回包(930060),设置设备签到签退.....返回码:%d, 消息:%s..\n",pTask->szDeviceID,  iResult, pData->sMsg);
		break;
	case 930061://设置每日最高消费限额
		WriteLog("设备:%s,返回包(930061),设置每日累计消费限额....返回代码:%d,消息:%s\n",pTask->szDeviceID,  iResult, pData->sMsg);
		break;
	case 930062:
		WriteLog("设备:%s,返回包(930062),初始化LPORT端口参数....返回代码:%d,消息:%s\n",pTask->szDeviceID,  iResult, pData->sMsg);
		break;
	case 930063:
		WriteLog("设备:%s,返回包(930063),设置终端管理员密码....返回代码:%d,消息:%s\n",pTask->szDeviceID,  iResult, pData->sMsg);
		break;
	case 930064:
		SetValue(&rpack.head,&rpack.pack, F_SDATE0, pData->sValue1);	//sdate0终端设备ID
		SetValue(&rpack.head,&rpack.pack, F_LVOL3, GetValue(pData->nValue1));	//lvol3机号	
		SetValue(&rpack.head,&rpack.pack, F_SEMP, pData->sValue2);	//semp类型
		SetValue(&rpack.head,&rpack.pack, F_SDATE3, pData->sValue3);	//sdate3 程序版本 
		SetValue(&rpack.head,&rpack.pack, F_SDATE2, pData->sValue4);	//sdate2注册号 
		WriteLog("设备:%s,返回包(930064),获取lport指定端口的设备参数....返回代码:%d,消息:%s\n",pTask->szDeviceID,  iResult, pData->sMsg);
		break;

	default:
		break;
	}
	
	//delete pTask;
	char buffer[8192];
	int  nLen = sizeof(buffer);
	char omsg[256];

	memset(buffer, 0, sizeof(buffer));

	HANDLE hHandle = ConnectPool.Alloc();
	//printf("-----------rpack->id=%d\n",rpack.pack.lvol1);
	if( EncodeBuf(&rpack, (unsigned char*)buffer, &nLen, omsg) ) 
	{
		//printf("-----ReportResult----------\n");
		if( SendData(hHandle, iServerNo, iFunc, (char*)buffer, nLen, 1, FALSE) == RET_OK ) 
		{
			nLen = sizeof(buffer);
			memset(buffer, 0, sizeof(buffer));
//modified by lina 20050402			int nlen = RecvData(hHandle, buffer, nLen, 0);
			int nlen = RecvData(hHandle, buffer, nLen, 1000);
			if( nlen > 0 )
			{
				ST_CPACK apack;
				if( DecodeBuf((unsigned char*)buffer,nlen,&apack,omsg) )
				{
					if( !apack.head.retCode )
					{
						ConnectPool.Free(hHandle);
						return RET_OK;
					}
					else
					{
						printf("%s\n", apack.pack.vsmess);
					}
				}
			}
		}
	}

	ConnectPool.Free(hHandle);

	return RET_SYSERROR;
}

long CInterfaceApp::OutputResultBlackCard(TSSmartDoc *pDoc, long nFlag, TSBlackCard *pCard)
{
	ST_CPACK rpack;

//	printf("!!!!!!发送930046包\n");
	memset(&rpack, 0, sizeof(rpack));

	rpack.head.firstflag = 1;   /* 是否第一个请求（首包请求）*/
	rpack.head.nextflag = 0;    /* 是否后续包请求*/
	rpack.head.recCount = 1;    /* 本包的记录数*/
	rpack.head.retCode = 0;     /* 返回代码*/
	rpack.head.userdata = 0;

	rpack.head.RequestType = 930046; 
	SetValue(&rpack.head,&rpack.pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
	SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
	SetValue(&rpack.head,&rpack.pack, F_SDATE1, pDoc->m_szDeviceID);
	SetValue(&rpack.head,&rpack.pack, F_SSERIAL0, pCard->sVersion);

	char buffer[8192];
	int  nLen = sizeof(buffer);
	char omsg[256];

	memset(buffer, 0, sizeof(buffer));

	HANDLE hHandle = ConnectPool.Alloc();
//	printf("获取930046 HANDLE = %ld\n",hHandle);
	if( EncodeBuf(&rpack, (unsigned char*)buffer, &nLen, omsg) ) 
	{
//		printf("OutputResultBlackCard 发送930046 HANDLE=%ld\n",hHandle);
		if( SendData(hHandle, iServerNo, iFunc, (char*)buffer, nLen, 1, FALSE) == RET_OK ) 
		{
			ConnectPool.Free(hHandle);
			return RET_OK;
		}
	}

	ConnectPool.Free(hHandle);

	return RET_SYSERROR;
}

long CInterfaceApp::DownloadBlackList(TSSmartDoc *pDoc, int nType, char *pszFileName)
{
	ST_CPACK rpack;
	char buffer[1024];
	int  nLen = sizeof(buffer);
	char omsg[256];
	int  kkkk=0;
	BOOL bEnd = FALSE ;
	ST_CPACK apack;
	ST_PACK  apackarray[60];

	int nRow = 0 ;
	int ifirst = 1 ;

	FILE *fp = fopen(pszFileName, "w");
	if( fp == NULL )
	{
		return RET_SYSERROR;
	}

	HANDLE hHandle = ConnectPool.Alloc(4);
	
	
	printf("申请HADLE=%ld\n",hHandle);
	while( !bEnd )
	{
		kkkk += 1;
		//printf("发送930039包第%d次\n",kkkk);
		memset(&rpack, 0, sizeof(rpack));
		rpack.head.firstflag = ifirst;   
		
		if( ifirst )
		{
			rpack.head.nextflag = 0; 
			rpack.head.recCount = 1;
			ifirst = 0 ;
		}
		else
		{
			rpack.head.nextflag = 1;
			memcpy(&rpack.head.hook,&apack.head.hook,sizeof(rpack.head.hook));
			rpack.head.hook.queuetype=apack.head.hook.queuetype;
		}

		rpack.head.RequestType = 930039; 
		SetValue(&rpack.head,&rpack.pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
		SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
		SetValue(&rpack.head,&rpack.pack, F_LVOL4, GetValue(nType));			//类别
		SetValue(&rpack.head,&rpack.pack, F_SDATE2, pDoc->m_szDeviceID);		//ID

		memset(buffer, 0, sizeof(buffer));
		
		if( !EncodeBuf(&rpack, (unsigned char*)buffer, &nLen, omsg) ) 
		{
			ConnectPool.Free(hHandle);
			fclose(fp);
			return RET_SYSERROR;
		}
		/*
		printf("send Buffer=:");
		unsigned long li;
		memcpy(&li,buffer,4);
		//for(int mmm=0;mmm<10;mmm++)
		printf("%ld\n",li);
		printf("下载黑名单,发送HADLE=%ld\n",hHandle);
		*/

		if( SendData(hHandle, iServerNo, iFunc, (char*)buffer, nLen, 1, FALSE) != RET_OK ) 
		{
			ConnectPool.Free(hHandle);
			fclose(fp);
			return RET_SYSERROR;
		}
L1:
		nLen = sizeof(buffer);
		memset(buffer, 0, sizeof(buffer));
		
		//printf("下载黑名单,发送HADLE=%ld\n",hHandle);
		

		int nlen = RecvData(hHandle, buffer, nLen, 1000);
		/*printf("接受HANDLE=%ld\n",hHandle);

		printf("Recv Buffer=:");
		memcpy(&li,buffer,4);
		printf("%ld\n",li);
		*/
		if( nlen <= 0 )
		{
			ConnectPool.Free(hHandle);
			fclose(fp);
			return RET_SYSERROR;
		}

		BOOL bRet=DecodeBufWithArray((unsigned char*)buffer,nlen,&apack,apackarray,&nRow,omsg);
		printf("apack.head.RequestType = %d apack->head->nextflag=%d,apack->head->recCount=%d\n",apack.head.RequestType,apack.head.nextflag,apack.head.recCount);
		if(apack.head.RequestType != 930039)
			goto L1;
		
		if( !bRet || apack.head.retCode )
		{
			ConnectPool.Free(hHandle);
			fclose(fp);
			return RET_SYSERROR;
		}

		TSBlackCard  card;
		printf("接收到%d条黑名单!\n", nRow);

		for(int i=0; i< nRow; i++)
		{
			ZeroMemory(&card, sizeof(card));

			if( !i )
			{
				card.nCardID = apack.pack.lvol0; 
				card.nFlag = apack.pack.lvol4;
				strcpy(card.sVersion, apack.pack.sserial0);
			}
			else
			{
				card.nCardID = apackarray[i-1].lvol0;
				card.nFlag = apackarray[i-1].lvol4;
				strcpy(card.sVersion, apackarray[i-1].sserial0);
			}

			fwrite(&card, sizeof(card), 1, fp);
		}

		if( apack.head.nextflag == 0 )
			bEnd = true ;
	}

	ConnectPool.Free(hHandle);
	fclose(fp);

	return RET_OK;
}

long CInterfaceApp::ReadGSCardInfo(int nflag, TSGSJRecord *pRecord)
{
	ST_CPACK rpack;

	memset(&rpack, 0, sizeof(rpack));

	rpack.head.firstflag = 1;   
	rpack.head.nextflag = 0;    
	rpack.head.recCount = 1;    
	rpack.head.retCode = 0;  
	rpack.head.userdata = 0;

	if( nflag == 0x81 )
	{
		rpack.head.RequestType = 930040; 

	}
	else if( nflag == 0x82 )
	{
		rpack.head.RequestType = 930041; 
	}
	else 
	{
		rpack.head.RequestType = 930042; 
	}

	SetValue(&rpack.head,&rpack.pack, F_LCERT_CODE, GetValue(iSmartKey));	//前置机注册号
	SetValue(&rpack.head,&rpack.pack, F_SCUST_LIMIT2, sSmartKey);			//动态密钥
	SetValue(&rpack.head,&rpack.pack, F_LVOL5, GetValue(pRecord->nCardID));	//交易卡号
	SetValue(&rpack.head,&rpack.pack, F_SSTOCK_CODE, pRecord->szPassword);	//密码
	SetValue(&rpack.head,&rpack.pack, F_SSTATION0, pRecord->szShowID);	//显示卡号

	char buffer[8192];
	int  nLen = sizeof(buffer);
	char omsg[256];

	memset(buffer, 0, sizeof(buffer));
	if( !EncodeBuf(&rpack, (unsigned char*)buffer, &nLen, omsg) ) 
		return RET_SYSERROR;

	HANDLE hHandle = ConnectPool.Alloc();

	if( SendData(hHandle, iServerNo, iFunc, (char*)buffer, nLen, 1, FALSE) != RET_OK ) 
	{
		ConnectPool.Free(hHandle);
		return RET_SYSERROR;
	}

	ST_CPACK apack;

	memset(buffer, 0, sizeof(buffer));
	nLen = sizeof(buffer);
//modifed lina 20050402	int nlen = RecvData(hHandle, buffer, nLen, 0);
	int nlen = RecvData(hHandle, buffer, nLen, 1000);
	if( nlen <= 0 )
	{
		ConnectPool.Free(hHandle);
		return RET_SYSERROR;
	}

	BOOL bRet=DecodeBuf((unsigned char*)buffer,nlen,&apack,omsg);
	if( !bRet || apack.head.retCode )
	{
		ConnectPool.Free(hHandle);
		return RET_NG;
	}

	if( rpack.head.RequestType == 930042 ) 
	{
		strcpy(pRecord->szSerial, apack.pack.scert_no);
	}

	ConnectPool.Free(hHandle);

	return RET_OK;
}

//检测设备档案是否合法
bool CInterfaceApp::CheckSmartDocValid(TSSmartDoc *pDoc)
{
	//设备ID, 注册号, 机号, 端口号
	if( strlen(pDoc->m_szDeviceID) != 8 || 
		strlen(pDoc->m_szRegister) != 8 || 
		pDoc->m_nMachineNo < 0 || pDoc->m_nMachineNo > 255 || 
		pDoc->m_nPortCount < 0 || pDoc->m_nPortCount > 24 )
	{
		printf("pDoc->m_szDeviceID = %s \n",pDoc->m_szDeviceID);
		printf("pDoc->m_szRegister = %s \n",pDoc->m_szRegister);
		return false;
	}

	//if( strcmp(pDoc->m_szMacCard, "64") &&  
	//	strcmp(pDoc->m_szMacCard, "65") && 
	//	strcmp(pDoc->m_szMacCard, "00") )
	//{
	//	return false;
	//}

	if( pDoc->m_nCommMode != 1 && pDoc->m_nCommMode != 2 && pDoc->m_nCommMode != 3 )
	{
		return false;
	}

	return true;
}


bool CInterfaceApp::CheckSmartDocValid(TSSmartDoc *pDoc, long k, long j)
{
	if( !CheckSmartDocValid(&pDoc[j]) )
		return false;

	if( j <= 0 ) return true;

	bool bBool = false ;
	bool bIP   = false;

	for(long i=0; i<k; i++)
	{
		if( i != j )
		{
			if( !strcmp(pDoc[i].m_szDeviceID, pDoc[j].m_szDeviceID) || 
				pDoc[i].m_nAuthID == pDoc[j].m_nAuthID ) 
			{
				return false;
			}

			//IP与端口不能相同
			if( !strcmp(pDoc[i].m_szAddr, pDoc[j].m_szAddr) && !strcmp(pDoc[i].m_szPort, pDoc[j].m_szPort) ) 
			{
				bIP = true;
			}

			//是否存在父设备
			if( !strcmp(pDoc[i].m_szDeviceID, pDoc[j].m_sClockVer) )
				bBool = true;
		}
	}

	if( !bBool )
	{
		if( !strcmp(pDoc[j].m_sClockVer, "" ) )
		{
			bBool = true;
		}
		//如果是LPort, 而父设备不存在, 此时认为Lport为直连方式
		else if( !strcmp(pDoc[j].m_szMacCode, "5301") )
		{
			strcpy(pDoc[j].m_sClockVer, "");
			bBool = true;
		}
	}

	return bBool;
}

void GetCurPath(LPTSTR pszDir)
{
	HANDLE  hHandle = GetCurrentProcess();
	GetModuleFileName(NULL, pszDir, MAX_PATH);
	for(int i=lstrlen(pszDir)-1; i>=0 ; i--)
	{
		if( pszDir[i] == '\\' )
		{
			pszDir[i] = 0;
			break;
		}
	}
}

