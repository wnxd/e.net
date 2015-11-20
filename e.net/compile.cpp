#include "stdafx.h"
#include "common.h"
#include "compile.h"

EInfo* ParseEcode(byte* code)
{
	EInfo* einfo = new EInfo;
	try
	{
		EFile_Header header;
		memcpy(&header, code, sizeof(EFile_Header));
		if (!arrcmp(header.Magic1, Magic1, 4) || !arrcmp(header.Magic2, Magic2, 4)) return false;
		long long offset = sizeof(EFile_Header);
		while (true)
		{
			ESection_Header sh = GetData<ESection_Header>(code, offset);
			if (!arrcmp(sh.Magic, Magic_Section, 4)) return false;
			ESection_Info si = GetData<ESection_Info>(code, offset);
			if (!arrcmp(si.Key, KEY, 4)) Decode_Str(si.Name, si.Key);
			char* Block_Name = (char*)si.Name;
			if (strcmp(Block_Name, "") == 0) break;
			else if (strcmp(Block_Name, "系统信息段") == 0)
			{
				long long t = offset;
				ESection_SystemInfo ssi = GetData<ESection_SystemInfo>(code, t);
				if (ssi.FileType != FileType::E || (ssi.CompileType != CompileType::WindowsForm && ssi.CompileType != CompileType::WindowsConsole && ssi.CompileType != CompileType::WindowsDLL)) throw;
				einfo->SystemInfo = ssi;
			}
			else if (strcmp(Block_Name, "用户信息段") == 0)
			{
				ESection_UserInfo sui = GetUserInfo(code + offset);
				einfo->UserInfo = sui;
			}
			else if (strcmp(Block_Name, "程序资源段") == 0)
			{

			}
			else if (strcmp(Block_Name, "程序段") == 0)
			{
				ESection_Program sp = GetLibraries(code + offset);
				einfo->Program = sp;
			}
			else if (strcmp(Block_Name, "辅助信息段2") == 0)
			{
				ESection_AuxiliaryInfo2 sai;
				UINT len = si.DataLength;
				size_t count = len / sizeof(ESection_TagStatus);
				long long t = offset;
				for (size_t i = 0; i < count; i++) sai.Tags.push_back(GetData<ESection_TagStatus>(code, t));
				einfo->TagStatus = sai;
			}
			offset += si.DataLength;
		}
		return einfo;
	}
	catch (...)
	{
		delete einfo;
		return NULL;
	}
}