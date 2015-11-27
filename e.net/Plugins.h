#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace Mono::Cecil;

namespace wnxd
{
	namespace E_NET
	{
		public enum class PluginType
		{
			DoNet,
			Mono
		};

		public enum class EMethodMode
		{
			Call,
			Newobj,
			Embed
		};

		public interface class Plugin
		{
			property PluginType Type
			{
				PluginType get();
			}
		};

		public ref struct MonoInfo
		{
			EMethodMode Mode;
			UINT Tag;
			MethodDefinition^ Method;
			MonoInfo();
			MonoInfo(EMethodMode mode, UINT tag, MethodDefinition^ method);
		};

		public interface class MonoPlugin
		{
			IList<MonoInfo^>^ GetMethods(ModuleDefinition^ module);
		};

		[AttributeUsage(AttributeTargets::Class, AllowMultiple = false)]
		public ref class LibGuidAttribute :Attribute
		{
		public:
			LibGuidAttribute(String^ LibGuid);
		internal:
			String^ _libguid;
		};

		[AttributeUsage(AttributeTargets::Method, AllowMultiple = false)]
		public ref class LibMethodAttribute :Attribute
		{
		public:
			LibMethodAttribute(UINT Tag);
		internal:
			UINT _tag;
		};

		[AttributeUsage(AttributeTargets::Parameter, AllowMultiple = false)]
		public ref class DefaultValueAttribute :Attribute
		{
		public:
			DefaultValueAttribute(Object^ val);
		internal:
			Object^ _val;
		};
	}
}

using namespace wnxd::E_NET;

ref struct PluginInfo
{
	String^ Lib;
	IList<MonoInfo^>^ Methods;
};

ref class Plugins
{
public:
	static IList<PluginInfo^>^ Load(ModuleDefinition^ module, String^ path);
	static IList<PluginInfo^>^ Load(ModuleDefinition^ module, System::Reflection::Assembly^ assembly);
	static PluginInfo^ Load(ModuleDefinition^ module, Type^ type);
	static PluginInfo^ Load(ModuleDefinition^ module, Plugin^ plugin);
};