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

MethodDefinition^ MethodClone(ModuleDefinition^ module, MethodDefinition^ method)
{
	MethodDefinition^ m = gcnew MethodDefinition(method->Name, STATICMETHOD, module->ImportReference(method->ReturnType));
	Dictionary<ParameterDefinition^, ParameterDefinition^>^ params = gcnew Dictionary<ParameterDefinition^, ParameterDefinition^>();
	for each (ParameterDefinition^ param in method->Parameters)
	{
		ParameterDefinition^ p = gcnew ParameterDefinition(param->Name, param->Attributes, module->ImportReference(param->ParameterType));
		if (param->HasConstant) p->Constant = param->Constant;
		CustomAttribute^ custom = FindCustom(param->CustomAttributes, module->ImportReference(typeof(DefaultValueAttribute)));
		if (custom != nullptr)
		{
			CustomAttributeArgument^ caa = dynamic_cast<CustomAttributeArgument^>(custom->ConstructorArguments[0].Value);
			if (caa == nullptr) p->Constant = custom->ConstructorArguments[0].Value;
			else p->Constant = caa->Value;
		}
		params->Add(param, p);
		m->Parameters->Add(p);
	}
	m->Body->InitLocals = method->Body->Variables->Count > 0;
	Dictionary<VariableDefinition^, VariableDefinition^>^ vars = gcnew Dictionary<VariableDefinition^, VariableDefinition^>();
	for each (VariableDefinition^ var in method->Body->Variables)
	{
		VariableDefinition^ v = gcnew VariableDefinition(var->Name, module->ImportReference(var->VariableType));
		vars->Add(var, v);
		m->Body->Variables->Add(v);
	}
	for each (Instruction^ ins in method->Body->Instructions)
	{
		if (ins->OpCode == OpCodes::Call || ins->OpCode == OpCodes::Calli || ins->OpCode == OpCodes::Callvirt) ins->Operand = module->ImportReference(dynamic_cast<MethodReference^>(ins->Operand));
		else if (ins->OpCode == OpCodes::Ldarga_S || ins->OpCode == OpCodes::Ldarg_S || ins->OpCode == OpCodes::Starg_S) ins->Operand = params[dynamic_cast<ParameterDefinition^>(ins->Operand)];
		else if (ins->OpCode == OpCodes::Ldloca_S || ins->OpCode == OpCodes::Ldloc_S || ins->OpCode == OpCodes::Stloc_S) ins->Operand = vars[dynamic_cast<VariableDefinition^>(ins->Operand)];
		else if (ins->OpCode == OpCodes::Ldflda || ins->OpCode == OpCodes::Ldfld || ins->OpCode == OpCodes::Stfld || ins->OpCode == OpCodes::Ldsflda || ins->OpCode == OpCodes::Ldsfld || ins->OpCode == OpCodes::Stsfld) ins->Operand = module->ImportReference(dynamic_cast<FieldReference^>(ins->Operand));
		else if (ins->OpCode == OpCodes::Castclass || ins->OpCode == OpCodes::Box || ins->OpCode == OpCodes::Unbox || ins->OpCode == OpCodes::Unbox_Any || ins->OpCode == OpCodes::Newarr) ins->Operand = module->ImportReference(dynamic_cast<TypeReference^>(ins->Operand));
		m->Body->Instructions->Add(ins);
	}
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
						mi->Method = MethodClone(module, method);
						methods->Add(mi);
						break;
					}
				}
			}
		}
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