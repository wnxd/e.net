#pragma once

using namespace std;

struct EInfo
{
	ESection_SystemInfo SystemInfo;
	ESection_UserInfo UserInfo;
	ESection_Program Program;
	ESection_AuxiliaryInfo2 TagStatus;
	ESection_ECList ECList;
	ESection_Resources Resources;
};

class CodeProcess
{
public:
	CodeProcess(byte* ecode, long long len);
	~CodeProcess();
	ESection_UserInfo GetUserInfo();
	ESection_SystemInfo GetSystemInfo();
	vector<ESection_Library> GetLibraries();
	vector<ESection_Program_Assembly> GetAssemblies();
	vector<ESection_Program_Assembly> GetReferAssemblies();
	vector<ESection_TagStatus> GetTagStatusList();
	vector<ESection_Program_Assembly> GetStructs();
	vector<ESection_Program_Assembly> GetReferStructs();
	vector<ESection_Program_Method> GetMethods();
	vector<ESection_Program_Method> GetReferMethods();
	vector<ESection_Variable> GetGlobalVariables();
	vector<ESection_Program_Dll> GetDllList();
	vector<ESection_ECList_Info> GetECList();
	vector<ESection_Resources_Form> GetFormList();
	ESection_Library FindLibrary(string guid, short& i);
	ETagStatus GetTagStatus(ETAG tag);
	ESection_Program_Method FindMethod(ETAG tag);
	ESection_Program_Method FindReferMethod(ETAG tag);
	ESection_Variable FindGlobalVariable(ETAG tag);
	ESection_Program_Assembly FindAssembly(ETAG tag);
	ESection_Program_Assembly FindReferAssembly(ETAG tag);
	ESection_Program_Assembly FindStruct(ETAG tag);
	ESection_Program_Assembly FindReferStruct(ETAG tag);
	PLIB_INFO FindLibInfo(UINT index);
	PLIB_INFO FindLibInfo(ESection_Library lib);
private:
	EInfo* _einfo;
};