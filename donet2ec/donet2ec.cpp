#include "stdafx.h"
#include <fstream>
#include <strstream>
#include "efs.h"
#include "donet2ec.h"

using namespace std;
using namespace System;
using namespace System::IO;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;

#define typeof(type) type::typeid
#define C(lpbyte) (char*)lpbyte
#define CStr2String(cstr) Marshal::PtrToStringAnsi((IntPtr)((void*)cstr.c_str()))
#define String2LPSTR(str) (char*)(void*)Marshal::StringToHGlobalAnsi(str)

#define KRNLN "krnln\rd09f2340818511d396f6aaf844c7e325\r3\r9\r系统核心支持库"

const int null = NULL;
const int _true = TRUE;
const int not = -1;
const BindingFlags BINDING_STATIC = BindingFlags::Public | BindingFlags::Static | BindingFlags::DeclaredOnly;
const BindingFlags BINDING_INSTANCE = BindingFlags::Public | BindingFlags::Instance | BindingFlags::DeclaredOnly;

int tagID = 65536;

static ref class Global
{
public:
	static Dictionary<Type^, UINT>^ typeList = gcnew Dictionary<Type^, UINT>();
};

TreeInfo::TreeInfo()
{

}

TreeInfo::TreeInfo(TreeType type)
{
	this->Type = type;
	this->Member = gcnew Dictionary<String^, TreeInfo^>();
}

TreeInfo^ AddTreeInfo(Dictionary<String^, TreeInfo^>^ tree, Type^ type)
{
	Dictionary<String^, TreeInfo^>^ member = tree;
	if (!String::IsNullOrEmpty(type->Namespace))
	{
		array<String^>^ arr = type->Namespace->Split('.');
		for each (String^ item in arr)
		{
			if (!member->ContainsKey(item)) member->Add(item, gcnew TreeInfo(TreeType::NameSpace));
			member = member[item]->Member;
		}
	}
	if (!member->ContainsKey(type->Name)) member->Add(type->Name, gcnew TreeInfo(TreeType::ClassName));
	return member[type->Name];
}

String^ Join(String^ sp, ...array<String^>^ arr)
{
	String^ r = String::Empty;
	for each (String^ item in arr)
	{
		if (r != String::Empty) r += sp;
		r += item;
	}
	return r;
}

int GetTagID()
{
	return tagID++;
}

int GetCheckValue(void* data, int len)
{
	byte* lpdata = (byte*)data;
	byte key[4] = { 0x00, 0x00, 0x00, 0x00 };
	for (size_t i = 0; i < len; i++) key[i % 4] = key[i % 4] ^ lpdata[i];
	int val;
	memcpy(&val, key, 4);
	return val;
}

DataType Type2EDT(Type^ type, EVariableAttr& attr)
{
	if (type == nullptr)
	{
		type = typeof(Object);
		if (Global::typeList->ContainsKey(type)) return (DataType)Global::typeList[type];
		else return DataType::EDT_ALL;
	}
	if (type->IsByRef)
	{
		attr = EVariableAttr::Out;
		type = type->GetElementType();
	}
	else attr = EVariableAttr::None;
	if (type->IsArray)
	{
		attr = (EVariableAttr)((USHORT)attr + (USHORT)EVariableAttr::Array);
		type = type->GetElementType();
	}
	if (Global::typeList->ContainsKey(type)) return (DataType)Global::typeList[type];
	else if (type == typeof(String)) return DataType::EDT_TEXT;
	else if (type == typeof(bool)) return DataType::EDT_BOOL;
	else if (type == typeof(int)) return DataType::EDT_INT;
	else if (type == typeof(Int64)) return DataType::EDT_LONG;
	else if (type == typeof(short)) return DataType::EDT_SHORT;
	else if (type == typeof(byte)) return DataType::EDT_BYTE;
	else if (type == typeof(float)) return DataType::EDT_FLOAT;
	else if (type == typeof(double)) return DataType::EDT_DOUBLE;
	else if (type == typeof(DateTime)) return DataType::EDT_DATETIME;
	else if (typeof(Delegate)->IsAssignableFrom(type)) return DataType::EDT_SUBPTR;
	else if (type == typeof(void)) return DataType::EDT_VOID;
	else return DataType::EDT_ALL;
}

UINT WriteGlobalVariable(strstream& globalvarhead, strstream& globalvaroffset, strstream& globalvardata, char* name, UINT type, char* remark = NULL)
{
	ETAG tag = GetTagID();
	tag.Type2 = ETYPE::GlobalField;
	globalvarhead.write(C(&tag), 4);
	int offset = globalvardata.pcount();
	globalvaroffset.write(C(&offset), 4);
	strstream vdata(0, 0, strstream::out | strstream::binary);
	vdata.write(C(&type), 4);
	EVariableAttr attr = EVariableAttr::Public;
	vdata.write(C(&attr), 2);
	vdata.write(C(&null), 1);
	int len = strlen(name);
	vdata.write(name, len + 1);
	if (remark == NULL) vdata.write(C(&null), 1);
	else
	{
		len = strlen(remark);
		vdata.write(remark, len + 1);
	}
	strstreambuf* tbuff = vdata.rdbuf();
	len = tbuff->pcount();
	globalvardata.write(C(&len), 4);
	globalvardata.write(tbuff->str(), len);
	return tag;
}

UINT WriteTree(strstream& structhead, strstream& structoffset, strstream& structdata, String^ name, Dictionary<String^, TreeInfo^>^ tree)
{
	strstream paramhead1(0, 0, strstream::out | strstream::binary);
	strstream paramhead2(0, 0, strstream::out | strstream::binary);
	strstream paramdata(0, 0, strstream::out | strstream::binary);
	strstreambuf* tbuff;
	int offset;
	char* lpstr;
	int len;
	for each (KeyValuePair<String^, TreeInfo^>^ item in tree)
	{
		ETAG ptag = GetTagID();
		ptag.Type2 = ETYPE::Variable;
		paramhead1.write(C(&ptag), 4);
		offset = paramdata.pcount();
		paramhead2.write(C(&offset), 4);
		strstream pdata(0, 0, strstream::out | strstream::binary);
		UINT tag;
		switch (item->Value->Type)
		{
		case TreeType::NameSpace:
			if (item->Value->Member == nullptr || item->Value->Member->Count == 0) tag = DataType::EDT_VOID;
			else tag = WriteTree(structhead, structoffset, structdata, name + "." + item->Key, item->Value->Member);
			break;
		case TreeType::ClassName:
			tag = item->Value->Tag;
			break;
		}
		pdata.write(C(&tag), 4);
		pdata.write(C(&null), 2);
		pdata.write(C(&null), 1);
		lpstr = String2LPSTR(item->Key);
		len = strlen(lpstr);
		pdata.write(lpstr, len + 1);
		pdata.write(C(&null), 1);
		tbuff = pdata.rdbuf();
		len = tbuff->pcount();
		paramdata.write(C(&len), 4);
		paramdata.write(tbuff->str(), len);
	}
	ETAG etag = GetTagID();
	etag.Type2 = ETYPE::Struct;
	structhead.write(C(&etag), 4);
	offset = structdata.pcount();
	structoffset.write(C(&offset), 4);
	structdata.write(C(&_true), 4);
	lpstr = String2LPSTR(name);
	len = strlen(lpstr);
	structdata.write(C(&len), 4);
	structdata.write(lpstr, len);
	lpstr = DONET_NAMESPACE;
	len = strlen(lpstr);
	structdata.write(C(&len), 4);
	structdata.write(lpstr, len);
	len = tree->Keys->Count;
	structdata.write(C(&len), 4);
	len = paramhead1.pcount() + paramhead2.pcount() + paramdata.pcount();
	structdata.write(C(&len), 4);
	tbuff = paramhead1.rdbuf();
	structdata.write(tbuff->str(), tbuff->pcount());
	tbuff = paramhead2.rdbuf();
	structdata.write(tbuff->str(), tbuff->pcount());
	tbuff = paramdata.rdbuf();
	structdata.write(tbuff->str(), tbuff->pcount());
	return etag;
}

void WriteHeader(fstream& fs)
{
	fs.write(C(Magic1), 4);
	fs.write(C(Magic2), 4);
}

void WriteSectionHeader(fstream& fs)
{
	fs.write(C(Magic_Section), 4);
}

void WriteSystemInfo(fstream& fs, int index)
{
	WriteSectionHeader(fs);
	ESection_Info esi;
	FillMemory(&esi, sizeof(ESection_Info), 0);
	const byte key[4] = { 0x19, 0x73, 0x00, 0x02 };
	memcpy(esi.Key, key, 4);
	memcpy(esi.Name, "系统信息段", 11);
	Decode_Str(esi.Name, key);
	esi.Index = index;
	esi.DataLength = sizeof(ESection_SystemInfo);
	ESection_SystemInfo essi =
	{
		5,
		0,
		1,
		1,
		1,
		FileType::EC,
		0,
		CompileType::WindowsEC
	};
	FillMemory(essi.Unknow_5, sizeof(essi.Unknow_5), 0);
	esi.Data_CheckSum = GetCheckValue(&essi, sizeof(ESection_SystemInfo));
	UINT Info_CheckSum = GetCheckValue(&esi, sizeof(ESection_Info));
	fs.write(C(&Info_CheckSum), 4);
	fs.write(C(&esi), sizeof(ESection_Info));
	fs.write(C(&essi), sizeof(ESection_SystemInfo));
}

void WriteUserInfo(fstream& fs, int index, string name)
{
	WriteSectionHeader(fs);
	ESection_Info esi;
	FillMemory(&esi, sizeof(ESection_Info), 0);
	const byte key[4] = { 0x19, 0x73, 0x00, 0x01 };
	memcpy(esi.Key, key, 4);
	memcpy(esi.Name, "用户信息段", 11);
	Decode_Str(esi.Name, key);
	esi.Index = index;
	int len = name.length();
	esi.DataLength = len + 4 * 15 + 28;
	byte* data = new byte[esi.DataLength];
	FillMemory(data, esi.DataLength, 0);
	memcpy(data, &len, 4);
	memcpy(data + 4, name.c_str(), len);
	memcpy(data + len + 4 * 15, "e.net", 6);
	esi.Data_CheckSum = GetCheckValue(data, esi.DataLength);
	UINT Info_CheckSum = GetCheckValue(&esi, sizeof(ESection_Info));
	fs.write(C(&Info_CheckSum), 4);
	fs.write(C(&esi), sizeof(ESection_Info));
	fs.write(C(data), esi.DataLength);
	delete data;
}

void WriteProgramResources(fstream& fs, int index)
{
	WriteSectionHeader(fs);
	ESection_Info esi;
	FillMemory(&esi, sizeof(ESection_Info), 0);
	const byte key[4] = { 0x19, 0x73, 0x00, 0x04 };
	memcpy(esi.Key, key, 4);
	memcpy(esi.Name, "程序资源段", 11);
	Decode_Str(esi.Name, key);
	esi.Index = index;
	esi.DataLength = 16;
	byte* data = new byte[esi.DataLength];
	FillMemory(data, esi.DataLength, 0);
	esi.Data_CheckSum = GetCheckValue(data, esi.DataLength);
	UINT Info_CheckSum = GetCheckValue(&esi, sizeof(ESection_Info));
	fs.write(C(&Info_CheckSum), 4);
	fs.write(C(&esi), sizeof(ESection_Info));
	fs.write(C(data), esi.DataLength);
	delete data;
}

void WriteProgramInfo(fstream& fs, int index, array<Type^>^ types, vector<ETAG>& classlist)
{
	WriteSectionHeader(fs);
	ESection_Info esi;
	FillMemory(&esi, sizeof(ESection_Info), 0);
	const byte key[4] = { 0x19, 0x73, 0x00, 0x03 };
	memcpy(esi.Key, key, 4);
	memcpy(esi.Name, "程序段", 7);
	Decode_Str(esi.Name, key);
	esi.Index = index;
	strstream stream(0, 0, strstream::out | strstream::binary);
	stream.write(C(&null), 4);
	stream.write(C(&null), 4);
	const byte unk1[8] = { 0x04, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00 };
	stream.write(C(unk1), 8);
	const byte unk2[6] = { 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 };
	stream.write(C(unk2), 6);
	stream.write(C(unk2), 6);
	short lib = 1;
	stream.write(C(&lib), 2);
	int len = strlen(KRNLN);
	stream.write(C(&len), 4);
	stream.write(KRNLN, len);
	stream.write(C(&null), 4);
	stream.write(C(&null), 4);
	stream.write(C(&null), 4);
	stream.write(C(&null), 4);
	strstream typehead1(0, 0, strstream::out | strstream::binary);
	strstream typehead2(0, 0, strstream::out | strstream::binary);
	strstream typedata(0, 0, strstream::out | strstream::binary);
	strstream methodhead1(0, 0, strstream::out | strstream::binary);
	strstream methodhead2(0, 0, strstream::out | strstream::binary);
	strstream methoddata(0, 0, strstream::out | strstream::binary);
	strstream globalvarhead1(0, 0, strstream::out | strstream::binary);
	strstream globalvarhead2(0, 0, strstream::out | strstream::binary);
	strstream globalvardata(0, 0, strstream::out | strstream::binary);
	strstream structhead1(0, 0, strstream::out | strstream::binary);
	strstream structhead2(0, 0, strstream::out | strstream::binary);
	strstream structdata(0, 0, strstream::out | strstream::binary);
	List<TypeOffsetInfo^>^ typeoffsetList = gcnew List<TypeOffsetInfo^>();
	List<TypeOffsetInfo^>^ structoffsetList = gcnew List<TypeOffsetInfo^>();
	List<TypeOffsetInfo^>^ offsetList = gcnew List<TypeOffsetInfo^>();
	Dictionary<String^, TreeInfo^>^ tree = gcnew Dictionary<String^, TreeInfo^>();
	for each (Type^ T in types)
	{
		if (T->IsPublic && !T->IsGenericType)
		{
			if (T->IsClass)
			{
				ETAG etag = GetTagID();
				etag.Type2 = ETYPE::Class;
				TreeInfo^ info = AddTreeInfo(tree, T);
				info->Tag = etag;
				classlist.push_back(etag);
				typehead1.write(C(&etag), 4);
				int offset = typedata.pcount();
				typehead2.write(C(&offset), 4);
				typedata.write(C(&_true), 4);
				typedata.write(C(&not), 4);
				String^ name;
				char* lpstr = "\1";
				len = strlen(lpstr);
				typedata.write(C(&len), 4);
				typedata.write(lpstr, len);
				name = Join(SP, DONET_CLASS, T->FullName);
				lpstr = String2LPSTR(name);
				len = strlen(lpstr);
				typedata.write(C(&len), 4);
				typedata.write(lpstr, len);
				array<MethodInfo^>^ mlist = T->GetMethods(BINDING_STATIC);
				len = mlist->Length * 4;
				typedata.write(C(&len), 4);
				Dictionary<String^, int>^ mnmap = gcnew Dictionary<String^, int>();
				for each (MethodInfo^ mi in mlist)
				{
					ETAG mtag = GetTagID();;
					mtag.Type2 = ETYPE::Method;
					typedata.write(C(&mtag), 4);
					methodhead1.write(C(&mtag), 4);
					offset = methoddata.pcount();
					methodhead2.write(C(&offset), 4);
					methoddata.write(C(&etag), 4);
					len = EMethodAttr::Public;
					methoddata.write(C(&len), 4);
					TypeOffsetInfo^ toi = gcnew TypeOffsetInfo();
					toi->type = mi->ReturnType;
					toi->offset = methoddata.pcount();
					toi->mode = TypeOffsetInfoMode::Method;
					offsetList->Add(toi);
					methoddata.write(C(&null), 4);
					name = mi->Name;
					if (mnmap->ContainsKey(name))
					{
						int i = mnmap[name] + 1;
						mnmap[name] = i;
						name += "_" + i;
					}
					else mnmap->Add(name, 0);
					lpstr = String2LPSTR(name);
					len = strlen(lpstr);
					methoddata.write(C(&len), 4);
					methoddata.write(lpstr, len);
					name = Join(SP, DONET, mi->MetadataToken.ToString());
					lpstr = String2LPSTR(name);
					len = strlen(lpstr);
					methoddata.write(C(&len), 4);
					methoddata.write(lpstr, len);
					methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					array<ParameterInfo^>^ plist = mi->GetParameters();
					len = plist->Length;
					methoddata.write(C(&len), 4);
					if (len > 0)
					{
						strstream paramhead1(0, 0, strstream::out | strstream::binary);
						strstream paramhead2(0, 0, strstream::out | strstream::binary);
						strstream paramdata(0, 0, strstream::out | strstream::binary);
						strstreambuf* tbuff;
						List<TypeOffsetInfo^>^ toffset = gcnew List<TypeOffsetInfo^>();
						for each (ParameterInfo^ param in plist)
						{
							ETAG ptag = GetTagID();
							ptag.Type2 = ETYPE::Variable;
							paramhead1.write(C(&ptag), 4);
							offset = paramdata.pcount();
							paramhead2.write(C(&offset), 4);
							strstream pdata(0, 0, strstream::out | strstream::binary);
							toi = gcnew TypeOffsetInfo();
							toi->type = param->ParameterType;
							toi->offset = paramdata.pcount() + pdata.pcount() + 4;
							toi->mode = TypeOffsetInfoMode::Param;
							toffset->Add(toi);
							pdata.write(C(&null), 4);
							pdata.write(C(&null), 2);
							pdata.write(C(&null), 1);
							name = param->Name;
							lpstr = String2LPSTR(name);
							len = strlen(lpstr);
							pdata.write(lpstr, len + 1);
							pdata.write(C(&null), 1);
							tbuff = pdata.rdbuf();
							len = tbuff->pcount();
							paramdata.write(C(&len), 4);
							paramdata.write(tbuff->str(), len);
						}
						len = paramhead1.pcount() + paramhead2.pcount() + paramdata.pcount();
						methoddata.write(C(&len), 4);
						tbuff = paramhead1.rdbuf();
						methoddata.write(tbuff->str(), tbuff->pcount());
						tbuff = paramhead2.rdbuf();
						methoddata.write(tbuff->str(), tbuff->pcount());
						len = methoddata.pcount();
						for each (TypeOffsetInfo^ item in toffset) item->offset += len;
						offsetList->AddRange(toffset);
						tbuff = paramdata.rdbuf();
						methoddata.write(tbuff->str(), tbuff->pcount());
					}
					else methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					const byte nop[19] = { 0x6A, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x01 };
					len = 19;
					methoddata.write(C(&len), 4);
					methoddata.write(C(nop), len);
				}
				typedata.write(C(&null), 4);
				typedata.write(C(&null), 4);
				etag = GetTagID();
				etag.Type2 = ETYPE::Class;
				classlist.push_back(etag);
				Global::typeList->Add(T, etag);
				typehead1.write(C(&etag), 4);
				offset = typedata.pcount();
				typehead2.write(C(&offset), 4);
				typedata.write(C(&_true), 4);
				TypeOffsetInfo^ toi = gcnew TypeOffsetInfo();
				toi->type = T->BaseType;
				toi->offset = typedata.pcount();
				toi->mode = TypeOffsetInfoMode::Method;
				typeoffsetList->Add(toi);
				typedata.write(C(&null), 4);
				name = T->FullName;
				lpstr = String2LPSTR(name);
				len = strlen(lpstr);
				typedata.write(C(&len), 4);
				typedata.write(lpstr, len);
				lpstr = DONET;
				len = strlen(lpstr);
				typedata.write(C(&len), 4);
				typedata.write(lpstr, len);
				array<ConstructorInfo^>^ clist = T->GetConstructors(BINDING_INSTANCE);
				mlist = T->GetMethods(BINDING_INSTANCE);
				len = (clist->Length + mlist->Length) * 4;
				typedata.write(C(&len), 4);
				mnmap = gcnew Dictionary<String^, int>();
				for each (ConstructorInfo^ ci in clist)
				{
					ETAG mtag = GetTagID();
					mtag.Type2 = ETYPE::Method;
					typedata.write(C(&mtag), 4);
					methodhead1.write(C(&mtag), 4);
					offset = methoddata.pcount();
					methodhead2.write(C(&offset), 4);
					methoddata.write(C(&etag), 4);
					len = EMethodAttr::Public;
					methoddata.write(C(&len), 4);
					methoddata.write(C(&null), 4);
					name = T->Name;
					if (mnmap->ContainsKey(name))
					{
						int i = mnmap[name] + 1;
						mnmap[name] = i;
						name += "_" + i;
					}
					else mnmap->Add(name, 0);
					lpstr = String2LPSTR(name);
					len = strlen(lpstr);
					methoddata.write(C(&len), 4);
					methoddata.write(lpstr, len);
					name = Join(SP, DONET, ci->MetadataToken.ToString());
					lpstr = String2LPSTR(name);
					len = strlen(lpstr);
					methoddata.write(C(&len), 4);
					methoddata.write(lpstr, len);
					methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					array<ParameterInfo^>^ plist = ci->GetParameters();
					len = plist->Length;
					methoddata.write(C(&len), 4);
					if (len > 0)
					{
						strstream paramhead1(0, 0, strstream::out | strstream::binary);
						strstream paramhead2(0, 0, strstream::out | strstream::binary);
						strstream paramdata(0, 0, strstream::out | strstream::binary);
						strstreambuf* tbuff;
						List<TypeOffsetInfo^>^ toffset = gcnew List<TypeOffsetInfo^>();
						for each (ParameterInfo^ param in plist)
						{
							ETAG ptag = GetTagID();
							ptag.Type2 = ETYPE::Variable;
							paramhead1.write(C(&ptag), 4);
							offset = paramdata.pcount();
							paramhead2.write(C(&offset), 4);
							strstream pdata(0, 0, strstream::out | strstream::binary);
							toi = gcnew TypeOffsetInfo();
							toi->type = param->ParameterType;
							toi->offset = paramdata.pcount() + pdata.pcount() + 4;
							toi->mode = TypeOffsetInfoMode::Param;
							toffset->Add(toi);
							pdata.write(C(&null), 4);
							pdata.write(C(&null), 2);
							pdata.write(C(&null), 1);
							name = param->Name;
							lpstr = String2LPSTR(name);
							len = strlen(lpstr);
							pdata.write(lpstr, len + 1);
							pdata.write(C(&null), 1);
							tbuff = pdata.rdbuf();
							len = tbuff->pcount();
							paramdata.write(C(&len), 4);
							paramdata.write(tbuff->str(), len);
						}
						len = paramhead1.pcount() + paramhead2.pcount() + paramdata.pcount();
						methoddata.write(C(&len), 4);
						tbuff = paramhead1.rdbuf();
						methoddata.write(tbuff->str(), tbuff->pcount());
						tbuff = paramhead2.rdbuf();
						methoddata.write(tbuff->str(), tbuff->pcount());
						len = methoddata.pcount();
						for each (TypeOffsetInfo^ item in toffset) item->offset += len;
						offsetList->AddRange(toffset);
						tbuff = paramdata.rdbuf();
						methoddata.write(tbuff->str(), tbuff->pcount());
					}
					else methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					const byte nop[19] = { 0x6A, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x01 };
					len = 19;
					methoddata.write(C(&len), 4);
					methoddata.write(C(nop), len);
				}
				for each (MethodInfo^ mi in mlist)
				{
					ETAG mtag = GetTagID();
					mtag.Type2 = ETYPE::Method;
					typedata.write(C(&mtag), 4);
					methodhead1.write(C(&mtag), 4);
					offset = methoddata.pcount();
					methodhead2.write(C(&offset), 4);
					methoddata.write(C(&etag), 4);
					len = EMethodAttr::Public;
					methoddata.write(C(&len), 4);
					toi = gcnew TypeOffsetInfo();
					toi->type = mi->ReturnType;
					toi->offset = methoddata.pcount();
					toi->mode = TypeOffsetInfoMode::Method;
					offsetList->Add(toi);
					methoddata.write(C(&null), 4);
					name = mi->Name;
					if (mnmap->ContainsKey(name))
					{
						int i = mnmap[name];
						name += "_" + (i++).ToString();
						mnmap[name] = i;
					}
					else mnmap->Add(name, 1);
					lpstr = String2LPSTR(name);
					len = strlen(lpstr);
					methoddata.write(C(&len), 4);
					methoddata.write(C(lpstr), len);
					name = Join(SP, DONET, mi->MetadataToken.ToString());
					lpstr = String2LPSTR(name);
					len = strlen(lpstr);
					methoddata.write(C(&len), 4);
					methoddata.write(C(lpstr), len);
					methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					array<ParameterInfo^>^ plist = mi->GetParameters();
					len = plist->Length;
					methoddata.write(C(&len), 4);
					if (len > 0)
					{
						strstream paramhead1(0, 0, strstream::out | strstream::binary);
						strstream paramhead2(0, 0, strstream::out | strstream::binary);
						strstream paramdata(0, 0, strstream::out | strstream::binary);
						strstreambuf* tbuff;
						List<TypeOffsetInfo^>^ toffset = gcnew List<TypeOffsetInfo^>();
						for each (ParameterInfo^ param in plist)
						{
							ETAG ptag = GetTagID();
							ptag.Type2 = ETYPE::Variable;
							paramhead1.write(C(&ptag), 4);
							offset = paramdata.pcount();
							paramhead2.write(C(&offset), 4);
							strstream pdata(0, 0, strstream::out | strstream::binary);
							toi = gcnew TypeOffsetInfo();
							toi->type = param->ParameterType;
							toi->offset = paramdata.pcount() + pdata.pcount() + 4;
							toi->mode = TypeOffsetInfoMode::Param;
							toffset->Add(toi);
							pdata.write(C(&null), 4);
							pdata.write(C(&null), 2);
							pdata.write(C(&null), 1);
							name = param->Name;
							lpstr = String2LPSTR(name);
							len = strlen(lpstr);
							pdata.write(lpstr, len + 1);
							pdata.write(C(&null), 1);
							tbuff = pdata.rdbuf();
							len = tbuff->pcount();
							paramdata.write(C(&len), 4);
							paramdata.write(tbuff->str(), len);
						}
						len = paramhead1.pcount() + paramhead2.pcount() + paramdata.pcount();
						methoddata.write(C(&len), 4);
						tbuff = paramhead1.rdbuf();
						methoddata.write(tbuff->str(), tbuff->pcount());
						tbuff = paramhead2.rdbuf();
						methoddata.write(tbuff->str(), tbuff->pcount());
						len = methoddata.pcount();
						for each (TypeOffsetInfo^ item in toffset) item->offset += len;
						offsetList->AddRange(toffset);
						tbuff = paramdata.rdbuf();
						methoddata.write(tbuff->str(), tbuff->pcount());
					}
					else methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					methoddata.write(C(&null), 4);
					const byte nop[19] = { 0x6A, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x01 };
					len = 19;
					methoddata.write(C(&len), 4);
					methoddata.write(C(nop), len);
				}
				typedata.write(C(&null), 4);
				typedata.write(C(&null), 4);
			}
			else if (T->IsValueType)
			{
				ETAG etag = GetTagID();
				etag.Type2 = ETYPE::Struct;
				classlist.push_back(etag);
				Global::typeList->Add(T, etag);
				structhead1.write(C(&etag), 4);
				int offset = structdata.pcount();
				structhead2.write(C(&offset), 4);
				structdata.write(C(&_true), 4);
				String^ name = T->FullName;
				char* lpstr = String2LPSTR(name);
				len = strlen(lpstr);
				structdata.write(C(&len), 4);
				structdata.write(lpstr, len);
				lpstr = DONET;
				len = strlen(lpstr);
				structdata.write(C(&len), 4);
				structdata.write(lpstr, len);
				array<PropertyInfo^>^ plist = T->GetProperties(BINDING_INSTANCE);
				array<FieldInfo^>^ flist = T->GetFields(BINDING_INSTANCE);
				len = plist->Length + flist->Length;
				structdata.write(C(&len), 4);
				if (len > 0)
				{
					strstream paramhead1(0, 0, strstream::out | strstream::binary);
					strstream paramhead2(0, 0, strstream::out | strstream::binary);
					strstream paramdata(0, 0, strstream::out | strstream::binary);
					strstreambuf* tbuff;
					List<TypeOffsetInfo^>^ toffset = gcnew List<TypeOffsetInfo^>();
					for each (PropertyInfo^ property in plist)
					{
						ETAG ptag = GetTagID();
						ptag.Type2 = ETYPE::Variable;
						paramhead1.write(C(&ptag), 4);
						offset = paramdata.pcount();
						paramhead2.write(C(&offset), 4);
						strstream pdata(0, 0, strstream::out | strstream::binary);
						TypeOffsetInfo^ toi = gcnew TypeOffsetInfo();
						toi->type = property->PropertyType;
						toi->offset = paramdata.pcount() + pdata.pcount() + 4;
						toi->mode = TypeOffsetInfoMode::Param;
						toffset->Add(toi);
						pdata.write(C(&null), 4);
						pdata.write(C(&null), 2);
						pdata.write(C(&null), 1);
						name = property->Name;
						lpstr = String2LPSTR(name);
						len = strlen(lpstr);
						pdata.write(lpstr, len + 1);
						pdata.write(C(&null), 1);
						tbuff = pdata.rdbuf();
						len = tbuff->pcount();
						paramdata.write(C(&len), 4);
						paramdata.write(tbuff->str(), len);
					}
					for each (FieldInfo^ field in flist)
					{
						ETAG ftag = GetTagID();
						ftag.Type2 = ETYPE::Variable;
						paramhead1.write(C(&ftag), 4);
						offset = paramdata.pcount();
						paramhead2.write(C(&offset), 4);
						strstream pdata(0, 0, strstream::out | strstream::binary);
						TypeOffsetInfo^ toi = gcnew TypeOffsetInfo();
						toi->type = field->FieldType;
						toi->offset = paramdata.pcount() + pdata.pcount() + 4;
						toi->mode = TypeOffsetInfoMode::Param;
						toffset->Add(toi);
						pdata.write(C(&null), 4);
						pdata.write(C(&null), 2);
						pdata.write(C(&null), 1);
						name = field->Name;
						lpstr = String2LPSTR(name);
						len = strlen(lpstr);
						pdata.write(lpstr, len + 1);
						pdata.write(C(&null), 1);
						tbuff = pdata.rdbuf();
						len = tbuff->pcount();
						paramdata.write(C(&len), 4);
						paramdata.write(tbuff->str(), len);
					}
					len = paramhead1.pcount() + paramhead2.pcount() + paramdata.pcount();
					structdata.write(C(&len), 4);
					tbuff = paramhead1.rdbuf();
					structdata.write(tbuff->str(), tbuff->pcount());
					tbuff = paramhead2.rdbuf();
					structdata.write(tbuff->str(), tbuff->pcount());
					len = structdata.pcount();
					for each (TypeOffsetInfo^ item in toffset) item->offset += len;
					structoffsetList->AddRange(toffset);
					tbuff = paramdata.rdbuf();
					structdata.write(tbuff->str(), tbuff->pcount());
				}
				else structdata.write(C(&null), 4);
			}
		}
	}
	for each (KeyValuePair<String^, TreeInfo^>^ item in tree)
	{
		UINT tag;
		char* type;
		switch (item->Value->Type)
		{
		case TreeType::NameSpace:
			if (item->Value->Member == nullptr || item->Value->Member->Count == 0) tag = DataType::EDT_VOID;
			else tag = WriteTree(structhead1, structhead2, structdata, "#using_" + item->Key, item->Value->Member);
			type = DONET_NAMESPACE;
			break;
		case TreeType::ClassName:
			tag = item->Value->Tag;
			type = DONET_CLASS;
			break;
		}
		WriteGlobalVariable(globalvarhead1, globalvarhead2, globalvardata, String2LPSTR(item->Key), tag, type);
	}
	strstreambuf* lpbuff = typehead1.rdbuf();
	len = lpbuff->pcount() * 2;
	stream.write(C(&len), 4);
	len = lpbuff->pcount();
	stream.write(lpbuff->str(), len);
	lpbuff = typehead2.rdbuf();
	len = lpbuff->pcount();
	stream.write(lpbuff->str(), len);
	lpbuff = typedata.rdbuf();
	len = lpbuff->pcount();
	char* buf = lpbuff->str();
	for each (TypeOffsetInfo^ item in typeoffsetList)
	{
		EVariableAttr attr;
		DataType edt = Type2EDT(item->type, attr);
		memcpy(buf + item->offset, &edt, 4);
	}
	stream.write(buf, len);
	lpbuff = methodhead1.rdbuf();
	len = lpbuff->pcount() * 2;
	stream.write(C(&len), 4);
	len = lpbuff->pcount();
	stream.write(lpbuff->str(), len);
	lpbuff = methodhead2.rdbuf();
	len = lpbuff->pcount();
	stream.write(lpbuff->str(), len);
	lpbuff = methoddata.rdbuf();
	len = lpbuff->pcount();
	buf = lpbuff->str();
	for each (TypeOffsetInfo^ item in offsetList)
	{
		EVariableAttr attr;
		DataType edt = Type2EDT(item->type, attr);
		memcpy(buf + item->offset, &edt, 4);
		if (item->mode == TypeOffsetInfoMode::Param) memcpy(buf + item->offset + 4, &attr, 2);
	}
	stream.write(buf, len);
	len = globalvarhead1.pcount() / 4;
	stream.write(C(&len), 4);
	len = globalvarhead1.pcount() + globalvarhead2.pcount() + globalvardata.pcount();
	stream.write(C(&len), 4);
	lpbuff = globalvarhead1.rdbuf();
	stream.write(lpbuff->str(), lpbuff->pcount());
	lpbuff = globalvarhead2.rdbuf();
	stream.write(lpbuff->str(), lpbuff->pcount());
	lpbuff = globalvardata.rdbuf();
	stream.write(lpbuff->str(), lpbuff->pcount());
	lpbuff = structhead1.rdbuf();
	len = lpbuff->pcount() * 2;
	stream.write(C(&len), 4);
	len = lpbuff->pcount();
	stream.write(lpbuff->str(), len);
	lpbuff = structhead2.rdbuf();
	len = lpbuff->pcount();
	stream.write(lpbuff->str(), len);
	lpbuff = structdata.rdbuf();
	len = lpbuff->pcount();
	buf = lpbuff->str();
	for each (TypeOffsetInfo^ item in structoffsetList)
	{
		EVariableAttr attr;
		DataType edt = Type2EDT(item->type, attr);
		memcpy(buf + item->offset, &edt, 4);
		if (item->mode == TypeOffsetInfoMode::Param) memcpy(buf + item->offset + 4, &attr, 2);
	}
	stream.write(buf, len);
	stream.write(C(&null), 4);
	byte data[40];
	FillMemory(data, 40, 0);
	stream.write(C(data), 40);
	lpbuff = stream.rdbuf();
	esi.DataLength = lpbuff->pcount();
	buf = lpbuff->str();
	memcpy(buf, &tagID, 4);
	esi.Data_CheckSum = GetCheckValue(buf, esi.DataLength);
	UINT Info_CheckSum = GetCheckValue(&esi, sizeof(ESection_Info));
	fs.write(C(&Info_CheckSum), 4);
	fs.write(C(&esi), sizeof(ESection_Info));
	fs.write(buf, esi.DataLength);
}

void WriteInitModule(fstream& fs, int index)
{
	WriteSectionHeader(fs);
	ESection_Info esi;
	FillMemory(&esi, sizeof(ESection_Info), 0);
	const byte key[4] = { 0x19, 0x73, 0x00, 0x08 };
	memcpy(esi.Key, key, 4);
	memcpy(esi.Name, "初始模块段", 11);
	Decode_Str(esi.Name, key);
	esi.Index = index;
	esi.DataLength = 6;
	byte* data = new byte[esi.DataLength];
	FillMemory(data, esi.DataLength, 0);
	esi.Data_CheckSum = GetCheckValue(data, esi.DataLength);
	UINT Info_CheckSum = GetCheckValue(&esi, sizeof(ESection_Info));
	fs.write(C(&Info_CheckSum), 4);
	fs.write(C(&esi), sizeof(ESection_Info));
	fs.write(C(data), esi.DataLength);
	delete data;
}

void WriteAuxiliaryInfo(fstream& fs, int index, vector<ETAG> classlist)
{
	WriteSectionHeader(fs);
	ESection_Info esi;
	FillMemory(&esi, sizeof(ESection_Info), 0);
	const byte key[4] = { 0x19, 0x73, 0x00, 0x0B };
	memcpy(esi.Key, key, 4);
	memcpy(esi.Name, "辅助信息段2", 12);
	Decode_Str(esi.Name, key);
	esi.Index = index;
	int len = classlist.size() * 8;
	esi.DataLength = len;
	byte* data = new byte[len];
	int offset = 0;
	for each (ETAG tag in classlist)
	{
		memcpy(data + offset, &tag, 4);
		offset += 4;
		memcpy(data + offset, &_true, 4);
		offset += 4;
	}
	esi.Data_CheckSum = GetCheckValue(data, len);
	UINT Info_CheckSum = GetCheckValue(&esi, sizeof(ESection_Info));
	fs.write(C(&Info_CheckSum), 4);
	fs.write(C(&esi), sizeof(ESection_Info));
	fs.write(C(data), len);
	delete data;
}

void WriteNull(fstream& fs, int index)
{
	WriteSectionHeader(fs);
	ESection_Info esi;
	FillMemory(&esi, sizeof(ESection_Info), 0);
	memcpy(esi.Key, KEY, 4);
	esi.Index = index;
	UINT Info_CheckSum = GetCheckValue(&esi, sizeof(ESection_Info));
	fs.write(C(&Info_CheckSum), 4);
	fs.write(C(&esi), sizeof(ESection_Info));
}

void Generate(string path)
{
	String^ mpath = CStr2String(path);
	if (File::Exists(mpath))
	{
		array<String^>^ list = File::ReadAllLines(mpath);
		String^ cur = Path::GetDirectoryName(mpath);
		List<Type^>^ tlist = gcnew List<Type^>();
		for each (String^ file in list)
		{
			if (Path::GetFileName(file) == file) file = cur + "\\" + file;
			Assembly^ assembly = Assembly::LoadFile(file);
			array<Type^>^ types = assembly->GetTypes();
			tlist->AddRange(types);
		}
		path = path.substr(0, path.length() - 7);
		fstream fs(path + "_refer.ec", fstream::out | fstream::binary);
		WriteHeader(fs);
		int index = 1;
		//WriteSystemInfo(fs, index);
		//index++;
		WriteUserInfo(fs, index, "DoNETRefer");
		index++;
		//WriteProgramResources(fs, index);
		//index++;
		vector<ETAG> classlist;
		WriteProgramInfo(fs, index, tlist->ToArray(), classlist);
		index++;
		//WriteInitModule(fs, index);
		//index++;
		WriteAuxiliaryInfo(fs, index, classlist);
		index++;
		WriteNull(fs, index);
		fs.close();
	}
}

int main(int argc, char* argv[])
{
	if (argc > 1) Generate(argv[1]);
	return NULL;
}