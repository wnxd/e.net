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
		byte* ptr = NULL;
		bool isai = false;
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
			else if (strcmp(Block_Name, "用户信息段") == 0) einfo->UserInfo = GetUserInfo(code + offset);
			else if (strcmp(Block_Name, "程序资源段") == 0) einfo->Resources = GetResources(code + offset);
			else if (strcmp(Block_Name, "程序段") == 0)
			{
				if (isai) einfo->Program = GetLibraries(code + offset, einfo->TagStatus.Tags);
				else ptr = code + offset;
			}
			else if (strcmp(Block_Name, "辅助信息段2") == 0)
			{
				ESection_AuxiliaryInfo2 sai;
				UINT len = si.DataLength;
				size_t count = len / sizeof(ESection_TagStatus);
				long long t = offset;
				for (size_t i = 0; i < count; i++) sai.Tags.push_back(GetData<ESection_TagStatus>(code, t));
				einfo->TagStatus = sai;
				isai = true;
				if (ptr != NULL)
				{
					einfo->Program = GetLibraries(ptr, sai.Tags);
					ptr = NULL;
				}
			}
			else if (strcmp(Block_Name, "易模块记录段") == 0) einfo->ECList = GetECList(code + offset);
			else if (strcmp(Block_Name, "辅助信息段1") == 0)
			{
				ESection_AuxiliaryInfo1 sai;
				UINT len = si.DataLength;
				size_t count = len / sizeof(ESection_UnitInfo);
				long long t = offset;
				for (size_t i = 0; i < count; i++) sai.UnitInfos.push_back(GetData<ESection_UnitInfo>(code, t));
				einfo->UnitInfo = sai;
			}
			offset += si.DataLength;
		}
		if (ptr != NULL) einfo->Program = GetLibraries(ptr, einfo->TagStatus.Tags);
		return einfo;
	}
	catch (...)
	{
		delete einfo;
		return NULL;
	}
}

template<typename T> extern T FindInfo(vector<T> list, ETAG tag);

CodeProcess::CodeProcess(byte* ecode, long long len)
{
	this->_einfo = ParseEcode(ecode);
}

CodeProcess::~CodeProcess()
{
	if (this->_einfo != NULL) delete this->_einfo;
}

ESection_UserInfo CodeProcess::GetUserInfo()
{
	return this->_einfo->UserInfo;
}

ESection_SystemInfo CodeProcess::GetSystemInfo()
{
	return this->_einfo->SystemInfo;
}

vector<ESection_Library> CodeProcess::GetLibraries()
{
	return this->_einfo->Program.Libraries;
}

vector<ESection_Program_Assembly> CodeProcess::GetAssemblies()
{
	return this->_einfo->Program.Assemblies;
}

vector<ESection_Program_Assembly> CodeProcess::GetReferAssemblies()
{
	return this->_einfo->Program.ReferAssemblies;
}

vector<ESection_TagStatus> CodeProcess::GetTagStatusList()
{
	return this->_einfo->TagStatus.Tags;
}

vector<ESection_Program_Assembly> CodeProcess::GetStructs()
{
	return this->_einfo->Program.Structs;
}

vector<ESection_Program_Assembly> CodeProcess::GetReferStructs()
{
	return this->_einfo->Program.ReferStructs;
}

vector<ESection_Program_Method> CodeProcess::GetMethods()
{
	return this->_einfo->Program.Methods;
}

vector<ESection_Program_Method> CodeProcess::GetReferMethods()
{
	return this->_einfo->Program.ReferMethods;
}

vector<ESection_Variable> CodeProcess::GetGlobalVariables()
{
	return this->_einfo->Program.GlobalVariables;
}

vector<ESection_Program_Dll> CodeProcess::GetDllList()
{
	return this->_einfo->Program.Dlls;
}

vector<ESection_ECList_Info> CodeProcess::GetECList()
{
	return this->_einfo->ECList.List;
}

vector<ESection_Resources_Form> CodeProcess::GetFormList()
{
	return this->_einfo->Resources.Forms;
}

ESection_Library CodeProcess::FindLibrary(string guid, short& i)
{
	vector<ESection_Library> libraries = this->GetLibraries();
	size_t len = libraries.size();
	for (i = 0; i < len; i++) if (stricmp(libraries[i].Guid.c_str(), guid.c_str())) return libraries[i];
	i = -1;
	return NULL;
}

ETagStatus CodeProcess::GetTagStatus(ETAG tag)
{
	ESection_TagStatus tagstatus = FindInfo(this->GetTagStatusList(), tag);
	if (tagstatus == NULL) return ETagStatus::C_None;
	return tagstatus.Status;
}

ESection_Program_Method CodeProcess::FindMethod(ETAG tag)
{
	return FindInfo(this->GetMethods(), tag);
}

ESection_Program_Method CodeProcess::FindReferMethod(ETAG tag)
{
	return FindInfo(this->GetReferMethods(), tag);
}

ESection_Variable CodeProcess::FindGlobalVariable(ETAG tag)
{
	return FindInfo(this->GetGlobalVariables(), tag);
}

ESection_Program_Assembly CodeProcess::FindAssembly(ETAG tag)
{
	return FindInfo(this->GetAssemblies(), tag);
}

ESection_Program_Assembly CodeProcess::FindReferAssembly(ETAG tag)
{
	return FindInfo(this->GetReferAssemblies(), tag);
}

ESection_Program_Assembly CodeProcess::FindStruct(ETAG tag)
{
	return FindInfo(this->GetStructs(), tag);
}

ESection_Program_Assembly CodeProcess::FindReferStruct(ETAG tag)
{
	return FindInfo(this->GetReferStructs(), tag);
}

vector<ESection_UnitInfo> CodeProcess::FindUnitInfo(ETAG element)
{
	vector<ESection_UnitInfo> arr;
	for each (ESection_UnitInfo info in this->_einfo->UnitInfo.UnitInfos) if (info.Element == element) arr.push_back(info);
	return arr;
}

ESection_Resources_Form CodeProcess::FindForm(ETAG element)
{
	for each (ESection_Resources_Form form in this->GetFormList()) if (FindInfo(form.Elements, element) != NULL) return form;
	return NULL;
}

PLIB_INFO CodeProcess::FindLibInfo(UINT index)
{
	return this->FindLibInfo(this->GetLibraries()[index]);
}

PLIB_INFO CodeProcess::FindLibInfo(ESection_Library lib)
{
	if (lib != NULL)
	{
		string libname = lib.FileName + ".fne";
		HMODULE module = GetModuleHandle(libname.c_str());
		if (module != NULL)
		{
			PFN_GET_LIB_INFO GetNewInf = (PFN_GET_LIB_INFO)GetProcAddress(module, FUNCNAME_GET_LIB_INFO);
			if (GetNewInf != NULL) return GetNewInf();
		}
	}
	return NULL;
}