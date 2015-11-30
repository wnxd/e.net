#include "stdafx.h"
#include "Plugins.h"
#include "common.net.h"

using namespace System::IO;

MonoInfo::MonoInfo()
{

}

MonoInfo::MonoInfo(EMethodMode mode, UINT tag, MethodDefinition^ method)
{
	this->Mode = mode;
	this->Tag = tag;
	this->Method = method;
}

LibGuidAttribute::LibGuidAttribute(String^ LibGuid)
{
	this->_libguid = LibGuid;
}

LibMethodAttribute::LibMethodAttribute(UINT Tag)
{
	this->_tag = Tag;
}

DefaultValueAttribute::DefaultValueAttribute(Object^ val)
{
	this->_val = val;
}

extern CustomAttribute^ FindCustom(IList<CustomAttribute^>^ list, TypeReference^ type);
extern FieldDefinition^ FindField(TypeDefinition^ type, String^ name);
TypeDefinition^ TypeClone(ModuleDefinition^ module, ModuleDefinition^ M, TypeDefinition^ type);

TypeDefinition^ FindType(IList<TypeDefinition^>^ alltype, String^ fullname)
{
	for each (TypeDefinition^ type in alltype) if (type->FullName == fullname) return type;
	return nullptr;
}

TypeReference^ GetTypeReference(ModuleDefinition^ module, ModuleDefinition^ M, TypeReference^ type)
{
	IList<int>^ list = gcnew List<int>();
	TypeReference^ ttype = type;
	do
	{
		if (ttype->IsArray) list->Insert(0, 0);
		else if (ttype->IsByReference) list->Insert(0, 1);
		else break;
		ttype = ttype->GetElementType();
	} while (true);
	TypeDefinition^ T = M->GetType(ttype->FullName);
	if (T == nullptr) return module->ImportReference(type);
	else
	{
		TypeDefinition^ t = FindType(Plugins::_refertype, (String::IsNullOrEmpty(ttype->Namespace) ? "" : ttype->Namespace + ".") + ttype->Name);
		if (t == nullptr)
		{
			t = TypeClone(module, M, T);
			Plugins::_refertype->Add(t);
		}
		ttype = t;
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

TypeDefinition^ TypeClone(ModuleDefinition^ module, ModuleDefinition^ M, TypeDefinition^ type)
{
	TypeDefinition^ t = gcnew TypeDefinition(type->Namespace, type->Name, type->Attributes);
	if (type->HasCustomAttributes) CustomClone(module, t->CustomAttributes, type->CustomAttributes);
	t->BaseType = GetTypeReference(module, M, type->BaseType);
	if (type->HasInterfaces) for each (TypeReference^ inter in type->Interfaces) t->Interfaces->Add(GetTypeReference(module, M, inter));
	if (type->HasFields)
	{
		for each (FieldDefinition^ field in type->Fields)
		{
			FieldDefinition^ f = gcnew FieldDefinition(field->Name, field->Attributes, GetTypeReference(module, M, field->FieldType));
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
	MethodDefinition^ m = gcnew MethodDefinition(method->Name, ma, GetTypeReference(module, M, method->ReturnType));
	if (method->HasCustomAttributes) CustomClone(module, m->CustomAttributes, method->CustomAttributes);
	Dictionary<ParameterDefinition^, ParameterDefinition^>^ params = gcnew Dictionary<ParameterDefinition^, ParameterDefinition^>();
	if (method->HasParameters)
	{
		for each (ParameterDefinition^ param in method->Parameters)
		{
			ParameterDefinition^ p = gcnew ParameterDefinition(param->Name, param->Attributes, GetTypeReference(module, M, param->ParameterType));
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
			m->Body->InitLocals = method->Body->Variables->Count > 0;
			for each (VariableDefinition^ var in method->Body->Variables)
			{
				VariableDefinition^ v = gcnew VariableDefinition(var->Name, GetTypeReference(module, M, var->VariableType));
				vars->Add(var, v);
				m->Body->Variables->Add(v);
			}
		}
		for each (Instruction^ ins in method->Body->Instructions)
		{
			if (ins->OpCode == OpCodes::Newobj || ins->OpCode == OpCodes::Call || ins->OpCode == OpCodes::Calli || ins->OpCode == OpCodes::Callvirt)
			{
				MethodReference^ mr = dynamic_cast<MethodReference^>(ins->Operand);
				if (M->GetType(mr->DeclaringType->FullName) != nullptr)
				{
					IList<Instruction^>^ list;
					if (Plugins::_refer->ContainsKey(mr)) list = Plugins::_refer[mr];
					else
					{
						list = gcnew List<Instruction^>();
						Plugins::_refer->Add(mr, list);
					}
					list->Add(ins);
				}
				else ins->Operand = module->ImportReference(mr);
			}
			else if (ins->OpCode == OpCodes::Ldarga_S || ins->OpCode == OpCodes::Ldarg_S || ins->OpCode == OpCodes::Starg_S) ins->Operand = params[dynamic_cast<ParameterDefinition^>(ins->Operand)];
			else if (ins->OpCode == OpCodes::Ldloca_S || ins->OpCode == OpCodes::Ldloc_S || ins->OpCode == OpCodes::Stloc_S) ins->Operand = vars[dynamic_cast<VariableDefinition^>(ins->Operand)];
			else if (ins->OpCode == OpCodes::Ldflda || ins->OpCode == OpCodes::Ldfld || ins->OpCode == OpCodes::Stfld || ins->OpCode == OpCodes::Ldsflda || ins->OpCode == OpCodes::Ldsfld || ins->OpCode == OpCodes::Stsfld)
			{
				FieldReference^ f = dynamic_cast<FieldReference^>(ins->Operand);
				TypeReference^ t = GetTypeReference(module, M, f->DeclaringType);
				if (t != f->DeclaringType) ins->Operand = FindField(t->Resolve(), f->Name);
				else ins->Operand = module->ImportReference(f);
			}
			else if (ins->OpCode == OpCodes::Castclass || ins->OpCode == OpCodes::Box || ins->OpCode == OpCodes::Unbox || ins->OpCode == OpCodes::Unbox_Any || ins->OpCode == OpCodes::Newarr || ins->OpCode == OpCodes::Ldelema || ins->OpCode == OpCodes::Initobj || ins->OpCode == OpCodes::Isinst) ins->Operand = GetTypeReference(module, M, dynamic_cast<TypeReference^>(ins->Operand));
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
		info->Lib = dynamic_cast<LibGuidAttribute^>(arr[0])->_libguid;
		List<MonoInfo^>^ methods = gcnew List<MonoInfo^>();
		ModuleDefinition^ M = ModuleDefinition::ReadModule(type->Assembly->Location);
		TypeDefinition^ T = M->GetType(type->FullName);
		Plugins::_refer = gcnew Dictionary<MethodReference^, IList<Instruction^>^>();
		Plugins::_refertype = gcnew List<TypeDefinition^>();
		IDictionary<MethodReference^, MethodReference^>^ dic = gcnew Dictionary<MethodReference^, MethodReference^>();
		for each (MethodDefinition^ method in T->Methods)
		{
			if (method->IsStatic)
			{
				for each (CustomAttribute^ ca in method->CustomAttributes)
				{
					if (ca->AttributeType == M->ImportReference(typeof(LibMethodAttribute)))
					{
						MonoInfo^ mi = gcnew MonoInfo();
						mi->Mode = EMethodMode::Call;
						mi->Tag = (UINT)ca->ConstructorArguments[0].Value;
						method->CustomAttributes->Remove(ca);
						mi->Method = MethodClone(module, M, method);
						dic->Add(method, mi->Method);
						methods->Add(mi);
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
			for each (MethodDefinition^ method in hidemethod)
			{
				MonoInfo^ mi = gcnew MonoInfo();
				mi->Mode = EMethodMode::Call;
				mi->Tag = -1;
				mi->Method = MethodClone(module, M, method);
				dic->Add(method, mi->Method);
				methods->Add(mi);
			}
		} while (true);
		info->Types = Plugins::_refertype;
		Plugins::_refer = nullptr;
		Plugins::_refertype = nullptr;
		if (plugin->Type == PluginType::Mono)
		{
			MonoPlugin^ monoplugin = dynamic_cast<MonoPlugin^>(plugin);
			if (monoplugin == nullptr) return nullptr;
			methods->AddRange(monoplugin->GetMethods(module));
		}
		info->Methods = methods;
		return info;
	}
	return nullptr;
}