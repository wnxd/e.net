#include "stdafx.h"
#include "libhandle.h"

WindowProperty::WindowProperty()
{
	this->Type = WindowPropertyType::WPT_NULL;
}

WindowProperty::WindowProperty(nullptr_t) : WindowProperty()
{

}

bool WindowProperty::operator==(nullptr_t)
{
	return this->Type == WindowPropertyType::WPT_NULL;
}

bool WindowProperty::operator!=(nullptr_t)
{
	return this->Type != WindowPropertyType::WPT_NULL;
}

INT_Property::INT_Property()
{
	this->Type = WindowPropertyType::WPT_INT;
}

INT_Property::INT_Property(INT v) : INT_Property()
{
	this->value = v;
}

DOUBLE_Property::DOUBLE_Property()
{
	this->Type = WindowPropertyType::WPT_DOUBLE;
}

DOUBLE_Property::DOUBLE_Property(DOUBLE v) : DOUBLE_Property()
{
	this->value = v;
}

DWORD_Property::DWORD_Property()
{
	this->Type = WindowPropertyType::WPT_DWORD;
}

DWORD_Property::DWORD_Property(DWORD v) : DWORD_Property()
{
	this->value = v;
}

LPTSTR_Property::LPTSTR_Property()
{
	this->Type = WindowPropertyType::WPT_LPTSTR;
}

LPTSTR_Property::LPTSTR_Property(LPTSTR v) : LPTSTR_Property()
{
	this->value = v;
}

LPBYTE_Property::LPBYTE_Property()
{
	this->Type = WindowPropertyType::WPT_LPBYTE;
}

LPBYTE_Property::LPBYTE_Property(LPBYTE v, INT s) : LPBYTE_Property()
{
	this->value = v;
	this->size = s;
}

UnitHandle::UnitHandle(LIB_DATA_TYPE_INFO info)
{
	this->_info = info;
	this->_hUnit = NULL;
	this->_create = (PFN_CREATE_UNIT)info.m_pfnGetInterface(ITF_CREATE_UNIT);
	this->_get = (PFN_GET_PROPERTY_DATA)info.m_pfnGetInterface(ITF_GET_PROPERTY_DATA);
}

UnitHandle::~UnitHandle()
{
	if (this->_hUnit != NULL) delete (void*)this->_hUnit;
}

bool UnitHandle::LoadData(LPBYTE data, UINT size)
{
	if (this->_create != NULL)
	{
		this->_hUnit = this->_create(data, size, 0, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, NULL, FALSE);
	}
	return this->_hUnit != NULL;
}

vector<UNIT_PROPERTY> UnitHandle::GetAllProperty()
{
	vector<UNIT_PROPERTY> arr(this->_info.m_pPropertyBegin, this->_info.m_pPropertyBegin + this->_info.m_nPropertyCount);
	return arr;
}

WindowProperty UnitHandle::GetProperty(UINT index)
{
	if (this->_hUnit != NULL)
	{
		UNIT_PROPERTY_VALUE value;
		this->_get(this->_hUnit, index, &value);
		WindowProperty ret = NULL;
		switch (this->_info.m_pPropertyBegin[index].m_shtType)
		{
		case UD_INT:
		case UD_PICK_INT:
		case UD_PICK_SPEC_INT:
			ret = INT_Property(value.m_int);
			break;
		case UD_DOUBLE:
			ret = DOUBLE_Property(value.m_double);
			break;
		case UD_BOOL:
			ret = INT_Property(value.m_bool);
			break;
		case UD_DATE_TIME:
			ret = DOUBLE_Property(value.m_dtDateTime);
		case UD_COLOR:
		case UD_COLOR_TRANS:
		case UD_COLOR_BACK:
			ret = DWORD_Property(value.m_clr);
			break;
		case UD_TEXT:
		case UD_PICK_TEXT:
		case UD_EDIT_PICK_TEXT:
			ret = LPTSTR_Property(value.m_szText);
			break;
		case UD_FILE_NAME:
			ret = LPTSTR_Property(value.m_szFileName);
			break;
		case UD_PIC:
		case UD_ICON:
		case UD_CURSOR:
		case UD_MUSIC:
		case UD_FONT:
		case UD_CUSTOMIZE:
		case UD_IMAGE_LIST:
			ret = LPBYTE_Property(value.m_data.m_pData, value.m_data.m_nDataSize);
			break;
		}
		return ret;
	}
	return NULL;
}

void UnitHandle::FreeProperty(WindowProperty prop)
{
	switch (prop.Type)
	{
	case WindowPropertyType::WPT_LPTSTR:
		delete static_cast<LPTSTR_Property*>(&prop)->value;
		break;
	case WindowPropertyType::WPT_LPBYTE:
		delete static_cast<LPBYTE_Property*>(&prop)->value;
		break;
	}
}

LibHandle::LibHandle(PLIB_INFO info)
{
	this->_info = info;
}

LIB_CONST_INFO LibHandle::GetConstInfo(UINT index)
{
	return this->_info->m_pLibConst[index];
}

LIB_DATA_TYPE_INFO LibHandle::GetTypeInfo(UINT index)
{
	return this->_info->m_pDataType[index];
}

UnitHandle LibHandle::GetUnitInfo(UINT index)
{
	return UnitHandle(this->GetTypeInfo(index));
}