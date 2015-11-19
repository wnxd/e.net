#include "stdafx.h"
#include "efs.h"

ETAG::ETAG()
{

}

ETAG::ETAG(UINT uint)
{
	memcpy(this, &uint, sizeof(UINT));
}

ETAG* ETAG::operator=(UINT uint)
{
	memcpy(this, &uint, sizeof(UINT));
	return this;
}

bool ETAG::operator==(ETAG tag)
{
	return *this == ETAG2UINT(tag);
}

bool ETAG::operator!=(ETAG tag)
{
	return *this != ETAG2UINT(tag);
}

bool ETAG::operator==(ECode_Method code)
{
	return *this == (UINT)code;
}

bool ETAG::operator!=(ECode_Method code)
{
	return *this != (UINT)code;
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

ETAG::operator ECode_Method()
{
	ECode_Method type;
	memcpy(&type, this, sizeof(ECode_Method));
	return type;
}

EFieldInfo::EFieldInfo()
{
	this->Class = NULL;
	this->Field = NULL;
}

EFieldInfo::EFieldInfo(UINT64 uint64)
{
	memcpy(&uint64, this, sizeof(UINT64));
}

EFieldInfo::operator UINT64()
{
	UINT64 uint64;
	memcpy(&uint64, this, sizeof(UINT64));
	return uint64;
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

bool ESection_Variable::operator==(nullptr_t)
{
	return this->Tag == NULL;
}

bool ESection_Variable::operator!=(nullptr_t)
{
	return this->Tag != NULL;
}

ESection_Program_Method::ESection_Program_Method()
{
	this->Tag = NULL;
}

ESection_Program_Method::ESection_Program_Method(nullptr_t) : ESection_Program_Method()
{

}

bool ESection_Program_Method::operator==(nullptr_t)
{
	return this->Tag == NULL;
}

bool ESection_Program_Method::operator!=(nullptr_t)
{
	return this->Tag != NULL;
}

template<typename T> T GetData(byte*& pointer)
{
	T data;
	memcpy(&data, pointer, sizeof(T));
	pointer += sizeof(T);
	return data;
}

string GetString(byte*& pointer, bool jump = false)
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

vector<ESection_Program_Assembly> GetAssemblies(byte*& pointer, bool isclass = true)
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
		spa.Public = GetData<int>(pointer);
		if (isclass) spa.Base = GetData<ETAG>(pointer);
		spa.Name = GetString(pointer);
		spa.Remark = GetString(pointer);
		if (isclass)
		{
			length = GetData<UINT>(pointer);
			size_t size = length / sizeof(ETAG);
			for (size_t n = 0; n < size; n++) spa.Methods.push_back(GetData<ETAG>(pointer));
		}
		spa.Variables = GetVariables(pointer);
		arr.push_back(spa);
	}
	delete tags;
	return arr;
}

vector<ESection_Program_Method> GetMethods(byte*& pointer)
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
		arr.push_back(spm);
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

ESection_Program GetLibraries(byte* pointer)
{
	ESection_Program sp;
	pointer += sizeof(UINT);
	pointer += sizeof(UINT);
	GetBytes(pointer, true);
	GetBytes(pointer, true);
	GetBytes(pointer, true);
	USHORT len = GetData<USHORT>(pointer);
	for (size_t i = 0; i < len; i++) sp.Libraries.push_back(GetString(pointer));
	int flag = GetData<int>(pointer);
	pointer += sizeof(UINT);
	if ((flag & 1) != 0) pointer += 16;
	GetBytes(pointer, true);
	GetString(pointer, true);
	sp.Assemblies = GetAssemblies(pointer);
	sp.Methods = GetMethods(pointer);
	sp.GlobalVariables = GetVariables(pointer);
	sp.Structs = GetAssemblies(pointer, false);
	return sp;
}