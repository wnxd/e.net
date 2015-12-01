#include "stdafx.h"
#include "efs.h"
#include "common.net.h"
#include "Plugins.h"
#include "compile.h"
#include "refer.h"

using namespace System::IO;

extern INT WINAPI NotifySys(INT nMsg, DWORD dwParam1, DWORD dwParam2);
extern EInfo* ParseEcode(byte* code);
extern array<byte>^ ReadFile(String^ path);

String^ GetMethodName(MethodReference^ method)
{
	String^ fullname;
	TypeReference^ type = method->DeclaringType;
	if (type != nullptr) fullname = type->FullName + (method->HasThis ? "." : ":") + method->Name;
	else fullname = "<Module>:" + method->Name;
	return fullname;
}

EMethodData::EMethodData()
{

}

EMethodData::EMethodData(MethodReference^ method)
{
	this->Method = method;
	this->Mode = EMethodMode::Call;
}

EMethodData::EMethodData(MethodReference^ method, EMethodMode mode)
{
	this->Method = method;
	this->Mode = mode;
}

bool EMethodData::operator==(EMethodData^ data)
{
	bool b1 = Object::Equals(this, nullptr);
	bool b2 = Object::Equals(data, nullptr);
	if (b1 == b2)
	{
		if (b1) return true;
		return this->Mode == data->Mode && this->Method == data->Method;
	}
	else return false;	
}

bool EMethodData::operator!=(EMethodData^ data)
{
	return !(this == data);
}

EMethodData::operator MethodReference ^ ()
{
	return this->Method;
}

EMethodData::operator MethodDefinition ^ ()
{
	return this->Method->Resolve();
}

CodeRefer::CodeRefer(ModuleDefinition^ module)
{
	this->_module = module;
	this->_type = gcnew Dictionary<String^, TypeDefinition^>();
	this->_typename = gcnew Dictionary<UINT, String^>();
	this->_typerefer = gcnew Dictionary<String^, TypeReference^>();
	this->_typerefername = gcnew Dictionary<UINT, String^>();
	this->_method = gcnew Dictionary<short, Dictionary<UINT, EMethodData^>^>();
	this->_methodname = gcnew Dictionary<String^, List<EMethodData^>^>();
	this->_var = gcnew Dictionary<UINT, VariableDefinition^>();
	this->_param = gcnew Dictionary<UINT, ParameterDefinition^>();
	this->_field = gcnew Dictionary<UINT, FieldDefinition^>();
	this->_globalvar = gcnew Dictionary<UINT, FieldDefinition^>();
	this->_prop = gcnew Dictionary<UINT, PropertyDefinition^>();
	this->_eclist = gcnew List<ECListInfo^>();
}

CodeRefer::~CodeRefer()
{
	for each (ECListInfo^ item in this->_eclist) delete item->Info;
}

void CodeRefer::AddType(ETAG tag, TypeDefinition^ type)
{
	if (!this->_typename->ContainsKey(tag))
	{
		this->_typename->Add(tag, type->FullName);
		if (!this->_type->ContainsKey(type->FullName)) this->_type->Add(type->FullName, type);
	}
}

void CodeRefer::AddTypeRefer(ETAG tag, TypeReference^ type)
{
	if (!this->_typerefername->ContainsKey(tag))
	{
		this->_typerefername->Add(tag, type->FullName);
		if (!this->_typerefer->ContainsKey(type->FullName)) this->_typerefer->Add(type->FullName, type);
	}
}

void CodeRefer::AddTypeRefer(ETAG tag, String^ fullname)
{
	if (!this->_typerefername->ContainsKey(tag)) this->_typerefername->Add(tag, fullname);
}

void CodeRefer::AddReferList(IEnumerable<String^>^ list)
{
	for each (String^ refer in list) this->AddReferList(refer);
}

void CodeRefer::AddReferList(String^ refer)
{
	ModuleDefinition^ module = ModuleDefinition::ReadModule(refer);
	for each (TypeDefinition^ type in module->GetTypes()) if (type->IsPublic && !(type->IsGenericParameter || type->IsGenericInstance)) this->_typerefer->Add(type->FullName, type);
}

void CodeRefer::AddMethodRefer(short index, ETAG tag, EMethodData^ method)
{
	Dictionary<UINT, EMethodData^>^ dic;
	if (this->_method->ContainsKey(index)) dic = this->_method[index];
	else
	{
		dic = gcnew Dictionary<UINT, EMethodData^>();
		this->_method->Add(index, dic);
	}
	if (dic->ContainsKey(tag))
	{
		if (dic[tag] != method) goto addsymbol;
	}
	else
	{
		dic->Add(tag, method);
	addsymbol:
		String^ methodname = GetMethodName(method);
		List<EMethodData^>^ list;
		if (this->_methodname->ContainsKey(methodname)) list = this->_methodname[methodname];
		else
		{
			list = gcnew List<EMethodData^>();
			this->_methodname->Add(methodname, list);
		}
		list->Add(method);
	}
}

void CodeRefer::AddMethodList(IEnumerable<EMethodData^>^ list)
{
	if (list != nullptr)
	{
		IEnumerator<EMethodData^>^ t = list->GetEnumerator();
		if (t != nullptr && t->Current != nullptr)
		{
			String^ methodname = GetMethodName(dynamic_cast<EMethodData^>(t));
			List<EMethodData^>^ list;
			if (this->_methodname->ContainsKey(methodname)) list = this->_methodname[methodname];
			else
			{
				list = gcnew List<EMethodData^>();
				this->_methodname->Add(methodname, list);
			}
			list->AddRange(list);
		}
	}
}

void CodeRefer::AddVariable(ETAG tag, VariableDefinition^ var)
{
	if (!this->_var->ContainsKey(tag)) this->_var->Add(tag, var);
}

void CodeRefer::AddParameter(ETAG tag, ParameterDefinition^ param)
{
	if (!this->_param->ContainsKey(tag)) this->_param->Add(tag, param);
}

void CodeRefer::AddField(ETAG tag, FieldDefinition^ field)
{
	if (!this->_field->ContainsKey(tag)) this->_field->Add(tag, field);
}

void CodeRefer::AddGlobalVariable(ETAG tag, FieldDefinition^ var)
{
	if (!this->_globalvar->ContainsKey(tag)) this->_globalvar->Add(tag, var);
}

void CodeRefer::AddProperty(ETAG tag, PropertyDefinition^ prop)
{
	if (!this->_prop->ContainsKey(tag)) this->_prop->Add(tag, prop);
}

void CodeRefer::AddECList(IEnumerable<String^>^ list)
{
	if (list != nullptr)
	{
		char c[MAX_PATH];
		NotifySys(NAS_GET_PATH, 1004, (DWORD)c);
		String^ path = LPSTR2String(c);
		for each (String^ item in list)
		{
			if (File::Exists(item))
			{
			add:
				array<byte>^ code = ReadFile(item);
				if (code != nullptr)
				{
					IntPtr ptr = Marshal::AllocHGlobal(code->LongLength);
					Marshal::Copy(code, 0, ptr, code->LongLength);
					EInfo* einfo = ParseEcode(Ptr2LPBYTE(ptr));
					if (einfo != NULL)
					{
						ECListInfo^ info = gcnew ECListInfo();
						info->Info = einfo;
						this->_eclist->Add(info);
					}
					Marshal::FreeHGlobal(ptr);
				}
			}
			else
			{
				item = path + "/" + item;
				if (File::Exists(item)) goto add;
			}
		}
	}
}

TypeDefinition^ CodeRefer::FindType(ETAG tag)
{
	if (this->_typename->ContainsKey(tag)) return this->FindType(this->_typename[tag]);
	return nullptr;
}

TypeDefinition^ CodeRefer::FindType(String^ fullname)
{
	if (this->_type->ContainsKey(fullname)) return this->_type[fullname];
	return nullptr;
}

TypeReference^ CodeRefer::FindTypeRefer(ETAG tag)
{
	TypeReference^ type = this->FindType(tag);
	if (type == nullptr) if (this->_typerefername->ContainsKey(tag)) type = this->FindTypeRefer(this->_typerefername[tag]);
	if (type == nullptr) return nullptr;
	return this->_module->ImportReference(type);
}

TypeReference^ CodeRefer::FindTypeRefer(String^ fullname)
{
	TypeReference^ type = this->FindType(fullname);
	if (type == nullptr) if (this->_typerefer->ContainsKey(fullname)) type = this->_typerefer[fullname];
	if (type == nullptr) return nullptr;
	return this->_module->ImportReference(type);
}

TypeDefinition^ CodeRefer::FindTypeDefine(ETAG tag)
{
	TypeDefinition^ type = this->FindType(tag);
	if (type == nullptr) if (this->_typerefername->ContainsKey(tag)) type = this->FindTypeDefine(this->_typerefername[tag]);
	if (type == nullptr) return nullptr;
	return type;
}

TypeDefinition^ CodeRefer::FindTypeDefine(String^ fullname)
{
	TypeReference^ type = this->FindType(fullname);
	if (type == nullptr) if (this->_typerefer->ContainsKey(fullname)) type = this->_typerefer[fullname];
	if (type == nullptr) return nullptr;
	return type->Resolve();
}

EMethodData^ CodeRefer::FindMethodRefer(short index, ETAG tag)
{
	if (this->_method->ContainsKey(index))
	{
		Dictionary<UINT, EMethodData^>^ dic = this->_method[index];
		if (dic->ContainsKey(tag)) return dic[tag];
	}
	return nullptr;
}

List<EMethodData^>^ CodeRefer::FindMethodList(MethodReference^ method)
{
	return this->FindMethodList(GetMethodName(method));
}

List<EMethodData^>^ CodeRefer::FindMethodList(String^ fullname)
{
	if (this->_methodname->ContainsKey(fullname)) return this->_methodname[fullname];
	return nullptr;
}

VariableDefinition^ CodeRefer::FindVariable(ETAG tag)
{
	if (this->_var->ContainsKey(tag)) return this->_var[tag];
	return nullptr;
}

ParameterDefinition^ CodeRefer::FindParameter(ETAG tag)
{
	if (this->_param->ContainsKey(tag)) return this->_param[tag];
	return nullptr;
}

FieldDefinition^ CodeRefer::FindField(ETAG tag)
{
	if (this->_field->ContainsKey(tag)) return this->_field[tag];
	return nullptr;
}

FieldDefinition^ CodeRefer::FindGlobalVariable(ETAG tag)
{
	if (this->_globalvar->ContainsKey(tag)) return this->_globalvar[tag];
	return nullptr;
}

PropertyDefinition^ CodeRefer::FindProperty(ETAG tag)
{
	if (this->_prop->ContainsKey(tag)) return this->_prop[tag];
	return nullptr;
}