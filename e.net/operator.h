#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace Mono::Cecil;

ref struct TypeOperator
{
	TypeReference^ Type;
	IDictionary<TypeReference^, MethodReference^>^ ConvertTo;
	IDictionary<TypeReference^, MethodReference^>^ Convert;
	IDictionary<int, MethodReference^>^ GenericConvertTo;
	IDictionary<int, MethodReference^>^ GenericConvert;
};

ref class Operator
{
public:
	Operator(ModuleDefinition^ module);
	bool IsConvert(TypeReference^ srctype, TypeReference^ dsttype);
	IList<Instruction^>^ Convert(TypeReference^ srctype, TypeReference^ dsttype, bool conv);
	TypeReference^ GetConvertType(IList<Instruction^>^ list);
private:
	ModuleDefinition^ _module;
	IDictionary<String^, TypeOperator^>^ _map;
	TypeOperator^ FindOperator(TypeReference^ type);
};