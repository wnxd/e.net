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

void Decode_Str(byte data[], const byte key[])
{
	size_t key_len = 4;
	size_t len = 30;
	for (size_t i = 0, key_i = 1; i < len; i++, key_i++) data[i] = data[i] ^ key[key_i % key_len];
}