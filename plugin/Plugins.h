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

		[AttributeUsage(AttributeTargets::Class, AllowMultiple = false)]
		public ref class LibTypeAttribute :Attribute
		{
		public:
			LibTypeAttribute(UINT Tag);
		};

		public ref class LibTypeTagAttribute :Attribute
		{
		public:
			LibTypeTagAttribute(UINT Tag);
		};
	}
}

using namespace wnxd::E_NET;

public ref struct MethodPackage
{
	EMethodMode Mode;
	IList<MethodDefinition^>^ Methods;
	IList<TypeDefinition^>^ Types;
};

public ref struct TypePackage
{
	UINT Tag;
	TypeDefinition^ Type;
	IDictionary<UINT, PropertyDefinition^>^ Properties;
	IDictionary<UINT, MethodDefinition^>^ Methods;
	IDictionary<UINT, EventDefinition^>^ Events;
	IList<MethodDefinition^>^ ReferMethods;
	IList<TypeDefinition^>^ ReferTypes;
};

public ref struct PluginInfo
{
	String^ Lib;
	IDictionary<UINT, IList<MethodPackage^>^>^ MethodPackages;
	TypePackage^ TypePackages;
};

public ref class Plugins
{
public:
	Plugins(ModuleDefinition^ module);
	IList<PluginInfo^>^ Load(String^ path);
	IList<PluginInfo^>^ Load(System::Reflection::Assembly^ assembly);
	PluginInfo^ Load(Type^ type);
	PluginInfo^ Load(Plugin^ plugin);
	PluginInfo^ LoadType(Type^ type);
private:
	ModuleDefinition^ _module;
	IList<MethodDefinition^>^ _refer;
	IList<TypeDefinition^>^ _refertype;
	IDictionary<MethodReference^, MethodDefinition^>^ _method;
	IDictionary<MethodDefinition^, IList<MethodReference^>^>^ _refermethod;
	IDictionary<MethodDefinition^, IList<TypeDefinition^>^>^ _refermethodtype;
	IDictionary<TypeDefinition^, TypePackage^>^ _typepackages;
	MethodDefinition^ MethodClone(ModuleDefinition^ M, MethodDefinition^ method);
	TypeDefinition^ TypeClone(MethodDefinition^ method, ModuleDefinition^ M, TypeDefinition^ type);
	void CustomClone(IList<CustomAttribute^>^ newarr, IList<CustomAttribute^>^ oldarr);
	TypeReference^ GetTypeReference(MethodDefinition^ method, ModuleDefinition^ M, TypeReference^ type);
	MethodReference^ GetMethodReference(MethodDefinition^ method, ModuleDefinition^ M, MethodReference^ m);
	IList<MethodDefinition^>^ GetMethodList(MethodDefinition^ method);
};