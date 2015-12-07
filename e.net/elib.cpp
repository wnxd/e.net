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

const char* GetToolsPath()
{
	if (epath.empty())
	{
		char path[MAX_PATH];
		NotifySys(NAS_GET_PATH, 1002, D(path));
		epath = path;
	}
	return epath.c_str();
}

INT WINAPI notify_lib(INT nMsg, DWORD dwParam1, DWORD dwParam2)
{
	switch (nMsg)
	{
	case NL_SYS_NOTIFY_FUNCTION:
		g_fnNotifySys = (PFN_NOTIFY_SYS)dwParam1;
		ehwnd = (HWND)NotifySys(NES_GET_MAIN_HWND, 0, 0);
		return NR_OK;
	default:
		return NR_ERR;
	}
}

LIB_CONST_INFO s_const_info[] =
{
	{ TEXT("空"), TEXT("null"), NULL, LVL_SIMPLE, CT_NULL, NULL, NULL },
	{ TEXT("构造函数"), TEXT("ctor"), NULL, LVL_SIMPLE, CT_TEXT, TEXT(".ctor"), NULL },
	{ TEXT("静态构造函数"), TEXT("cctor"), NULL, LVL_SIMPLE, CT_TEXT, TEXT(".cctor"), NULL }
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
				char cur[MAX_PATH];
				GetCurrentDirectory(MAX_PATH, cur);
				SetCurrentDirectory(dic.c_str());
				CompileILByFile(path.c_str(), dic.c_str());
				SetCurrentDirectory(cur);
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

ARG_INFO s_ArgInfo[] =
{
#define new_Args s_ArgInfo
#define GetType_Args s_ArgInfo
	{ TEXT("类型"), NULL, 0, 0, _SDT_ALL, NULL, AS_RECEIVE_VAR },
	{ TEXT("参数"), NULL, 0, 0, _SDT_ALL, NULL, AS_DEFAULT_VALUE_IS_EMPTY | AS_RECEIVE_ALL_TYPE_DATA },
#define GetSubPtr_Args &s_ArgInfo[2]
	{ TEXT("序号"), TEXT("只能传入常量"), 0, 0, SDT_INT, NULL, 0 }
};

static CMD_INFO s_CmdInfo[] =
{
	{
		TEXT("实例化"),
		TEXT("new"),
		TEXT("根据所传参数执行对象的相应构造函数"),
		1,
		CT_ALLOW_APPEND_NEW_ARG,
		_SDT_ALL,
		NULL,
		LVL_SIMPLE,
		0,
		0,
		2,
		new_Args
	},
	{
		TEXT("取类函数"),
		TEXT("GetFuntion"),
		TEXT("当前程序集或类从上往下的序号,序号从0开始"),
		1,
		0,
		SDT_SUB_PTR,
		NULL,
		LVL_SIMPLE,
		0,
		0,
		1,
		GetSubPtr_Args
	},
	{
		TEXT("取类型"),
		TEXT("typeof"),
		NULL,
		1,
		0,
		_SDT_ALL,
		NULL,
		LVL_SIMPLE,
		0,
		0,
		1,
		GetType_Args
	}
};

static LIB_INFO s_lib_info =
{
	LIB_FORMAT_VER,
	TEXT(LI_LIB_GUID_STR),
	MAJOR,
	MINOR,
	BUILD,
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
	sizeof(s_CmdInfo) / sizeof(CMD_INFO),
	s_CmdInfo,
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