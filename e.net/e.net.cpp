#include "stdafx.h"
#include "common.h"
#include "common.net.h"
#include "Plugins.h"
#include "compile.h"
#include "krnln.net.h"
#include "refer.h"
#include "e.net.h"

using namespace std;
using namespace System::IO;
using namespace System::Windows::Forms;
using namespace System::Runtime::CompilerServices;
using namespace System::Text::RegularExpressions;

#define E_NET LI_LIB_GUID_STR

extern "C" PLIB_INFO WINAPI GetNewInf();
extern String^ GetMethodName(MethodReference^ method);

String^ DotDecode(String^ str)
{
	str = str->Replace("__", "!");
	str = str->Replace("_", ".");
	str = str->Replace("!", "_");
	return str;
}

array<byte>^ ReadFile(String^ path)
{
	if (String::IsNullOrEmpty(path)) return nullptr;
	FileStream^ fs = File::Open(path, FileMode::Open, FileAccess::Read, FileShare::ReadWrite);
	array<byte>^ bytes = gcnew array<byte>(fs->Length);
	fs->Read(bytes, 0, fs->Length);
	delete fs;
	return bytes;
}

bool CompileIL(byte* data, Int64 size, const char* savePath, array<String^>^ refer)
{
	return Compile(data, size, LPSTR2String(savePath), refer);
}

bool CompileILByFile(const char* path, const char* savePath)
{
	String^ mpath = LPSTR2String(path);
	array<byte>^ arr = ReadFile(mpath);
	if (arr == nullptr) return false;
	IntPtr ptr = Marshal::AllocHGlobal(arr->LongLength);
	Marshal::Copy(arr, 0, ptr, arr->LongLength);
	array<String^>^ refer;
	mpath += ".wnxd";
	if (File::Exists(mpath)) refer = File::ReadAllLines(mpath);
	else refer = gcnew array<String^>(0);
	bool r = Compile(Ptr2LPBYTE(ptr), arr->LongLength, LPSTR2String(savePath), refer);
	Marshal::FreeHGlobal(ptr);
	return r;
}

bool Compile(byte* pointer, Int64 len, String^ path, array<String^>^ refer)
{
	ECompile^ compile = gcnew ECompile(pointer, len, refer);
	bool r = compile->Compile();
	if (r)
	{
		MessageBox::Show("编译成功");
		compile->Write(path);
	}
	delete compile;
	return r;
}

void DefaultConstructor(ModuleDefinition^ module, MethodDefinition^ method, TypeDefinition^ basetype = nullptr)
{
	if (basetype == nullptr) basetype = module->TypeSystem->Object->Resolve();
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	for each (MethodDefinition^ item in basetype->Methods)
	{
		if (item->IsConstructor && item->Parameters->Count == 0)
		{
			AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(item));
			break;
		}
	}
}

template<typename T> vector<T> ReadOffset(vector<byte> data)
{
	vector<T> arr;
	size_t count = data.size() / sizeof(T);
	for (vector<byte>::const_iterator ptr = data.cbegin(); ptr != data.cend(); ptr += sizeof(T))
	{
		T m;
		memcpy(&m, ptr._Ptr, sizeof(T));
		arr.push_back(m);
	}
	return arr;
}

MethodDefinition^ CreateConstructor(ModuleDefinition^ module, MethodAttributes attr = MethodAttributes::Private, IList<Mono::Cecil::ParameterDefinition^>^ params = nullptr, TypeDefinition^ basetype = nullptr)
{
	MethodDefinition^ ctor;
	if (attr.HasFlag(MethodAttributes::Static))
	{
		if (attr.HasFlag(MethodAttributes::Public)) attr = attr ^ MethodAttributes::Public | MethodAttributes::Private;
		ctor = gcnew MethodDefinition(".cctor", attr | CTOR, module->TypeSystem->Void);
	}
	else
	{
		ctor = gcnew MethodDefinition(".ctor", attr | CTOR, module->TypeSystem->Void);
		DefaultConstructor(module, ctor, basetype);
	}
	if (params != nullptr && params->Count > 0) for (int i = 0; i < params->Count; i++) ctor->Parameters->Add(params[i]);
	return ctor;
}

void AddDefaultConstructor(ModuleDefinition^ module, TypeDefinition^ type)
{
	MethodDefinition^ method;
	for each (method in type->Methods) if (method->IsConstructor && method->Parameters->Count == 0) return;
	TypeDefinition^ basetype = type->BaseType->Resolve();
	AddDefaultConstructor(module, basetype);
	method = CreateConstructor(module, MethodAttributes::Private, nullptr, basetype);
	method->Body->Instructions->Add(Instruction::Create(OpCodes::Ret));
	type->Methods->Add(method);
}

String^ GetTypeName(ESection_Program_Assembly assembly)
{
	String^ classname = CStr2String(assembly.Name);
	classname = DotDecode(classname);
	return classname;
}

PropertyDefinition^ FindProperty(TypeDefinition^ type, String^ name)
{
	for each (PropertyDefinition^ property in type->Properties) if (property->Name == name) return property;
	return nullptr;
}

MethodDefinition^ FindMethod(TypeDefinition^ type, String^ name, bool isstatic, IList<TypeReference^>^ params = nullptr)
{
	for each (MethodDefinition^ method in type->Methods)
	{
		if (method->Name == name && method->IsStatic == isstatic)
		{
			if (params != nullptr && params->Count > 0)
			{
				if (method->Parameters->Count != params->Count) continue;
				for (int i = 0; i < method->Parameters->Count; i++) if (method->Parameters[i]->ParameterType != params[i]) goto _continue;
			}
			return method;
		_continue:;
		}
	}
	return nullptr;
}

MethodDefinition^ FindMethod(TypeDefinition^ type, UINT token)
{
	for each (MethodDefinition^ method in type->Methods) if (method->MetadataToken.ToUInt32() == token) return method;
	return nullptr;
}

List<MethodDefinition^>^ FindAllMethod(TypeDefinition^ type, String^ name, bool isstatic)
{
	List<MethodDefinition^>^ list = gcnew List<MethodDefinition^>();
	for each (MethodDefinition^ method in type->Methods) if (method->IsStatic == isstatic && method->Name == name) list->Add(method);
	return list;
}

CustomAttribute^ FindCustom(IList<CustomAttribute^>^ list, TypeReference^ type)
{
	for each (CustomAttribute^ custom in list) if (custom->AttributeType == type) return custom;
	return nullptr;
}

FieldDefinition^ FindField(TypeDefinition^ type, String^ name)
{
	for each (FieldDefinition^ field in type->Fields) if (field->Name == name) return field;
	return nullptr;
}

MethodDefinition^ CreateMethod(String^ name, TypeReference^ returntype, IList<ParameterDefinition^>^ params = nullptr, MethodAttributes attr = MethodAttributes::HideBySig)
{
	MethodDefinition^ method = gcnew MethodDefinition(name, attr, returntype);
	if (params != nullptr && params->Count > 0) for each (ParameterDefinition^ item in params) method->Parameters->Add(item);
	return method;
}

ParameterDefinition^ CreateParameter(String^ name, TypeReference^ type, ParameterAttributes attr = ParameterAttributes::None)
{
	ParameterDefinition^ param = gcnew ParameterDefinition(name, attr, type);
	return param;
}

void AddModule(IList<ModuleReference^>^ modules, ModuleReference^ module)
{
	for each (ModuleReference^ m in modules) if (m->Name == module->Name) return;
	modules->Add(module);
}

Exception^ Error(String^ methodname, String^ paramname, String^ error)
{
	return gcnew Exception("函数: " + methodname + " - 参数: " + paramname + " " + error);
}

Exception^ Error(String^ methodname, String^ error)
{
	return gcnew Exception("函数: " + methodname + " " + error);
}

Exception^ Error(String^ error)
{
	return gcnew Exception(error);
}

ECompile::ECompile(byte* ecode, Int64 len, array<String^>^ refer)
{
	this->_CodeProcess = new CodeProcess(ecode, len);
	this->_refer = refer;
}

ECompile::~ECompile()
{
	delete this->_CodeProcess;
}

bool ECompile::Compile()
{
	return this->CompileHead() && this->CompileRefer() && this->CompileClass() && this->CompileCode();
}

void ECompile::Write(String^ path)
{
	if (!String::IsNullOrEmpty(path))
	{
		ESection_UserInfo userinfo = this->_CodeProcess->GetUserInfo();
		ESection_SystemInfo systeminfo = this->_CodeProcess->GetSystemInfo();
		String^ str = CStr2String(userinfo.ProjectName);
		if (!String::IsNullOrEmpty(str))
		{
			char c = path[path->Length - 1];
			if (c != '/' && c != '\\') path += "\\";
			String^ ext;
			switch (systeminfo.CompileType)
			{
			case WindowsForm:
			case WindowsConsole:
				ext = ".exe";
				break;
			case WindowsDLL:
				ext = ".dll";
				break;
			}
			this->_assembly->Write(path + str + ext);
		}
	}
}

bool ECompile::CompileHead()
{
	try
	{
		ESection_UserInfo userinfo = this->_CodeProcess->GetUserInfo();
		ESection_SystemInfo systeminfo = this->_CodeProcess->GetSystemInfo();
		String^ str = CStr2String(userinfo.ProjectName);
		Version^ version = gcnew Version(userinfo.Major, userinfo.Minor, userinfo.Build, userinfo.Revision);
		AssemblyNameDefinition^ name = gcnew AssemblyNameDefinition(str, version);
		ModuleParameters^ mp = gcnew ModuleParameters();
		mp->Runtime = TargetRuntime::Net_4_0;
		switch (systeminfo.CompileType)
		{
		case WindowsForm:
			mp->Kind = ModuleKind::Windows;
			break;
		case WindowsConsole:
			mp->Kind = ModuleKind::Console;
			break;
		case WindowsDLL:
			mp->Kind = ModuleKind::Dll;
			break;
		}
		this->_assembly = AssemblyDefinition::CreateAssembly(name, str, mp);
		short id;
		this->_CodeProcess->FindLibrary(E_NET, id);
		this->e_net_id = id;
		return true;
	}
	catch (Exception^ ex)
	{
		this->_assembly = nullptr;
		MessageBox::Show(ex->Message);
		return false;
	}
}

bool ECompile::CompileRefer()
{
	try
	{
		ModuleDefinition^ module = this->_assembly->MainModule;
		if (this->_refer->Length == 0)
		{
			String^ mscorlib = Environment::GetFolderPath(Environment::SpecialFolder::Windows) + "\\Microsoft.NET\\Framework\\v4.0.30319\\mscorlib.dll";
			this->_refer = gcnew array < String^ > { mscorlib };
		}
		this->_CodeRefer = gcnew CodeRefer(module);
		this->_CodeRefer->AddReferList(this->_refer);
		List<String^>^ list = gcnew List<String^>();
		for each (ESection_ECList_Info info in this->_CodeProcess->GetECList()) list->Add(CStr2String(info.Path));
		this->_CodeRefer->AddECList(list);
		return true;
	}
	catch (Exception^ ex)
	{
		MessageBox::Show(ex->Message);
		return false;
	}
}

bool ECompile::CompileClass()
{
	try
	{
		ModuleDefinition^ module = this->_assembly->MainModule;
		this->Type_DateTime = module->ImportReference(typeof(DateTime));
		this->Type_Bin = gcnew ArrayType(module->TypeSystem->Byte);
		this->Type_Decimal = module->ImportReference(typeof(Decimal));
		this->Type_ValueType = module->ImportReference(typeof(ValueType));
		this->Type_StrArr = module->ImportReference(typeof(array<String^>));
		Type^ type = Type::GetType("System.Nullable`1");
		this->Type_Nullable = module->ImportReference(type);
		this->Nullable_Ctor = module->ImportReference(type->GetConstructors()[0]);
		IDictionary<TypeDefinition^, UINT>^ typelist = gcnew Dictionary<TypeDefinition^, UINT>();
		for each (ESection_Program_Assembly assembly in this->_CodeProcess->GetAssemblies())
		{
			bool isstatic = assembly.Tag.Type2 == ETYPE::StaticClass;
			String^ classname = GetTypeName(assembly);
			TypeDefinition^ type = this->_CodeRefer->FindType(classname);
			if (type == nullptr)
			{
				String^ _namespace;
				String^ _typename;
				int index = classname->LastIndexOf(".");
				if (index != -1)
				{
					_namespace = classname->Substring(0, index);
					_typename = classname->Substring(index + 1);
				}
				else
				{
					_namespace = "";
					_typename = classname;
				}
				type = gcnew TypeDefinition(_namespace, _typename, TypeAttributes::Class, module->TypeSystem->Object);
				module->Types->Add(type);
			}
			if (!isstatic)
			{
				typelist->Add(type, assembly.Base);
				if (assembly.Status == ETagStatus::C_Public || this->_CodeProcess->GetTagStatus(assembly.Tag) == ETagStatus::C_Public) type->Attributes = type->Attributes | TypeAttributes::Public;
			}
			this->_CodeRefer->AddType(assembly.Tag, type);
		}
		MethodReference^ StructLayout = module->ImportReference(GetCtor(StructLayoutAttribute, typeof(LayoutKind)));
		CustomAttribute^ custom = gcnew CustomAttribute(StructLayout);
		custom->ConstructorArguments->Add(CustomAttributeArgument(module->ImportReference(typeof(LayoutKind)), LayoutKind::Sequential));
		for each (ESection_Program_Assembly assembly in this->_CodeProcess->GetStructs())
		{
			TypeAttributes attr = STRUCT;
			if (assembly.Status == ETagStatus::C_Public) attr = attr | TypeAttributes::Public;
			String^ classname = GetTypeName(assembly);
			String^ _namespace;
			String^ _typename;
			int index = classname->LastIndexOf(".");
			if (index != -1)
			{
				_namespace = classname->Substring(0, index);
				_typename = classname->Substring(index + 1);
			}
			else
			{
				_namespace = "";
				_typename = classname;
			}
			TypeDefinition^ type = gcnew TypeDefinition(_namespace, _typename, attr, this->Type_ValueType);
			type->CustomAttributes->Add(custom);
			module->Types->Add(type);
			this->_CodeRefer->AddType(assembly.Tag, type);
		}
		for each(KeyValuePair<TypeDefinition^, UINT>^ kv in typelist)
		{
			TypeReference^ t = this->EDT2Type((DataType)kv->Value);
			if (t != module->TypeSystem->Void) kv->Key->BaseType = t;
		}
		for each (ESection_Program_Assembly assembly in this->_CodeProcess->GetStructs())
		{
			TypeDefinition^ type = this->_CodeRefer->FindType(assembly.Tag);
			for each (ESection_Variable var in assembly.Variables)
			{
				TypeReference^ t = this->EDT2Type(var.DataType);
				if (var.ArrayInfo.Dimension > 0) t = gcnew ArrayType(t, var.ArrayInfo.Dimension);
				String^ name = CStr2String(var.Name);
				FieldDefinition^ f = gcnew FieldDefinition(name, var.Remark == "private" ? FieldAttributes::Private : FieldAttributes::Public, t);
				type->Fields->Add(f);
				this->_CodeRefer->AddField(var.Tag, f);
			}
		}
		for each (ESection_Program_Assembly assembly in this->_CodeProcess->GetAssemblies())
		{
			bool isstatic = assembly.Tag.Type2 == ETYPE::StaticClass;
			TypeDefinition^ type = this->_CodeRefer->FindType(assembly.Tag);
			for each (ESection_Variable var in assembly.Variables)
			{
				FieldAttributes attr;
				if (var.Remark == "public") attr = FieldAttributes::Public;
				if (isstatic) attr = attr | FieldAttributes::Static;
				TypeReference^ t = this->EDT2Type(var.DataType);
				if (var.ArrayInfo.Dimension > 0) t = gcnew ArrayType(t, var.ArrayInfo.Dimension);
				FieldDefinition^ f = gcnew FieldDefinition(CStr2String(var.Name), attr, t);
				type->Fields->Add(f);
				this->_CodeRefer->AddField(var.Tag, f);
			}
			if (!this->CompileMethod(type, assembly, isstatic)) return false;
		}
		for each (ESection_Program_Assembly assembly in this->_CodeProcess->GetAssemblies())
		{
			if (assembly.Tag.Type2 == ETYPE::Class)
			{
				TypeDefinition^ type = this->_CodeRefer->FindType(assembly.Tag);
				AddDefaultConstructor(module, type);
			rt:;
			}
		}
		return true;
	}
	catch (Exception^ ex)
	{
		MessageBox::Show(ex->Message);
		return false;
	}
}

bool ECompile::CompileMethod(TypeDefinition^ type, ESection_Program_Assembly assembly, bool isstatic)
{
	try
	{
		ModuleDefinition^ module = this->_assembly->MainModule;
		for each (ETAG tag in assembly.Methods)
		{
			ESection_Program_Method pm = this->_CodeProcess->FindMethod(tag);
			if (pm != NULL)
			{
				String^ name = CStr2String(pm.Name);
				MethodDefinition^ method;
				bool ctor = false;
				MethodAttributes attr = ((pm.Attributes & EMethodAttr::M_Public) == EMethodAttr::M_Public) ? MethodAttributes::Public : MethodAttributes::Private;
				if (isstatic) attr = attr | MethodAttributes::Static;
				if (name == type->Name && pm.ReturnType == DataType::EDT_VOID)
				{
					ctor = true;
					method = CreateConstructor(module, attr, nullptr, type->BaseType->Resolve());
				}
				else
				{
					TypeReference^ t = this->EDT2Type(pm.ReturnType);
					if (pm.Remark == "array") t = gcnew ArrayType(t);
					method = CreateMethod(name, t, nullptr, attr | MethodAttributes::HideBySig);
				}
				for each (ESection_Variable param in pm.Parameters)
				{
					TypeReference^ t = this->EDT2Type(param.DataType);
					ParameterAttributes attr = ParameterAttributes::None;
					if ((param.Attributes & EVariableAttr::V_Array) == EVariableAttr::V_Array) t = gcnew ArrayType(t);
					if ((param.Attributes & EVariableAttr::V_Optional) == EVariableAttr::V_Optional)
					{
						attr = attr | ParameterAttributes::Optional;
						if (t->IsValueType)
						{
							GenericInstanceType^ g = gcnew GenericInstanceType(this->Type_Nullable);
							g->GenericArguments->Add(t);
							t = g;
						}
					}
					if ((param.Attributes & EVariableAttr::V_Out) == EVariableAttr::V_Out)
					{
						attr = attr | ParameterAttributes::Out;
						t = gcnew ByReferenceType(t);
					}
					ParameterDefinition^ p = CreateParameter(CStr2String(param.Name), t, attr);
					method->Parameters->Add(p);
					this->_CodeRefer->AddParameter(param.Tag, p);
				}
				if (pm.Variables.size() > 0)
				{
					method->Body->InitLocals = true;
					ILProcessor^ ILProcessor = method->Body->GetILProcessor();
					for each (ESection_Variable var in pm.Variables)
					{
						TypeReference^ t = this->EDT2Type(var.DataType);
						if (var.ArrayInfo.Dimension > 0) t = gcnew ArrayType(t, var.ArrayInfo.Dimension);
						VariableDefinition^ v = gcnew VariableDefinition(CStr2String(var.Name), t);
						method->Body->Variables->Add(v);
						this->_CodeRefer->AddVariable(var.Tag, v);
					}
				}
				if (name->Length > 4)
				{
					String^ prefix = name->Substring(0, 4);
					if (prefix == "get_")
					{
						if (method->ReturnType != module->TypeSystem->Void)
						{
							name = name->Substring(4);
							PropertyDefinition^ p = FindProperty(type, name);
							if (p == nullptr)
							{
								p = gcnew PropertyDefinition(name, PropertyAttributes::None, method->ReturnType);
								type->Properties->Add(p);
							}
							method->Attributes = method->Attributes | PROPERTYMETHOD;
							p->GetMethod = method;
						}
					}
					else if (prefix == "set_")
					{
						if (method->Parameters->Count > 0)
						{
							name = name->Substring(4);
							PropertyDefinition^ p = FindProperty(type, name);
							if (p == nullptr)
							{
								p = gcnew PropertyDefinition(name, PropertyAttributes::None, method->Parameters[method->Parameters->Count - 1]->ParameterType);
								type->Properties->Add(p);
							}
							method->Attributes = method->Attributes | PROPERTYMETHOD;
							p->SetMethod = method;
						}
					}
				}
				type->Methods->Add(method);
				if (method->Name == "Main")
				{
					if (module->EntryPoint == nullptr) module->EntryPoint = method;
					else if (module->EntryPoint->Parameters->Count == 1 && module->EntryPoint->Parameters[0]->ParameterType == this->Type_StrArr) module->EntryPoint = method;
				}
				EMethodData^ md = gcnew EMethodData(method, ctor ? EMethodMode::Newobj : EMethodMode::Call);
				this->_CodeRefer->AddMethodRefer(CUSTOM, tag, md);
			}
		}
		return true;
	}
	catch (Exception^ ex)
	{
		MessageBox::Show(ex->Message);
		return false;
	}
}

bool ECompile::CompileCode()
{
	try
	{
		ModuleDefinition^ module = this->_assembly->MainModule;
		TypeDefinition^ global = module->GetType("<Module>");
		global->Attributes = global->Attributes | STATICCLASS;
		for each (ESection_Variable var in this->_CodeProcess->GetGlobalVariables())
		{
			if ((var.Attributes & EVariableAttr::V_Extern) == EVariableAttr::V_Extern)
			{

			}
			else
			{
				TypeReference^ t = this->EDT2Type(var.DataType);
				if (var.ArrayInfo.Dimension > 0) t = gcnew ArrayType(t, var.ArrayInfo.Dimension);
				FieldDefinition^ f = gcnew FieldDefinition(CStr2String(var.Name), FieldAttributes::Static, t);
				global->Fields->Add(f);
				this->_CodeRefer->AddGlobalVariable(var.Tag, f);
			}
		}
		for each (ESection_Program_Dll dll in this->_CodeProcess->GetDllList())
		{
			TypeReference^ t = this->EDT2Type(dll.ReturnType);
			if (dll.Remark == "array") t = gcnew ArrayType(t);
			MethodDefinition^ method = gcnew MethodDefinition(CStr2String(dll.ShowName), EXTERNMETHOD, t);
			for each (ESection_Variable param in dll.Parameters)
			{
				TypeReference^ t = this->EDT2Type(param.DataType);
				ParameterAttributes attr = ParameterAttributes::None;
				if ((param.Attributes & EVariableAttr::V_Array) == EVariableAttr::V_Array) t = gcnew ArrayType(t);
				if ((param.Attributes & EVariableAttr::V_Out) == EVariableAttr::V_Out)
				{
					attr = attr | ParameterAttributes::Out;
					t = gcnew ByReferenceType(t);
				}
				ParameterDefinition^ p = CreateParameter(CStr2String(param.Name), t, attr);
				method->Parameters->Add(p);
			}
			method->IsPreserveSig = true;
			String^ dllpath = CStr2String(dll.Lib);
			if (String::IsNullOrEmpty(dllpath))
			{

			}
			else if (String::IsNullOrEmpty(Path::GetExtension(dllpath))) dllpath += ".dll";
			ModuleReference^ dllmodule = gcnew ModuleReference(dllpath);
			AddModule(module->ModuleReferences, dllmodule);
			method->PInvokeInfo = gcnew PInvokeInfo(PInvokeAttributes::CallConvWinapi, CStr2String(dll.Name), dllmodule);
			global->Methods->Add(method);
			EMethodData^ md = gcnew EMethodData(method, EMethodMode::Call);
			this->_CodeRefer->AddMethodRefer(DLL, dll.Tag, md);
		}
		this->LoadKrnln();
		for each (ESection_Program_Assembly assembly in this->_CodeProcess->GetAssemblies())
		{
			TypeDefinition^ type = this->_CodeRefer->FindType(GetTypeName(assembly));
			if (type != nullptr)
			{
				for each (ETAG tag in assembly.Methods)
				{
					ESection_Program_Method pm = this->_CodeProcess->FindMethod(tag);
					if (pm != NULL)
					{
						EMethodInfo^ MethodInfo = gcnew EMethodInfo();
						MethodInfo->MethodInfo = &pm;
						MethodInfo->Method = this->_CodeRefer->FindMethodRefer(CUSTOM, pm.Tag);
						ILProcessor^ ILProcessor = MethodInfo->Method->Body->GetILProcessor();
						size_t len = pm.Code.size();
						byte* lpcode = pm.Code.cbegin()._Ptr;
						vector<UINT> offset = ReadOffset<UINT>(pm.RowsOffset);
						if (!this->CompileCode_Begin(MethodInfo, ILProcessor, lpcode, len, offset)) return false;
						if (pm.ReturnType == DataType::EDT_VOID) AddILCode(ILProcessor, OpCodes::Ret);
					}
				}
			}
		}
		return true;
	}
	catch (Exception^ ex)
	{
		MessageBox::Show(ex->Message);
		return false;
	}
}

bool ECompile::CompileCode_Begin(EMethodInfo^ MethodInfo, ILProcessor^ ILProcessor, byte* Code, size_t Length, vector<UINT> Offset)
{
	try
	{
		ModuleDefinition^ module = this->_assembly->MainModule;
		size_t count = Offset.size();
		size_t i = 0;
		while (i < count)
		{
			byte* current = Code + Offset[i];
			size_t size;
			if (count == i + 1) size = Length - Offset[i];
			else size = Offset[i + 1] - Offset[i];
			switch (GetData<ECode_Head>(current))
			{
			case Call:
				if (this->CompileCode_Call(MethodInfo, ILProcessor, current, current + size) != module->TypeSystem->Void) AddILCode(ILProcessor, OpCodes::Pop);
				i++;
				break;
			case Ife:
			case If:
			case Switch:
			case LoopBegin:
				this->CompileCode_Proc(MethodInfo, ILProcessor, Code, Length, Offset, i);
				break;
			default:
				return false;
			}
		}
		return true;
	}
	catch (Exception^ ex)
	{
		MessageBox::Show(ex->Message);
		return false;
	}
}

TypeReference^ ECompile::CompileCode_Call(EMethodInfo^ MethodInfo, ILProcessor^ ILProcessor, byte*& Code, byte* End)
{
	ModuleDefinition^ module = this->_assembly->MainModule;
	UINT etag = GetData<UINT>(Code);
	short LibID = GetData<short>(Code);
	UINT len;
	if (LibID == -1)
	{
		if ((GetData<USHORT>(Code) & 64) == 64)
		{
			len = GetData<UINT>(Code);
			Code += len + 6;
		}
		else
		{
			Code += 4;
			len = GetData<UINT>(Code);
			Code += len + 2;
		}
	}
	else
	{
		EMethodReference^ mr = this->GetMethodReference(LibID, etag);
		if (mr == nullptr) throw gcnew Exception("使用了未支持函数");
		USHORT tag = GetData<USHORT>(Code);
		if ((tag & 32) == 32)
		{

		}
		else if ((tag & 64) == 64)
		{
			len = GetData<UINT>(Code);
			Code += len + 6;
		}
		else
		{
			Code += 4;
			len = GetData<UINT>(Code);
			Code += len;
			ECode_Type head = GetData<ECode_Type>(Code);
			TypeReference^ thistype;
			IList<Instruction^>^ headcode;
			bool staticmethod = !mr->MethodData->Method->HasThis;
			if (head == ECode_Type::ParameterBegin)
			{
				MethodDefinition^ t = CreateMethod("tmp", module->TypeSystem->Void);
				EVariableData^ vardata = this->CompileCode_Var(MethodInfo, t->Body->GetILProcessor(), Code, End);
				if (vardata == nullptr) throw  Error(mr->Name, "使用了未知类");
				thistype = vardata->Type;
				headcode = t->Body->Instructions;
			}
			else if (!staticmethod)
			{
				thistype = mr->MethodData->Method->DeclaringType;
				AddILCode(ILProcessor, OpCodes::Ldarg_0);
			}
			if (head == ECode_Type::ParamBegin || head == ECode_Type::ParameterBegin)
			{
				if (mr->Tag == krnln_method::返回) mr->Params[0]->Type = MethodInfo->Method->ReturnType;
				IList<EParamData^>^ params = gcnew List<EParamData^>();
				do
				{
					EParamData^ param = gcnew EParamData();
					switch (GetData<ECode_Type>(Code))
					{
					case ECode_Type::NullParam:
						param->Type = module->TypeSystem->Void;
						param->Data = nullptr;
						param->DataType = EParamDataType::Null;
						break;
					case ECode_Type::NumberConst:
					{
						double number = GetData<double>(Code);
						int int32;
						TypeReference^ type = module->TypeSystem->Double;
						try
						{
							Int64 int64 = (Int64)number;
							if (int64 == number)
							{
								int32 = (int)number;
								if (int32 == int64) type = module->TypeSystem->Int32;
								else type = module->TypeSystem->Int64;
							}
						}
						catch (...)
						{
							int32 = (int)number;
							if (int32 == number) type = module->TypeSystem->Int32;
						}
						param->Type = type;
						param->Data = number;
						param->DataType = EParamDataType::Number;
						break;
					}
					case ECode_Type::BoolConst:
					{
						USHORT boolean = GetData<USHORT>(Code);
						param->Type = module->TypeSystem->Boolean;
						param->Data = boolean;
						param->DataType = EParamDataType::Bool;
						break;
					}
					case ECode_Type::DatetimeConst:
					{
						INT64 tick = GetData<INT64>(Code);
						param->Type = this->Type_DateTime;
						param->Data = tick;
						param->DataType = EParamDataType::Time;
						break;
					}
					case ECode_Type::StringConst:
					{
						string text = GetString(Code);
						param->Type = module->TypeSystem->String;
						param->Data = CStr2String(text);
						param->DataType = EParamDataType::String;
						break;
					}
					case ECode_Type::Variable:
					{
						if (GetData<ECode_Type>(Code) == ECode_Type::ParameterBegin)
						{
							MethodDefinition^ t = CreateMethod("tmp", module->TypeSystem->Void);
							EVariableData^ vardata = this->CompileCode_Var(MethodInfo, t->Body->GetILProcessor(), Code, End);
							if (vardata == nullptr || vardata->VariableType == EVariableType::DoNET) throw  Error(mr->Name, "参数" + params->Count + "类型错误");
							IList<Instruction^>^ code;
							if (mr->Tag == krnln_method::赋值 && params->Count == 0)
							{
								mr->Params[0]->Type = vardata->Type;
								mr->Params[1]->Type = vardata->Type;
								code = gcnew List<Instruction^>();
								Instruction^ lastins = t->Body->Instructions[t->Body->Instructions->Count - 1];
								t->Body->Instructions->Remove(lastins);
								for each (Instruction^ ins in t->Body->Instructions) ILProcessor->Append(ins);
								switch (vardata->VariableType)
								{
								case EVariableType::Var:
									param->DataType = EParamDataType::Var;
									lastins->OpCode = OpCodes::Stloc_S;
									break;
								case EVariableType::Param:
									param->DataType = EParamDataType::Param;
									lastins->OpCode = OpCodes::Starg_S;
									break;
								case EVariableType::Field:
								{
									param->DataType = EParamDataType::Field;
									FieldDefinition^ f = (FieldDefinition^)vardata->Data;
									if (f->IsStatic) lastins->OpCode = OpCodes::Stsfld;
									else lastins->OpCode = OpCodes::Stfld;
									break;
								}
								case EVariableType::Property:
								{
									param->DataType = EParamDataType::Property;
									PropertyDefinition^ p = (PropertyDefinition^)vardata->Data;
									lastins->Operand = p->SetMethod;
								}
								case EVariableType::GlobalField:
									param->DataType = EParamDataType::GlobalField;
									lastins->OpCode = OpCodes::Stsfld;
									break;
								case EVariableType::Array:
									param->DataType = EParamDataType::Array;
									if (vardata->Type == module->TypeSystem->Byte) lastins->OpCode = OpCodes::Stelem_I1;
									else if (vardata->Type == module->TypeSystem->Int16) lastins->OpCode = OpCodes::Stelem_I2;
									else if (vardata->Type == module->TypeSystem->Int32) lastins->OpCode = OpCodes::Stelem_I4;
									else if (vardata->Type == module->TypeSystem->Int64) lastins->OpCode = OpCodes::Stelem_I8;
									else if (vardata->Type == module->TypeSystem->Single) lastins->OpCode = OpCodes::Stelem_R4;
									else if (vardata->Type == module->TypeSystem->Double) lastins->OpCode = OpCodes::Stelem_R8;
									else if (vardata->Type == module->TypeSystem->IntPtr) lastins->OpCode = OpCodes::Stelem_I;
									else lastins->OpCode = OpCodes::Stelem_Ref;
									break;
								}
								code->Add(lastins);
							}
							else
							{
								switch (vardata->VariableType)
								{
								case EVariableType::Var:
									param->DataType = EParamDataType::Var;
									break;
								case EVariableType::Param:
									param->DataType = EParamDataType::Param;
									break;
								case EVariableType::Field:
									param->DataType = EParamDataType::Field;
									break;
								case EVariableType::Property:
									param->DataType = EParamDataType::Property;
									break;
								case EVariableType::GlobalField:
									param->DataType = EParamDataType::GlobalField;
									break;
								case EVariableType::Array:
									param->DataType = EParamDataType::Array;
									break;
								}
								code = t->Body->Instructions;
							}
							param->Type = vardata->Type;
							param->Data = code;
						}
						break;
					}
					case ECode_Type::BinConst:
					{
						IList<byte>^ bin;
						do
						{
							switch (GetData<ECode_Type>(Code))
							{
							case ECode_Type::NumberConst:
							{
								double number = GetData<double>(Code);
								bin->Add((byte)number);
								break;
							}
							case ECode_Type::BinEnd:
								goto binend;
							}
						} while (Code < End);
					binend:
						param->Type = this->Type_Bin;
						param->Data = bin;
						param->DataType = EParamDataType::Bin;
						break;
					}
					case ECode_Type::Method:
					{
						MethodDefinition^ t = CreateMethod("tmp", module->TypeSystem->Void);
						TypeReference^ tr = this->CompileCode_Call(MethodInfo, t->Body->GetILProcessor(), Code, End);
						IList<Instruction^>^ code = t->Body->Instructions;
						param->Type = tr;
						param->Data = code;
						param->DataType = EParamDataType::IL;
						break;
					}
					case ECode_Type::ParamEnd:
						goto paramend;
					case ECode_Type::Const:
					case ECode_Type::n4:
						//未完成
						Code += 4;
						param->Type = module->TypeSystem->Void;
						param->Data = nullptr;
						param->DataType = EParamDataType::Null;
						break;
					case ECode_Type::LibConst:
					{
						ELibConstData^ constdata = this->CompileCode_LibConst(GetData<LIBCONST>(Code));
						if (constdata == nullptr) throw Error(mr->Name, "使用了未支持的支持库常量");
						param->Type = constdata->Type;
						param->Data = constdata->Data;
						switch (constdata->ConstType)
						{
						case ELibConstType::Null:
							param->DataType = EParamDataType::Null;
							if (mr->Tag == krnln_method::返回 && params->Count == 0) param->Type = MethodInfo->Method->ReturnType;
							break;
						case ELibConstType::Num:
							param->DataType = EParamDataType::Number;
							break;
						case ELibConstType::Bool:
							param->DataType = EParamDataType::Bool;
							break;
						case ELibConstType::Text:
							param->DataType = EParamDataType::String;
							break;
						}
						break;
					}
					}
					params->Add(param);
				} while (Code < End);
			paramend:
				if (mr->Tag == krnln_method::重定义数组 && params->Count >= 3)
				{
					EParamData^ param = gcnew EParamData();
					param->Type = module->ImportReference(typeof(RuntimeTypeHandle));
					IList<Instruction^>^ codes = gcnew List<Instruction^>();
					codes->Add(Instruction::Create(OpCodes::Ldtoken, params[0]->Type));
					param->Data = codes;
					param->DataType = EParamDataType::IL;
					params->Insert(0, param);
				}
				EMethodData^ md;
				if (mr->Params->Length == 0 && params->Count == 0) md = mr->MethodData;
				else
				{
					String^ tagName = GetMethodName(mr->MethodData);
					IList<EMethodData^>^ mdlist = nullptr;
					if (!(LibID == krnln_id && (etag == krnln_method::赋值 || etag == krnln_method::返回))) mdlist = this->_CodeRefer->FindMethodList(tagName);
					IList<EMethodReference^>^ conform = gcnew List<EMethodReference^>();
					if (mdlist != nullptr)
					{
						mr = nullptr;
						TypeReference^ paatr = module->ImportReference(typeof(ParamArrayAttribute));
						TypeReference^ oatr = module->ImportReference(typeof(OptionalAttribute));
						for each (EMethodData^ item in mdlist)
						{
							if (item->Method->Parameters->Count == params->Count)
							{
								mr = this->GetMethodReference(item, LibID, etag);
								conform->Add(mr);
							}
							else if (item->Method->Parameters->Count > 0)
							{
								int ii = item->Method->Parameters->Count - 1;
								ParameterDefinition^ pd = item->Method->Parameters[ii];
								if (FindCustom(pd->CustomAttributes, paatr) != nullptr)
								{
									if (params->Count >= ii)
									{
										mr = this->GetMethodReference(item, LibID, etag);
										conform->Add(mr);
									}
								}
								else if (params->Count < item->Method->Parameters->Count)
								{
									int i;
									for (i = item->Method->Parameters->Count - params->Count; i > 0; i--)
									{
										ParameterDefinition^ pd = item->Method->Parameters[item->Method->Parameters->Count - i];
										if (!pd->IsOptional && FindCustom(pd->CustomAttributes, oatr) == nullptr) break;
									}
									if (i == 0)
									{
										mr = this->GetMethodReference(item, LibID, etag);
										conform->Add(mr);
									}
								}
							}
						}
						if (mr == nullptr) throw Error(tagName, "参数不符");
					}
					else conform->Add(mr);
					IDictionary<EMethodReference^, float>^ similar = gcnew Dictionary<EMethodReference^, float>();
					mr = nullptr;
					for each (EMethodReference^ item in conform)
					{
						int satisfy = 0;
						float similarity = 0;
						array<EParamInfo^>^ mrparams = item->Params;
						IList<EParamData^>^ tparams = gcnew List<EParamData^>(params);
						if (mrparams->Length > tparams->Count)
						{
							for (int i = mrparams->Length - tparams->Count; i > 0; i--)
							{
								if (mrparams[mrparams->Length - i]->IsOptional)
								{
									EParamData^ param = gcnew EParamData();
									param->Type = module->TypeSystem->Void;
									param->Data = nullptr;
									param->DataType = EParamDataType::Null;
									tparams->Add(param);
								}
								else break;
							}
						}
						for (size_t i = 0; i < tparams->Count; i++)
						{
							if (i < mrparams->Length)
							{
								if (mrparams[i]->Type == tparams[i]->Type || IsAssignableFrom(tparams[i]->Type, mrparams[i]->Type))
								{
									similarity++;
									satisfy++;
								}
								else if (tparams[i]->Type == module->TypeSystem->Void && mrparams[i]->IsOptional)
								{
									similarity++;
									satisfy++;
								}
								else if (mrparams[i]->Type == module->TypeSystem->Object)
								{
									similarity += 0.45;
									satisfy++;
								}
								else if (mrparams[i]->Type->IsValueType && tparams[i]->Type->IsValueType)
								{
									if ((mrparams[i]->Type == module->TypeSystem->Byte || mrparams[i]->Type == module->TypeSystem->Int16 || mrparams[i]->Type == module->TypeSystem->Int32 || mrparams[i]->Type == module->TypeSystem->Int64) && (params[i]->Type == module->TypeSystem->Byte || tparams[i]->Type == module->TypeSystem->Int16 || tparams[i]->Type == module->TypeSystem->Int32 || tparams[i]->Type == module->TypeSystem->Int64))
									{
										similarity += 0.5;
										satisfy++;
									}
									else if ((mrparams[i]->Type == module->TypeSystem->Single || mrparams[i]->Type == module->TypeSystem->Double || mrparams[i]->Type == module->TypeSystem->Int32) && (params[i]->Type == module->TypeSystem->Single || tparams[i]->Type == module->TypeSystem->Double || tparams[i]->Type == module->TypeSystem->Int32))
									{
										similarity += 0.5;
										satisfy++;
									}
									else if ((mrparams[i]->Type == module->TypeSystem->IntPtr || mrparams[i]->Type == module->TypeSystem->Int32) && (params[i]->Type == module->TypeSystem->IntPtr || tparams[i]->Type == module->TypeSystem->Int32))
									{
										similarity += 0.5;
										satisfy++;
									}
								}
								else if (mrparams[i]->IsVariable && (i + 1) == mrparams->Length) goto isvar;
							}
							else
							{
							isvar:
								EParamInfo^ pi = mrparams[mrparams->Length - 1];
								if (pi->IsVariable)
								{
									ArrayType^ arrtype = dynamic_cast<ArrayType^>(pi->Type);
									if (arrtype != nullptr)
									{
										if (arrtype->ElementType == tparams[i]->Type || IsAssignableFrom(params[i]->Type, arrtype->ElementType))
										{
											similarity++;
											satisfy++;
										}
										else if (arrtype->ElementType == module->TypeSystem->Object)
										{
											similarity += 0.5;
											satisfy++;
										}
										else if (arrtype->ElementType->IsValueType && tparams[i]->Type->IsValueType)
										{
											if ((arrtype->ElementType == module->TypeSystem->Byte || arrtype->ElementType == module->TypeSystem->Int16 || arrtype->ElementType == module->TypeSystem->Int32 || arrtype->ElementType == module->TypeSystem->Int64) && (params[i]->Type == module->TypeSystem->Byte || tparams[i]->Type == module->TypeSystem->Int16 || tparams[i]->Type == module->TypeSystem->Int32 || tparams[i]->Type == module->TypeSystem->Int64))
											{
												similarity += 0.5;
												satisfy++;
											}
											else if ((arrtype->ElementType == module->TypeSystem->Single || arrtype->ElementType == module->TypeSystem->Double || arrtype->ElementType == module->TypeSystem->Int32) && (params[i]->Type == module->TypeSystem->Single || tparams[i]->Type == module->TypeSystem->Double || tparams[i]->Type == module->TypeSystem->Int32))
											{
												similarity += 0.5;
												satisfy++;
											}
											else if ((arrtype->ElementType == module->TypeSystem->IntPtr || arrtype->ElementType == module->TypeSystem->Int32) && (params[i]->Type == module->TypeSystem->IntPtr || tparams[i]->Type == module->TypeSystem->Int32))
											{
												similarity += 0.5;
												satisfy++;
											}
										}
									}
								}
							}
						}
						if (similarity == tparams->Count)
						{
							mr = item;
							params = tparams;
							break;
						}
						else if (satisfy == tparams->Count) similar->Add(item, similarity);
					}
					if (mr == nullptr)
					{
						float similarity = 0;
						for each (KeyValuePair<EMethodReference^, float>^ item in similar)
						{
							if (similarity < item->Value)
							{
								similarity = item->Value;
								mr = item->Key;
							}
						}
						if (mr == nullptr) throw Error(tagName, "参数不符");
						if (mr->Params->Length > params->Count)
						{
							for (int i = mr->Params->Length - params->Count; i > 0; i--)
							{
								if (mr->Params[mr->Params->Length - i]->IsOptional)
								{
									EParamData^ param = gcnew EParamData();
									param->Type = module->TypeSystem->Void;
									param->Data = nullptr;
									param->DataType = EParamDataType::Null;
									params->Add(param);
								}
								else break;
							}
						}
					}
					md = mr->MethodData;
					if (mr->Tag == krnln_method::赋值)
					{
						EParamInfo^ item = mr->Params[1];
						EParamData^ param = params[1];
						switch (param->DataType)
						{
						case EParamDataType::Number:
						{
							if (param->Type == module->TypeSystem->Int32) AddILCode(ILProcessor, OpCodes::Ldc_I4, (int)Convert::ChangeType(param->Data, typeof(int)));
							else if (param->Type == module->TypeSystem->Int64) AddILCode(ILProcessor, OpCodes::Ldc_I8, (Int64)Convert::ChangeType(param->Data, typeof(Int64)));
							else if (param->Type == module->TypeSystem->Double) AddILCode(ILProcessor, OpCodes::Ldc_R8, (double)param->Data);
							if (item->Type->IsValueType)
							{
								if (item->Type == module->TypeSystem->Byte || item->Type == module->TypeSystem->Int16 || item->Type == module->TypeSystem->Int32)
								{
									if (param->Type != module->TypeSystem->Int32) AddILCode(ILProcessor, OpCodes::Conv_I4);
								}
								else if (item->Type == module->TypeSystem->Int64)
								{
									if (param->Type != module->TypeSystem->Int64) AddILCode(ILProcessor, OpCodes::Conv_I8);
								}
								else if (item->Type == module->TypeSystem->Single) AddILCode(ILProcessor, OpCodes::Conv_R4);
								else if (item->Type == module->TypeSystem->Double)
								{
									if (param->Type != module->TypeSystem->Double) AddILCode(ILProcessor, OpCodes::Conv_R8);
								}
								else if (item->Type == module->TypeSystem->IntPtr)
								{
									if (param->Type == module->TypeSystem->Int32) AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(IntPtr, "op_Explicit", typeof(int))));
									else if (param->Type == module->TypeSystem->Int64) AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(IntPtr, "op_Explicit", typeof(Int64))));
									else if (param->Type == module->TypeSystem->Double)
									{
										AddILCode(ILProcessor, OpCodes::Conv_I8);
										AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(IntPtr, "op_Explicit", typeof(Int64))));
									}
								}
							}
							else if (item->Type == module->TypeSystem->Object) AddILCode(ILProcessor, OpCodes::Box, param->Type);
							break;
						}
						case EParamDataType::Bool:
						{
							if ((USHORT)param->Data == 0) AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
							else AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
							if (item->Type == module->TypeSystem->Object) AddILCode(ILProcessor, OpCodes::Box, module->TypeSystem->Boolean);
							break;
						}
						case EParamDataType::Time:
						{
							AddILCode(ILProcessor, OpCodes::Ldc_I8, (Int64)param->Data);
							AddILCode(ILProcessor, OpCodes::Newobj, module->ImportReference(GetCtor(DateTime, typeof(Int64))));
							break;
						}
						case EParamDataType::String:
						{
							AddILCode(ILProcessor, OpCodes::Ldstr, (String^)param->Data);
							break;
						}
						case EParamDataType::Bin:
						{
							IList<byte>^ bin = (IList<byte>^)param->Data;
							AddILCode(ILProcessor, OpCodes::Ldc_I4, bin->Count);
							AddILCode(ILProcessor, OpCodes::Newarr, module->TypeSystem->Byte);
							for (int i = 0; i < bin->Count; i++)
							{
								AddILCode(ILProcessor, OpCodes::Dup);
								AddILCode(ILProcessor, OpCodes::Ldc_I4, i);
								AddILCode(ILProcessor, OpCodes::Ldc_I4, bin[i]);
								AddILCode(ILProcessor, OpCodes::Stelem_I1);
							}
							break;
						}
						case EParamDataType::Param:
						case EParamDataType::Var:
						case EParamDataType::Field:
						case EParamDataType::Property:
						case EParamDataType::GlobalField:
						case EParamDataType::Array:
						case EParamDataType::IL:
						{
							IList<Instruction^>^ code = (IList<Instruction^>^)param->Data;
							for each (Instruction^ item in code) ILProcessor->Append(item);
							break;
						}
						}
						item = mr->Params[0];
						param = params[0];
						switch (param->DataType)
						{
						case EParamDataType::Param:
						case EParamDataType::Var:
						case EParamDataType::Field:
						case EParamDataType::Property:
						case EParamDataType::GlobalField:
						case EParamDataType::Array:
						case EParamDataType::IL:
						{
							IList<Instruction^>^ code = (IList<Instruction^>^)param->Data;
							for each (Instruction^ item in code) ILProcessor->Append(item);
							break;
						}
						}
					}
					else
					{
						if (headcode != nullptr && md->Mode != EMethodMode::Newobj) for each (Instruction^ item in headcode) ILProcessor->Append(item);
						int i = 0;
						for each (EParamInfo^ item in  mr->Params)
						{
							int ii = 0;
							if (item->IsVariable)
							{
								ArrayType^ arrtype = dynamic_cast<ArrayType^>(item->Type);
								if (arrtype != nullptr) item->Type = arrtype->ElementType;
								AddILCode(ILProcessor, OpCodes::Ldc_I4, params->Count - i);
								AddILCode(ILProcessor, OpCodes::Newarr, item->Type);
							}
							while (i < params->Count)
							{
								if (item->IsVariable)
								{
									AddILCode(ILProcessor, OpCodes::Dup);
									AddILCode(ILProcessor, OpCodes::Ldc_I4, ii);
								}
								else if (ii > 0) goto end;
								EParamData^ param = params[i];
								switch (param->DataType)
								{
								case EParamDataType::Null:
									if (item->Defualt == nullptr)
									{
										if (item->Type->IsValueType)
										{
											if (item->OriginalType->Name == "Nullable`1")
											{
												MethodInfo->Method->Body->InitLocals = true;
												VariableDefinition^ var = gcnew VariableDefinition(module->ImportReference(item->OriginalType));
												MethodInfo->Method->Body->Variables->Add(var);
												AddILCode(ILProcessor, OpCodes::Ldloca_S, var);
												AddILCode(ILProcessor, OpCodes::Initobj, item->OriginalType);
												AddILCode(ILProcessor, OpCodes::Ldloc_S, var);
											}
											else
											{
												if (item->Type == module->TypeSystem->Int64) AddILCode(ILProcessor, OpCodes::Ldc_I8, 0);
												else if (item->Type == module->TypeSystem->Single) AddILCode(ILProcessor, OpCodes::Ldc_R4, 0);
												else if (item->Type == module->TypeSystem->Double) AddILCode(ILProcessor, OpCodes::Ldc_R8, 0);
												else AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
											}
										}
										else AddILCode(ILProcessor, OpCodes::Ldnull);
									}
									else
									{
										if (item->Type == module->TypeSystem->Byte || item->Type == module->TypeSystem->Int16 || item->Type == module->TypeSystem->Int32 || item->Type == module->TypeSystem->Boolean) AddILCode(ILProcessor, OpCodes::Ldc_I4, (int)item->Defualt);
										else if (item->Type == module->TypeSystem->Int64) AddILCode(ILProcessor, OpCodes::Ldc_I8, (Int64)item->Defualt);
										else if (item->Type == module->TypeSystem->Single) AddILCode(ILProcessor, OpCodes::Ldc_R4, (float)item->Defualt);
										else if (item->Type == module->TypeSystem->Double) AddILCode(ILProcessor, OpCodes::Ldc_R8, (double)item->Defualt);
										else if (item->Type == module->TypeSystem->String) AddILCode(ILProcessor, OpCodes::Ldstr, (String^)item->Defualt);
										else AddILCode(ILProcessor, OpCodes::Ldnull);
									}
									break;
								case EParamDataType::Number:
								{
									if (param->Type == module->TypeSystem->Int32) AddILCode(ILProcessor, OpCodes::Ldc_I4, (int)Convert::ChangeType(param->Data, typeof(int)));
									else if (param->Type == module->TypeSystem->Int64) AddILCode(ILProcessor, OpCodes::Ldc_I8, (Int64)Convert::ChangeType(param->Data, typeof(Int64)));
									else if (param->Type == module->TypeSystem->Double) AddILCode(ILProcessor, OpCodes::Ldc_R8, (double)param->Data);
									if (item->Type->IsValueType)
									{
										if (item->Type == module->TypeSystem->Byte || item->Type == module->TypeSystem->Int16 || item->Type == module->TypeSystem->Int32)
										{
											if (param->Type != module->TypeSystem->Int32) AddILCode(ILProcessor, OpCodes::Conv_I4);
										}
										else if (item->Type == module->TypeSystem->Int64)
										{
											if (param->Type != module->TypeSystem->Int64) AddILCode(ILProcessor, OpCodes::Conv_I8);
										}
										else if (item->Type == module->TypeSystem->Single) AddILCode(ILProcessor, OpCodes::Conv_R4);
										else if (item->Type == module->TypeSystem->Double)
										{
											if (param->Type != module->TypeSystem->Double) AddILCode(ILProcessor, OpCodes::Conv_R8);
										}
										else if (item->Type == module->TypeSystem->IntPtr)
										{
											if (param->Type == module->TypeSystem->Int32) AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(IntPtr, "op_Explicit", typeof(int))));
											else if (param->Type == module->TypeSystem->Int64) AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(IntPtr, "op_Explicit", typeof(Int64))));
											else if (param->Type == module->TypeSystem->Double)
											{
												AddILCode(ILProcessor, OpCodes::Conv_I8);
												AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(IntPtr, "op_Explicit", typeof(Int64))));
											}
										}
									}
									break;
								}
								case EParamDataType::Bool:
								{
									if ((USHORT)param->Data == 0) AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
									else AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
									break;
								}
								case EParamDataType::Time:
								{
									AddILCode(ILProcessor, OpCodes::Ldc_I8, (Int64)param->Data);
									AddILCode(ILProcessor, OpCodes::Newobj, module->ImportReference(GetCtor(DateTime, typeof(Int64))));
									break;
								}
								case EParamDataType::String:
								{
									AddILCode(ILProcessor, OpCodes::Ldstr, (String^)param->Data);
									break;
								}
								case EParamDataType::Bin:
								{
									IList<byte>^ bin = (IList<byte>^)param->Data;
									AddILCode(ILProcessor, OpCodes::Ldc_I4, bin->Count);
									AddILCode(ILProcessor, OpCodes::Newarr, module->TypeSystem->Byte);
									for (int i = 0; i < bin->Count; i++)
									{
										AddILCode(ILProcessor, OpCodes::Dup);
										AddILCode(ILProcessor, OpCodes::Ldc_I4, i);
										AddILCode(ILProcessor, OpCodes::Ldc_I4, bin[i]);
										AddILCode(ILProcessor, OpCodes::Stelem_I1);
									}
									break;
								}
								case EParamDataType::Param:
								case EParamDataType::Var:
								case EParamDataType::Field:
								case EParamDataType::Property:
								case EParamDataType::GlobalField:
								case EParamDataType::Array:
								{
									IList<Instruction^>^ code = (IList<Instruction^>^)param->Data;
									if (item->IsAddress)
									{
										if (param->DataType == EParamDataType::Property) throw Error(item->Name, "属性不能传址");
										Instruction^ lastins = code[code->Count - 1];
										switch (param->DataType)
										{
										case EParamDataType::Param:
											lastins->OpCode = OpCodes::Ldarga_S;
											break;
										case EParamDataType::Var:
											lastins->OpCode = OpCodes::Ldloca_S;
											break;
										case EParamDataType::Field:
										{
											if (lastins->OpCode == OpCodes::Ldfld) lastins->OpCode = OpCodes::Ldflda;
											else lastins->OpCode = OpCodes::Ldsflda;
											break;
										}
										case EParamDataType::GlobalField:
											lastins->OpCode = OpCodes::Ldsflda;
											break;
										case EParamDataType::Array:
											lastins->OpCode = OpCodes::Ldelema;
											break;
										}
									}
									for each (Instruction^ item in code) ILProcessor->Append(item);
									break;
								}
								case EParamDataType::IL:
								{
									IList<Instruction^>^ code = (IList<Instruction^>^)param->Data;
									for each (Instruction^ item in code) ILProcessor->Append(item);
									break;
								}
								}
								if (param->Type->IsValueType)
								{
									if (item->IsOptional && item->OriginalType->Name == "Nullable`1")
									{
										GenericInstanceType^ g = gcnew GenericInstanceType(this->Type_Nullable);
										g->GenericArguments->Add(param->Type);
										MethodReference^ m = gcnew MethodReference(".ctor", module->TypeSystem->Void, g);
										m->HasThis = true;
										m->Parameters->Add(this->Nullable_Ctor->Parameters[0]);
										AddILCode(ILProcessor, OpCodes::Newobj, module->ImportReference(m));
									}
									else if (!item->IsAddress && param->Type->IsValueType && item->OriginalType == module->TypeSystem->Object) AddILCode(ILProcessor, OpCodes::Box, param->Type);
								}
								i++;
								ii++;
								if (item->IsVariable)
								{
									if (item->Type == module->TypeSystem->Byte) AddILCode(ILProcessor, OpCodes::Stelem_I1);
									else if (item->Type == module->TypeSystem->Int16) AddILCode(ILProcessor, OpCodes::Stelem_I2);
									else if (item->Type == module->TypeSystem->Int32) AddILCode(ILProcessor, OpCodes::Stelem_I4);
									else if (item->Type == module->TypeSystem->Int64) AddILCode(ILProcessor, OpCodes::Stelem_I8);
									else if (item->Type == module->TypeSystem->Single) AddILCode(ILProcessor, OpCodes::Stelem_R4);
									else if (item->Type == module->TypeSystem->Double) AddILCode(ILProcessor, OpCodes::Stelem_R8);
									else if (item->Type == module->TypeSystem->IntPtr) AddILCode(ILProcessor, OpCodes::Stelem_I);
									else AddILCode(ILProcessor, OpCodes::Stelem_Ref);
								}
							}
							break;
						end:;
						}
					}
				}
				switch (md->Mode)
				{
				case EMethodMode::Call:
					AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(md->Method));
					break;
				case EMethodMode::Newobj:
					AddILCode(ILProcessor, OpCodes::Newobj, module->ImportReference(md->Method));
					mr->ReturnType = thistype;
					break;
				case EMethodMode::Embed:
				{
					MethodDefinition^ method = md;
					for each (Instruction^ code in method->Body->Instructions) if (code != nullptr) ILProcessor->Append(code);
					break;
				}
				}
				return mr->ReturnType;
			}
		}
	}
	return module->TypeSystem->Void;
}

EVariableData^ ECompile::CompileCode_Var(EMethodInfo^ MethodInfo, ILProcessor^ ILProcessor, byte*& Code, byte* End)
{
	ModuleDefinition^ module = this->_assembly->MainModule;
	ETAG tag = GetData<ETAG>(Code);
	EVariableData^ vardata = gcnew EVariableData();
	IList<EVariableIndex^>^ varindex = gcnew List<EVariableIndex^>();
	do
	{
		EVariableIndex^ vi;
		switch (GetData<ECode_Type>(Code))
		{
		case ECode_Type::ParameterEnd:
			goto varend;
		case ECode_Type::ArrayBegin:
		{
			vi = gcnew EVariableIndex();
			EVariableIndex^ tvi = gcnew EVariableIndex();
			vi->IndexType = EIndexType::Array;
			switch (GetData<ECode_Type>(Code))
			{
			case ECode_Type::NumberIndex:
				tvi->IndexType = EIndexType::Number;
				tvi->IndexData = GetData<int>(Code);
				break;
			case ECode_Type::ParameterBegin:
			{
				tvi->IndexType = EIndexType::Var;
				MethodDefinition^ t = CreateMethod("tmp", module->TypeSystem->Void);
				EVariableData^ tvd = this->CompileCode_Var(MethodInfo, t->Body->GetILProcessor(), Code, End);
				if (tvd == nullptr || tvd->VariableType == EVariableType::DoNET || (tvd->Type != module->TypeSystem->Byte && tvd->Type != module->TypeSystem->Int16 && tvd->Type != module->TypeSystem->Int32 && tvd->Type != module->TypeSystem->Int64)) return nullptr;
				tvi->IndexData = t->Body->Instructions;
				break;
			}
			case ECode_Type::Method:
			{
				tvi->IndexType = EIndexType::Method;
				MethodDefinition^ t = CreateMethod("tmp", module->TypeSystem->Void);
				TypeReference^ ttype = this->CompileCode_Call(MethodInfo, t->Body->GetILProcessor(), Code, End);
				if (ttype != module->TypeSystem->Byte && ttype != module->TypeSystem->Int16 && ttype != module->TypeSystem->Int32 && ttype != module->TypeSystem->Int64) return nullptr;
				tvi->IndexData = t->Body->Instructions;
				break;
			}
			}
			vi->IndexData = tvi;
			varindex->Add(vi);
			break;
		}
		case ECode_Type::Field:
			vi = gcnew EVariableIndex();
			vi->IndexType = EIndexType::Field;
			vi->IndexData = (UINT64)GetData<EFieldInfo>(Code);
			varindex->Add(vi);
			break;
		default:
			return nullptr;
		}
	} while (Code < End);
varend:
	VariableDefinition^ v;
	ParameterDefinition^ p;
	FieldDefinition^ f;
	FieldDefinition^ g;
	PropertyDefinition^ pp;
	bool donetnp = false;
	switch (tag.Type2)
	{
	case Variable:
		v = this->_CodeRefer->FindVariable(tag);
		if (v == nullptr) p = this->_CodeRefer->FindParameter(tag);
		break;
	case Field:
		f = this->_CodeRefer->FindField(tag);
		break;
	case GlobalField:
		g = this->_CodeRefer->FindGlobalVariable(tag);
		if (g == nullptr)
		{
			ESection_Variable gv = this->_CodeProcess->FindGlobalVariable(tag);
			if (gv != NULL && (gv.Attributes & EVariableAttr::V_Extern) == EVariableAttr::V_Extern)
			{
				vector<string> arr = split(gv.Remark, SP);
				if (arr[0] == DONET_CLASS || arr[0] == DONET_NAMESPACE) donetnp = true;
				else
				{
					TypeReference^ t = this->EDT2Type(gv.DataType);
					if (gv.ArrayInfo.Dimension > 0) t = gcnew ArrayType(t, gv.ArrayInfo.Dimension);
					g = gcnew FieldDefinition(CStr2String(gv.Name), FieldAttributes::Static, t);
					TypeDefinition^ global = module->GetType("<Module>");
					global->Fields->Add(g);
					this->_CodeRefer->AddGlobalVariable(gv.Tag, g);
				}
			}
		}
		break;
	}
	if (v != nullptr)
	{
		AddILCode(ILProcessor, OpCodes::Ldloc_S, v);
		vardata->Type = v->VariableType;
		if (varindex->Count == 0)
		{
			vardata->VariableType = EVariableType::Var;
			vardata->Data = v;
		}
	}
	else if (p != nullptr)
	{
		AddILCode(ILProcessor, OpCodes::Ldarg_S, p);
		vardata->Type = p->ParameterType;
		if (varindex->Count == 0)
		{
			vardata->VariableType = EVariableType::Param;
			vardata->Data = p;
		}
	}
	else if (f != nullptr)
	{
		if (f->IsStatic) AddILCode(ILProcessor, OpCodes::Ldsfld, f);
		else
		{
			AddILCode(ILProcessor, OpCodes::Ldarg_0);
			AddILCode(ILProcessor, OpCodes::Ldfld, f);
		}
		vardata->Type = f->FieldType;
		if (varindex->Count == 0)
		{
			vardata->VariableType = EVariableType::Field;
			vardata->Data = f;
		}
	}
	else if (g != nullptr)
	{
		AddILCode(ILProcessor, OpCodes::Ldsfld, g);
		vardata->Type = g->FieldType;
		if (varindex->Count == 0)
		{
			vardata->VariableType = EVariableType::GlobalField;
			vardata->Data = g;
		}
	}
	else
	{
		if (donetnp)
		{
			vardata->VariableType = EVariableType::DoNET;
			return vardata;
		}
		else return nullptr;
	}
	if (varindex->Count > 0)
	{
		for (size_t i = 0; i < varindex->Count; i++)
		{
			EVariableIndex^ item = varindex[i];
			switch (item->IndexType)
			{
			case EIndexType::Array:
			{
				ArrayType^ arrtype = dynamic_cast<ArrayType^>(vardata->Type);
				if (arrtype == nullptr) return nullptr;
				vardata->VariableType = EVariableType::Array;
				vardata->Type = arrtype->ElementType;
				vardata->Data = arrtype;
				size_t ii = 0;
				do
				{
					item = (EVariableIndex^)item->IndexData;
					switch (item->IndexType)
					{
					case EIndexType::Number:
						AddILCode(ILProcessor, OpCodes::Ldc_I4, (int)item->IndexData);
						break;
					case EIndexType::Var:
					case EIndexType::Method:
					{
						IList<Instruction^>^ tcode = (IList<Instruction^>^)item->IndexData;
						for each (Instruction^ ins in tcode) ILProcessor->Append(ins);
						break;
					}
					}
					if (ii > 0)
					{
						AddILCode(ILProcessor, OpCodes::Mul);
						goto next;

					}
					else if (dynamic_cast<ArrayType^>(vardata->Type)) break;
					else
					{
					next:
						if (varindex->Count > i + 1)
						{
							item = varindex[i + 1];
							if (item->IndexType != EIndexType::Array) break;
							i++;
						}
						else break;
					}
					ii++;
				} while (true);
				AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
				AddILCode(ILProcessor, OpCodes::Sub);
				if (vardata->Type == module->TypeSystem->Byte) AddILCode(ILProcessor, OpCodes::Ldelem_I1);
				else if (vardata->Type == module->TypeSystem->Int16) AddILCode(ILProcessor, OpCodes::Ldelem_I2);
				else if (vardata->Type == module->TypeSystem->Int32) AddILCode(ILProcessor, OpCodes::Ldelem_I4);
				else if (vardata->Type == module->TypeSystem->Int64) AddILCode(ILProcessor, OpCodes::Ldelem_I8);
				else if (vardata->Type == module->TypeSystem->Single) AddILCode(ILProcessor, OpCodes::Ldelem_R4);
				else if (vardata->Type == module->TypeSystem->Double) AddILCode(ILProcessor, OpCodes::Ldelem_R8);
				else if (vardata->Type == module->TypeSystem->IntPtr) AddILCode(ILProcessor, OpCodes::Ldelem_I);
				else AddILCode(ILProcessor, OpCodes::Ldelem_Ref);
				break;
			}
			case EIndexType::Field:
			{
				EFieldInfo fi = (UINT64)item->IndexData;
				FieldDefinition^ fd;
				PropertyDefinition^ pd;
				fd = this->_CodeRefer->FindField(fi.Field);
				if (fd == nullptr) pd = this->_CodeRefer->FindProperty(fi.Field);
				if (fd == nullptr && pd == nullptr)
				{
					ESection_Program_Assembly assembly = this->_CodeProcess->FindReferStruct(fi.Class);
					if (assembly == NULL) return nullptr;
					ESection_Variable var = assembly.FindField(fi.Field);
					if (var == NULL) return nullptr;
					TypeDefinition^ type = this->FindTypeDefinition(fi.Class);
					if (type == nullptr) return nullptr;
					String^ varname = CStr2String(var.Name);
					fd = FindField(type, varname);
					if (fd == nullptr)
					{
						pd = FindProperty(type, varname);
						if (pd == nullptr) return nullptr;
						this->_CodeRefer->AddProperty(var.Tag, pd);
					}
					else this->_CodeRefer->AddField(var.Tag, fd);
				}
				if (fd == nullptr)
				{
					vardata->VariableType = EVariableType::Field;
					vardata->Type = fd->FieldType;
					vardata->Data = fd;
					if (fd->IsStatic) AddILCode(ILProcessor, OpCodes::Ldsfld, fd);
					else AddILCode(ILProcessor, OpCodes::Ldfld, fd);
				}
				else
				{
					vardata->VariableType = EVariableType::Property;
					vardata->Type = pd->PropertyType;
					vardata->Data = pd;
					AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(pd->GetMethod));
				}
				break;
			}
			}
		}
	}
	return vardata;
}

ELibConstData^ ECompile::CompileCode_LibConst(LIBCONST libconst)
{
	libconst.ID--;
	libconst.LibID--;
	if (libconst.ID < 0 || libconst.LibID < 0) return nullptr;
	PLIB_INFO info = NULL;
	if (libconst.LibID == this->e_net_id) info = GetNewInf();
	else
	{
		ESection_Library libinfo = this->_CodeProcess->GetLibraries()[libconst.LibID];
		string libname = libinfo.FileName + ".fne";
		HMODULE module = GetModuleHandle(libname.c_str());
		if (module == NULL) return nullptr;
		PFN_GET_LIB_INFO GetNewInf = (PFN_GET_LIB_INFO)GetProcAddress(module, FUNCNAME_GET_LIB_INFO);
		info = GetNewInf();
	}
	if (info == NULL) return nullptr;
	ModuleDefinition^ module = this->_assembly->MainModule;
	LIB_CONST_INFO constinfo = info->m_pLibConst[libconst.ID];
	ELibConstData^ data = gcnew ELibConstData();
	data->ConstType = (ELibConstType)constinfo.m_shtType;
	switch (data->ConstType)
	{
	case ELibConstType::Null:
		data->Data = nullptr;
		data->Type = module->TypeSystem->Void;
		break;
	case ELibConstType::Num:
	{
		double number = constinfo.m_dbValue;
		int int32;
		TypeReference^ type = module->TypeSystem->Double;
		try
		{
			Int64 int64 = (Int64)number;
			if (int64 == number)
			{
				int32 = (int)number;
				if (int32 == int64) type = module->TypeSystem->Int32;
				else type = module->TypeSystem->Int64;
			}
		}
		catch (...)
		{
			int32 = (int)number;
			if (int32 == number) type = module->TypeSystem->Int32;
		}
		data->Data = number;
		data->Type = type;
		break;
	}
	case ELibConstType::Bool:
		data->Data = constinfo.m_dbValue;
		data->Type = module->TypeSystem->Boolean;
		break;
	case ELibConstType::Text:
		data->Data = LPSTR2String(constinfo.m_szText);
		data->Type = module->TypeSystem->String;
		break;
	}
	return data;
}

void ECompile::CompileCode_Proc(EMethodInfo^ MethodInfo, ILProcessor^ ILProcessor, byte* Code, size_t Length, vector<UINT> Offset, size_t& Index)
{
	ModuleDefinition^ module = this->_assembly->MainModule;
	Instruction^ tag;
	Instruction^ end;
	VariableDefinition^ var;
	VariableDefinition^ var2;
	size_t count = Offset.size();
	byte* current = Code + Offset[Index];
	size_t size;
	if (count == Index + 1) size = Length - Offset[Index];
	else size = Offset[Index + 1] - Offset[Index];
	switch (GetData<ECode_Head>(current))
	{
	case Ife:
	case If:
	case Switch:
		this->CompileCode_Call(MethodInfo, ILProcessor, current, current + size);
		Index++;
		tag = ILProcessor->Create(OpCodes::Nop);
		AddILCode(ILProcessor, OpCodes::Brfalse_S, tag);
		end = ILProcessor->Create(OpCodes::Nop);
		break;
	case LoopBegin:
	{
		tag = ILProcessor->Create(OpCodes::Nop);
		ILProcessor->Append(tag);
		ETAG etag = GetData<ETAG>(current);
		current -= sizeof(ETAG);
		this->CompileCode_Call(MethodInfo, ILProcessor, current, current + size);
		Index++;
		switch ((UINT)etag)
		{
		case 判断循环首:
			end = ILProcessor->Create(OpCodes::Nop);
			AddILCode(ILProcessor, OpCodes::Brfalse_S, end);
			break;
		case 循环判断首:
			break;
		case 计次循环首:
		{
			end = ILProcessor->Create(OpCodes::Nop);
			var = gcnew VariableDefinition(module->TypeSystem->Int32);
			MethodInfo->Method->Body->Variables->Add(var);
			VariableDefinition^ bl = gcnew VariableDefinition(module->TypeSystem->Int32);
			MethodInfo->Method->Body->Variables->Add(bl);
			var2 = gcnew VariableDefinition(module->TypeSystem->Int32);
			MethodInfo->Method->Body->Variables->Add(var2);
			AddILCode(ILProcessor, OpCodes::Stloc_S, bl);
			AddILCode(ILProcessor, OpCodes::Stloc_S, var2);
			AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
			AddILCode(ILProcessor, OpCodes::Stloc_S, var);
			tag = ILProcessor->Create(OpCodes::Ldloc_S, var);
			ILProcessor->Append(tag);
			AddILCode(ILProcessor, OpCodes::Ldloc_S, var2);
			AddILCode(ILProcessor, OpCodes::Bgt_S, end);
			AddILCode(ILProcessor, OpCodes::Ldloc_S, bl);
			AddILCode(ILProcessor, OpCodes::Ldloc_S, var);
			AddILCode(ILProcessor, OpCodes::Stind_I4);
			break;
		}
		case 变量循环首:
		{
			end = ILProcessor->Create(OpCodes::Nop);
			var = gcnew VariableDefinition(module->TypeSystem->Int32);
			MethodInfo->Method->Body->Variables->Add(var);
			VariableDefinition^ bl = gcnew VariableDefinition(module->TypeSystem->Int32);
			MethodInfo->Method->Body->Variables->Add(bl);
			var2 = gcnew VariableDefinition(module->TypeSystem->Int32);
			MethodInfo->Method->Body->Variables->Add(var2);
			VariableDefinition^ mb = gcnew VariableDefinition(module->TypeSystem->Int32);
			MethodInfo->Method->Body->Variables->Add(mb);
			AddILCode(ILProcessor, OpCodes::Stloc_S, bl);
			AddILCode(ILProcessor, OpCodes::Stloc_S, var2);
			AddILCode(ILProcessor, OpCodes::Stloc_S, mb);
			AddILCode(ILProcessor, OpCodes::Stloc_S, var);
			tag = ILProcessor->Create(OpCodes::Ldloc_S, var);
			ILProcessor->Append(tag);
			AddILCode(ILProcessor, OpCodes::Ldloc_S, mb);
			AddILCode(ILProcessor, OpCodes::Bgt_S, end);
			AddILCode(ILProcessor, OpCodes::Ldloc_S, bl);
			AddILCode(ILProcessor, OpCodes::Ldloc_S, var);
			AddILCode(ILProcessor, OpCodes::Stind_I4);
			break;
		}
		}
		break;
	}
	default:
		throw Error("未知标记");
	}
	while (Index < count)
	{
		current = Code + Offset[Index];
		if (count == Index + 1) size = Length - Offset[Index];
		else size = Offset[Index + 1] - Offset[Index];
		byte* codeend = current + size;
		switch (GetData<ECode_Head>(current))
		{
		case Call:
			this->CompileCode_Call(MethodInfo, ILProcessor, current, codeend);
			Index++;
			if (codeend > current)
			{
				switch (GetData<EProcess_Tag>(current))
				{
				case EProcess_Tag::Else:
				case EProcess_Tag::Case:
					AddILCode(ILProcessor, OpCodes::Br, end);
					ILProcessor->Append(tag);
					tag = nullptr;
					break;
				case EProcess_Tag::IfeEnd:
				case EProcess_Tag::IfEnd:
				case EProcess_Tag::SwitchEnd:
					goto end;
				}
			}
			break;
		case Ife:
		case If:
		case LoopBegin:
			this->CompileCode_Proc(MethodInfo, ILProcessor, Code, Length, Offset, Index);
			break;
		case Switch:
			this->CompileCode_Call(MethodInfo, ILProcessor, current, current + size);
			Index++;
			tag = ILProcessor->Create(OpCodes::Nop);
			AddILCode(ILProcessor, OpCodes::Brfalse_S, tag);
			break;
		case LoopEnd:
		{
			ETAG etag = GetData<ETAG>(current);
			current -= sizeof(ETAG);
			this->CompileCode_Call(MethodInfo, ILProcessor, current, current + size);
			Index++;
			switch (etag)
			{
			case 判断循环尾:
				AddILCode(ILProcessor, OpCodes::Br, tag);
				break;
			case 循环判断尾:
				AddILCode(ILProcessor, OpCodes::Brtrue_S, tag);
				break;
			case 计次循环尾:
				AddILCode(ILProcessor, OpCodes::Ldloc_S, var);
				AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
				AddILCode(ILProcessor, OpCodes::Add);
				AddILCode(ILProcessor, OpCodes::Stloc_S, var);
				AddILCode(ILProcessor, OpCodes::Br, tag);
				break;
			case 变量循环尾:
				AddILCode(ILProcessor, OpCodes::Ldloc_S, var);
				AddILCode(ILProcessor, OpCodes::Ldloc_S, var2);
				AddILCode(ILProcessor, OpCodes::Add);
				AddILCode(ILProcessor, OpCodes::Stloc_S, var);
				AddILCode(ILProcessor, OpCodes::Br, tag);
				break;
			}
			tag = nullptr;
			goto end;
		}
		default:
			throw Error("未知标记");
		}
	}
end:
	if (tag != nullptr) ILProcessor->Append(tag);
	if (end != nullptr) ILProcessor->Append(end);
}

void ECompile::LoadKrnln()
{
	ModuleDefinition^ module = this->_assembly->MainModule;
	TypeDefinition^ global = module->GetType("<Module>");
	PluginInfo^ info = Plugins::Load(module, typeof(Krnln));
	short id;
	this->_CodeProcess->FindLibrary(String2LPSTR(info->Lib), id);
	this->krnln_id = id;
	for each (MonoInfo^ mi in info->Methods)
	{
		EMethodData^ md = gcnew EMethodData(mi->Method, mi->Mode);
		this->_CodeRefer->AddMethodRefer(id, mi->Tag, md);
		if (mi->Mode != EMethodMode::Embed)
		{
			if (mi->Method->HasPInvokeInfo)
			{
				if (mi->Method->PInvokeInfo->Module->Name == "") AddModule(module->ModuleReferences, gcnew ModuleReference("kernel32.dll"));
				AddModule(module->ModuleReferences, mi->Method->PInvokeInfo->Module);
			}
			global->Methods->Add(mi->Method);
		}
	}
	for each (TypeDefinition^ type in info->Types) module->Types->Add(type);
}

void ECompile::LoadE_net()
{

}

EMethodReference^ ECompile::GetMethodReference(short index, ETAG tag)
{
	EMethodReference^ mr;
	EMethodData^ md = this->_CodeRefer->FindMethodRefer(index, tag);
	if (md == nullptr) md = this->FindReferMethod(index, tag);
	if (md != nullptr) mr = this->GetMethodReference(md, index, tag);
	return mr;
}

EMethodReference^ ECompile::GetMethodReference(EMethodData^ methoddata, short index, ETAG tag)
{
	ModuleDefinition^ module = this->_assembly->MainModule;
	EMethodReference^ mr = gcnew EMethodReference();
	MethodDefinition^ method = methoddata;
	mr->Lib = index;
	mr->Tag = tag;
	mr->Name = method->Name;
	mr->ReturnType = method->ReturnType;
	List<EParamInfo^>^ params = gcnew List<EParamInfo^>();
	for each (ParameterDefinition^ param in method->Parameters)
	{
		EParamInfo^ pi = gcnew EParamInfo();
		pi->Name = param->Name;
		pi->Type = param->ParameterType;
		pi->OriginalType = param->ParameterType;
		pi->IsAddress = param->IsOut;
		if (pi->IsAddress)
		{
			ByReferenceType^ t = dynamic_cast<ByReferenceType^>(pi->Type);
			if (t != nullptr) pi->Type = t->ElementType;
		}
		pi->IsArray = pi->Type->IsArray;
		pi->IsOptional = param->IsOptional;
		if (pi->IsOptional)
		{
			if (param->HasConstant) pi->Defualt = param->Constant;
			if (pi->Type->Name == "Nullable`1")
			{
				GenericInstanceType^ t = dynamic_cast<GenericInstanceType^>(pi->Type);
				if (t != nullptr) pi->Type = t->GenericArguments[0];
			}
		}
		pi->IsVariable = FindCustom(param->CustomAttributes, module->ImportReference(typeof(ParamArrayAttribute))) != nullptr;
		params->Add(pi);
	}
	mr->Params = params->ToArray();
	mr->MethodData = methoddata;
	return mr;
}

TypeReference^ ECompile::EDT2Type(DataType edt)
{
	ModuleDefinition^ module = this->_assembly->MainModule;
	if (edt == -1) return module->TypeSystem->Void;
	TypeReference^ type;
	switch (edt)
	{
	case EDT_VOID:
		type = module->TypeSystem->Void;
		break;
	case EDT_ALL:
		type = module->TypeSystem->Object;
		break;
	case EDT_BYTE:
		type = module->TypeSystem->Byte;
		break;
	case EDT_SHORT:
		type = module->TypeSystem->Int16;
		break;
	case EDT_INT:
		type = module->TypeSystem->Int32;
		break;
	case EDT_LONG:
		type = module->TypeSystem->Int64;
		break;
	case EDT_FLOAT:
		type = module->TypeSystem->Single;
		break;
	case EDT_DOUBLE:
		type = module->TypeSystem->Double;
		break;
	case EDT_BOOL:
		type = module->TypeSystem->Boolean;
		break;
	case EDT_DATETIME:
		type = module->ImportReference(typeof(DateTime));
		break;
	case EDT_TEXT:
		type = module->TypeSystem->String;
		break;
	case EDT_BIN:
		type = gcnew ArrayType(module->TypeSystem->Byte);
		break;
	case EDT_SUBPTR:
		type = module->ImportReference(typeof(Delegate));
		break;
	default:
		type = this->FindTypeDefinition(edt);
		if (type == nullptr) type = module->TypeSystem->IntPtr;
		else type = module->ImportReference(type);
		break;
	}
	return type;
}

TypeDefinition^ ECompile::FindTypeDefinition(UINT tag)
{
	TypeDefinition^ type = this->_CodeRefer->FindTypeDefine(tag);
	if (type == nullptr)
	{
		ModuleDefinition^ module = this->_assembly->MainModule;
		ESection_Program_Assembly assembly;
		for each (assembly in this->_CodeProcess->GetReferAssemblies()) if (assembly.Tag == tag) goto add;
		for each (assembly in this->_CodeProcess->GetReferStructs()) if (assembly.Tag == tag) goto add;
		return nullptr;
	add:
		vector<string> arr = split(assembly.Remark, SP);
		String^ classname;
		if (arr[0] == DONET_CLASS) classname = CStr2String(arr[1]);
		else  classname = GetTypeName(assembly);
		type = this->_CodeRefer->FindTypeDefine(classname);
		if (type == nullptr)
		{

		}
		this->_CodeRefer->AddTypeRefer(assembly.Tag, classname);
	}
	return type;
}

EMethodData^ ECompile::FindReferMethod(short index, ETAG tag)
{
	if (index == CUSTOM)
	{
		ModuleDefinition^ module = this->_assembly->MainModule;
		ESection_Program_Method method = this->_CodeProcess->FindReferMethod(tag);
		if (method != NULL && (method.Attributes & EMethodAttr::M_Extern) == EMethodAttr::M_Extern)
		{
			ESection_Program_Assembly type = this->_CodeProcess->FindReferAssembly(method.Class);
			if (type != NULL)
			{
				TypeDefinition^ t;
				if (type.Name == "__HIDDEN_TEMP_MOD__")
				{

					return nullptr;
				}
				else
				{
					t = this->FindTypeDefinition(type.Tag);
					if (t != nullptr)
					{
						vector<string> arr = split(method.Remark, SP);
						if (arr.size() > 1 && arr[0] == DONET)
						{
							bool staticmethod = arr[1] == "1";
							String^ methodname = CStr2String(method.Name);
							if (methodname == t->Name) methodname = ".ctor";
							List<MethodDefinition^>^ list = FindAllMethod(t, methodname, staticmethod);
							if (list->Count > 0)
							{
								for each (MethodDefinition^ m in list)
								{
									EMethodData^ md = gcnew EMethodData(m, m->IsConstructor ? EMethodMode::Newobj : EMethodMode::Call);
									this->_CodeRefer->AddMethodRefer(index, tag, md);
								}
								return this->_CodeRefer->FindMethodRefer(index, tag);
							}
						}
						else
						{

						}
					}
				}
			}
		}
	}
	else if (index == DLL)
	{

	}
	else
	{

	}
	return nullptr;
}