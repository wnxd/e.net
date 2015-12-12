#include "stdafx.h"
#include "common.h"

ETAG::ETAG()
{

}

ETAG::ETAG(UINT uint)
{
	memcpy(this, &uint, sizeof(UINT));
}

bool ETAG::operator==(ETAG tag)
{
	return *this == ETAG2UINT(tag);
}

bool ETAG::operator!=(ETAG tag)
{
	return *this != ETAG2UINT(tag);
}

bool ETAG::operator==(UINT uint)
{
	return ETAG2UINT((*this)) == uint;
}

bool ETAG::operator!=(UINT uint)
{
	return ETAG2UINT((*this)) != uint;
}

bool ETAG::operator==(int uint)
{
	return ETAG2UINT((*this)) == uint;
}

bool ETAG::operator!=(int uint)
{
	return ETAG2UINT((*this)) != uint;
}

ETAG::operator UINT()
{
	UINT type;
	memcpy(&type, this, sizeof(UINT));
	return type;
}

LIBTAG::LIBTAG()
{

}

LIBTAG::LIBTAG(UINT uint)
{
	memcpy(this, &uint, sizeof(UINT));
}

LIBTAG* LIBTAG::operator=(UINT uint)
{
	memcpy(this, &uint, sizeof(UINT));
	return this;
}

LIBTAG::operator UINT()
{
	UINT type;
	memcpy(&type, this, sizeof(UINT));
	return type;
}

LIBCONST::LIBCONST()
{

}

LIBCONST::LIBCONST(UINT uint)
{
	memcpy(this, &uint, sizeof(UINT));
}

LIBCONST* LIBCONST::operator=(UINT uint)
{
	memcpy(this, &uint, sizeof(UINT));
	return this;
}

LIBCONST::operator UINT()
{
	UINT type;
	memcpy(&type, this, sizeof(UINT));
	return type;
}

EKeyValPair::EKeyValPair()
{
	this->Key = NULL;
	this->Value = NULL;
}

EKeyValPair::EKeyValPair(UINT64 uint64)
{
	memcpy(this, &uint64, sizeof(UINT64));
}

EKeyValPair::operator UINT64()
{
	UINT64 uint64;
	memcpy(&uint64, this, sizeof(UINT64));
	return uint64;
}

ESection_Library::ESection_Library()
{

}

ESection_Library::ESection_Library(nullptr_t) : ESection_Library()
{

}

bool ESection_Library::operator==(nullptr_t)
{
	return this->Guid.empty();
}

bool ESection_Library::operator!=(nullptr_t)
{
	return !this->Guid.empty();
}

bool EBase::operator==(nullptr_t)
{
	return this->Tag == NULL;
}

bool EBase::operator!=(nullptr_t)
{
	return this->Tag != NULL;
}

ESection_Variable::ESection_Variable()
{
	this->Tag = NULL;
}

ESection_Variable::ESection_Variable(nullptr_t) : ESection_Variable()
{

}

ESection_Variable* ESection_Variable::operator=(nullptr_t)
{
	this->Tag = NULL;
	return this;
}

ESection_Program_Method::ESection_Program_Method()
{
	this->Tag = NULL;
}

ESection_Program_Method::ESection_Program_Method(nullptr_t) : ESection_Program_Method()
{

}

ESection_Program_Assembly::ESection_Program_Assembly()
{
	this->Tag = NULL;
}

ESection_Program_Assembly::ESection_Program_Assembly(nullptr_t) : ESection_Program_Assembly()
{

}

ESection_TagStatus::ESection_TagStatus()
{
	this->Tag = NULL;
}

ESection_TagStatus::ESection_TagStatus(nullptr_t) : ESection_TagStatus()
{

}

ESection_Variable ESection_Program_Assembly::FindField(ETAG tag)
{
	return FindInfo(this->Variables, tag);
}

string GetString(byte*& pointer, bool jump)
{
	string str;
	UINT size = GetData<UINT>(pointer);
	if (!jump) str = string((char*)pointer, size);
	pointer += size;
	return str;
}

void Decode_Str(byte data[], const byte key[])
{
	size_t key_len = 4;
	size_t len = 30;
	for (size_t i = 0, key_i = 1; i < len; i++, key_i++) data[i] = data[i] ^ key[key_i % key_len];
}

string GetAutoString(byte*& pointer, bool jump = false)
{
	string str;
	char* lpstr = (char*)pointer;
	size_t len = strlen(lpstr);
	if (!jump) str = string(lpstr, len);
	pointer += len + 1;
	return str;
}

vector<byte> GetBytes(byte*& pointer, bool jump = false)
{
	vector<byte> arr;
	UINT size = GetData<UINT>(pointer);
	if (!jump) arr = vector<byte>(pointer, pointer + size);
	pointer += size;
	return arr;
}

vector<ESection_Variable> GetVariables(byte*& pointer, bool jump = false)
{
	vector<ESection_Variable> arr;
	UINT count = GetData<UINT>(pointer);
	UINT length = GetData<UINT>(pointer);
	byte* oldptr = pointer + length;
	if (!jump)
	{
		arr = vector<ESection_Variable>(count);
		ETAG* tags = new ETAG[count];
		byte** os = new byte*[count];
		for (size_t i = 0; i < count; i++) tags[i] = GetData<ETAG>(pointer);
		byte* mptr = pointer + count * sizeof(UINT);
		for (size_t i = 0; i < count; i++) os[i] = mptr + GetData<UINT>(pointer);
		for (size_t i = 0; i < count; i++)
		{
			ESection_Variable sv;
			pointer = os[i];
			UINT size = GetData<UINT>(pointer);
			sv.Tag = tags[i];
			sv.DataType = GetData<DataType>(pointer);
			sv.Attributes = GetData<EVariableAttr>(pointer);
			sv.ArrayInfo.Dimension = GetData<byte>(pointer);
			for (size_t n = 0; n < sv.ArrayInfo.Dimension; n++) sv.ArrayInfo.Subscript.push_back(GetData<UINT>(pointer));
			sv.Name = GetAutoString(pointer);
			sv.Remark = GetAutoString(pointer);
			arr[i] = sv;
		}
		delete tags;
		delete os;
	}
	pointer = oldptr;
	return arr;
}

vector<ESection_Program_Assembly> GetAssemblies(byte*& pointer, vector<ESection_TagStatus> tagstatus, vector<ESection_Program_Assembly>* refer = NULL, bool isclass = true)
{
	vector<ESection_Program_Assembly> arr;
	UINT length = GetData<UINT>(pointer) / 2;
	size_t count = length / sizeof(ETAG);
	ETAG* tags = new ETAG[count];
	for (size_t i = 0; i < count; i++) tags[i] = GetData<ETAG>(pointer);
	pointer += length;
	for (size_t i = 0; i < count; i++)
	{
		ESection_Program_Assembly spa;
		spa.Tag = tags[i];
		if (spa.Tag.Type2 == ETYPE::WindowClass)
		{
			spa.Base = GetData<ETAG>(pointer);
			pointer += 4;
		}
		else
		{
			spa.Status = GetData<ETagStatus>(pointer);
			if (isclass) spa.Base = GetData<ETAG>(pointer);
		}
		spa.Name = GetString(pointer);
		spa.Remark = GetString(pointer);
		if (isclass)
		{
			length = GetData<UINT>(pointer);
			size_t size = length / sizeof(ETAG);
			for (size_t n = 0; n < size; n++) spa.Methods.push_back(GetData<ETAG>(pointer));
		}
		spa.Variables = GetVariables(pointer);
		if (refer != NULL)
		{
			if ((spa.Status & ETagStatus::C_Extern) == ETagStatus::C_Extern)
			{
			referadd:
				refer->push_back(spa);
			}
			else
			{
				ESection_TagStatus status = FindInfo(tagstatus, spa.Tag);
				if (status != NULL && (status.Status & ETagStatus::C_Extern) == ETagStatus::C_Extern) goto referadd;
				else goto add;
			}
		}
		else
		{
		add:
			arr.push_back(spa);
		}
	}
	delete tags;
	return arr;
}

vector<ESection_Program_Method> GetMethods(byte*& pointer, vector<ESection_Program_Method>* refer = NULL)
{
	vector<ESection_Program_Method> arr;
	UINT length = GetData<UINT>(pointer) / 2;
	size_t count = length / sizeof(ETAG);
	ETAG* tags = new ETAG[count];
	for (size_t i = 0; i < count; i++) tags[i] = GetData<ETAG>(pointer);
	pointer += length;
	for (size_t i = 0; i < count; i++)
	{
		ESection_Program_Method spm;
		spm.Tag = tags[i];
		spm.Class = GetData<ETAG>(pointer);
		spm.Attributes = GetData<EMethodAttr>(pointer);
		spm.ReturnType = GetData<DataType>(pointer);
		spm.Name = GetString(pointer);
		spm.Remark = GetString(pointer);
		spm.Variables = GetVariables(pointer);
		spm.Parameters = GetVariables(pointer);
		spm.RowsOffset = GetBytes(pointer);
		spm.Offset1 = GetBytes(pointer);
		spm.Offset2 = GetBytes(pointer);
		spm.ParameterOffset = GetBytes(pointer);
		spm.VariableOffset = GetBytes(pointer);
		spm.Code = GetBytes(pointer);
		if (refer != NULL && (spm.Attributes & EMethodAttr::M_Extern) == EMethodAttr::M_Extern) refer->push_back(spm);
		else arr.push_back(spm);
	}
	delete tags;
	return arr;
}

vector<ESection_Program_Dll> GetDlls(byte*& pointer)
{
	vector<ESection_Program_Dll> arr;
	UINT length = GetData<UINT>(pointer) / 2;
	size_t count = length / sizeof(ETAG);
	ETAG* tags = new ETAG[count];
	for (size_t i = 0; i < count; i++) tags[i] = GetData<ETAG>(pointer);
	pointer += length;
	for (size_t i = 0; i < count; i++)
	{
		ESection_Program_Dll spd;
		spd.Tag = tags[i];
		spd.Status = GetData<ETagStatus>(pointer);
		spd.ReturnType = GetData<DataType>(pointer);
		spd.ShowName = GetString(pointer);
		spd.Remark = GetString(pointer);
		spd.Lib = GetString(pointer);
		spd.Name = GetString(pointer);
		spd.Parameters = GetVariables(pointer);
		arr.push_back(spd);
	}
	delete tags;
	return arr;
}

vector<ESection_Resources_FormElement> GetElements(byte*& pointer)
{
	vector<ESection_Resources_FormElement> arr;
	UINT count = GetData<UINT>(pointer);
	UINT length = GetData<UINT>(pointer);
	byte* oldptr = pointer + length;
	ETAG* tags = new ETAG[count];
	byte** os = new byte*[count];
	for (size_t i = 0; i < count; i++) tags[i] = GetData<ETAG>(pointer);
	byte* mptr = pointer + count * sizeof(UINT);
	for (size_t i = 0; i < count; i++) os[i] = mptr + GetData<UINT>(pointer);
	for (size_t i = 0; i < count; i++)
	{
		ESection_Resources_FormElement sre;
		pointer = os[i];
		UINT size = GetData<UINT>(pointer);
		byte* end = pointer + size;
		sre.Tag = tags[i];
		sre.Type = GetData<ETAG>(pointer);
		pointer += 20;
		sre.Name = GetAutoString(pointer);
		sre.Remark = GetAutoString(pointer);
		pointer += 4;
		sre.Left = GetData<UINT>(pointer);
		sre.Top = GetData<UINT>(pointer);
		sre.Width = GetData<UINT>(pointer);
		sre.Height = GetData<UINT>(pointer);
		pointer += 12;
		sre.Cursor = GetBytes(pointer);
		sre.Mark = GetAutoString(pointer);
		pointer += 4;
		sre.Status = GetData<EElementStatus>(pointer);
		pointer += 4;
		size = GetData<UINT>(pointer);
		for (size_t i = 0; i < size; i++) sre.Events.push_back(GetData<EKeyValPair>(pointer));
		pointer += 20;
		sre.Data = pointer;
		sre.DataSize = end - pointer;
		arr.push_back(sre);
	}
	delete tags;
	delete os;
	pointer = oldptr;
	return arr;
}

vector<ESection_Resources_Form> GetForms(byte*& pointer)
{
	vector<ESection_Resources_Form> arr;
	UINT length = GetData<UINT>(pointer) / 2;
	size_t count = length / sizeof(ETAG);
	ETAG* tags = new ETAG[count];
	for (size_t i = 0; i < count; i++) tags[i] = GetData<ETAG>(pointer);
	pointer += length;
	for (size_t i = 0; i < count; i++)
	{
		ESection_Resources_Form srf;
		srf.Tag = tags[i];
		pointer += 4;
		srf.Class = GetData<ETAG>(pointer);
		srf.Name = GetString(pointer);
		srf.Remark = GetString(pointer);
		srf.Elements = GetElements(pointer);
		arr.push_back(srf);
	}
	delete tags;
	return arr;
}

ESection_UserInfo GetUserInfo(byte* pointer)
{
	ESection_UserInfo sui;
	sui.ProjectName = GetString(pointer);
	sui.Remark = GetString(pointer);
	sui.Author = GetString(pointer);
	GetString(pointer, true);
	GetString(pointer, true);
	GetString(pointer, true);
	GetString(pointer, true);
	GetString(pointer, true);
	GetString(pointer, true);
	GetString(pointer, true);
	sui.Major = GetData<UINT>(pointer);
	sui.Minor = GetData<UINT>(pointer);
	sui.Build = GetData<UINT>(pointer);
	sui.Revision = GetData<UINT>(pointer);
	GetString(pointer, true);
	sui.PluginName = (char*)pointer;
	return sui;
}

ESection_Program GetLibraries(byte* pointer, vector<ESection_TagStatus> tagstatus)
{
	ESection_Program sp;
	pointer += sizeof(UINT);
	pointer += sizeof(UINT);
	GetBytes(pointer, true);
	GetBytes(pointer, true);
	GetBytes(pointer, true);
	USHORT len = GetData<USHORT>(pointer);
	for (size_t i = 0; i < len; i++)
	{
		string lib = GetString(pointer);
		vector<string> arr = split(lib, SP);
		ESection_Library info;
		info.FileName = arr[0];
		info.Guid = arr[1];
		info.Major = atoi(arr[2].c_str());
		info.Minor = atoi(arr[3].c_str());
		info.Name = arr[4];
		sp.Libraries.push_back(info);
	}
	int flag = GetData<int>(pointer);
	pointer += sizeof(UINT);
	if ((flag & 1) != 0) pointer += 16;
	GetBytes(pointer, true);
	GetString(pointer, true);
	sp.Assemblies = GetAssemblies(pointer, tagstatus, &sp.ReferAssemblies);
	sp.Methods = GetMethods(pointer, &sp.ReferMethods);
	sp.GlobalVariables = GetVariables(pointer);
	sp.Structs = GetAssemblies(pointer, tagstatus, &sp.ReferStructs, false);
	sp.Dlls = GetDlls(pointer);
	return sp;
}

ESection_ECList GetECList(byte* pointer)
{
	ESection_ECList list;
	UINT count = GetData<UINT>(pointer);
	for (size_t i = 0; i < count; i++)
	{
		ESection_ECList_Info info;
		pointer += 20;
		info.Name = GetString(pointer);
		info.Path = GetString(pointer);
		GetBytes(pointer, true);
		GetBytes(pointer, true);
		if (info.Name != DoNETRefer) list.List.push_back(info);
	}
	return list;
}

ESection_Resources GetResources(byte* pointer)
{
	ESection_Resources resource;
	resource.Forms = GetForms(pointer);

	return resource;
}