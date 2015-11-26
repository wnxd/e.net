#include "stdafx.h"
#include "Plugins.h"
#include "common.net.h"

using namespace System::IO;
using namespace System::Reflection;

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

IList<PluginInfo^>^ Plugins::Load(ModuleDefinition^ module, String^ path)
{
	List<PluginInfo^>^ list = gcnew List<PluginInfo^>();
	for each (String^ file in Directory::GetFiles(path, "*.dll"))
	{
		try
		{
			Assembly^ assembly = Assembly::LoadFrom(file);
			list->AddRange(Plugins::Load(module, assembly));
		}
		catch (...)
		{

		}
	}
	return list;
}

IList<PluginInfo^>^ Plugins::Load(ModuleDefinition^ module, Assembly^ assembly)
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
		for each (MethodInfo^ method in type->GetMethods(BINDING_ALLINSTANCE))
		{
			arr = method->GetCustomAttributes(typeof(LibMethodAttribute), false);
			if (arr != nullptr && arr->Length == 1)
			{
				MonoInfo^ mi = gcnew MonoInfo();
				mi->Mode = EMethodMode::Call;
				mi->Tag = dynamic_cast<LibMethodAttribute^>(arr[0])->_tag;
				mi->Method = module->ImportReference(method)->Resolve();
				methods->Add(mi);
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