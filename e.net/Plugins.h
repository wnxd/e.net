#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace Mono::Cecil;
using namespace Mono::Cecil::Cil;

namespace wnxd
{
	namespace E_NET
	{
		public enum class EMethodMode
		{
			Call,
			Newobj,
			Embed
		};

		public interface class Plugin
		{

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
			LibMethodAttribute(UINT Tag, EMethodMode Mode);
		internal:
			UINT _tag;
			EMethodMode _mode;
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

ref struct Package
{
	EMethodMode Mode;
	IList<ModuleReference^>^ Refers;
	IList<MethodDefinition^>^ Methods;
	IList<TypeDefinition^>^ Types;
};

ref struct PluginInfo
{
	String^ Lib;
	IDictionary<UINT, IList<Package^>^>^ Packages;
};

ref class Plugins
{
public:
	static IList<PluginInfo^>^ Load(ModuleDefinition^ module, String^ path);
	static IList<PluginInfo^>^ Load(ModuleDefinition^ module, System::Reflection::Assembly^ assembly);
	static PluginInfo^ Load(ModuleDefinition^ module, Type^ type);
	static PluginInfo^ Load(ModuleDefinition^ module, Plugin^ plugin);
internal:
	static IDictionary<MethodReference^, IList<Instruction^>^>^ _refer;
	static IList<TypeDefinition^>^ _refertype;
	static IDictionary<MethodDefinition^, IList<MethodReference^>^>^ _refermethod;
	static IDictionary<MethodDefinition^, IList<TypeDefinition^>^>^ _refermethodtype;
};