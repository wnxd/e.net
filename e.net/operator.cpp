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
	return this->Convert(type1, type2, false) != nullptr;
}

IList<Instruction^>^ Operator::Convert(TypeReference^ type1, TypeReference^ type2, bool conv)
{
	List<Instruction^>^ list = gcnew List<Instruction^>();
	if (type1 == type2) return list;
	else if (IsInherit(type1, type2, false))
	{
		if (conv)
		{
			if (type2 == this->_module->TypeSystem->Int16 || type2 == this->_module->TypeSystem->UInt16) list->Add(Instruction::Create(OpCodes::Conv_I2));
			else if (type2 == this->_module->TypeSystem->Int32 || type2 == this->_module->TypeSystem->UInt32) list->Add(Instruction::Create(OpCodes::Conv_I4));
			else if (type2 == this->_module->TypeSystem->Int64 || type2 == this->_module->TypeSystem->UInt64) list->Add(Instruction::Create(OpCodes::Conv_I8));
			else if (type2 == this->_module->TypeSystem->Single) list->Add(Instruction::Create(OpCodes::Conv_R4));
			else if (type2 == this->_module->TypeSystem->Double) list->Add(Instruction::Create(OpCodes::Conv_R8));
			else if (type2 == this->_module->TypeSystem->IntPtr || type2 == this->_module->TypeSystem->UIntPtr) list->Add(Instruction::Create(OpCodes::Conv_I));
		}
		return list;
	}
	else if (IsAssignableFrom(type2, type1)) return list;
	TypeOperator^ op = this->FindOperator(type1);
	for each (KeyValuePair<TypeReference^, MethodReference^>^ item in op->ConvertTo)
	{
		IList<Instruction^>^ l = this->Convert(item->Key, type2, true);
		if (l != nullptr)
		{
			list->Add(Instruction::Create(OpCodes::Call, this->_module->ImportReference(item->Value)));
			list->AddRange(l);
			return list;
		}
	}
	GenericInstanceType^ gt = dynamic_cast<GenericInstanceType^>(type1);
	if (gt != nullptr)
	{
		for each (KeyValuePair<int, MethodReference^>^ item in op->GenericConvertTo)
		{
			int index = item->Key;
			if (index >= 0 && index < gt->GenericArguments->Count)
			{
				IList<Instruction^>^ l = this->Convert(gt->GenericArguments[index], type2, true);
				if (l != nullptr)
				{
					MethodReference^ m = this->_module->ImportReference(item->Value);
					m->DeclaringType = this->_module->ImportReference(gt);
					list->Add(Instruction::Create(OpCodes::Call, this->_module->ImportReference(m)));
					list->AddRange(l);
					return list;
				}
			}
		}
	}
	op = this->FindOperator(type2);
	for each (KeyValuePair<TypeReference^, MethodReference^>^ item in op->Convert)
	{
		IList<Instruction^>^ l = this->Convert(type1, item->Key, conv);
		if (l != nullptr)
		{
			list->AddRange(l);
			list->Add(Instruction::Create(OpCodes::Call, this->_module->ImportReference(item->Value)));
			return list;
		}
	}
	gt = dynamic_cast<GenericInstanceType^>(type2);
	if (gt != nullptr)
	{
		for each (KeyValuePair<int, MethodReference^>^ item in op->GenericConvert)
		{
			int index = item->Key;
			if (index >= 0 && index < gt->GenericArguments->Count)
			{
				IList<Instruction^>^ l = this->Convert(type1, gt->GenericArguments[index], conv);
				if (l != nullptr)
				{
					list->AddRange(l);
					MethodReference^ m = this->_module->ImportReference(item->Value);
					m->DeclaringType = this->_module->ImportReference(gt);
					list->Add(Instruction::Create(OpCodes::Call, this->_module->ImportReference(m)));
					return list;
				}
			}
		}
	}
	return nullptr;
}

TypeReference^ Operator::GetConvertType(IList<Instruction^>^ list)
{
	if (list == nullptr || list->Count == 0) return nullptr;
	Instruction^ ins = list[0];
	if (ins->OpCode == OpCodes::Call)
	{
		MethodReference^ m = dynamic_cast<MethodReference^>(ins->Operand);
		return GenericHandle(dynamic_cast<GenericInstanceType^>(m->DeclaringType), m->Parameters[0]->ParameterType);
	}
	else if (ins->OpCode == OpCodes::Conv_I) return this->_module->TypeSystem->IntPtr;
	else if (ins->OpCode == OpCodes::Conv_I2) return this->_module->TypeSystem->Int16;
	else if (ins->OpCode == OpCodes::Conv_I4) return this->_module->TypeSystem->Int32;
	else if (ins->OpCode == OpCodes::Conv_I8) return this->_module->TypeSystem->Int64;
	else if (ins->OpCode == OpCodes::Conv_R4) return this->_module->TypeSystem->Single;
	else if (ins->OpCode == OpCodes::Conv_R8) return this->_module->TypeSystem->Double;
	return nullptr;
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