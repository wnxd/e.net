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

ref struct EMethodReference
{
	short Lib;
	UINT Tag;
	String^ Name;
	TypeReference^ ReturnType;
	array<EParamInfo^>^ Params;
	EMethodData^ MethodData;
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
	CodeRefer^ _CodeRefer;
	TypeReference^ Type_DateTime;
	TypeReference^ Type_Bin;
	TypeReference^ Type_Decimal;
	TypeReference^ Type_ValueType;
	TypeReference^ Type_StrArr;
	TypeReference^ Type_Nullable;
	MethodReference^ Nullable_Ctor;
	array<String^>^ _refer;
	AssemblyDefinition^ _assembly;
	short e_net_id;
	short krnln_id;
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
	EMethodReference^ GetMethodReference(short index, ETAG tag);
	EMethodReference^ GetMethodReference(EMethodData^ methoddata, short index, ETAG tag);
	TypeReference^ EDT2Type(DataType edt);
	TypeDefinition^ FindTypeDefinition(UINT tag);
	EMethodData^ FindReferMethod(short index, ETAG tag);
public:
	ECompile(byte* ecode, Int64 len, array<String^>^ refer);
	bool Compile();
	void Write(String^ path);
};