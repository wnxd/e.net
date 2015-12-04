#include "stdafx.h"
#include "common.net.h"
#include "operator.h"

Operator::Operator(ModuleDefinition^ module)
{
	this->_module = module;
	this->_map = gcnew Dictionary<String^, TypeOperator^>();
}

bool Operator::IsConvert(TypeReference^ type1, TypeReference^ type2)
{
	return this->Convert(type1, type2) != nullptr;
}

IList<MethodReference^>^ Operator::Convert(TypeReference^ type1, TypeReference^ type2)
{
	List<MethodReference^>^ list = gcnew List<MethodReference^>();
	if (IsInherit(type1, type2)) return list;
	TypeOperator^ op = this->FindOperator(type1);
	for each (KeyValuePair<TypeReference^, MethodReference^>^ item in op->ConvertTo)
	{
		TypeReference^ t = item->Key;
		if (IsInherit(t, type2))
		{
			list->Add(item->Value);
			return list;
		}
		IList<MethodReference^>^ l = this->Convert(t, type2);
		if (l != nullptr)
		{
			list->Add(item->Value);
			list->AddRange(l);
			return list;
		}
	}
	GenericInstanceType^ gt = dynamic_cast<GenericInstanceType^>(type1);
	if (gt != nullptr)
	{
		for each (KeyValuePair<int, MethodReference^>^ item in op->GenericConvertTo)
		{
			if (item->Key >= 0 && item->Key < gt->GenericArguments->Count)
			{
				TypeReference^ t = gt->GenericArguments[item->Key];
				item->Value->DeclaringType = gt;
				if (IsInherit(t, type2))
				{

					list->Add(item->Value);
					return list;
				}
				IList<MethodReference^>^ l = this->Convert(t, type2);
				if (l != nullptr)
				{
					list->Add(item->Value);
					list->AddRange(l);
					return list;
				}
			}
		}
	}
	op = this->FindOperator(type2);
	for each (KeyValuePair<TypeReference^, MethodReference^>^ item in op->Convert)
	{
		TypeReference^ t = item->Key;
		if (IsInherit(type1, t))
		{
			list->Add(item->Value);
			return list;
		}
		IList<MethodReference^>^ l = this->Convert(type1, t);
		if (l != nullptr)
		{
			list->Add(item->Value);
			list->AddRange(l);
			return list;
		}
	}
	gt = dynamic_cast<GenericInstanceType^>(type2);
	if (gt != nullptr)
	{
		for each (KeyValuePair<int, MethodReference^>^ item in op->GenericConvert)
		{
			if (item->Key >= 0 && item->Key < gt->GenericArguments->Count)
			{
				TypeReference^ t = gt->GenericArguments[item->Key];
				//item->Value->DeclaringType =  type2;
				if (IsInherit(type1, t))
				{
					list->Add(item->Value);
					return list;
				}
				IList<MethodReference^>^ l = this->Convert(type1, t);
				if (l != nullptr)
				{
					list->Add(item->Value);
					list->AddRange(l);
					return list;
				}
			}
		}
	}
	return nullptr;
}

TypeReference^ Operator::GetConvertType(IList<MethodReference^>^ list)
{
	if (list == nullptr || list->Count == 0) return nullptr;
	return list[0]->Parameters[0]->ParameterType;
}

TypeOperator^ Operator::FindOperator(TypeReference^ type)
{
	TypeOperator^ op;
	if (!type->IsArray) type = type->GetElementType();
	if (this->_map->ContainsKey(type->FullName)) op = this->_map[type->FullName];
	else
	{
		op = gcnew TypeOperator();
		op->Type = type;
		op->ConvertTo = gcnew Dictionary<TypeReference^, MethodReference^>();
		op->Convert = gcnew Dictionary<TypeReference^, MethodReference^>();
		op->GenericConvertTo = gcnew Dictionary<int, MethodReference^>();
		op->GenericConvert = gcnew Dictionary<int, MethodReference^>();
		this->_map->Add(type->FullName, op);
		for each (MethodDefinition^ method in type->Resolve()->Methods)
		{
			if (method->IsStatic && method->Parameters->Count == 1 && (method->Name == "op_Explicit" || method->Name == "op_Implicit"))
			{
				TypeReference^ intype = method->Parameters[0]->ParameterType;
				TypeReference^ outtype = method->ReturnType;
				if (intype->Namespace == type->Namespace && intype->Name == type->Name)
				{
					if (outtype->IsGenericParameter)
					{
						GenericParameter^ gp = dynamic_cast<GenericParameter^>(outtype);
						int index = method->DeclaringType->GenericParameters->IndexOf(gp);
						if (index == -1) method->GenericParameters->IndexOf(gp);
						if (index != -1) op->GenericConvertTo->Add(index, method);
					}
					else op->ConvertTo->Add(outtype, method);
				}
				else if (outtype->Namespace == type->Namespace && outtype->Name == type->Name)
				{
					if (intype->IsGenericParameter)
					{
						GenericParameter^ gp = dynamic_cast<GenericParameter^>(intype);
						int index = method->DeclaringType->GenericParameters->IndexOf(gp);
						if (index == -1) method->GenericParameters->IndexOf(gp);
						if (index != -1) op->GenericConvert->Add(index, method);
					}
					else op->Convert->Add(intype, method);
				}
			}
		}
	}
	return op;
}