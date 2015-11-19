#include "stdafx.h"
#include "common.h"
#include "common.net.h"
#include "krnln.net.h"
#include "e.net.h"

using namespace std;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Runtime::CompilerServices;
using namespace System::Text::RegularExpressions;

EDataInfo::EDataInfo()
{
	this->Methods = gcnew Dictionary<ELib_Method^, EMethodData^>();
	this->Symbols = gcnew Dictionary<String^, IList<EMethodData^>^>();
	this->Types = gcnew Dictionary<UINT, TypeDefinition^>();
	this->Variables = gcnew Dictionary<UINT, VariableDefinition^>();
	this->Parameters = gcnew Dictionary<UINT, ParameterDefinition^>();
	this->Fields = gcnew Dictionary<UINT, FieldDefinition^>();
	this->GlobalVariables = gcnew Dictionary<UINT, FieldDefinition^>();
	this->Propertys = gcnew Dictionary<UINT, PropertyDefinition^>();
}

EMethodData::EMethodData()
{

}

EMethodData::EMethodData(MethodDefinition^ method)
{
	this->Method = method;
	this->Mode = EMethodMode::Call;
}

EMethodData::EMethodData(MethodDefinition^ method, EMethodMode mode)
{
	this->Method = method;
	this->Mode = mode;
}

EMethodData::operator MethodDefinition ^ ()
{
	return this->Method;
}

ELib_Method::ELib_Method()
{

}

ELib_Method::ELib_Method(short index, UINT tag)
{
	this->Index = index;
	this->Tag = tag;
}

bool ELib_Method::operator == (ELib_Method^ elm)
{
	bool b1 = Object::Equals(this, nullptr);
	bool b2 = Object::Equals(elm, nullptr);
	if (b1 == b2)
	{
		if (b1) return true;
		return (this->Index == elm->Index) && (this->Tag == elm->Tag);
	}
	else return false;
}
bool ELib_Method::operator != (ELib_Method^ elm)
{
	return !(this == elm);
}

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
	if (r) compile->Write(path);
	delete compile;
	return r;
}

void DefaultConstructor(ModuleDefinition^ module, MethodDefinition^ method)
{
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(typeof(Object)->GetConstructor(Type::EmptyTypes)));
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

MethodDefinition^ CreateConstructor(ModuleDefinition^ module, MethodAttributes attr = MethodAttributes::Private, IList<Mono::Cecil::ParameterDefinition^>^ params = nullptr)
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
		DefaultConstructor(module, ctor);
	}
	if (params != nullptr && params->Count > 0) for (int i = 0; i < params->Count; i++) ctor->Parameters->Add(params[i]);
	return ctor;
}

ETagStatus GetTagStatus(vector<ESection_TagStatus> tags, ETAG tag)
{
	for each (ESection_TagStatus tagstatus in tags) if (tagstatus.Tag == tag) return tagstatus.Status;
	return ETagStatus::None;
}

TypeDefinition^ FindType(IList<TypeDefinition^>^ alltype, String^ fullname)
{
	for each (TypeDefinition^ type in alltype) if (type->FullName == fullname) return type;
	return nullptr;
}

ESection_Program_Assembly FindAssemblyByTag(vector<ESection_Program_Assembly> list, ETAG tag)
{
	for each (ESection_Program_Assembly assembly in list) if (assembly.Tag == tag) return assembly;
	return NULL;
}

TypeDefinition^ FindTypeByAssembly(IList<TypeDefinition^>^ alltype, ESection_Program_Assembly assembly)
{
	String^ classname = CStr2String(assembly.Name);
	classname = DotDecode(classname);
	return FindType(alltype, classname);
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

ESection_Program_Method FindMethodByTag(vector<ESection_Program_Method> methods, ETAG tag)
{
	for each (ESection_Program_Method method in methods) if (method.Tag == tag) return method;
	return NULL;
}

CustomAttribute^ FindCustom(IList<CustomAttribute^>^ list, TypeReference^ type)
{
	for each (CustomAttribute^ custom in list) if (custom->AttributeType == type) return custom;
	return nullptr;
}

ESection_Variable FindVariableByTag(vector<ESection_Variable> variables, ETAG tag)
{
	for each (ESection_Variable var in variables) if (var.Tag == tag) return var;
	return NULL;
}

FieldDefinition^ FindField(TypeDefinition^ type, String^ name)
{
	for each (FieldDefinition^ field in type->Fields) if (field->Name == name) return field;
	return nullptr;
}

EMethodData^ GetItemValue(IDictionary<ELib_Method^, EMethodData^>^ dictionary, ELib_Method^ key)
{
	for each (KeyValuePair<ELib_Method^, EMethodData^>^ item in dictionary) if (item->Key == key) return item->Value;
	return nullptr;
}

String^ FindLibrary(vector<string> libraries, string guid, short& i)
{
	size_t len = libraries.size();
	for (i = 0; i < len; i++)
	{
		vector<string> arr = split(libraries[i], "\r");
		if (arr[1] == guid) return CStr2String(libraries[i]);
	}
	return nullptr;
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
	this->_ecode = ecode;
	this->_len = len;
	this->_refer = refer;
}

ECompile::~ECompile()
{
	if (this->_einfo != NULL) delete this->_einfo;
}

bool ECompile::Compile()
{
	return this->ParseCode() && this->CompileHead() && this->CompileRefer() && this->CompileClass() && this->CompileCode();
}

void ECompile::Write(String^ path)
{
	if (this->_einfo != NULL && !String::IsNullOrEmpty(path))
	{
		String^ str = CStr2String(this->_einfo->UserInfo.ProjectName);
		if (!String::IsNullOrEmpty(str))
		{
			char c = path[path->Length - 1];
			if (c != '/' && c != '\\') path += "\\";
			String^ ext;
			switch (this->_einfo->SystemInfo.CompileType)
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

bool ECompile::ParseCode()
{
	this->_einfo = ParseEcode(this->_ecode);
	return this->_einfo != NULL;
}

bool ECompile::CompileHead()
{
	try
	{
		this->_edata = gcnew EDataInfo();
		String^ str = CStr2String(this->_einfo->UserInfo.ProjectName);
		Version^ version = gcnew Version(this->_einfo->UserInfo.Major, this->_einfo->UserInfo.Minor, this->_einfo->UserInfo.Build, this->_einfo->UserInfo.Revision);
		AssemblyNameDefinition^ name = gcnew AssemblyNameDefinition(str, version);
		ModuleParameters^ mp = gcnew ModuleParameters();
		mp->Runtime = TargetRuntime::Net_4_0;
		switch (this->_einfo->SystemInfo.CompileType)
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
		FindLibrary(this->_einfo->Program.Libraries, KRNLN, id);
		this->krnln_id = id;
		FindLibrary(this->_einfo->Program.Libraries, E_NET, id);
		this->e_net_id = id;
		return true;
	}
	catch (Exception^ ex)
	{
		this->_assembly = nullptr;
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
			this->_refer = gcnew array<String^> { mscorlib };
		}
		this->_alltype = gcnew List<TypeDefinition^>();
		for each (String^ path in this->_refer)
		{
			ModuleDefinition^ m = ModuleDefinition::ReadModule(path);
			for each (TypeDefinition^ type in m->GetTypes()) if (type->IsPublic && !(type->IsGenericParameter || type->IsGenericInstance)) this->_alltype->Add(type);
		}
		return true;
	}
	catch (Exception^ ex)
	{
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
		IDictionary<TypeDefinition^, UINT>^ typelist = gcnew Dictionary<TypeDefinition^, UINT>();
		for each (ESection_Program_Assembly assembly in this->_einfo->Program.Assemblies)
		{
			bool isstatic = assembly.Tag.Type2 == ETYPE::StaticClass;
			TypeDefinition^ type = FindTypeByAssembly(this->_alltype, assembly);
			if (type == nullptr)
			{
				String^ _namespace = String::Empty;
				String^ classname = CStr2String(assembly.Name);
				classname = DotDecode(classname);
				int index = classname->LastIndexOf(".");
				if (index != -1)
				{
					_namespace = classname->Substring(0, index);
					classname = classname->Substring(index + 1);
				}
				type = gcnew TypeDefinition(_namespace, classname, TypeAttributes::Class, module->TypeSystem->Object);
				module->Types->Add(type);
				this->_alltype->Add(type);
			}
			if (!isstatic)
			{
				typelist->Add(type, assembly.Base);
				if (assembly.Public || GetTagStatus(this->_einfo->TagStatus.Tags, assembly.Tag) == ETagStatus::Public) type->Attributes = type->Attributes | TypeAttributes::Public;
			}
			this->_edata->Types->Add(assembly.Tag, type);
		}
		for each (ESection_Program_Assembly assembly in this->_einfo->Program.Structs)
		{
			TypeAttributes attr = STRUCT;
			if (assembly.Public) attr = attr | TypeAttributes::Public;
			String^ _namespace = String::Empty;
			String^ classname = CStr2String(assembly.Name);
			classname = DotDecode(classname);
			int index = classname->LastIndexOf(".");
			if (index != -1)
			{
				_namespace = classname->Substring(0, index);
				classname = classname->Substring(index + 1);
			}
			TypeDefinition^ type = gcnew TypeDefinition(_namespace, classname, attr, this->Type_ValueType);
			module->Types->Add(type);
			this->_alltype->Add(type);
			this->_edata->Types->Add(assembly.Tag, type);
		}
		for each(KeyValuePair<TypeDefinition^, UINT>^ kv in typelist)
		{
			TypeReference^ t = this->EDT2Type((DataType)kv->Value);
			if (t != module->TypeSystem->Void) kv->Key->BaseType = t;
		}
		for each (ESection_Program_Assembly assembly in this->_einfo->Program.Structs)
		{
			TypeDefinition^ type = this->_edata->Types[assembly.Tag];
			for each (ESection_Variable var in assembly.Variables)
			{
				TypeReference^ t = this->EDT2Type(var.DataType);
				if (var.ArrayInfo.Dimension > 0) t = gcnew ArrayType(t, var.ArrayInfo.Dimension);
				String^ name = CStr2String(var.Name);
				MethodReference^ ctor = module->ImportReference(typeof(::CompilerGeneratedAttribute)->GetConstructor(Type::EmptyTypes));
				CustomAttribute^ custom = gcnew CustomAttribute(ctor);
				FieldDefinition^ f = gcnew FieldDefinition("<c>k__BackingField", FieldAttributes::Private, t);
				f->CustomAttributes->Add(custom);
				type->Fields->Add(f);
				this->_edata->Fields->Add(var.Tag, f);
				PropertyDefinition^ p = gcnew PropertyDefinition(name, PropertyAttributes::None, t);
				MethodAttributes attr = (var.Remark == "private" ? MethodAttributes::Private : MethodAttributes::Public);
				MethodDefinition^ get_method = gcnew MethodDefinition("get_" + name, attr | PROPERTYMETHOD, t);
				get_method->CustomAttributes->Add(custom);
				ILProcessor^ ILProcessor = get_method->Body->GetILProcessor();
				AddILCode(ILProcessor, OpCodes::Ldarg_0);
				AddILCode(ILProcessor, OpCodes::Ldfld, f);
				AddILCode(ILProcessor, OpCodes::Ret);
				type->Methods->Add(get_method);
				p->GetMethod = get_method;
				MethodDefinition^ set_method = gcnew MethodDefinition("set_" + name, attr | PROPERTYMETHOD, module->TypeSystem->Void);
				set_method->CustomAttributes->Add(custom);
				set_method->Parameters->Add(gcnew ParameterDefinition("value", ParameterAttributes::None, t));
				ILProcessor = set_method->Body->GetILProcessor();
				AddILCode(ILProcessor, OpCodes::Ldarg_0);
				AddILCode(ILProcessor, OpCodes::Ldarg_1);
				AddILCode(ILProcessor, OpCodes::Stfld, f);
				AddILCode(ILProcessor, OpCodes::Ret);
				type->Methods->Add(set_method);
				p->SetMethod = set_method;
				type->Properties->Add(p);
			}
		}
		for each (ESection_Program_Assembly assembly in this->_einfo->Program.Assemblies)
		{
			bool isstatic = assembly.Tag.Type2 == ETYPE::StaticClass;
			TypeDefinition^ type = this->_edata->Types[assembly.Tag];
			for each (ESection_Variable var in assembly.Variables)
			{
				FieldAttributes attr;
				if (var.Remark == "public") attr = FieldAttributes::Public;
				if (isstatic) attr = attr | FieldAttributes::Static;
				TypeReference^ t = this->EDT2Type(var.DataType);
				if (var.ArrayInfo.Dimension > 0) t = gcnew ArrayType(t, var.ArrayInfo.Dimension);
				FieldDefinition^ f = gcnew FieldDefinition(CStr2String(var.Name), attr, t);
				type->Fields->Add(f);
				this->_edata->Fields->Add(var.Tag, f);
			}
			if (!this->CompileMethod(type, assembly, isstatic)) return false;
		}
		return true;
	}
	catch (Exception^ ex)
	{
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
			ESection_Program_Method pm = FindMethodByTag(this->_einfo->Program.Methods, tag);
			if (pm != NULL)
			{
				String^ name = CStr2String(pm.Name);
				MethodDefinition^ method = nullptr;
				bool ctor = false;
				MethodAttributes attr = ((pm.Attributes & EMethodAttr::Public) == EMethodAttr::Public) ? MethodAttributes::Public : MethodAttributes::Private;
				if (isstatic) attr = attr | MethodAttributes::Static;
				if (name == type->Name && pm.ReturnType == DataType::EDT_VOID)
				{
					ctor = true;
					method = CreateConstructor(module, attr);
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
					if ((param.Attributes & EVariableAttr::Array) == EVariableAttr::Array) t = gcnew ArrayType(t);
					if ((param.Attributes & EVariableAttr::Optional) == EVariableAttr::Optional) attr = attr | ParameterAttributes::Optional;
					if ((param.Attributes & EVariableAttr::Out) == EVariableAttr::Out)
					{
						attr = attr | ParameterAttributes::Out;
						t = gcnew ByReferenceType(t);
					}
					ParameterDefinition^ p = CreateParameter(CStr2String(param.Name), t, attr);
					method->Parameters->Add(p);
					this->_edata->Parameters->Add(param.Tag, p);
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
						this->_edata->Variables->Add(var.Tag, v);
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
				this->_edata->Methods->Add(gcnew ELib_Method(-2, tag), md);
				String^ tagName = type->FullName + (isstatic ? "." : ":") + method->Name;
				IList<EMethodData^>^ mdlist;
				if (this->_edata->Symbols->ContainsKey(tagName)) mdlist = this->_edata->Symbols[tagName];
				else
				{
					mdlist = gcnew List<EMethodData^>();
					this->_edata->Symbols->Add(tagName, mdlist);
				}
				mdlist->Add(md);
			}
		}
		return true;
	}
	catch (Exception^ ex)
	{
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
		for each (ESection_Variable var in this->_einfo->Program.GlobalVariables)
		{
			TypeReference^ t = this->EDT2Type(var.DataType);
			if (var.ArrayInfo.Dimension > 0) t = gcnew ArrayType(t, var.ArrayInfo.Dimension);
			FieldDefinition^ f = gcnew FieldDefinition(CStr2String(var.Name), FieldAttributes::Static, t);
			global->Fields->Add(f);
			this->_edata->GlobalVariables->Add(var.Tag, f);
		}
		this->LoadKrnln();
		for each (ESection_Program_Assembly assembly in this->_einfo->Program.Assemblies)
		{
			TypeDefinition^ type = FindTypeByAssembly(this->_alltype, assembly);
			if (type != nullptr)
			{
				for each (ETAG tag in assembly.Methods)
				{
					ESection_Program_Method pm = FindMethodByTag(this->_einfo->Program.Methods, tag);
					if (pm != NULL)
					{
						EMethodInfo^ MethodInfo = gcnew EMethodInfo();
						MethodInfo->MethodInfo = &pm;
						MethodInfo->Method = GetItemValue(this->_edata->Methods, gcnew ELib_Method(-2, pm.Tag));
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
		return false;
	}
}

bool ECompile::CompileCode_Begin(EMethodInfo^ MethodInfo, ILProcessor^ ILProcessor, byte* Code, size_t Length, vector<UINT> Offset)
{
	try
	{
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
				this->CompileCode_Call(MethodInfo, ILProcessor, current, current + size);
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
		EMethodReference^ mr = this->GetMethodReference(gcnew ELib_Method(LibID, etag));
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
			bool staticmethod = mr->MethodData->Method->IsStatic;
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
				if (mr->Tag == ECode_Method::返回) mr->Params[0]->Type = MethodInfo->Method->ReturnType;
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
							if (vardata == nullptr) throw  Error(mr->Name, "参数" + params->Count + "类型错误");
							IList<Instruction^>^ code;
							if (mr->Tag == ECode_Method::赋值 && params->Count == 0)
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
					case ECode_Type::LibConst:
					case ECode_Type::n4:
						//未完成
						Code += 4;
						param->Type = module->TypeSystem->Void;
						param->Data = nullptr;
						param->DataType = EParamDataType::Null;
						break;
					}
					params->Add(param);
				} while (Code < End);
			paramend:
				EMethodData^ md;
				if (mr->Params->Length == 0 && params->Count == 0) md = mr->MethodData;
				else
				{
					if (thistype == nullptr) thistype = mr->MethodData->Method->DeclaringType;
					String^ tagName;
					if (thistype == nullptr) tagName = mr->Name;
					else tagName = thistype->FullName + (staticmethod ? "." : ":") + mr->Name;
					IList<EMethodData^>^ mdlist;
					if (this->_edata->Symbols->ContainsKey(tagName)) mdlist = this->_edata->Symbols[tagName];
					IList<EMethodReference^>^ conform = gcnew List<EMethodReference^>();
					if (mdlist != nullptr)
					{
						mr = nullptr;
						TypeReference^ paatr = module->ImportReference(typeof(ParamArrayAttribute));
						for each (EMethodData^ item in mdlist)
						{
							if (item->Method->Parameters->Count == params->Count)
							{
								mr = this->GetMethodReference(item, nullptr);
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
										mr = this->GetMethodReference(item, nullptr);
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
						for (size_t i = 0; i < params->Count; i++)
						{
							if (i < mrparams->Length)
							{
								if (mrparams[i]->Type == params[i]->Type)
								{
									similarity++;
									satisfy++;
								}
								else if (params[i]->Type == module->TypeSystem->Void && mrparams[i]->IsOptional)
								{
									similarity++;
									satisfy++;
								}
								else if (mrparams[i]->Type == module->TypeSystem->Object)
								{
									similarity += 0.45;
									satisfy++;
								}
								else if (mrparams[i]->Type->IsValueType && params[i]->Type->IsValueType)
								{
									if ((mrparams[i]->Type == module->TypeSystem->Byte || mrparams[i]->Type == module->TypeSystem->Int16 || mrparams[i]->Type == module->TypeSystem->Int32 || mrparams[i]->Type == module->TypeSystem->Int64) && (params[i]->Type == module->TypeSystem->Byte || params[i]->Type == module->TypeSystem->Int16 || params[i]->Type == module->TypeSystem->Int32 || params[i]->Type == module->TypeSystem->Int64))
									{
										similarity += 0.5;
										satisfy++;
									}
									else if ((mrparams[i]->Type == module->TypeSystem->Single || mrparams[i]->Type == module->TypeSystem->Double || mrparams[i]->Type == module->TypeSystem->Int32) && (params[i]->Type == module->TypeSystem->Single || params[i]->Type == module->TypeSystem->Double || params[i]->Type == module->TypeSystem->Int32))
									{
										similarity += 0.5;
										satisfy++;
									}
									else if ((mrparams[i]->Type == module->TypeSystem->IntPtr || mrparams[i]->Type == module->TypeSystem->Int32) && (params[i]->Type == module->TypeSystem->IntPtr || params[i]->Type == module->TypeSystem->Int32))
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
										if (arrtype->ElementType == params[i]->Type)
										{
											similarity++;
											satisfy++;
										}
										else if (arrtype->ElementType == module->TypeSystem->Object)
										{
											similarity += 0.5;
											satisfy++;
										}
										else if (arrtype->ElementType->IsValueType && params[i]->Type->IsValueType)
										{
											if ((arrtype->ElementType == module->TypeSystem->Byte || arrtype->ElementType == module->TypeSystem->Int16 || arrtype->ElementType == module->TypeSystem->Int32 || arrtype->ElementType == module->TypeSystem->Int64) && (params[i]->Type == module->TypeSystem->Byte || params[i]->Type == module->TypeSystem->Int16 || params[i]->Type == module->TypeSystem->Int32 || params[i]->Type == module->TypeSystem->Int64))
											{
												similarity += 0.5;
												satisfy++;
											}
											else if ((arrtype->ElementType == module->TypeSystem->Single || arrtype->ElementType == module->TypeSystem->Double || arrtype->ElementType == module->TypeSystem->Int32) && (params[i]->Type == module->TypeSystem->Single || params[i]->Type == module->TypeSystem->Double || params[i]->Type == module->TypeSystem->Int32))
											{
												similarity += 0.5;
												satisfy++;
											}
											else if ((arrtype->ElementType == module->TypeSystem->IntPtr || arrtype->ElementType == module->TypeSystem->Int32) && (params[i]->Type == module->TypeSystem->IntPtr || params[i]->Type == module->TypeSystem->Int32))
											{
												similarity += 0.5;
												satisfy++;
											}
										}
									}
								}
							}
						}
						if (similarity == params->Count)
						{
							mr = item;
							break;
						}
						else if (satisfy == params->Count) similar->Add(item, similarity);
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
					}
					md = mr->MethodData;
					if (mr->Tag == ECode_Method::赋值)
					{
						EParamInfo^ item = mr->Params[1];
						EParamData^ param = params[1];
						switch (param->DataType)
						{
						case EParamDataType::Null:
							if (item->Type == module->TypeSystem->Byte || item->Type == module->TypeSystem->Int16 || item->Type == module->TypeSystem->Int32 || item->Type == module->TypeSystem->Boolean) AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
							else if (item->Type == module->TypeSystem->Int64) AddILCode(ILProcessor, OpCodes::Ldc_I8, 0);
							else if (item->Type == module->TypeSystem->Single) AddILCode(ILProcessor, OpCodes::Ldc_R4, 0);
							else if (item->Type == module->TypeSystem->Double) AddILCode(ILProcessor, OpCodes::Ldc_R8, 0);
							else if (item->Type == this->Type_DateTime) AddILCode(ILProcessor, OpCodes::Ldsfld, module->ImportReference(GetStaticField(DateTime, "MinValue")));
							else if (item->Type == module->TypeSystem->IntPtr) AddILCode(ILProcessor, OpCodes::Ldsfld, module->ImportReference(GetStaticField(IntPtr, "Zero")));
							else AddILCode(ILProcessor, OpCodes::Ldnull);
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
							AddILCode(ILProcessor, OpCodes::Newobj, module->ImportReference(typeof(DateTime)->GetConstructor(gcnew array < Type^ > { typeof(Int64) })));
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
									if (item->Type == module->TypeSystem->Byte || item->Type == module->TypeSystem->Int16 || item->Type == module->TypeSystem->Int32 || item->Type == module->TypeSystem->Boolean) AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
									else if (item->Type == module->TypeSystem->Int64) AddILCode(ILProcessor, OpCodes::Ldc_I8, 0);
									else if (item->Type == module->TypeSystem->Single) AddILCode(ILProcessor, OpCodes::Ldc_R4, 0);
									else if (item->Type == module->TypeSystem->Double) AddILCode(ILProcessor, OpCodes::Ldc_R8, 0);
									else if (item->Type == this->Type_DateTime) AddILCode(ILProcessor, OpCodes::Ldsfld, module->ImportReference(GetStaticField(DateTime, "MinValue")));
									else if (item->Type == module->TypeSystem->IntPtr) AddILCode(ILProcessor, OpCodes::Ldsfld, module->ImportReference(GetStaticField(IntPtr, "Zero")));
									else AddILCode(ILProcessor, OpCodes::Ldnull);
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
									AddILCode(ILProcessor, OpCodes::Newobj, module->ImportReference(typeof(DateTime)->GetConstructor(gcnew array < Type^ > { typeof(Int64) })));
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
								case EParamDataType::GlobalField:
								case EParamDataType::Array:
								{
									IList<Instruction^>^ code = (IList<Instruction^>^)param->Data;
									if (item->IsAddress)
									{
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
									else if (item->Type == module->TypeSystem->Object && param->Type->IsValueType) code->Add(ILProcessor->Create(OpCodes::Box, param->Type));
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
				if (tvd == nullptr || (tvd->Type != module->TypeSystem->Byte && tvd->Type != module->TypeSystem->Int16 && tvd->Type != module->TypeSystem->Int32 && tvd->Type != module->TypeSystem->Int64)) return nullptr;
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
	switch (tag.Type2)
	{
	case Variable:
		if (this->_edata->Variables->ContainsKey(tag)) v = this->_edata->Variables[tag];
		else if (this->_edata->Parameters->ContainsKey(tag)) p = this->_edata->Parameters[tag];
		break;
	case Field:
		if (this->_edata->Fields->ContainsKey(tag)) f = this->_edata->Fields[tag];
		break;
	case GlobalField:
		if (this->_edata->GlobalVariables->ContainsKey(tag)) g = this->_edata->GlobalVariables[tag];
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
	else return nullptr;
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
						if (varindex->Count > i + 1)
						{
							item = varindex[i + 1];
							if (item->IndexType != EIndexType::Array) break;
							i++;
						}
						else break;
					}
					else if (dynamic_cast<ArrayType^>(vardata->Type)) break;
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
				if (this->_edata->Fields->ContainsKey(fi.Field)) fd = this->_edata->Fields[fi.Field];
				else if (this->_edata->Propertys->ContainsKey(fi.Field)) pd = this->_edata->Propertys[fi.Field];
				else
				{
					ESection_Program_Assembly assembly = FindAssemblyByTag(this->_einfo->Program.ReferStructs, fi.Class);
					if (assembly == NULL) return nullptr;
					ESection_Variable var = FindVariableByTag(assembly.Variables, fi.Field);
					if (var == NULL) return nullptr;
					TypeDefinition^ type = this->FindReferType(fi.Class);
					if (type == nullptr) return nullptr;
					String^ varname = CStr2String(var.Name);
					fd = FindField(type, varname);
					if (fd == nullptr)
					{
						pd = FindProperty(type, varname);
						if (pd == nullptr) return nullptr;
						this->_edata->Propertys->Add(var.Tag, pd);
					}
					else this->_edata->Fields->Add(var.Tag, fd);
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
		switch (etag)
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
			AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
			AddILCode(ILProcessor, OpCodes::Stloc_S, var);
			tag = ILProcessor->Create(OpCodes::Ldloc_S, var);
			ILProcessor->Append(tag);
			AddILCode(ILProcessor, OpCodes::Ldloc_S, var2);
			AddILCode(ILProcessor, OpCodes::Bge_S, end);
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
	MethodDefinition^ method = CreateReturn(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::返回), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateMod(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::求余数), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateIntAdd(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::相加), gcnew EMethodData(method, EMethodMode::Embed));
	IList<EMethodData^>^ mlist = gcnew List<EMethodData^>();
	mlist->Add(gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateEvenIntAdd(module);
	global->Methods->Add(method);
	mlist->Add(gcnew EMethodData(method, EMethodMode::Call));
	method = CreateLongAdd(module);
	mlist->Add(gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateEvenLongAdd(module);
	global->Methods->Add(method);
	mlist->Add(gcnew EMethodData(method, EMethodMode::Call));
	method = CreateDoubleAdd(module);
	mlist->Add(gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateEvenDoubleAdd(module);
	global->Methods->Add(method);
	mlist->Add(gcnew EMethodData(method, EMethodMode::Call));
	method = CreateEvenBinAdd(module);
	global->Methods->Add(method);
	mlist->Add(gcnew EMethodData(method, EMethodMode::Call));
	method = CreateAdd(module);
	mlist->Add(gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateEvenAdd(module);
	global->Methods->Add(method);
	mlist->Add(gcnew EMethodData(method, EMethodMode::Call));
	this->_edata->Symbols->Add(method->Name, mlist);
	method = CreateSub(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::相减), gcnew EMethodData(method, EMethodMode::Embed));
	mlist = gcnew List<EMethodData^>();
	mlist->Add(gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateIntSub(module);
	mlist->Add(gcnew EMethodData(method, EMethodMode::Embed));
	this->_edata->Symbols->Add(method->Name, mlist);
	method = CreateNeg(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::负), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateMul(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::相乘), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateDiv(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::相除), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateIDiv(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::整除), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateEqual(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::等于), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateNotEqual(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::不等于), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateLess(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::小于), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateMore(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::大于), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateLessOrEqual(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::小于或等于), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateMoreOrEqual(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::大于或等于), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateAnd(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::并且), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateOr(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::或者), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateNot(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::取反), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateBnot(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::位取反), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateBand(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::位与), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateBor(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::位或), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateBxor(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::位异或), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateSet(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::赋值), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateToDouble(module);
	global->Methods->Add(method);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::到数值), gcnew EMethodData(method, EMethodMode::Call));
	method = CreateToStr(module);
	global->Methods->Add(method);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::到文本), gcnew EMethodData(method, EMethodMode::Call));
	method = CreateToByte(module);
	global->Methods->Add(method);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::到字节), gcnew EMethodData(method, EMethodMode::Call));
	method = CreateToShort(module);
	global->Methods->Add(method);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::到短整数), gcnew EMethodData(method, EMethodMode::Call));
	method = CreateToInt(module);
	global->Methods->Add(method);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::到整数), gcnew EMethodData(method, EMethodMode::Call));
	method = CreateToLong(module);
	global->Methods->Add(method);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::到长整数), gcnew EMethodData(method, EMethodMode::Call));
	method = CreateToFloat(module);
	global->Methods->Add(method);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::到小数), gcnew EMethodData(method, EMethodMode::Call));
	method = CreateShl(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::左移), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateShr(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::右移), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateIfe(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::如果), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateIf(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::如果真), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateIf(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::判断), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateWhile(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::判断循环首), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateWend(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::判断循环尾), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateDoWhile(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::循环判断首), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateLoop(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::循环判断尾), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateCounter(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::计次循环首), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateCounterLoop(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::计次循环尾), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateFor(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::变量循环首), gcnew EMethodData(method, EMethodMode::Embed));
	method = CreateNext(module);
	this->_edata->Methods->Add(gcnew ELib_Method(this->krnln_id, ECode_Method::变量循环尾), gcnew EMethodData(method, EMethodMode::Embed));
}

void ECompile::LoadE_net()
{

}

EMethodReference^ ECompile::GetMethodReference(ELib_Method^ libmethod)
{
	EMethodReference^ mr;
	EMethodData^ md = GetItemValue(this->_edata->Methods, libmethod);
	if (md == nullptr) md = this->FindReferMethod(libmethod);
	if (md != nullptr) mr = this->GetMethodReference(md, libmethod);
	return mr;
}

EMethodReference^ ECompile::GetMethodReference(EMethodData^ methoddata, ELib_Method^ libmethod)
{
	ModuleDefinition^ module = this->_assembly->MainModule;
	EMethodReference^ mr = gcnew EMethodReference();
	MethodDefinition^ method = methoddata;
	if (libmethod == nullptr)
	{
		for each (KeyValuePair<ELib_Method^, EMethodData^>^ item in this->_edata->Methods)
		{
			if (item->Value == methoddata)
			{
				libmethod = item->Key;
				break;
			}
			else if (item->Value->Method->Name == method->Name) libmethod = item->Key;
		}
		if (libmethod == nullptr) libmethod = gcnew ELib_Method();
	}
	mr->Lib = libmethod->Index;
	mr->Tag = libmethod->Tag;
	mr->Name = method->Name;
	mr->ReturnType = method->ReturnType;
	List<EParamInfo^>^ params = gcnew List<EParamInfo^>();
	for each (ParameterDefinition^ param in method->Parameters)
	{
		EParamInfo^ pi = gcnew EParamInfo();
		pi->Name = param->Name;
		pi->Type = param->ParameterType;
		pi->IsAddress = param->IsOut;
		if (pi->IsAddress)
		{
			ByReferenceType^ t = dynamic_cast<ByReferenceType^>(pi->Type);
			if (t != nullptr) pi->Type = t->ElementType;
		}
		pi->IsArray = pi->Type->IsArray;
		pi->IsOptional = param->IsOptional;
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
		type = this->FindReferType(edt);
		if (type == nullptr) type = module->TypeSystem->IntPtr;
		else type = module->ImportReference(type);
		break;
	}
	return type;
}

TypeDefinition^ ECompile::FindReferType(UINT tag)
{
	if (this->_edata->Types->ContainsKey(tag)) return this->_edata->Types[tag];
	else
	{
		ModuleDefinition^ module = this->_assembly->MainModule;
		ESection_Program_Assembly assembly;
		for each (assembly in this->_einfo->Program.ReferAssemblies) if (assembly.Tag == tag) goto add;
		for each (assembly in this->_einfo->Program.ReferStructs) if (assembly.Tag == tag) goto add;
		return nullptr;
	add:
		TypeDefinition^ type = FindTypeByAssembly(this->_alltype, assembly);
		this->_edata->Types->Add(assembly.Tag, type);
		return type;
	}
}

EMethodData^ ECompile::FindReferMethod(ELib_Method^ tag)
{
	if (tag->Index == -2)
	{
		ModuleDefinition^ module = this->_assembly->MainModule;
		for each (ESection_Program_Method method in this->_einfo->Program.ReferMethods)
		{
			if (method.Tag == tag->Tag)
			{
				ESection_Program_Assembly type = FindAssemblyByTag(this->_einfo->Program.ReferAssemblies, method.Class);
				if (type != NULL)
				{
					TypeDefinition^ t;
					MethodDefinition^ m;
					if (type.Name == "__HIDDEN_TEMP_MOD__")
					{
						String^ name = CStr2String(method.Name);
						name = DotDecode(name);
						int index = name->LastIndexOf(".");
						if (index != -1)
						{
							String^ classname = name->Substring(0, index);
							t = FindType(this->_alltype, classname);
							goto add;
						}
					}
					else
					{
						if (this->_edata->Types->ContainsKey(type.Tag))
						{
							t = this->_edata->Types[type.Tag];
						add:
							m = FindMethod(t, UINT::Parse(CStr2String(method.Remark)));
							if (m != nullptr)
							{
								EMethodData^ md = gcnew EMethodData(m, m->IsConstructor ? EMethodMode::Newobj : EMethodMode::Call);
								this->_edata->Methods->Add(gcnew ELib_Method(-2, method.Tag), md);
								String^ tagName = t->FullName + (m->IsStatic ? "." : ":") + m->Name;
								IList<EMethodData^>^ mdlist;
								if (this->_edata->Symbols->ContainsKey(tagName)) mdlist = this->_edata->Symbols[tagName];
								else
								{
									mdlist = gcnew List<EMethodData^>();
									this->_edata->Symbols->Add(tagName, mdlist);
								}
								mdlist->Add(md);
								if (m->IsStatic)
								{
									for each (ESection_Program_Method mm in this->_einfo->Program.ReferMethods)
									{
										if (mm.Tag != method.Tag && mm.Class == method.Class && mm.Name == method.Name)
										{
											m = FindMethod(t, UINT::Parse(CStr2String(mm.Remark)));
											md = gcnew EMethodData(m, m->IsConstructor ? EMethodMode::Newobj : EMethodMode::Call);
											mdlist->Add(md);
										}
									}
								}
								return md;
							}
						}
					}
				}
			}
		}
	}
	return nullptr;
}