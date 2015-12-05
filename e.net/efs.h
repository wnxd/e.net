//易源码结构
#pragma once

using namespace std;

enum ECode_Head : byte
{
	Call = 0x6A,
	Ife = 0x6B,
	If = 0x6C,
	Switch = 0x6E,
	LoopBegin = 0x70,
	LoopEnd = 0x71
};

enum class ECode_Type : byte
{
	ParamBegin = 0x36,
	ParamEnd = 0x01,
	NullParam = 0x16,
	NumberConst = 0x17,
	BoolConst = 0x18,
	DatetimeConst = 0x19,
	StringConst = 0x1A,
	Const = 0x1B,
	LibConst = 0x1C,
	Variable = 0x1D,
	n4 = 0x1E,
	BinConst = 0x1F,
	BinEnd = 0x20,
	Method = 0x21,
	ParameterBegin = 0x38,
	ParameterEnd = 0x37,
	Field = 0x39,
	ArrayBegin = 0x3A,
	NumberIndex = 0x3B
};

enum class EProcess_Tag : byte
{
	Else = 0x50,
	IfeEnd = 0x51,
	IfEnd = 0x52,
	Case = 0x53,
	SwitchEnd = 0x54
};

enum FileType : UINT
{
	E = 0x01,
	EC = 0x03
};

enum CompileType : UINT
{
	WindowsForm = 0x00,
	WindowsConsole = 0x01,
	WindowsDLL = 0x02,
	WindowsEC = 0x03E8,
	LinuxConsole = 0x2710,
	LinuxEC = 0x2AF8
};

enum DataType : UINT
{
	EDT_VOID = _SDT_NULL,
	EDT_ALL = _SDT_ALL,
	EDT_BYTE = SDT_BYTE,
	EDT_SHORT = SDT_SHORT,
	EDT_INT = SDT_INT,
	EDT_LONG = SDT_INT64,
	EDT_FLOAT = SDT_FLOAT,
	EDT_DOUBLE = SDT_DOUBLE,
	EDT_BOOL = SDT_BOOL,
	EDT_DATETIME = SDT_DATE_TIME,
	EDT_TEXT = SDT_TEXT,
	EDT_BIN = SDT_BIN,
	EDT_SUBPTR = SDT_SUB_PTR
};

enum ETYPE : byte
{
	StaticClass = 0x09,
	Class = 0x49,
	Struct = 0x41,
	Variable = 0x25,
	Field = 0x15,
	GlobalField = 0x05,
	Method = 0x04,
	Form = 0x52,
	Element = 0x06
};

enum EMethodAttr : UINT
{
	M_None = 0x00,
	M_Public = 0x08,
	M_Extern = 0x80
};

enum EVariableAttr : USHORT
{
	V_None = 0x00,
	V_Out = 0x02,
	V_Optional = 0x04,
	V_Array = 0x08,
	V_Public = 0x0100,
	V_Extern = 0x0200
};

enum ETagStatus : UINT
{
	C_None = 0x00,
	C_Public = 0x01,
	C_Extern = 0x02
};

enum Cursor : UINT
{
	ARROW = (UINT)IDC_ARROW,
	IBEAM = (UINT)IDC_IBEAM,
	WAIT = (UINT)IDC_WAIT,
	CROSS = (UINT)IDC_CROSS,
	UPARROW = (UINT)IDC_UPARROW,
	SIZE = (UINT)IDC_SIZE,
	ICON = (UINT)IDC_ICON,
	SIZENWSE = (UINT)IDC_SIZENWSE,
	SIZENESW = (UINT)IDC_SIZENESW,
	SIZEWE = (UINT)IDC_SIZEWE,
	SIZENS = (UINT)IDC_SIZENS,
	SIZEALL = (UINT)IDC_SIZEALL,
	NO = (UINT)IDC_NO,
	HAND = (UINT)IDC_HAND,
	APPSTARTING = (UINT)IDC_APPSTARTING,
	HELP = (UINT)IDC_HELP
};

enum EElementStatus : UINT
{
	E_None = 0x00,
	E_Visible = 0x01,
	E_Disable = 0x02
};

struct ETAG
{
	USHORT ID;
	byte Type1;
	ETYPE Type2;
	ETAG();
	ETAG(UINT uint);
	bool operator==(ETAG tag);
	bool operator!=(ETAG tag);
	bool operator==(UINT uint);
	bool operator!=(UINT uint);
	bool operator==(int uint);
	bool operator!=(int uint);
	operator UINT();
};

struct LIBCONST
{
	USHORT LibID;
	USHORT ID;
	LIBCONST();
	LIBCONST(UINT uint);
	LIBCONST* operator=(UINT uint);
	operator UINT();
};

struct EKeyValPair
{
	ETAG Key;
	ETAG Value;
	EKeyValPair();
	EKeyValPair(UINT64 uint);
	operator UINT64();
};

struct EFile_Header
{
	byte Magic1[4];
	byte Magic2[4];
};

struct ESection_Header
{
	byte Magic[4];
	UINT Info_CheckSum;
};

struct ESection_Info
{
	byte Key[4];
	byte Name[30];
	short reserve_fill_1;
	UINT Index;
	UINT CodeType;
	UINT Data_CheckSum;
	UINT DataLength;
	int reserve_item[10];
};

struct ESection_SystemInfo
{
	USHORT Version1;
	USHORT Version2;
	int Unknow_1;
	int Unknow_2;
	int Unknow_3;
	FileType FileType;
	int Unknow_4;
	CompileType CompileType;
	int Unknow_5[8];
};

struct ESection_UserInfo
{
	string ProjectName;
	string Remark;
	string Author;
	UINT Major;
	UINT Minor;
	UINT Build;
	UINT Revision;
	string PluginName;
};

struct ESection_ArrayInfo
{
	byte Dimension;
	vector<UINT> Subscript;
};

struct ESection_Library
{
	string FileName;
	string Guid;
	int Major;
	int Minor;
	string Name;
	ESection_Library();
	ESection_Library(nullptr_t);
	bool operator==(nullptr_t);
	bool operator!=(nullptr_t);
};

struct EBase
{
	ETAG Tag;
	bool operator==(nullptr_t);
	bool operator!=(nullptr_t);
};

struct ESection_Variable : EBase
{
	DataType DataType;
	EVariableAttr Attributes;
	ESection_ArrayInfo ArrayInfo;
	string Name;
	string Remark;
	ESection_Variable();
	ESection_Variable(nullptr_t);
	ESection_Variable* operator=(nullptr_t);
};

struct ESection_Program_Assembly : EBase
{
	ETagStatus Status;
	ETAG Base;
	string Name;
	string Remark;
	vector<ETAG> Methods;
	vector<ESection_Variable> Variables;
	ESection_Program_Assembly();
	ESection_Program_Assembly(nullptr_t);
	ESection_Variable FindField(ETAG tag);
};

struct ESection_Program_Method : EBase
{
	ETAG Class;
	EMethodAttr Attributes;
	DataType ReturnType;
	string Name;
	string Remark;
	vector<ESection_Variable> Variables;
	vector<ESection_Variable> Parameters;
	vector<byte> RowsOffset;
	vector<byte> Offset1;
	vector<byte> Offset2;
	vector<byte> ParameterOffset;
	vector<byte> VariableOffset;
	vector<byte> Code;
	ESection_Program_Method();
	ESection_Program_Method(nullptr_t);
};

struct ESection_Program_Dll : EBase
{
	ETagStatus Status;
	DataType ReturnType;
	string ShowName;
	string Remark;
	string Lib;
	string Name;
	vector<ESection_Variable> Parameters;
};

struct ESection_Program
{
	vector<ESection_Library> Libraries;
	vector<ESection_Program_Assembly> Assemblies;
	vector<ESection_Program_Assembly> ReferAssemblies;
	vector<ESection_Program_Method> Methods;
	vector<ESection_Program_Method> ReferMethods;
	vector<ESection_Variable> GlobalVariables;
	vector<ESection_Variable> ReferGlobalVariables;
	vector<ESection_Program_Assembly> Structs;
	vector<ESection_Program_Assembly> ReferStructs;
	vector<ESection_Program_Dll> Dlls;
};

struct ESection_TagStatus : EBase
{
	ETagStatus Status;
	ESection_TagStatus();
	ESection_TagStatus(nullptr_t);
};

struct ESection_AuxiliaryInfo2
{
	vector<ESection_TagStatus> Tags;
};

struct ESection_ECList_Info
{
	string Name;
	string Path;
};

struct ESection_ECList
{
	vector<ESection_ECList_Info> List;
};

struct ESection_Resources_FormElement :EBase
{
	UINT Left;
	UINT Top;
	UINT Width;
	UINT Height;
	vector<byte> Cursor;
	string Mark;
	EElementStatus Status;
	vector<EKeyValPair> Events;
	LPBYTE Data;
	UINT DataSize;
};

struct ESection_Resources_Form : EBase
{
	string Name;
	string Remark;
	ETAG Class;
	vector<ESection_Resources_FormElement> Elements;
};

struct ESection_Resources_Const
{

};

struct ESection_Resources
{
	vector<ESection_Resources_Form> Forms;
	vector<ESection_Resources_Const> Constants;
};

const byte Magic1[4] = { 'C', 'N', 'W', 'T' };
const byte Magic2[4] = { 'E', 'P', 'R', 'G' };
const byte Magic_Section[4] = { 0x19, 0x73, 0x11, 0x15 };
const byte KEY[4] = { 25, 115, 0, 7 };

#define DoNETRefer "@DoNETRefer"
#define SP "\r"
#define DONET "@donet"
#define DONET_NAMESPACE "@namespace"
#define DONET_CLASS "@class"
#define DONET_ENUM "@enum"
#define REMARK -1
#define CUSTOM -2
#define DLL -3
#define ETAG2UINT(etag) (UINT)MAKELONG(etag.ID, MAKEWORD(etag.Type1, etag.Type2))

void Decode_Str(byte data[], const byte key[]);
ESection_UserInfo GetUserInfo(byte* pointer);
ESection_Program GetLibraries(byte* pointer, vector<ESection_TagStatus> tagstatus);
ESection_ECList GetECList(byte* pointer);
ESection_Resources GetResources(byte* pointer);