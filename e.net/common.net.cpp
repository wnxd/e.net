#include "stdafx.h"
#include "common.net.h"

using namespace std;

vector<string> split(string s, string delim)
{
	vector<string> elems;
	size_t pos = 0;
	size_t len = s.length();
	size_t delim_len = delim.length();
	if (delim_len == 0) return elems;
	while (pos < len)
	{
		int find_pos = s.find(delim, pos);
		if (find_pos < 0)
		{
			elems.push_back(s.substr(pos, len - pos));
			break;
		}
		elems.push_back(s.substr(pos, find_pos - pos));
		pos = find_pos + delim_len;
	}
	return elems;
}

generic<typename T> IList<T>^ ToList(...array<T>^ args)
{
	return gcnew List<T>(args);
}

bool operator==(TypeReference^ type1, TypeReference^ type2)
{
	bool b1 = Object::Equals(type1, nullptr);
	bool b2 = Object::Equals(type2, nullptr);
	if (b1 == b2)
	{
		if (b1) return true;
		return type1->FullName->Replace("/", ".") == type2->FullName->Replace("/", ".");
	}
	else return false;
}
bool operator!=(TypeReference^ type1, TypeReference^ type2)
{
	return !(type1 == type2);
}

TypeReference^ GetElementType(TypeReference^ type, bool workarr)
{
	if (type->IsArray)
	{
		if (workarr)
		{
			ArrayType^ t = dynamic_cast<ArrayType^>(type);
			if (t != nullptr) type = t->ElementType;
		}
	}
	else if (type->IsByReference)
	{
		ByReferenceType^ t = dynamic_cast<ByReferenceType^>(type);
		if (t != nullptr) type = t->ElementType;
	}
	return type;
}

bool IsAssignableFrom(TypeReference^ base, TypeReference^ type)
{
	if (Object::Equals(base, nullptr) || Object::Equals(type, nullptr)) return false;
	else if (base == type) return true;
	else
	{
		if (type->IsByReference)
		{
			type = dynamic_cast<ByReferenceType^>(type)->ElementType;
			if (base == type) return true;
		}
		TypeDefinition^ td = type->Resolve();
		for each (TypeReference^ t in td->Interfaces) if (IsAssignableFrom(base, t)) return true;
		if (type->IsArray) type = type->Module->ImportReference(typeof(Array));
		else type = td->BaseType;
		if (type == nullptr) return false;
		return IsAssignableFrom(base, type);
	}
}

Linked<TypeReference^>^ FindLinked(TypeReference^ type)
{
	if (Global::valuetype == nullptr)
	{
		ModuleDefinition^ module = type->Module;
		Global::valuetype = gcnew List<Linked<TypeReference^>^>();
		Linked<TypeReference^>^ _Byte = gcnew Linked<TypeReference^>();
		_Byte->Value = module->TypeSystem->Byte;
		Global::valuetype->Add(_Byte);
		Linked<TypeReference^>^ _SByte = gcnew Linked<TypeReference^>();
		_SByte->Value = module->TypeSystem->SByte;
		Global::valuetype->Add(_SByte);
		Linked<TypeReference^>^ _Int16 = gcnew Linked<TypeReference^>();
		_Int16->Value = module->TypeSystem->Int16;
		_Int16->Next = ToList(_Byte, _SByte);
		Global::valuetype->Add(_Int16);
		Linked<TypeReference^>^ _UInt16 = gcnew Linked<TypeReference^>();
		_UInt16->Value = module->TypeSystem->UInt16;
		_UInt16->Next = ToList(_Byte, _SByte);
		Global::valuetype->Add(_UInt16);
		Linked<TypeReference^>^ _Int32 = gcnew Linked<TypeReference^>();
		_Int32->Value = module->TypeSystem->Int32;
		_Int32->Next = ToList(_Int16, _UInt16);
		Global::valuetype->Add(_Int32);
		Linked<TypeReference^>^ _UInt32 = gcnew Linked<TypeReference^>();
		_UInt32->Value = module->TypeSystem->UInt32;
		_UInt32->Next = ToList(_Int16, _UInt16);
		Global::valuetype->Add(_UInt32);
		Linked<TypeReference^>^ _Int64 = gcnew Linked<TypeReference^>();
		_Int64->Value = module->TypeSystem->Int64;
		_Int64->Next = ToList(_Int32, _UInt32);
		Global::valuetype->Add(_Int64);
		Linked<TypeReference^>^ _UInt64 = gcnew Linked<TypeReference^>();
		_UInt64->Value = module->TypeSystem->UInt64;
		_UInt64->Next = ToList(_Int32, _UInt32);
		Global::valuetype->Add(_UInt64);
		Linked<TypeReference^>^ _Single = gcnew Linked<TypeReference^>();
		_Single->Value = module->TypeSystem->Single;
		_Single->Next = ToList(_Int16, _UInt16);
		Global::valuetype->Add(_Single);
		Linked<TypeReference^>^ _Double = gcnew Linked<TypeReference^>();
		_Double->Value = module->TypeSystem->Double;
		_Double->Next = ToList(_Single, _Int32, _UInt32);
		Global::valuetype->Add(_Double);
		Linked<TypeReference^>^ _Decimal = gcnew Linked<TypeReference^>();
		_Decimal->Value = module->ImportReference(typeof(Decimal));
		_Decimal->Next = ToList(_Double, _Int64, _UInt64);
		Global::valuetype->Add(_Decimal);
		Linked<TypeReference^>^ _IntPtr = gcnew Linked<TypeReference^>();
		_IntPtr->Value = module->TypeSystem->IntPtr;
		if (IntPtr::Size == 4) _IntPtr->Next = ToList(_Int32, _UInt32);
		else _IntPtr->Next = ToList(_Int64, _UInt64);
		Global::valuetype->Add(_IntPtr);
		Linked<TypeReference^>^ _UIntPtr = gcnew Linked<TypeReference^>();
		_UIntPtr->Value = module->TypeSystem->IntPtr;
		if (IntPtr::Size == 4) _UIntPtr->Next = ToList(_UInt32);
		else _UIntPtr->Next = ToList(_UInt64);
		Global::valuetype->Add(_UIntPtr);
	}
	for each (Linked<TypeReference^>^ link in Global::valuetype) if (link->Value == type) return link;
	return nullptr;
}

bool IsInherit(TypeReference^ srctype, TypeReference^ dsttype, bool assignable)
{
	if (srctype->IsValueType && dsttype->IsValueType)
	{
		if (srctype == dsttype) return true;
		Linked<TypeReference^>^ link = FindLinked(dsttype);
		if (link != nullptr)
		{
			if (link->Next == nullptr) return false;
			for each (Linked<TypeReference^>^ item in link->Next) if (IsInherit(srctype, item->Value, assignable)) return true;
			return false;
		}
	}
	return assignable ? IsAssignableFrom(dsttype, srctype) : false;
}

generic<typename T> void AddList(ICollection<T>^ list1, T item)
{
	if (item != nullptr && !list1->Contains(item)) list1->Add(item);
}

generic<typename T> void AddList(ICollection<T>^ list1, ICollection<T>^ list2)
{
	for each (T item in list2) AddList(list1, item);
}

generic<typename T> void DelList(ICollection<T>^ list1, ICollection<T>^ list2)
{
	for each (T item in list2) list1->Remove(item);
}

generic<typename T1, typename T2> T2 GetDictionary(IDictionary<T1, T2>^ dictionary, T1 key)
{
	if (dictionary->ContainsKey(key)) return dictionary[key];
	T2 T;
	return T;
}

generic<typename T1, typename T2> void AddDictionary(IDictionary<T1, IList<T2>^>^ dictionary, T1 key, T2 item)
{
	IList<T2>^ list;
	if (dictionary->ContainsKey(key)) list = dictionary[key];
	else
	{
		list = gcnew List<T2>();
		dictionary->Add(key, list);
	}
	AddList(list, item);
}

generic<typename T1, typename T2> void AddItem(IDictionary<T1, T2>^ dictionary, T1 key, T2 item)
{
	if (key != nullptr && item != nullptr && !dictionary->ContainsKey(key)) dictionary->Add(key, item);
}

MethodDefinition^ CreateMethod(String^ name, TypeReference^ returntype, IList<ParameterDefinition^>^ params, MethodAttributes attr)
{
	MethodDefinition^ method = gcnew MethodDefinition(name, attr, returntype);
	if (params != nullptr && params->Count > 0) for each (ParameterDefinition^ item in params) method->Parameters->Add(item);
	return method;
}

ParameterDefinition^ CreateParameter(String^ name, TypeReference^ type, ParameterAttributes attr)
{
	ParameterDefinition^ param = gcnew ParameterDefinition(name, attr, type);
	return param;
}

GenericInstanceType^ CreateGenericType(TypeReference^ type, IList<TypeReference^>^ generics)
{
	if (type == nullptr || generics == nullptr) return nullptr;
	GenericInstanceType^ t = dynamic_cast<GenericInstanceType^>(type);
	if (t == nullptr)
	{
		t = gcnew GenericInstanceType(type);
		for each (TypeReference^ item in generics)
		{
			t->GenericArguments->Add(item);
			t->GenericParameters->Add(gcnew GenericParameter(t));
		}
	}
	return t;
}

TypeReference^ CreateNullable(TypeReference^ type)
{
	if (Global::nullable == nullptr)
	{
		ModuleDefinition^ module = type->Module;
		Global::nullable = module->ImportReference(Type::GetType("System.Nullable`1"));
	}
	return CreateGenericType(Global::nullable, ToList(type));
}

MethodReference^ CreateMethodReference(TypeReference^ type, String^ name, TypeReference^ returntype, bool isstatic, IList<TypeReference^>^ params)
{
	MethodReference^ method = gcnew MethodReference(name, returntype, type);
	method->HasThis = !isstatic;
	for each (TypeReference^ item in params) method->Parameters->Add(gcnew ParameterDefinition(item));
	return method;
}

TypeReference^ GenericHandle(GenericInstanceType^ generictype, TypeReference^ type)
{
	if (generictype != nullptr && type != nullptr)
	{
		if (type->IsGenericParameter)
		{
			GenericParameter^ g = dynamic_cast<GenericParameter^>(type);
			int index = g->DeclaringType->GenericParameters->IndexOf(g);
			type = generictype->GenericArguments[index];
		}
		else if (type->IsGenericInstance)
		{
			GenericInstanceType^ g = dynamic_cast<GenericInstanceType^>(type);
			IList<TypeReference^>^ list = gcnew List<TypeReference^>();
			for (int i = 0; i < g->GenericArguments->Count; i++) list->Add(GenericHandle(generictype, g->GenericArguments[i]));
			type = CreateGenericType(g->GetElementType(), list);
		}
	}
	return type;
}