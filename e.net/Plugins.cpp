#include "stdafx.h"
#include "Plugins.h"
#include "common.net.h"

using namespace System::IO;

extern CustomAttribute^ FindCustom(IList<CustomAttribute^>^ list, TypeReference^ type);
extern FieldDefinition^ FindField(TypeDefinition^ type, String^ name);

String^ GetTypeFullName(TypeReference^ type)
{
	if (type == nullptr) return nullptr;
	return  (String::IsNullOrEmpty(type->Namespace) ? "" : type->Namespace + ".") + type->Name;
}

String^ GetMethodFullName(MethodReference^ method)
{
	TypeReference^ type = method->DeclaringType;
	String^ fullname;
	if (type == nullptr) fullname = "<module>";
	else fullname = GetTypeFullName(type);
	fullname += (method->HasThis ? "" : "") + method->Name;
	for each (ParameterDefinition^ param in method->Parameters) fullname += "," + GetTypeFullName(param->ParameterType);
	return fullname;
}

TypeDefinition^ FindType(IList<TypeDefinition^>^ alltype, String^ fullname)
{
	for each (TypeDefinition^ type in alltype) if (GetTypeFullName(type) == fullname) return type;
	return nullptr;
}

MethodDefinition^ FindMethod(IList<MethodDefinition^>^ allmethod, String^ fullname)
{
	for each (MethodDefinition^ method in allmethod) if (GetMethodFullName(method) == fullname) return method;
	return nullptr;
}

MethodDefinition^ FindMethod(TypeDefinition^ type, String^ fullname)
{
	return FindMethod(type->Methods, fullname);
}

LibGuidAttribute::LibGuidAttribute(String^ LibGuid)
{
	this->_libguid = LibGuid;
}

LibMethodAttribute::LibMethodAttribute(UINT Tag)
{
	this->_tag = Tag;
	this->_mode = EMethodMode::Call;
}

LibMethodAttribute::LibMethodAttribute(UINT Tag, EMethodMode Mode)
{
	this->_tag = Tag;
	this->_mode = Mode;
}

DefaultValueAttribute::DefaultValueAttribute(Object^ val)
{
	this->_val = val;
}
Plugins::Plugins(ModuleDefinition^ module)
{
	this->_module = module;
	this->_refer = gcnew List<MethodDefinition^>();
	this->_refertype = gcnew List<TypeDefinition^>();
	this->_method = gcnew Dictionary<MethodReference^, MethodDefinition^>();
	this->_refermethod = gcnew Dictionary<MethodDefinition^, IList<MethodReference^>^>();
	this->_refermethodtype = gcnew Dictionary<MethodDefinition^, IList<TypeDefinition^>^>();
}

IList<PluginInfo^>^ Plugins::Load(String^ path)
{
	List<PluginInfo^>^ list = gcnew List<PluginInfo^>();
	for each (String^ file in Directory::GetFiles(path, "*.dll"))
	{
		try
		{
			System::Reflection::Assembly^ assembly = System::Reflection::Assembly::LoadFrom(file);
			list->AddRange(this->Load(assembly));
		}
		catch (...)
		{

		}
	}
	return list;
}

IList<PluginInfo^>^ Plugins::Load(System::Reflection::Assembly^ assembly)
{
	List<PluginInfo^>^ list = gcnew List<PluginInfo^>();
	for each (Type^ type in assembly->GetTypes())
	{
		PluginInfo^ info = this->Load(type);
		if (info != nullptr) list->Add(info);
	}
	return list;
}

PluginInfo^ Plugins::Load(Type^ type)
{
	if (typeof(Plugin)->IsAssignableFrom(type))
	{
		PluginInfo^ info = this->Load(dynamic_cast<Plugin^>(Activator::CreateInstance(type)));
		return info;
	}
	return nullptr;
}

PluginInfo^ Plugins::Load(Plugin^ plugin)
{
	Type^ type = plugin->GetType();
	array<Object^>^ arr = type->GetCustomAttributes(typeof(LibGuidAttribute), false);
	if (arr != nullptr && arr->Length == 1)
	{
		PluginInfo^ info = gcnew PluginInfo();
		info->Lib = dynamic_cast<LibGuidAttribute^>(arr[0])->_libguid->ToLower();
		info->Packages = gcnew Dictionary<UINT, IList<Package^>^>();
		for each (System::Reflection::MethodInfo^ method in type->GetMethods(BINDING_ALLINSTANCE))
		{
			array<Object^>^ arr2 = method->GetCustomAttributes(typeof(LibMethodAttribute), false);
			if (arr2 != nullptr && arr2->Length == 1 && method->ReturnType == typeof(MethodDefinition))
			{
				LibMethodAttribute^ attr = dynamic_cast<LibMethodAttribute^>(arr2[0]);
				UINT tag = attr->_tag;
				Package^ package = gcnew Package();
				package->Mode = attr->_mode;
				package->Methods = gcnew List<MethodDefinition^>();
				package->Methods->Add(dynamic_cast<MethodDefinition^>(method->Invoke(plugin, gcnew array < Object^ > { this->_module })));
				AddDictionary(info->Packages, tag, package);
			}
		}
		ModuleDefinition^ M = ModuleDefinition::ReadModule(type->Assembly->Location);
		TypeDefinition^ T = M->GetType(type->FullName);
		IDictionary<UINT, IList<MethodDefinition^>^>^ methods = gcnew Dictionary<UINT, IList<MethodDefinition^>^>();
		for each (MethodDefinition^ method in T->Methods)
		{
			if (method->IsStatic)
			{
				for each (CustomAttribute^ ca in method->CustomAttributes)
				{
					if (ca->AttributeType == M->ImportReference(typeof(LibMethodAttribute)))
					{
						UINT tag = (UINT)ca->ConstructorArguments[0].Value;
						method->CustomAttributes->Remove(ca);
						MethodDefinition^ m = this->MethodClone(M, method);
						AddItem<MethodReference^, MethodDefinition^>(this->_method, method, m);
						AddDictionary(methods, tag, m);
						break;
					}
				}
			}
		}
		for each (KeyValuePair<UINT, IList<MethodDefinition^>^>^ item in methods)
		{
			for each (MethodDefinition^ method in item->Value)
			{
				Package^ package = gcnew Package();
				package->Mode = EMethodMode::Call;
				package->Methods = gcnew List<MethodDefinition^>();
				package->Methods->Add(method);
				AddList(package->Methods, this->GetMethodList(method, this->_method));
				package->Types = GetDictionary(this->_refermethodtype, method);
				AddDictionary(info->Packages, item->Key, package);
			}
		}
		return info;
	}
	return nullptr;
}

IList<MethodDefinition^>^ Plugins::GetMethodList(MethodDefinition^ method, IDictionary<MethodReference^, MethodDefinition^>^ chart)
{
	List<MethodDefinition^>^ list = gcnew List<MethodDefinition^>();
	IList<MethodReference^>^ refer = GetDictionary(this->_refermethod, method);
	if (refer != nullptr)
	{
		for each (MethodReference^ item in refer)
		{
			MethodDefinition^ m = GetDictionary(chart, item);
			if (m == nullptr) m = item->Resolve();
			AddList(list, m);
			AddList(list, this->GetMethodList(m, chart));
		}
	}
	return list;
}

TypeReference^ Plugins::GetTypeReference(MethodDefinition^ method, ModuleDefinition^ M, TypeReference^ type)
{
	if (type->Scope == M)
	{
		IList<int>^ list = gcnew List<int>();
		TypeReference^ ttype = type;
		do
		{
			if (ttype->IsArray) list->Insert(0, 0);
			else if (ttype->IsByReference) list->Insert(0, 1);
			else break;
			ttype = GetElementType(ttype);
		} while (true);
		TypeDefinition^ t = FindType(this->_refertype, GetTypeFullName(ttype));
		if (t == nullptr) t = this->TypeClone(method, M, M->GetType(ttype->FullName));
		AddDictionary(this->_refermethodtype, method, t);
		type = t;
		for each (int item in list)
		{
			switch (item)
			{
			case 0:
				type = gcnew ArrayType(type);
				break;
			case 1:
				type = gcnew ByReferenceType(type);
				break;;
			}
		}
	}
	else
	{
		if (type->Scope == nullptr) AddDictionary(this->_refermethodtype, method, type->GetElementType()->Resolve());
		else type = this->_module->ImportReference(type);
	}
	return type;
}

MethodReference^ Plugins::GetMethodReference(MethodDefinition^ method, ModuleDefinition^ M, MethodReference^ m)
{
	TypeReference^ type = m->DeclaringType;
	if (type == nullptr) AddDictionary(this->_refermethod, method, m);
	else
	{
		if (type->Scope == nullptr) AddDictionary(this->_refermethodtype, method, type->Resolve());
		else if (type->Scope == M)
		{
			if (m->HasThis)
			{
				type = this->GetTypeReference(method, M, type);
				m = FindMethod(type->Resolve(), GetMethodFullName(m));
			}
			else
			{
				MethodDefinition^ mm = this->MethodClone(M, m->Resolve());
				AddItem<MethodReference^, MethodDefinition^>(this->_method, m, mm);
				AddDictionary(this->_refermethod, method, m);
				m = mm;
			}
		}
		else m = this->_module->ImportReference(m);
	}
	return m;
}

void Plugins::CustomClone(IList<CustomAttribute^>^ newarr, IList<CustomAttribute^>^ oldarr)
{
	for each (CustomAttribute^ custom in oldarr)
	{
		CustomAttribute^ c = gcnew CustomAttribute(this->_module->ImportReference(custom->Constructor));
		if (custom->HasConstructorArguments) for each (CustomAttributeArgument argument in custom->ConstructorArguments) c->ConstructorArguments->Add(CustomAttributeArgument(this->_module->ImportReference(argument.Type), argument.Value));
		newarr->Add(c);
	}
}

TypeDefinition^ Plugins::TypeClone(MethodDefinition^ method, ModuleDefinition^ M, TypeDefinition^ type)
{
	TypeDefinition^ t = FindType(this->_refertype, GetTypeFullName(type));
	if (t == nullptr)
	{
		t = gcnew TypeDefinition(type->Namespace, type->Name, type->Attributes);
		this->_refertype->Add(t);
		if (type->HasCustomAttributes) CustomClone(t->CustomAttributes, type->CustomAttributes);
		t->BaseType = this->GetTypeReference(method, M, type->BaseType);
		if (type->HasInterfaces) for each (TypeReference^ inter in type->Interfaces) t->Interfaces->Add(this->GetTypeReference(method, M, inter));
		if (type->HasFields)
		{
			for each (FieldDefinition^ field in type->Fields)
			{
				FieldDefinition^ f = gcnew FieldDefinition(field->Name, field->Attributes, this->GetTypeReference(method, M, field->FieldType));
				if (field->HasCustomAttributes) this->CustomClone(f->CustomAttributes, field->CustomAttributes);
				t->Fields->Add(f);
			}
		}
		if (type->HasMethods) for each (MethodDefinition^ mm in type->Methods) if (mm->HasThis) AddList(t->Methods, this->MethodClone(M, mm));
		t->PackingSize = type->PackingSize;
		t->ClassSize = type->ClassSize;
	}
	return t;
}

MethodDefinition^ Plugins::MethodClone(ModuleDefinition^ M, MethodDefinition^ method)
{
	MethodDefinition^ m = FindMethod(this->_refer, GetMethodFullName(method));
	if (m == nullptr)
	{
		MethodAttributes ma = method->Attributes;
		if (ma.HasFlag(MethodAttributes::Private)) ma = ma - MethodAttributes::Private;
		m = gcnew MethodDefinition(method->Name, ma, this->_module->TypeSystem->Object);
		AddList(this->_refer, m);
		m->ReturnType = this->GetTypeReference(m, M, method->ReturnType);
		if (method->HasCustomAttributes) this->CustomClone(m->CustomAttributes, method->CustomAttributes);
		Dictionary<ParameterDefinition^, ParameterDefinition^>^ params = gcnew Dictionary<ParameterDefinition^, ParameterDefinition^>();
		if (method->HasParameters)
		{
			for each (ParameterDefinition^ param in method->Parameters)
			{
				ParameterDefinition^ p = gcnew ParameterDefinition(param->Name, param->Attributes, this->GetTypeReference(m, M, param->ParameterType));
				if (param->HasConstant) p->Constant = param->Constant;
				if (param->HasCustomAttributes)
				{
					CustomAttribute^ custom = FindCustom(param->CustomAttributes, M->ImportReference(typeof(DefaultValueAttribute)));
					if (custom != nullptr)
					{
						CustomAttributeArgument^ caa = dynamic_cast<CustomAttributeArgument^>(custom->ConstructorArguments[0].Value);
						if (caa == nullptr) p->Constant = custom->ConstructorArguments[0].Value;
						else p->Constant = caa->Value;
						param->CustomAttributes->Remove(custom);
					}
					this->CustomClone(p->CustomAttributes, param->CustomAttributes);
				}
				params->Add(param, p);
				m->Parameters->Add(p);
			}
		}
		if (method->HasBody)
		{
			Dictionary<VariableDefinition^, VariableDefinition^>^ vars = gcnew Dictionary<VariableDefinition^, VariableDefinition^>();
			if (method->Body->HasVariables)
			{
				m->Body->InitLocals = method->Body->InitLocals;
				for each (VariableDefinition^ var in method->Body->Variables)
				{
					VariableDefinition^ v = gcnew VariableDefinition(var->Name, this->GetTypeReference(m, M, var->VariableType));
					vars->Add(var, v);
					m->Body->Variables->Add(v);
				}
			}
			for each (Instruction^ ins in method->Body->Instructions)
			{
				if (ins->OpCode == OpCodes::Newobj || ins->OpCode == OpCodes::Call || ins->OpCode == OpCodes::Calli || ins->OpCode == OpCodes::Callvirt || ins->OpCode == OpCodes::Ldftn) ins->Operand = this->GetMethodReference(m, M, dynamic_cast<MethodReference^>(ins->Operand));
				else if (ins->OpCode == OpCodes::Ldarga_S || ins->OpCode == OpCodes::Ldarg_S || ins->OpCode == OpCodes::Starg_S) ins->Operand = params[dynamic_cast<ParameterDefinition^>(ins->Operand)];
				else if (ins->OpCode == OpCodes::Ldloca_S || ins->OpCode == OpCodes::Ldloc_S || ins->OpCode == OpCodes::Stloc_S) ins->Operand = vars[dynamic_cast<VariableDefinition^>(ins->Operand)];
				else if (ins->OpCode == OpCodes::Ldflda || ins->OpCode == OpCodes::Ldfld || ins->OpCode == OpCodes::Stfld || ins->OpCode == OpCodes::Ldsflda || ins->OpCode == OpCodes::Ldsfld || ins->OpCode == OpCodes::Stsfld)
				{
					FieldReference^ f = dynamic_cast<FieldReference^>(ins->Operand);
					TypeReference^ t = this->GetTypeReference(m, M, f->DeclaringType);
					if (t != f->DeclaringType) ins->Operand = FindField(t->Resolve(), f->Name);
					else ins->Operand = this->_module->ImportReference(f);
				}
				else if (ins->OpCode == OpCodes::Castclass || ins->OpCode == OpCodes::Box || ins->OpCode == OpCodes::Unbox || ins->OpCode == OpCodes::Unbox_Any || ins->OpCode == OpCodes::Newarr || ins->OpCode == OpCodes::Ldelema || ins->OpCode == OpCodes::Initobj || ins->OpCode == OpCodes::Isinst || ins->OpCode == OpCodes::Ldobj || ins->OpCode == OpCodes::Stobj) ins->Operand = this->GetTypeReference(m, M, dynamic_cast<TypeReference^>(ins->Operand));
				m->Body->Instructions->Add(ins);
			}
		}
		if (method->HasPInvokeInfo) m->PInvokeInfo = gcnew PInvokeInfo(method->PInvokeInfo->Attributes, method->PInvokeInfo->EntryPoint, method->PInvokeInfo->Module);
		m->ImplAttributes = method->ImplAttributes;
		if (method->MethodReturnType->HasMarshalInfo) m->MethodReturnType->MarshalInfo = method->MethodReturnType->MarshalInfo;
	}
	return m;
}