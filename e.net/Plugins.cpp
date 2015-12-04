#include "stdafx.h"
#include "Plugins.h"
#include "common.net.h"

using namespace System::IO;

extern CustomAttribute^ FindCustom(IList<CustomAttribute^>^ list, TypeReference^ type);
extern FieldDefinition^ FindField(TypeDefinition^ type, String^ name);
TypeDefinition^ TypeClone(MethodDefinition^ method, ModuleDefinition^ module, ModuleDefinition^ M, TypeDefinition^ type);

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

TypeDefinition^ FindType(IList<TypeDefinition^>^ alltype, String^ fullname)
{
	for each (TypeDefinition^ type in alltype) if (type->FullName == fullname) return type;
	return nullptr;
}

IList<MethodDefinition^>^ GetMethodList(MethodDefinition^ method, IDictionary<MethodReference^, MethodDefinition^>^ chart)
{
	List<MethodDefinition^>^ list = gcnew List<MethodDefinition^>();
	IList<MethodReference^>^ refer = GetDictionary(Plugins::_refermethod, method);
	if (refer != nullptr)
	{
		for each (MethodReference^ item in refer)
		{
			MethodDefinition^ m = GetDictionary(chart, item);
			if (m == nullptr) m = item->Resolve();
			AddList(list, m);
			AddList(list, GetMethodList(m, chart));
		}
	}
	return list;
}

TypeReference^ GetTypeReference(MethodDefinition^ method, ModuleDefinition^ module, ModuleDefinition^ M, TypeReference^ type)
{
	if (type->Module == module) return type;
	IList<int>^ list = gcnew List<int>();
	TypeReference^ ttype = type;
	do
	{
		if (ttype->IsArray) list->Insert(0, 0);
		else if (ttype->IsByReference) list->Insert(0, 1);
		else if (ttype->Name == "Nullable`1") list->Insert(0, 2);
		else break;
		ttype = GetElementType(ttype);
	} while (true);
	TypeDefinition^ T = M->GetType(ttype->FullName);
	if (T == nullptr)
	{
		ttype = module->ImportReference(ttype);
		goto rt;
	}
	else
	{
		TypeDefinition^ t = FindType(Plugins::_refertype, (String::IsNullOrEmpty(ttype->Namespace) ? "" : ttype->Namespace + ".") + ttype->Name);
		if (t == nullptr)
		{
			t = TypeClone(method, module, M, T);
			Plugins::_refertype->Add(t);
		}
		AddDictionary(Plugins::_refermethodtype, method, t);
		ttype = t;
	rt:
		for each (int item in list)
		{
			switch (item)
			{
			case 0:
				ttype = gcnew ArrayType(ttype);
				break;
			case 1:
				ttype = gcnew ByReferenceType(ttype);
				break;
			case 2:
				ttype = CreateNullable(ttype);
				break;
			}
		}
		return ttype;
	}
}

void CustomClone(ModuleDefinition^ module, IList<CustomAttribute^>^ newarr, IList<CustomAttribute^>^ oldarr)
{
	for each (CustomAttribute^ custom in oldarr)
	{
		CustomAttribute^ c = gcnew CustomAttribute(module->ImportReference(custom->Constructor));
		if (custom->HasConstructorArguments) for each (CustomAttributeArgument argument in custom->ConstructorArguments) c->ConstructorArguments->Add(CustomAttributeArgument(module->ImportReference(argument.Type), argument.Value));
		newarr->Add(c);
	}
}

TypeDefinition^ TypeClone(MethodDefinition^ method, ModuleDefinition^ module, ModuleDefinition^ M, TypeDefinition^ type)
{
	TypeDefinition^ t = gcnew TypeDefinition(type->Namespace, type->Name, type->Attributes);
	if (type->HasCustomAttributes) CustomClone(module, t->CustomAttributes, type->CustomAttributes);
	t->BaseType = GetTypeReference(method, module, M, type->BaseType);
	if (type->HasInterfaces) for each (TypeReference^ inter in type->Interfaces) t->Interfaces->Add(GetTypeReference(method, module, M, inter));
	if (type->HasFields)
	{
		for each (FieldDefinition^ field in type->Fields)
		{
			FieldDefinition^ f = gcnew FieldDefinition(field->Name, field->Attributes, GetTypeReference(method, module, M, field->FieldType));
			if (field->HasCustomAttributes) CustomClone(module, f->CustomAttributes, field->CustomAttributes);
			t->Fields->Add(f);
		}
	}
	t->PackingSize = type->PackingSize;
	t->ClassSize = type->ClassSize;
	return t;
}

MethodDefinition^ MethodClone(ModuleDefinition^ module, ModuleDefinition^ M, MethodDefinition^ method)
{
	MethodAttributes ma = method->Attributes;
	if (ma.HasFlag(MethodAttributes::Private)) ma = ma - MethodAttributes::Private;
	MethodDefinition^ m = gcnew MethodDefinition(method->Name, ma, module->TypeSystem->Object);
	m->ReturnType = GetTypeReference(m, module, M, method->ReturnType);
	if (method->HasCustomAttributes) CustomClone(module, m->CustomAttributes, method->CustomAttributes);
	Dictionary<ParameterDefinition^, ParameterDefinition^>^ params = gcnew Dictionary<ParameterDefinition^, ParameterDefinition^>();
	if (method->HasParameters)
	{
		for each (ParameterDefinition^ param in method->Parameters)
		{
			ParameterDefinition^ p = gcnew ParameterDefinition(param->Name, param->Attributes, GetTypeReference(m, module, M, param->ParameterType));
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
				CustomClone(module, p->CustomAttributes, param->CustomAttributes);
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
				VariableDefinition^ v = gcnew VariableDefinition(var->Name, GetTypeReference(m, module, M, var->VariableType));
				vars->Add(var, v);
				m->Body->Variables->Add(v);
			}
		}
		for each (Instruction^ ins in method->Body->Instructions)
		{
			if (ins->OpCode == OpCodes::Newobj || ins->OpCode == OpCodes::Call || ins->OpCode == OpCodes::Calli || ins->OpCode == OpCodes::Callvirt || ins->OpCode == OpCodes::Ldftn)
			{
				MethodReference^ mr = dynamic_cast<MethodReference^>(ins->Operand);
				if (M->GetType(mr->DeclaringType->FullName) != nullptr)
				{
					AddDictionary(Plugins::_refermethod, m, mr);
					AddDictionary(Plugins::_refer, mr, ins);
				}
				else ins->Operand = module->ImportReference(mr);
			}
			else if (ins->OpCode == OpCodes::Ldarga_S || ins->OpCode == OpCodes::Ldarg_S || ins->OpCode == OpCodes::Starg_S) ins->Operand = params[dynamic_cast<ParameterDefinition^>(ins->Operand)];
			else if (ins->OpCode == OpCodes::Ldloca_S || ins->OpCode == OpCodes::Ldloc_S || ins->OpCode == OpCodes::Stloc_S) ins->Operand = vars[dynamic_cast<VariableDefinition^>(ins->Operand)];
			else if (ins->OpCode == OpCodes::Ldflda || ins->OpCode == OpCodes::Ldfld || ins->OpCode == OpCodes::Stfld || ins->OpCode == OpCodes::Ldsflda || ins->OpCode == OpCodes::Ldsfld || ins->OpCode == OpCodes::Stsfld)
			{
				FieldReference^ f = dynamic_cast<FieldReference^>(ins->Operand);
				TypeReference^ t = GetTypeReference(m, module, M, f->DeclaringType);
				if (t != f->DeclaringType) ins->Operand = FindField(t->Resolve(), f->Name);
				else ins->Operand = module->ImportReference(f);
			}
			else if (ins->OpCode == OpCodes::Castclass || ins->OpCode == OpCodes::Box || ins->OpCode == OpCodes::Unbox || ins->OpCode == OpCodes::Unbox_Any || ins->OpCode == OpCodes::Newarr || ins->OpCode == OpCodes::Ldelema || ins->OpCode == OpCodes::Initobj || ins->OpCode == OpCodes::Isinst || ins->OpCode == OpCodes::Ldobj) ins->Operand = GetTypeReference(m, module, M, dynamic_cast<TypeReference^>(ins->Operand));
			m->Body->Instructions->Add(ins);
		}
	}
	if (method->HasPInvokeInfo) m->PInvokeInfo = gcnew PInvokeInfo(method->PInvokeInfo->Attributes, method->PInvokeInfo->EntryPoint, method->PInvokeInfo->Module);
	m->ImplAttributes = method->ImplAttributes;
	if (method->MethodReturnType->HasMarshalInfo) m->MethodReturnType->MarshalInfo = method->MethodReturnType->MarshalInfo;
	return m;
}

IList<PluginInfo^>^ Plugins::Load(ModuleDefinition^ module, String^ path)
{
	List<PluginInfo^>^ list = gcnew List<PluginInfo^>();
	for each (String^ file in Directory::GetFiles(path, "*.dll"))
	{
		try
		{
			System::Reflection::Assembly^ assembly = System::Reflection::Assembly::LoadFrom(file);
			list->AddRange(Plugins::Load(module, assembly));
		}
		catch (...)
		{

		}
	}
	return list;
}

IList<PluginInfo^>^ Plugins::Load(ModuleDefinition^ module, System::Reflection::Assembly^ assembly)
{
	List<PluginInfo^>^ list = gcnew List<PluginInfo^>();
	for each (Type^ type in assembly->GetTypes())
	{
		PluginInfo^ info = Plugins::Load(module, type);
		if (info != nullptr) list->Add(info);
	}
	return list;
}

PluginInfo^ Plugins::Load(ModuleDefinition^ module, Type^ type)
{
	if (typeof(Plugin)->IsAssignableFrom(type))
	{
		PluginInfo^ info = Plugins::Load(module, dynamic_cast<Plugin^>(Activator::CreateInstance(type)));
		return info;
	}
	return nullptr;
}

PluginInfo^ Plugins::Load(ModuleDefinition^ module, Plugin^ plugin)
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
				package->Methods->Add(dynamic_cast<MethodDefinition^>(method->Invoke(plugin, gcnew array < Object^ > { module })));
				AddDictionary(info->Packages, tag, package);
			}
		}
		ModuleDefinition^ M = ModuleDefinition::ReadModule(type->Assembly->Location);
		TypeDefinition^ T = M->GetType(type->FullName);
		Plugins::_refer = gcnew Dictionary<MethodReference^, IList<Instruction^>^>();
		Plugins::_refertype = gcnew List<TypeDefinition^>();
		Plugins::_refermethod = gcnew Dictionary<MethodDefinition^, IList<MethodReference^>^>();
		Plugins::_refermethodtype = gcnew Dictionary<MethodDefinition^, IList<TypeDefinition^>^>();
		IDictionary<UINT, IList<MethodDefinition^>^>^ methods = gcnew Dictionary<UINT, IList<MethodDefinition^>^>();
		IDictionary<MethodReference^, MethodDefinition^>^ dic = gcnew Dictionary<MethodReference^, MethodDefinition^>();
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
						MethodDefinition^ m = MethodClone(module, M, method);
						dic->Add(method, m);
						AddDictionary(methods, tag, m);
						break;
					}
				}
			}
		}
		do
		{
			IList<MethodDefinition^>^ hidemethod = gcnew List<MethodDefinition^>();
			IDictionary<MethodReference^, IList<Instruction^>^>^ dic2 = gcnew Dictionary<MethodReference^, IList<Instruction^>^>();
			for each (KeyValuePair<MethodReference^, IList<Instruction^>^>^ item in Plugins::_refer)
			{
				if (dic->ContainsKey(item->Key))
				{
					MethodReference^ method = dic[item->Key];
					for each (Instruction^ ins in item->Value) ins->Operand = method;
				}
				else
				{
					hidemethod->Add(item->Key->Resolve());
					dic2->Add(item->Key, item->Value);
				}
			}
			if (hidemethod->Count == 0) break;
			Plugins::_refer = dic2;
			for each (MethodDefinition^ method in hidemethod) dic->Add(method, MethodClone(module, M, method));
		} while (true);
		for each (KeyValuePair<UINT, IList<MethodDefinition^>^>^ item in methods)
		{
			for each (MethodDefinition^ method in item->Value)
			{
				Package^ package = gcnew Package();
				package->Mode = EMethodMode::Call;
				package->Methods = gcnew List<MethodDefinition^>();
				package->Methods->Add(method);
				AddList(package->Methods, GetMethodList(method, dic));
				package->Types = GetDictionary(Plugins::_refermethodtype, method);
				AddDictionary(info->Packages, item->Key, package);
			}
		}
		Plugins::_refer = nullptr;
		Plugins::_refertype = nullptr;
		Plugins::_refermethod = nullptr;
		Plugins::_refermethodtype = nullptr;
		return info;
	}
	return nullptr;
}