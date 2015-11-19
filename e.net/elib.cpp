#include "stdafx.h"
#include "../elib/lang.h"
#include "common.h"
#include <io.h>
#include "elib.h"

PFN_NOTIFY_SYS g_fnNotifySys = NULL;
HWND ehwnd = NULL;
string epath;

extern bool CompileILByFile(const char* path, const char* savePath);

INT WINAPI NotifySys(INT nMsg, DWORD dwParam1, DWORD dwParam2)
{
	if (g_fnNotifySys != NULL) return g_fnNotifySys(nMsg, dwParam1, dwParam2);
	else return 0;
}

INT WINAPI notify_lib(INT nMsg, DWORD dwParam1, DWORD dwParam2)
{
	switch (nMsg)
	{
	case NL_SYS_NOTIFY_FUNCTION:
		g_fnNotifySys = (PFN_NOTIFY_SYS)dwParam1;
		ehwnd = (HWND)g_fnNotifySys(NES_GET_MAIN_HWND, 0, 0);
		char path[MAX_PATH];
		g_fnNotifySys(NAS_GET_PATH, 1002, D(path));
		epath = path;
		return NR_OK;
	default:
		return NR_ERR;
	}
}

LIB_CONST_INFO s_const_info[] =
{
	{ TEXT("空"), TEXT("null"), NULL, LVL_SIMPLE, CT_NULL, NULL, NULL }
};

INT WINAPI addin_func(INT nAddInFnIndex)
{
	char title[MAX_PATH];
	GetWindowText(ehwnd, title, MAX_PATH);
	vector<string> arr = split(title, " - ");
	if (arr.size() > 2)
	{
		string path = arr[1];
		if (access(path.c_str(), 0) == 0)
		{
			switch (nAddInFnIndex)
			{
			case 0:
			{
				int index = path.find_last_of("\\");
				string dic = path.substr(0, index);
				CompileILByFile(path.c_str(), dic.c_str());
				break;
			}
			case 1:
			{
				path = epath + "gff.exe \"" + path + "\"";
				WinExec(path.c_str(), SW_SHOWNORMAL);
				break;
			}
			}
		}
	}
	return TRUE;
}

LIB_INFO s_lib_info =
{
	LIB_FORMAT_VER,
	TEXT(LI_LIB_GUID_STR),
	1,
	0,
	0,
	0,
	0,
	0,
	0,
	TEXT("易.net编译插件"),
	__GBK_LANG_VER,
	TEXT("编译易语言代码为.net程序"),
	NULL,
	TEXT("为你芯冻"),
	TEXT("100000"),
	TEXT("qq:514543271"),
	TEXT("qq:514543271"),
	TEXT("514543271"),
	TEXT("514543271@qq.com"),
	TEXT("http://wnxd.me"),
	TEXT("本插件为开源项目 https://github.com/wnxd/e.net"),
	0,
	NULL,
	1,
	TEXT("0000.net命令\0"),
	0,
	NULL,
	NULL,
	addin_func,
	TEXT("e.net编译\0编译为.net程序\0DoNET引用\0修改当前源码引用\0\0"),
	notify_lib,
	NULL,
	NULL,
	sizeof(s_const_info) / sizeof(LIB_CONST_INFO),
	s_const_info,
	NULL
};

extern "C" PLIB_INFO WINAPI GetNewInf()
{
	return &s_lib_info;
}