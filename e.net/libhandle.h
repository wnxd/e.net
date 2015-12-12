#pragma once

using namespace std;

enum WindowPropertyType
{
	WPT_NULL,
	WPT_INT,
	WPT_DOUBLE,
	WPT_DWORD,
	WPT_LPTSTR,
	WPT_LPBYTE,
};

struct WindowProperty
{
	WindowPropertyType Type;
	WindowProperty();
	WindowProperty(nullptr_t);
	bool operator==(nullptr_t);
	bool operator!=(nullptr_t);
};

struct INT_Property : WindowProperty
{
	INT value;
	INT_Property();
	INT_Property(INT v);
};

struct DOUBLE_Property : WindowProperty
{
	DOUBLE value;
	DOUBLE_Property();
	DOUBLE_Property(DOUBLE v);
};

struct DWORD_Property : WindowProperty
{
	DWORD value;
	DWORD_Property();
	DWORD_Property(DWORD v);
};

struct LPTSTR_Property : WindowProperty
{
	LPTSTR value;
	LPTSTR_Property();
	LPTSTR_Property(LPTSTR v);
};

struct LPBYTE_Property : WindowProperty
{
	LPBYTE value;
	INT size;
	LPBYTE_Property();
	LPBYTE_Property(LPBYTE v, INT s);
};

class UnitHandle
{
public:
	UnitHandle(LIB_DATA_TYPE_INFO info);
	~UnitHandle();
	bool LoadData(LPBYTE data, UINT size, HWND hwnd = NULL);
	vector<UNIT_PROPERTY> GetAllProperty();
	WindowProperty* GetProperty(UINT index);
	HUNIT hUnit();
	void FreeProperty(WindowProperty* prop);
private:
	LIB_DATA_TYPE_INFO _info;
	PFN_CREATE_UNIT _create;
	PFN_GET_PROPERTY_DATA _get;
	HUNIT _hUnit;
};

class LibHandle
{
public:
	LibHandle(PLIB_INFO info);
	LIB_CONST_INFO GetConstInfo(UINT index);
	LIB_DATA_TYPE_INFO GetTypeInfo(UINT index);
	UnitHandle GetUnitInfo(UINT index);
private:
	PLIB_INFO _info;
};