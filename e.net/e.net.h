#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace Mono::Cecil;
using namespace Mono::Cecil::Cil;
using namespace wnxd::E_NET;

bool Compile(byte* pointer, Int64 len, String^ path, array<String^>^ refer);

enum class EParamDataType
{
	Null,
	Number,
	Bool,
	Time,
	String,
	Bin,
	Param,
	Var,
	Field,
	Property,
	GlobalField,
	Array,
	IL
};

enum class EIndexType
{
	Number,
	Text,
	Array,
	Field,
	Var,
	Method
};

enum class EVariableType
{
	Param,
	Var,
	Array,
	Field,
	Property,
	GlobalField,
	DoNET
};

enum class ELibConstType : SHORT
{
	Null = CT_NULL,
	Num = CT_NUM,
	Bool = CT_BOOL,
	Text = CT_TEXT
};

ref struct EVariableIndex
{
	EIndexType IndexType;
	Object^ IndexData;
};

ref struct EVariableData
{
	TypeReference^ Type;
	EVariableType VariableType;
	Object^ Data;
};

ref struct ELibConstData
{
	TypeReference^ Type;
	ELibConstType ConstType;
	Object^ Data;
};

ref struct EParamData
{
	TypeReference^ Type;
	Object^ Data;
	EParamDataType DataType;
};

ref struct EParamInfo
{
	String^ Name;
	TypeReference^ Type;
	TypeReference^ OriginalType;
	bool IsArray;
	bool IsOptional;
	Object^ Defualt;
	bool IsAddress;
	bool IsVariable;
};

ref struct EMethodData
{
	MethodDefinition^ Method;
	EMethodMode Mode;
	EMethodData();
	EMethodData(MethodDefinition^ method);
	EMethodData(MethodDefinition^ method, EMethodMode mode);
	operator MethodDefinition ^ ();
};

ref struct EMethodReference
{
	short Lib;
	UINT Tag;
	String^ Name;
	TypeReference^ ReturnType;
	array<EParamInfo^>^ Params;
	EMethodData^ MethodData;
};

ref struct ELib_Method
{
	short Index;
	UINT Tag;
	ELib_Method();
	ELib_Method(short index, UINT tag);
	bool operator==(ELib_Method^ elm);
	bool operator!=(ELib_Method^ elm);
};

ref struct EDataInfo
{
	IDictionary<ELib_Method^, EMethodData^>^ Methods;
	IDictionary<String^, IList<EMethodData^>^>^ Symbols;
	IDictionary<UINT, TypeDefinition^>^ Types;
	IDictionary<UINT, VariableDefinition^>^ Variables;
	IDictionary<UINT, ParameterDefinition^>^ Parameters;
	IDictionary<UINT, FieldDefinition^>^ Fields;
	IDictionary<UINT, FieldDefinition^>^ GlobalVariables;
	IDictionary<UINT, PropertyDefinition^>^ Propertys;
	EDataInfo();
};

value struct EMethodInfo
{
	ESection_Program_Method* MethodInfo;
	MethodDefinition^ Method;
};

ref class ECompile
{
	~ECompile();
private:
	CodeProcess* _CodeProcess;
	TypeReference^ Type_DateTime;
	TypeReference^ Type_Bin;
	TypeReference^ Type_Decimal;
	TypeReference^ Type_ValueType;
	TypeReference^ Type_StrArr;
	TypeReference^ Type_Nullable;
	MethodReference^ Nullable_Ctor;
	array<String^>^ _refer;
	AssemblyDefinition^ _assembly;
	EDataInfo^ _edata;
	List<TypeDefinition^>^ _alltype;
	short e_net_id;
	bool CompileHead();
	bool CompileRefer();
	bool CompileClass();
	bool CompileMethod(TypeDefinition^ type, ESection_Program_Assembly assembly, bool isstatic);
	bool CompileCode();
	bool CompileCode_Begin(EMethodInfo^ MethodInfo, ILProcessor^ ILProcessor, byte* Code, size_t Length, vector<UINT> Offset);
	TypeReference^ CompileCode_Call(EMethodInfo^ MethodInfo, ILProcessor^ ILProcessor, byte*& Code, byte* End);
	EVariableData^ CompileCode_Var(EMethodInfo^ MethodInfo, ILProcessor^ ILProcessor, byte*& Code, byte* End);
	ELibConstData^ CompileCode_LibConst(LIBCONST libconst);
	void CompileCode_Proc(EMethodInfo^ MethodInfo, ILProcessor^ ILProcessor, byte* Code, size_t Length, vector<UINT> Offset, size_t& Index);
	void LoadKrnln();
	void LoadE_net();
	EMethodReference^ GetMethodReference(ELib_Method^ libmethod);
	EMethodReference^ GetMethodReference(EMethodData^ methoddata, ELib_Method^ libmethod);
	TypeReference^ EDT2Type(DataType edt);
	TypeDefinition^ FindReferType(UINT tag);
	EMethodData^ FindReferMethod(ELib_Method^ tag);
public:
	ECompile(byte* ecode, Int64 len, array<String^>^ refer);
	bool Compile();
	void Write(String^ path);
};