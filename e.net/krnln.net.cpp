#include "stdafx.h"
#include "common.h"
#include "common.net.h"
#include "krnln.net.h"

using namespace System::Text;

extern MethodDefinition^ CreateMethod(String^ name, TypeReference^ returntype, IList<ParameterDefinition^>^ params = nullptr, MethodAttributes attr = MethodAttributes::HideBySig);
extern ParameterDefinition^ CreateParameter(String^ name, TypeReference^ type, ParameterAttributes attr = ParameterAttributes::None);

MethodDefinition^ Krnln::CreateReturn(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("返回", module->TypeSystem->Void, ToList(CreateParameter("返回到调用方的值", module->TypeSystem->Void)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ret);
	return method;
}

MethodDefinition^ Krnln::CreateMod(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("求余数", module->TypeSystem->Int32, ToList(CreateParameter("被除数", module->TypeSystem->Double), CreateParameter("除数", module->TypeSystem->Double)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Rem);
	return method;
}

MethodDefinition^ Krnln::CreateAdd(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("相加", module->TypeSystem->String, ToList(CreateParameter("被加文本", module->TypeSystem->Object), CreateParameter("加文本", module->TypeSystem->Object)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(String, "Concat", typeof(Object), typeof(Object))));
	return method;
}

MethodDefinition^ Krnln::CreateEvenAdd(ModuleDefinition^ module)
{
	TypeReference^ objarr = gcnew ArrayType(module->TypeSystem->Object);
	ParameterDefinition^ params = CreateParameter("加文本", objarr);
	MethodReference^ ctor = module->ImportReference(GetCtor(ParamArrayAttribute));
	params->CustomAttributes->Add(gcnew CustomAttribute(ctor));
	MethodDefinition^ method = CreateMethod("相加", module->TypeSystem->String, ToList(CreateParameter("被加文本", module->TypeSystem->Object), params), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	method->Body->InitLocals = true;
	method->Body->Variables->Add(gcnew VariableDefinition(module->TypeSystem->Int32));
	method->Body->Variables->Add(gcnew VariableDefinition(objarr));
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldlen);
	AddILCode(ILProcessor, OpCodes::Dup);
	AddILCode(ILProcessor, OpCodes::Stloc_0);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Add);
	AddILCode(ILProcessor, OpCodes::Newarr, module->TypeSystem->Object);
	AddILCode(ILProcessor, OpCodes::Dup);
	AddILCode(ILProcessor, OpCodes::Stloc_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Stelem_Ref);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Ldloc_0);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Array, "Copy", typeof(Array), typeof(int), typeof(Array), typeof(int), typeof(int))));
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(String, "Concat", typeof(array<Object^>))));
	AddILCode(ILProcessor, OpCodes::Ret);
	return method;
}

MethodDefinition^ Krnln::CreateIntAdd(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("相加", module->TypeSystem->Int32, ToList(CreateParameter("被加数", module->TypeSystem->Int32), CreateParameter("加数", module->TypeSystem->Int32)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Add);
	return method;
}

MethodDefinition^ Krnln::CreateEvenIntAdd(ModuleDefinition^ module)
{
	ParameterDefinition^ params = CreateParameter("加数", gcnew ArrayType(module->TypeSystem->Int32));
	MethodReference^ ctor = module->ImportReference(GetCtor(ParamArrayAttribute));
	params->CustomAttributes->Add(gcnew CustomAttribute(ctor));
	MethodDefinition^ method = CreateMethod("相加", module->TypeSystem->Int32, ToList(CreateParameter("被加数", module->TypeSystem->Int32), params), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	method->Body->InitLocals = true;
	method->Body->Variables->Add(gcnew VariableDefinition(module->TypeSystem->Int32));
	method->Body->Variables->Add(gcnew VariableDefinition(module->TypeSystem->Int32));
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Stloc_0);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Stloc_1);
	Instruction^ ret = ILProcessor->Create(OpCodes::Ldloc_0);
	Instruction^ loop = ILProcessor->Create(OpCodes::Ldloc_1);
	ILProcessor->Append(loop);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldlen);
	AddILCode(ILProcessor, OpCodes::Bge_S, ret);
	AddILCode(ILProcessor, OpCodes::Ldloc_0);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Ldelem_I4);
	AddILCode(ILProcessor, OpCodes::Add);
	AddILCode(ILProcessor, OpCodes::Stloc_0);
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Add);
	AddILCode(ILProcessor, OpCodes::Stloc_1);
	AddILCode(ILProcessor, OpCodes::Br_S, loop);
	ILProcessor->Append(ret);
	AddILCode(ILProcessor, OpCodes::Ret);
	return method;
}

MethodDefinition^ Krnln::CreateLongAdd(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("相加", module->TypeSystem->Int64, ToList(CreateParameter("被加数", module->TypeSystem->Int64), CreateParameter("加数", module->TypeSystem->Int64)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Add);
	return method;
}

MethodDefinition^ Krnln::CreateEvenLongAdd(ModuleDefinition^ module)
{
	ParameterDefinition^ params = CreateParameter("加数", gcnew ArrayType(module->TypeSystem->Int64));
	MethodReference^ ctor = module->ImportReference(GetCtor(ParamArrayAttribute));
	params->CustomAttributes->Add(gcnew CustomAttribute(ctor));
	MethodDefinition^ method = CreateMethod("相加", module->TypeSystem->Int64, ToList(CreateParameter("被加数", module->TypeSystem->Int64), params), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	method->Body->InitLocals = true;
	method->Body->Variables->Add(gcnew VariableDefinition(module->TypeSystem->Int64));
	method->Body->Variables->Add(gcnew VariableDefinition(module->TypeSystem->Int32));
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Stloc_0);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Stloc_1);
	Instruction^ ret = ILProcessor->Create(OpCodes::Ldloc_0);
	Instruction^ loop = ILProcessor->Create(OpCodes::Ldloc_1);
	ILProcessor->Append(loop);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldlen);
	AddILCode(ILProcessor, OpCodes::Bge_S, ret);
	AddILCode(ILProcessor, OpCodes::Ldloc_0);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Ldelem_I8);
	AddILCode(ILProcessor, OpCodes::Add);
	AddILCode(ILProcessor, OpCodes::Stloc_0);
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Add);
	AddILCode(ILProcessor, OpCodes::Stloc_1);
	AddILCode(ILProcessor, OpCodes::Br_S, loop);
	ILProcessor->Append(ret);
	AddILCode(ILProcessor, OpCodes::Ret);
	return method;
}

MethodDefinition^ Krnln::CreateDoubleAdd(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("相加", module->TypeSystem->Double, ToList(CreateParameter("被加数", module->TypeSystem->Double), CreateParameter("加数", module->TypeSystem->Double)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Add);
	return method;
}

MethodDefinition^ Krnln::CreateEvenDoubleAdd(ModuleDefinition^ module)
{
	ParameterDefinition^ params = CreateParameter("加数", gcnew ArrayType(module->TypeSystem->Double));
	MethodReference^ ctor = module->ImportReference(GetCtor(ParamArrayAttribute));
	params->CustomAttributes->Add(gcnew CustomAttribute(ctor));
	MethodDefinition^ method = CreateMethod("相加", module->TypeSystem->Double, ToList(CreateParameter("被加数", module->TypeSystem->Double), params), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	method->Body->InitLocals = true;
	method->Body->Variables->Add(gcnew VariableDefinition(module->TypeSystem->Double));
	method->Body->Variables->Add(gcnew VariableDefinition(module->TypeSystem->Int32));
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Stloc_0);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Stloc_1);
	Instruction^ ret = ILProcessor->Create(OpCodes::Ldloc_0);
	Instruction^ loop = ILProcessor->Create(OpCodes::Ldloc_1);
	ILProcessor->Append(loop);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldlen);
	AddILCode(ILProcessor, OpCodes::Bge_S, ret);
	AddILCode(ILProcessor, OpCodes::Ldloc_0);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Ldelem_R8);
	AddILCode(ILProcessor, OpCodes::Add);
	AddILCode(ILProcessor, OpCodes::Stloc_0);
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Add);
	AddILCode(ILProcessor, OpCodes::Stloc_1);
	AddILCode(ILProcessor, OpCodes::Br_S, loop);
	ILProcessor->Append(ret);
	AddILCode(ILProcessor, OpCodes::Ret);
	return method;
}

//MethodDefinition^ Krnln::CreateEvenBinAdd(ModuleDefinition^ module)
//{
//	TypeReference^ Bin = gcnew ArrayType(module->TypeSystem->Byte);
//	MethodReference^ Copy = module->ImportReference(GetStaticMethod(Array, "Copy", typeof(Array), typeof(int), typeof(Array), typeof(int), typeof(int)));
//	ParameterDefinition^ params = CreateParameter("加字节集", gcnew ArrayType(Bin));
//	MethodReference^ ctor = module->ImportReference(GetCtor(ParamArrayAttribute));
//	params->CustomAttributes->Add(gcnew CustomAttribute(ctor));
//	MethodDefinition^ method = CreateMethod("相加", Bin, ToList(CreateParameter("被加字节集", Bin), params), STATICMETHOD);
//	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
//	method->Body->InitLocals = true;
//	method->Body->Variables->Add(gcnew VariableDefinition(module->TypeSystem->Int32));
//	method->Body->Variables->Add(gcnew VariableDefinition(module->TypeSystem->Int32));
//	method->Body->Variables->Add(gcnew VariableDefinition(module->TypeSystem->Int32));
//	method->Body->Variables->Add(gcnew VariableDefinition(Bin));
//	method->Body->Variables->Add(gcnew VariableDefinition(Bin));
//	AddILCode(ILProcessor, OpCodes::Ldarg_0);
//	AddILCode(ILProcessor, OpCodes::Ldlen);
//	AddILCode(ILProcessor, OpCodes::Stloc_0);
//	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
//	AddILCode(ILProcessor, OpCodes::Stloc_1);
//	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
//	AddILCode(ILProcessor, OpCodes::Stloc_2);
//	Instruction^ ret = ILProcessor->Create(OpCodes::Ldloc_0);
//	Instruction^ loop = ILProcessor->Create(OpCodes::Ldloc_2);
//	ILProcessor->Append(loop);
//	AddILCode(ILProcessor, OpCodes::Ldarg_1);
//	AddILCode(ILProcessor, OpCodes::Ldlen);
//	AddILCode(ILProcessor, OpCodes::Bge_S, ret);
//	AddILCode(ILProcessor, OpCodes::Ldloc_1);
//	AddILCode(ILProcessor, OpCodes::Ldarg_1);
//	AddILCode(ILProcessor, OpCodes::Ldloc_2);
//	AddILCode(ILProcessor, OpCodes::Ldelem_Ref);
//	AddILCode(ILProcessor, OpCodes::Ldlen);
//	AddILCode(ILProcessor, OpCodes::Add);
//	AddILCode(ILProcessor, OpCodes::Stloc_1);
//	AddILCode(ILProcessor, OpCodes::Ldloc_2);
//	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
//	AddILCode(ILProcessor, OpCodes::Add);
//	AddILCode(ILProcessor, OpCodes::Stloc_2);
//	AddILCode(ILProcessor, OpCodes::Br_S, loop);
//	ILProcessor->Append(ret);
//	AddILCode(ILProcessor, OpCodes::Ldloc_1);
//	AddILCode(ILProcessor, OpCodes::Add);
//	AddILCode(ILProcessor, OpCodes::Newarr, module->TypeSystem->Byte);
//	AddILCode(ILProcessor, OpCodes::Stloc_3);
//	AddILCode(ILProcessor, OpCodes::Ldarg_0);
//	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
//	AddILCode(ILProcessor, OpCodes::Ldloc_3);
//	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
//	AddILCode(ILProcessor, OpCodes::Ldloc_0);
//	AddILCode(ILProcessor, OpCodes::Call, Copy);
//	AddILCode(ILProcessor, OpCodes::Ldloc_0);
//	AddILCode(ILProcessor, OpCodes::Stloc_1);
//	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
//	AddILCode(ILProcessor, OpCodes::Stloc_2);
//	ret = ILProcessor->Create(OpCodes::Ldloc_3);
//	loop = ILProcessor->Create(OpCodes::Ldloc_2);
//	ILProcessor->Append(loop);
//	AddILCode(ILProcessor, OpCodes::Ldarg_1);
//	AddILCode(ILProcessor, OpCodes::Ldlen);
//	AddILCode(ILProcessor, OpCodes::Bge_S, ret);
//	AddILCode(ILProcessor, OpCodes::Ldarg_1);
//	AddILCode(ILProcessor, OpCodes::Ldloc_2);
//	AddILCode(ILProcessor, OpCodes::Ldelem_Ref);
//	AddILCode(ILProcessor, OpCodes::Dup);
//	AddILCode(ILProcessor, OpCodes::Stloc, 4);
//	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
//	AddILCode(ILProcessor, OpCodes::Ldloc_3);
//	AddILCode(ILProcessor, OpCodes::Ldloc_1);
//	AddILCode(ILProcessor, OpCodes::Ldloc, 4);
//	AddILCode(ILProcessor, OpCodes::Ldlen);
//	AddILCode(ILProcessor, OpCodes::Call, Copy);
//	AddILCode(ILProcessor, OpCodes::Ldloc_1);
//	AddILCode(ILProcessor, OpCodes::Ldloc, 4);
//	AddILCode(ILProcessor, OpCodes::Ldlen);
//	AddILCode(ILProcessor, OpCodes::Add);
//	AddILCode(ILProcessor, OpCodes::Stloc_1);
//	AddILCode(ILProcessor, OpCodes::Ldloc_2);
//	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
//	AddILCode(ILProcessor, OpCodes::Add);
//	AddILCode(ILProcessor, OpCodes::Stloc_2);
//	AddILCode(ILProcessor, OpCodes::Br_S, loop);
//	ILProcessor->Append(ret);
//	AddILCode(ILProcessor, OpCodes::Ret);
//	return method;
//}

MethodDefinition^ Krnln::CreateSub(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("相减", module->TypeSystem->Double, ToList(CreateParameter("被减数", module->TypeSystem->Double), CreateParameter("减数", module->TypeSystem->Double)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Sub);
	AddILCode(ILProcessor, OpCodes::Conv_R8);
	return method;
}

MethodDefinition^ Krnln::CreateIntSub(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("相减", module->TypeSystem->Int32, ToList(CreateParameter("被减数", module->TypeSystem->Int32), CreateParameter("减数", module->TypeSystem->Int32)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Sub);
	return method;
}

MethodDefinition^ Krnln::CreateNeg(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("负", module->TypeSystem->Double, ToList(CreateParameter("数值", module->TypeSystem->Double)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Neg);
	AddILCode(ILProcessor, OpCodes::Conv_R8);
	return method;
}

MethodDefinition^ Krnln::CreateMul(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("相乘", module->TypeSystem->Double, ToList(CreateParameter("被乘数", module->TypeSystem->Double), CreateParameter("乘数", module->TypeSystem->Double)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Mul);
	AddILCode(ILProcessor, OpCodes::Conv_R8);
	return method;
}

MethodDefinition^ Krnln::CreateDiv(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("相除", module->TypeSystem->Double, ToList(CreateParameter("被除数", module->TypeSystem->Double), CreateParameter("除数", module->TypeSystem->Double)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Div);
	AddILCode(ILProcessor, OpCodes::Conv_R8);
	return method;
}

MethodDefinition^ Krnln::CreateIDiv(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("整除", module->TypeSystem->Int32, ToList(CreateParameter("被除数", module->TypeSystem->Double), CreateParameter("除数", module->TypeSystem->Double)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Div);
	return method;
}

MethodDefinition^ Krnln::CreateEqual(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("等于", module->TypeSystem->Boolean, ToList(CreateParameter("被比较值", module->TypeSystem->Object), CreateParameter("比较值", module->TypeSystem->Object)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Object, "Equals", typeof(Object), typeof(Object))));
	return method;
}

MethodDefinition^ Krnln::CreateEqualNull1(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("等于", module->TypeSystem->Boolean, ToList(CreateParameter("被比较值", module->TypeSystem->Object), CreateParameter("比较值", module->TypeSystem->Void)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ceq);
	return method;
}

MethodDefinition^ Krnln::CreateEqualNull2(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("等于", module->TypeSystem->Boolean, ToList(CreateParameter("被比较值", module->TypeSystem->Void), CreateParameter("比较值", module->TypeSystem->Object)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ceq);
	return method;
}

MethodDefinition^ Krnln::CreateNotEqual(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("不等于", module->TypeSystem->Boolean, ToList(CreateParameter("被比较值", module->TypeSystem->Object), CreateParameter("比较值", module->TypeSystem->Object)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Object, "Equals", typeof(Object), typeof(Object))));
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ceq);
	return method;
}

MethodDefinition^ Krnln::CreateNotEqualNull1(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("不等于", module->TypeSystem->Boolean, ToList(CreateParameter("被比较值", module->TypeSystem->Object), CreateParameter("比较值", module->TypeSystem->Void)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ceq);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ceq);
	return method;
}

MethodDefinition^ Krnln::CreateNotEqualNull2(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("不等于", module->TypeSystem->Boolean, ToList(CreateParameter("被比较值", module->TypeSystem->Void), CreateParameter("比较值", module->TypeSystem->Object)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ceq);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ceq);
	return method;
}

MethodDefinition^ Krnln::CreateLess(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("小于", module->TypeSystem->Boolean, ToList(CreateParameter("被比较值", module->TypeSystem->Double), CreateParameter("比较值", module->TypeSystem->Double)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Clt);
	return method;
}

MethodDefinition^ Krnln::CreateMore(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("大于", module->TypeSystem->Boolean, ToList(CreateParameter("被比较值", module->TypeSystem->Double), CreateParameter("比较值", module->TypeSystem->Double)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Cgt);
	return method;
}

MethodDefinition^ Krnln::CreateLessOrEqual(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("小于或等于", module->TypeSystem->Boolean, ToList(CreateParameter("被比较值", module->TypeSystem->Double), CreateParameter("比较值", module->TypeSystem->Double)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Cgt);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ceq);
	return method;
}

MethodDefinition^ Krnln::CreateMoreOrEqual(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("大于或等于", module->TypeSystem->Boolean, ToList(CreateParameter("被比较值", module->TypeSystem->Double), CreateParameter("比较值", module->TypeSystem->Double)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Clt);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ceq);
	return method;
}

MethodDefinition^ Krnln::CreateAnd(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("并且", module->TypeSystem->Boolean, ToList(CreateParameter("逻辑值一", module->TypeSystem->Boolean), CreateParameter("逻辑值二", module->TypeSystem->Boolean)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	Instruction^ onefalse = ILProcessor->Create(OpCodes::Pop);
	Instruction^ nop = ILProcessor->Create(OpCodes::Nop);
	AddILCode(ILProcessor, OpCodes::Brfalse, onefalse);
	AddILCode(ILProcessor, OpCodes::Br, nop);
	ILProcessor->Append(onefalse);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	ILProcessor->Append(nop);
	return method;
}

MethodDefinition^ Krnln::CreateOr(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("或者", module->TypeSystem->Boolean, ToList(CreateParameter("逻辑值一", module->TypeSystem->Boolean), CreateParameter("逻辑值二", module->TypeSystem->Boolean)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	Instruction^ onetrue = ILProcessor->Create(OpCodes::Pop);
	Instruction^ nop = ILProcessor->Create(OpCodes::Nop);
	AddILCode(ILProcessor, OpCodes::Brtrue, onetrue);
	AddILCode(ILProcessor, OpCodes::Br, nop);
	ILProcessor->Append(onetrue);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	ILProcessor->Append(nop);
	return method;
}

MethodDefinition^ Krnln::CreateNot(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("取反", module->TypeSystem->Boolean, ToList(CreateParameter("被反转的逻辑值", module->TypeSystem->Boolean)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ceq);
	return method;
}

MethodDefinition^ Krnln::CreateBnot(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("位取反", module->TypeSystem->Int32, ToList(CreateParameter("欲取反的数值", module->TypeSystem->Int32)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Not);
	return method;
}

MethodDefinition^ Krnln::CreateBand(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("位与", module->TypeSystem->Int32, ToList(CreateParameter("位运算数值一", module->TypeSystem->Int32), CreateParameter("位运算数值二", module->TypeSystem->Int32)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::And);
	return method;
}

MethodDefinition^ Krnln::CreateBor(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("位或", module->TypeSystem->Int32, ToList(CreateParameter("位运算数值一", module->TypeSystem->Int32), CreateParameter("位运算数值二", module->TypeSystem->Int32)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Or);
	return method;
}

MethodDefinition^ Krnln::CreateBxor(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("位异或", module->TypeSystem->Int32, ToList(CreateParameter("位运算数值一", module->TypeSystem->Int32), CreateParameter("位运算数值二", module->TypeSystem->Int32)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Xor);
	return method;
}

MethodDefinition^ Krnln::CreateSet(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("赋值", module->TypeSystem->Void, ToList(CreateParameter("被赋值的变量或变量数组", gcnew ByReferenceType(module->TypeSystem->Object), ParameterAttributes::Out), CreateParameter("用作赋于的值或资源", module->TypeSystem->Object)), STATICMETHOD);
	return method;
}

MethodDefinition^ Krnln::CreateToDouble(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("到数值", module->TypeSystem->Double, ToList(CreateParameter("待转换的文本或数值", module->TypeSystem->Object)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldtoken, module->TypeSystem->Double);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod_D(Type, "GetTypeFromHandle")));
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Convert, "ChangeType", typeof(Object), typeof(Type))));
	AddILCode(ILProcessor, OpCodes::Dup);
	Instruction^ brfalse = ILProcessor->Create(OpCodes::Pop);
	AddILCode(ILProcessor, OpCodes::Brfalse_S, brfalse);
	Instruction^ ret = ILProcessor->Create(OpCodes::Ret);
	AddILCode(ILProcessor, OpCodes::Unbox_Any, module->TypeSystem->Double);
	AddILCode(ILProcessor, OpCodes::Br_S, ret);
	ILProcessor->Append(brfalse);
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(Object, "ToString")));
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(double, "Parse", typeof(String))));
	ILProcessor->Append(ret);
	return method;
}

MethodDefinition^ Krnln::CreateToStr(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("到文本", module->TypeSystem->String, ToList(CreateParameter("待转换的数据", module->TypeSystem->Object)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(Object, "ToString")));
	return method;
}

MethodDefinition^ Krnln::CreateToByte(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("到字节", module->TypeSystem->Byte, ToList(CreateParameter("待转换的文本或数值", module->TypeSystem->Object)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldtoken, module->TypeSystem->Byte);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod_D(Type, "GetTypeFromHandle")));
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Convert, "ChangeType", typeof(Object), typeof(Type))));
	AddILCode(ILProcessor, OpCodes::Dup);
	Instruction^ brfalse = ILProcessor->Create(OpCodes::Pop);
	AddILCode(ILProcessor, OpCodes::Brfalse_S, brfalse);
	Instruction^ ret = ILProcessor->Create(OpCodes::Ret);
	AddILCode(ILProcessor, OpCodes::Unbox_Any, module->TypeSystem->Byte);
	AddILCode(ILProcessor, OpCodes::Br_S, ret);
	ILProcessor->Append(brfalse);
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(Object, "ToString")));
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(byte, "Parse", typeof(String))));
	ILProcessor->Append(ret);
	return method;
}

MethodDefinition^ Krnln::CreateToShort(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("到短整数", module->TypeSystem->Int16, ToList(CreateParameter("待转换的文本或数值", module->TypeSystem->Object)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldtoken, module->TypeSystem->Int16);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod_D(Type, "GetTypeFromHandle")));
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Convert, "ChangeType", typeof(Object), typeof(Type))));
	AddILCode(ILProcessor, OpCodes::Dup);
	Instruction^ brfalse = ILProcessor->Create(OpCodes::Pop);
	AddILCode(ILProcessor, OpCodes::Brfalse_S, brfalse);
	Instruction^ ret = ILProcessor->Create(OpCodes::Ret);
	AddILCode(ILProcessor, OpCodes::Unbox_Any, module->TypeSystem->Int16);
	AddILCode(ILProcessor, OpCodes::Br_S, ret);
	ILProcessor->Append(brfalse);
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(Object, "ToString")));
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(short, "Parse", typeof(String))));
	ILProcessor->Append(ret);
	return method;
}

MethodDefinition^ Krnln::CreateToInt(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("到整数", module->TypeSystem->Int32, ToList(CreateParameter("待转换的文本或数值", module->TypeSystem->Object)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldtoken, module->TypeSystem->Int32);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod_D(Type, "GetTypeFromHandle")));
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Convert, "ChangeType", typeof(Object), typeof(Type))));
	AddILCode(ILProcessor, OpCodes::Dup);
	Instruction^ brfalse = ILProcessor->Create(OpCodes::Pop);
	AddILCode(ILProcessor, OpCodes::Brfalse_S, brfalse);
	Instruction^ ret = ILProcessor->Create(OpCodes::Ret);
	AddILCode(ILProcessor, OpCodes::Unbox_Any, module->TypeSystem->Int32);
	AddILCode(ILProcessor, OpCodes::Br_S, ret);
	ILProcessor->Append(brfalse);
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(Object, "ToString")));
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(int, "Parse", typeof(String))));
	ILProcessor->Append(ret);
	return method;
}

MethodDefinition^ Krnln::CreateToLong(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("到长整数", module->TypeSystem->Int64, ToList(CreateParameter("待转换的文本或数值", module->TypeSystem->Object)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldtoken, module->TypeSystem->Int64);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod_D(Type, "GetTypeFromHandle")));
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Convert, "ChangeType", typeof(Object), typeof(Type))));
	AddILCode(ILProcessor, OpCodes::Dup);
	Instruction^ brfalse = ILProcessor->Create(OpCodes::Pop);
	AddILCode(ILProcessor, OpCodes::Brfalse_S, brfalse);
	Instruction^ ret = ILProcessor->Create(OpCodes::Ret);
	AddILCode(ILProcessor, OpCodes::Unbox_Any, module->TypeSystem->Int64);
	AddILCode(ILProcessor, OpCodes::Br_S, ret);
	ILProcessor->Append(brfalse);
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(Object, "ToString")));
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Int64, "Parse", typeof(String))));
	ILProcessor->Append(ret);
	return method;
}

MethodDefinition^ Krnln::CreateToFloat(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("到小数", module->TypeSystem->Single, ToList(CreateParameter("待转换的文本或数值", module->TypeSystem->Object)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldtoken, module->TypeSystem->Single);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod_D(Type, "GetTypeFromHandle")));
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Convert, "ChangeType", typeof(Object), typeof(Type))));
	AddILCode(ILProcessor, OpCodes::Dup);
	Instruction^ brfalse = ILProcessor->Create(OpCodes::Pop);
	AddILCode(ILProcessor, OpCodes::Brfalse_S, brfalse);
	Instruction^ ret = ILProcessor->Create(OpCodes::Ret);
	AddILCode(ILProcessor, OpCodes::Unbox_Any, module->TypeSystem->Single);
	AddILCode(ILProcessor, OpCodes::Br_S, ret);
	ILProcessor->Append(brfalse);
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(Object, "ToString")));
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(float, "Parse", typeof(String))));
	ILProcessor->Append(ret);
	return method;
}

MethodDefinition^ Krnln::CreateShl(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("左移", module->TypeSystem->Int32, ToList(CreateParameter("欲移动的整数", module->TypeSystem->Int32), CreateParameter("欲移动的位数", module->TypeSystem->Int32)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Shl);
	return method;
}

MethodDefinition^ Krnln::CreateShr(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("右移", module->TypeSystem->Int32, ToList(CreateParameter("欲移动的整数", module->TypeSystem->Int32), CreateParameter("欲移动的位数", module->TypeSystem->Int32)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Shr);
	return method;
}

MethodDefinition^ Krnln::CreateIfe(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("如果", module->TypeSystem->Void, ToList(CreateParameter("条件", module->TypeSystem->Boolean)), STATICMETHOD);
	return method;
}

MethodDefinition^ Krnln::CreateIf(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("如果真", module->TypeSystem->Void, ToList(CreateParameter("条件", module->TypeSystem->Boolean)), STATICMETHOD);
	return method;
}

MethodDefinition^ Krnln::CreateSwitch(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("判断", module->TypeSystem->Void, ToList(CreateParameter("条件", module->TypeSystem->Boolean)), STATICMETHOD);
	return method;
}

MethodDefinition^ Krnln::CreateWhile(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("判断循环首", module->TypeSystem->Void, ToList(CreateParameter("条件", module->TypeSystem->Boolean)), STATICMETHOD);
	return method;
}

MethodDefinition^ Krnln::CreateWend(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("判断循环尾", module->TypeSystem->Void, nullptr, STATICMETHOD);
	return method;
}

MethodDefinition^ Krnln::CreateDoWhile(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("循环判断首", module->TypeSystem->Void, nullptr, STATICMETHOD);
	return method;
}

MethodDefinition^ Krnln::CreateLoop(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("循环判断尾", module->TypeSystem->Void, ToList(CreateParameter("条件", module->TypeSystem->Boolean)), STATICMETHOD);
	return method;
}

MethodDefinition^ Krnln::CreateCounter(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("计次循环首", module->TypeSystem->Void, ToList(CreateParameter("循环次数", module->TypeSystem->Int32), CreateParameter("已循环次数记录变量", gcnew ByReferenceType(module->TypeSystem->Int32), ParameterAttributes::Out)), STATICMETHOD);
	return method;
}

MethodDefinition^ Krnln::CreateCounterLoop(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("计次循环尾", module->TypeSystem->Void, nullptr, STATICMETHOD);
	return method;
}

MethodDefinition^ Krnln::CreateFor(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("变量循环首", module->TypeSystem->Void, ToList(CreateParameter("变量起始值", module->TypeSystem->Int32), CreateParameter("变量目标值", module->TypeSystem->Int32), CreateParameter("变量递增值", module->TypeSystem->Int32), CreateParameter("循环变量", gcnew ByReferenceType(module->TypeSystem->Int32), ParameterAttributes::Out | ParameterAttributes::Optional)), STATICMETHOD);
	return method;
}

MethodDefinition^ Krnln::CreateNext(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("变量循环尾", module->TypeSystem->Void, nullptr, STATICMETHOD);
	return method;
}

MethodDefinition^ Krnln::CreateEnd(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("结束", module->TypeSystem->Void, nullptr, STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Environment, "Exit", typeof(int))));
	return method;
}

MethodDefinition^ Krnln::CreateReDim(ModuleDefinition^ module)
{
	IList<ParameterDefinition^>^ params = ToList(CreateParameter("数组类型", module->ImportReference(typeof(RuntimeTypeHandle))), CreateParameter("欲重定义的数组变量", gcnew ByReferenceType(module->ImportReference(typeof(Array))), ParameterAttributes::Out), CreateParameter("是否保留以前的内容", module->TypeSystem->Boolean), CreateParameter("数组对应维的上限值", gcnew ArrayType(module->TypeSystem->Int32)));
	MethodReference^ ctor = module->ImportReference(GetCtor(ParamArrayAttribute));
	params[3]->CustomAttributes->Add(gcnew CustomAttribute(ctor));
	MethodDefinition^ method = CreateMethod("重定义数组", module->TypeSystem->Void, params, STATICMETHOD);
	method->Body->InitLocals = true;
	method->Body->Variables->Add(gcnew VariableDefinition(module->TypeSystem->Object));
	method->Body->Variables->Add(gcnew VariableDefinition(module->TypeSystem->Int32));
	MethodReference^ get_Length = module->ImportReference(GetInstanceMethod(Array, "get_Length"));
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldarg_2);
	Instruction^ ins = ILProcessor->Create(OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Brfalse_S, ins);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldind_Ref);
	AddILCode(ILProcessor, OpCodes::Ldnull);
	AddILCode(ILProcessor, OpCodes::Ceq);
	AddILCode(ILProcessor, OpCodes::Brfalse_S, ins);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Starg_S, params[2]);
	ILProcessor->Append(ins);
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod_D(Type, "GetTypeFromHandle")));
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Newarr, module->TypeSystem->Object);
	AddILCode(ILProcessor, OpCodes::Dup);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Stloc_1);
	ins = ILProcessor->Create(OpCodes::Ldarg_3);
	Instruction^ ins2 = ILProcessor->Create(OpCodes::Box, module->TypeSystem->Int32);
	ILProcessor->Append(ins);
	AddILCode(ILProcessor, OpCodes::Callvirt, get_Length);
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Ble_S, ins2);
	AddILCode(ILProcessor, OpCodes::Ldarg_3);
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Ldelem_I4);
	AddILCode(ILProcessor, OpCodes::Mul);
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Add);
	AddILCode(ILProcessor, OpCodes::Stloc_1);
	AddILCode(ILProcessor, OpCodes::Br_S, ins);
	ILProcessor->Append(ins2);
	AddILCode(ILProcessor, OpCodes::Stelem_Ref);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Activator, "CreateInstance", typeof(Type), typeof(array<Object^>))));
	AddILCode(ILProcessor, OpCodes::Ldarg_2);
	ins = ILProcessor->Create(OpCodes::Stind_Ref);
	AddILCode(ILProcessor, OpCodes::Brfalse_S, ins);
	AddILCode(ILProcessor, OpCodes::Stloc_0);
	AddILCode(ILProcessor, OpCodes::Ldind_Ref);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ldloc_0);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldind_Ref);
	AddILCode(ILProcessor, OpCodes::Callvirt, get_Length);
	AddILCode(ILProcessor, OpCodes::Dup);
	AddILCode(ILProcessor, OpCodes::Ldloc_0);
	AddILCode(ILProcessor, OpCodes::Callvirt, get_Length);
	ins2 = ILProcessor->Create(OpCodes::Call, module->ImportReference(GetStaticMethod(Array, "Copy", typeof(Array), typeof(int), typeof(Array), typeof(int), typeof(int))));
	AddILCode(ILProcessor, OpCodes::Blt_S, ins2);
	AddILCode(ILProcessor, OpCodes::Pop);
	AddILCode(ILProcessor, OpCodes::Ldloc_0);
	AddILCode(ILProcessor, OpCodes::Callvirt, get_Length);
	ILProcessor->Append(ins2);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldloc_0);
	ILProcessor->Append(ins);
	AddILCode(ILProcessor, OpCodes::Ret);
	return method;
}

MethodDefinition^ Krnln::CreateGetAryElementCount(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("取数组成员数", module->TypeSystem->Int32, ToList(CreateParameter("欲检查的数组变量", module->ImportReference(typeof(Array)))), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldnull);
	AddILCode(ILProcessor, OpCodes::Ceq);
	Instruction^ ins = ILProcessor->Create(OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Brfalse_S, ins);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_M1);
	AddILCode(ILProcessor, OpCodes::Ret);
	ILProcessor->Append(ins);
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(Array, "get_Length")));
	AddILCode(ILProcessor, OpCodes::Ret);
	return method;
}

MethodDefinition^ Krnln::CreateUBound(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("取数组下标", module->TypeSystem->Int32, ToList(CreateParameter("欲取某维最大下标的数组变量", module->ImportReference(typeof(Array))), CreateParameter("欲取其最大下标的维", module->TypeSystem->Int32, ParameterAttributes::Optional)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldnull);
	AddILCode(ILProcessor, OpCodes::Ceq);
	Instruction^ ins = ILProcessor->Create(OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Brfalse_S, ins);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ret);
	ILProcessor->Append(ins);
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(Array, "get_Length")));
	AddILCode(ILProcessor, OpCodes::Ret);
	return method;
}

MethodDefinition^ Krnln::CreateCopyAry(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("复制数组", module->TypeSystem->Void, ToList(CreateParameter("复制到的数组变量", gcnew ByReferenceType(module->ImportReference(typeof(Array))), ParameterAttributes::Out), CreateParameter("待复制的数组数据", module->ImportReference(typeof(Array)))), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldnull);
	Instruction^ ins = ILProcessor->Create(OpCodes::Ret);
	AddILCode(ILProcessor, OpCodes::Beq_S, ins);
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(Array, "Clone")));
	AddILCode(ILProcessor, OpCodes::Stind_Ref);
	ILProcessor->Append(ins);
	return method;
}

MethodDefinition^ Krnln::CreateAddElement(ModuleDefinition^ module)
{
	IList<ParameterDefinition^>^ params = ToList(CreateParameter("欲加入成员的数组变量", gcnew ByReferenceType(module->ImportReference(typeof(Array))), ParameterAttributes::Out), CreateParameter("欲加入的成员数据", module->TypeSystem->Object));
	MethodDefinition^ method = CreateMethod("加入成员", module->TypeSystem->Void, params, STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	method->Body->InitLocals = true;
	method->Body->Variables->Add(gcnew VariableDefinition(module->TypeSystem->Object));
	method->Body->Variables->Add(gcnew VariableDefinition(module->TypeSystem->Int32));
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldind_Ref);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetInstanceMethod(Object, "GetType")));
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Newarr, module->TypeSystem->Object);
	AddILCode(ILProcessor, OpCodes::Dup);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldind_Ref);
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(Array, "get_Length")));
	AddILCode(ILProcessor, OpCodes::Stloc_1);
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Add);
	AddILCode(ILProcessor, OpCodes::Box, module->TypeSystem->Int32);
	AddILCode(ILProcessor, OpCodes::Stelem_Ref);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Activator, "CreateInstance", typeof(Type), typeof(array<Object^>))));
	AddILCode(ILProcessor, OpCodes::Stloc_0);
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldind_Ref);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ldloc_0);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Array, "Copy", typeof(Array), typeof(int), typeof(Array), typeof(int), typeof(int))));
	AddILCode(ILProcessor, OpCodes::Ldloc_0);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(Array, "SetValue", typeof(Object), typeof(int))));
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldloc_0);
	AddILCode(ILProcessor, OpCodes::Stind_Ref);
	AddILCode(ILProcessor, OpCodes::Ret);
	return method;
}

MethodDefinition^ Krnln::CreateInsElement(ModuleDefinition^ module)
{
	IList<ParameterDefinition^>^ params = ToList(CreateParameter("欲插入成员的数组变量", gcnew ByReferenceType(module->ImportReference(typeof(Array))), ParameterAttributes::Out), CreateParameter("欲插入的位置", module->TypeSystem->Int32), CreateParameter("欲插入的成员数据", module->TypeSystem->Object));
	MethodDefinition^ method = CreateMethod("插入成员", module->TypeSystem->Void, params, STATICMETHOD);
	MethodReference^ Copy = module->ImportReference(GetStaticMethod(Array, "Copy", typeof(Array), typeof(int), typeof(Array), typeof(int), typeof(int)));
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	method->Body->InitLocals = true;
	method->Body->Variables->Add(gcnew VariableDefinition(module->TypeSystem->Object));
	method->Body->Variables->Add(gcnew VariableDefinition(module->TypeSystem->Int32));
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	Instruction^ ins = ILProcessor->Create(OpCodes::Ret);
	AddILCode(ILProcessor, OpCodes::Blt_S, ins);
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldind_Ref);
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(Array, "get_Length")));
	AddILCode(ILProcessor, OpCodes::Stloc_1);
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Add);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Blt_S, ins);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Sub);
	AddILCode(ILProcessor, OpCodes::Starg_S, params[1]);
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldind_Ref);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetInstanceMethod(Object, "GetType")));
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Newarr, module->TypeSystem->Object);
	AddILCode(ILProcessor, OpCodes::Dup);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Add);
	AddILCode(ILProcessor, OpCodes::Box, module->TypeSystem->Int32);
	AddILCode(ILProcessor, OpCodes::Stelem_Ref);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Activator, "CreateInstance", typeof(Type), typeof(array<Object^>))));
	AddILCode(ILProcessor, OpCodes::Stloc_0);
	Instruction^ ins2 = ILProcessor->Create(OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Brfalse_S, ins2);
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldind_Ref);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ldloc_0);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Call, Copy);
	ILProcessor->Append(ins2);
	AddILCode(ILProcessor, OpCodes::Ldind_Ref);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldloc_0);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Add);
	AddILCode(ILProcessor, OpCodes::Ldloc_1);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Sub);
	AddILCode(ILProcessor, OpCodes::Call, Copy);
	AddILCode(ILProcessor, OpCodes::Ldloc_0);
	AddILCode(ILProcessor, OpCodes::Ldarg_2);
	AddILCode(ILProcessor, OpCodes::Ldarg_1);
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(Array, "SetValue", typeof(Object), typeof(int))));
	AddILCode(ILProcessor, OpCodes::Ldarg_0);
	AddILCode(ILProcessor, OpCodes::Ldloc_0);
	AddILCode(ILProcessor, OpCodes::Stind_Ref);
	ILProcessor->Append(ins);
	return method;
}

MethodDefinition^ Krnln::CreateChr(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("字符", module->TypeSystem->String, ToList(CreateParameter("欲取其字符的字符代码", module->TypeSystem->Byte)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(Char, "ToString")));
	return method;
}

MethodDefinition^ Krnln::CreateUCase(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("到大写", module->TypeSystem->String, ToList(CreateParameter("欲变换的文本", module->TypeSystem->String)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(String, "ToUpper")));
	return method;
}

MethodDefinition^ Krnln::CreateLCase(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("到小写", module->TypeSystem->String, ToList(CreateParameter("欲变换的文本", module->TypeSystem->String)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(String, "ToLower")));
	return method;
}

MethodDefinition^ Krnln::CreateLTrim(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("删首空", module->TypeSystem->String, ToList(CreateParameter("欲删除空格的文本", module->TypeSystem->String)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldc_I4_2);
	AddILCode(ILProcessor, OpCodes::Newarr, module->TypeSystem->Char);
	AddILCode(ILProcessor, OpCodes::Dup);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ldc_I4, 32);
	AddILCode(ILProcessor, OpCodes::Stelem_I2);
	AddILCode(ILProcessor, OpCodes::Dup);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4, 12288);
	AddILCode(ILProcessor, OpCodes::Stelem_I2);
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(String, "TrimStart", typeof(array<Char>))));
	return method;
}

MethodDefinition^ Krnln::CreateRTrim(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("删尾空", module->TypeSystem->String, ToList(CreateParameter("欲删除空格的文本", module->TypeSystem->String)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldc_I4_2);
	AddILCode(ILProcessor, OpCodes::Newarr, module->TypeSystem->Char);
	AddILCode(ILProcessor, OpCodes::Dup);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ldc_I4, 32);
	AddILCode(ILProcessor, OpCodes::Stelem_I2);
	AddILCode(ILProcessor, OpCodes::Dup);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4, 12288);
	AddILCode(ILProcessor, OpCodes::Stelem_I2);
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(String, "TrimEnd", typeof(array<Char>))));
	return method;
}

MethodDefinition^ Krnln::CreateTrim(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("删首尾空", module->TypeSystem->String, ToList(CreateParameter("欲删除空格的文本", module->TypeSystem->String)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldc_I4_2);
	AddILCode(ILProcessor, OpCodes::Newarr, module->TypeSystem->Char);
	AddILCode(ILProcessor, OpCodes::Dup);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ldc_I4, 32);
	AddILCode(ILProcessor, OpCodes::Stelem_I2);
	AddILCode(ILProcessor, OpCodes::Dup);
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Ldc_I4, 12288);
	AddILCode(ILProcessor, OpCodes::Stelem_I2);
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(String, "Trim", typeof(array<Char>))));
	return method;
}

MethodDefinition^ Krnln::CreateTrimAll(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("删全部空", module->TypeSystem->String, ToList(CreateParameter("欲删除空格的文本", module->TypeSystem->String)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	MethodReference^ Replace = module->ImportReference(GetInstanceMethod(String, "Replace", typeof(String), typeof(String)));
	AddILCode(ILProcessor, OpCodes::Ldstr, " ");
	AddILCode(ILProcessor, OpCodes::Ldnull);
	AddILCode(ILProcessor, OpCodes::Callvirt, Replace);
	AddILCode(ILProcessor, OpCodes::Ldstr, ((Char)12288).ToString());
	AddILCode(ILProcessor, OpCodes::Ldnull);
	AddILCode(ILProcessor, OpCodes::Callvirt, Replace);
	return method;
}

MethodDefinition^ Krnln::CreateStrComp(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("文本比较", module->TypeSystem->Int32, ToList(CreateParameter("待比较文本一", module->TypeSystem->String), CreateParameter("待比较文本二", module->TypeSystem->String), CreateParameter("是否区分大小写", module->TypeSystem->Boolean)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldc_I4_0);
	AddILCode(ILProcessor, OpCodes::Ceq);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(String, "Compare", typeof(String), typeof(String), typeof(bool))));
	return method;
}

MethodDefinition^ Krnln::CreateToTime(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("到时间", module->ImportReference(typeof(DateTime)), ToList(CreateParameter("欲转换的文本", module->TypeSystem->String)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(DateTime, "Parse", typeof(String))));
	return method;
}

MethodDefinition^ Krnln::CreateGetDaysOfSpecMonth(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("取某月天数", module->TypeSystem->Int32, ToList(CreateParameter("年份", module->TypeSystem->Int32), CreateParameter("月份", module->TypeSystem->Int32)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(DateTime, "DaysInMonth", typeof(int), typeof(int))));
	return method;
}

MethodDefinition^ Krnln::CreateYear(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("取年份", module->TypeSystem->Int32, ToList(CreateParameter("时间", module->ImportReference(typeof(DateTime)))), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(DateTime, "get_Year")));
	return method;
}

MethodDefinition^ Krnln::CreateMonth(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("取月份", module->TypeSystem->Int32, ToList(CreateParameter("时间", module->ImportReference(typeof(DateTime)))), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(DateTime, "get_Month")));
	return method;
}

MethodDefinition^ Krnln::CreateDay(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("取日", module->TypeSystem->Int32, ToList(CreateParameter("时间", module->ImportReference(typeof(DateTime)))), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(DateTime, "get_Day")));
	return method;
}

MethodDefinition^ Krnln::CreateWeekDay(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("取星期几", module->TypeSystem->Int32, ToList(CreateParameter("时间", module->ImportReference(typeof(DateTime)))), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(DateTime, "get_DayOfWeek")));
	AddILCode(ILProcessor, OpCodes::Ldc_I4_1);
	AddILCode(ILProcessor, OpCodes::Add);
	return method;
}

MethodDefinition^ Krnln::CreateHour(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("取小时", module->TypeSystem->Int32, ToList(CreateParameter("时间", module->ImportReference(typeof(DateTime)))), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(DateTime, "get_Hour")));
	return method;
}

MethodDefinition^ Krnln::CreateMinute(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("取分钟", module->TypeSystem->Int32, ToList(CreateParameter("时间", module->ImportReference(typeof(DateTime)))), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(DateTime, "get_Minute")));
	return method;
}

MethodDefinition^ Krnln::CreateSecond(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("取秒", module->TypeSystem->Int32, ToList(CreateParameter("时间", module->ImportReference(typeof(DateTime)))), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(DateTime, "get_Second")));
	return method;
}

MethodDefinition^ Krnln::CreateGetSpecTime(ModuleDefinition^ module)
{
	IList<ParameterDefinition^>^ params = ToList(CreateParameter("年", module->TypeSystem->Int32), CreateParameter("月", module->TypeSystem->Int32), CreateParameter("日", module->TypeSystem->Int32), CreateParameter("小时", module->TypeSystem->Int32), CreateParameter("分钟", module->TypeSystem->Int32), CreateParameter("秒", module->TypeSystem->Int32));
	params[1]->IsOptional = true;
	params[1]->Constant = 1;
	params[2]->IsOptional = true;
	params[2]->Constant = 1;
	params[3]->IsOptional = true;
	params[3]->Constant = 0;
	params[4]->IsOptional = true;
	params[4]->Constant = 0;
	params[5]->IsOptional = true;
	params[5]->Constant = 0;
	MethodDefinition^ method = CreateMethod("指定时间", module->ImportReference(typeof(DateTime)), params, STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Newobj, module->ImportReference(GetCtor(DateTime, typeof(int), typeof(int), typeof(int), typeof(int), typeof(int), typeof(int))));
	return method;
}

MethodDefinition^ Krnln::CreateNow(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("取现行时间", module->ImportReference(typeof(DateTime)), nullptr, STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(DateTime, "get_Now")));
	return method;
}

MethodDefinition^ Krnln::CreateGetDatePart(ModuleDefinition^ module)
{
	TypeReference^ dt = module->ImportReference(typeof(DateTime));
	MethodDefinition^ method = CreateMethod("取日期", dt, ToList(CreateParameter("时间", dt)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(DateTime, "get_Date")));
	return method;
}

MethodDefinition^ Krnln::CreateGetTimePart(ModuleDefinition^ module)
{
	TypeReference^ dt = module->ImportReference(typeof(DateTime));
	MethodDefinition^ method = CreateMethod("取时间", dt, ToList(CreateParameter("时间", dt)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Callvirt, module->ImportReference(GetInstanceMethod(DateTime, "ToLongTimeString")));
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(DateTime, "Parse", typeof(String))));
	return method;
}

MethodDefinition^ Krnln::CreateGetHexText(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("取十六进制文本", module->TypeSystem->String, ToList(CreateParameter("欲取进制文本的数值", module->TypeSystem->Int32)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldc_I4, 16);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Convert, "ToString", typeof(int), typeof(int))));
	return method;
}

MethodDefinition^ Krnln::CreateGetOctText(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("取八进制文本", module->TypeSystem->String, ToList(CreateParameter("欲取进制文本的数值", module->TypeSystem->Int32)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldc_I4, 8);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Convert, "ToString", typeof(int), typeof(int))));
	return method;
}

MethodDefinition^ Krnln::CreateHex(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("十六进制", module->TypeSystem->Int32, ToList(CreateParameter("十六进制文本常量", module->TypeSystem->String)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldc_I4, 16);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Convert, "ToInt32", typeof(String), typeof(int))));
	return method;
}

MethodDefinition^ Krnln::CreateBinary(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("二进制", module->TypeSystem->Int32, ToList(CreateParameter("二进制文本常量", module->TypeSystem->String)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Ldc_I4, 2);
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Convert, "ToInt32", typeof(String), typeof(int))));
	return method;
}

MethodDefinition^ CreateSpaceBin(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("取空白字节集", gcnew ArrayType(module->TypeSystem->Byte), ToList(CreateParameter("零字节数目", module->TypeSystem->Int32)), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Newarr, module->TypeSystem->Byte);
	return method;
}

array<byte>^ Krnln::相加(array<byte>^ 被加字节集, ...array<array<byte>^>^ 加字节集)
{
	int num;
	if (被加字节集 == nullptr) num = 0;
	else num = 被加字节集->Length;
	int num2 = 0;
	for (int i = 0; i < 加字节集->Length; i++) num2 += 加字节集[i]->Length;
	array<byte>^ arr = gcnew array<byte>(num + num2);
	if (被加字节集 != nullptr) Array::Copy(被加字节集, 0, arr, 0, num);
	num2 = num;
	for (int i = 0; i < 加字节集->Length; i++)
	{
		array<byte>^ arr2 = 加字节集[i];
		Array::Copy(arr2, 0, arr, num2, arr2->Length);
		num2 += arr2->Length;
	}
	return arr;
}

int Krnln::删除成员(Array^% 欲删除成员的数组变量, int 欲删除的位置, int 欲删除的成员数目)
{
	if (欲删除成员的数组变量 == nullptr) return 0;
	int len = 欲删除成员的数组变量->Length;
	if (欲删除的位置 < 1 || 欲删除的位置> len + 1) return 0;
	欲删除的位置--;
	System::Type^ type = 欲删除成员的数组变量->GetType();
	len -= 欲删除的成员数目 + 欲删除的位置;
	if (len <= 0)
	{
		欲删除的成员数目 += len;
		if (欲删除的位置 == 0)
		{
			欲删除成员的数组变量 = (Array^)Activator::CreateInstance(type, (Object^)0);
			return 欲删除的成员数目;
		}
	}
	Array^ arr = (Array^)Activator::CreateInstance(type, (Object^)(欲删除成员的数组变量->Length - 欲删除的成员数目));
	if (欲删除的位置 > 0) Array::Copy(欲删除成员的数组变量, 0, arr, 0, 欲删除的位置);
	if (len > 0) Array::Copy(欲删除成员的数组变量, 欲删除的位置 + 欲删除的成员数目, arr, 欲删除的位置, len);
	欲删除成员的数组变量 = arr;
	return 欲删除的成员数目;
}

void Krnln::清除数组(Array^% 欲删除成员的数组变量)
{
	if (欲删除成员的数组变量 != nullptr) 欲删除成员的数组变量 = (Array^)Activator::CreateInstance(欲删除成员的数组变量->GetType(), (Object^)0);
}

void Krnln::数组排序(Array^% 数值数组变量, bool 排序方向是否为从小到大)
{
	if (数值数组变量 != nullptr)
	{
		Array::Sort(数值数组变量);
		if (!排序方向是否为从小到大) Array::Reverse(数值数组变量);
	}
}

void Krnln::数组清零(Array^% 数值数组变量)
{
	if (数值数组变量 != nullptr)
	{
		System::Type^ type = 数值数组变量->GetType();
		Object^ null;
		if (type->GetElementType()->IsValueType) null = 0;
		for (int i = 0; i < 数值数组变量->Length; i++) 数值数组变量->SetValue(null, i);
	}
}

int Krnln::取文本长度(String^ 文本数据)
{
	if (文本数据 == nullptr) return 0;
	return 文本数据->Length;
}

String^ Krnln::取文本左边(String^ 欲取其部分的文本, int 欲取出字符的数目)
{
	if (欲取其部分的文本 == nullptr) return nullptr;
	return 欲取其部分的文本->Substring(0, 欲取出字符的数目);
}

String^ Krnln::取文本右边(String^ 欲取其部分的文本, int 欲取出字符的数目)
{
	if (欲取其部分的文本 == nullptr) return nullptr;
	return 欲取其部分的文本->Substring(欲取其部分的文本->Length - 欲取出字符的数目);
}

String^ Krnln::取文本中间(String^ 欲取其部分的文本, int 起始取出位置, int 欲取出字符的数目)
{
	if (欲取其部分的文本 == nullptr) return nullptr;
	return 欲取其部分的文本->Substring(起始取出位置 - 1, 欲取出字符的数目);
}

int Krnln::取代码(String^ 欲取字符代码的文本, [Optional][DefaultValue(1)]int 欲取其代码的字符位置)
{
	if (欲取字符代码的文本 == nullptr) return 0;
	return (int)欲取字符代码的文本[欲取其代码的字符位置];
}

int Krnln::寻找文本(String^ 被搜寻的文本, String^ 欲寻找的文本, int 起始搜寻位置, bool 是否不区分大小写)
{
	if (被搜寻的文本 == nullptr) return -1;
	int index = 被搜寻的文本->IndexOf(欲寻找的文本, 起始搜寻位置 - 1, 是否不区分大小写 ? StringComparison::CurrentCultureIgnoreCase : StringComparison::CurrentCulture);
	if (index != -1) index++;
	return index;
}

int Krnln::倒找文本(String^ 被搜寻的文本, String^ 欲寻找的文本, int 起始搜寻位置, bool 是否不区分大小写)
{
	if (被搜寻的文本 == nullptr) return -1;
	int index = 被搜寻的文本->LastIndexOf(欲寻找的文本, 起始搜寻位置 - 1, 是否不区分大小写 ? StringComparison::CurrentCultureIgnoreCase : StringComparison::CurrentCulture);
	if (index != -1) index++;
	return index;
}

String^ Krnln::到全角(String^ 欲变换的文本)
{
	array<Char>^ c = 欲变换的文本->ToCharArray();
	for (int i = 0; i < c->Length; i++)
	{
		if (c[i] == 32) c[i] = (Char)12288;
		else if (c[i] < 127) c[i] = (Char)(c[i] + 65248);
	}
	return gcnew String(c);
}

String^ Krnln::到半角(String^ 欲变换的文本)
{
	array<Char>^ c = 欲变换的文本->ToCharArray();
	for (int i = 0; i < c->Length; i++)
	{
		if (c[i] == 12288) c[i] = (Char)32;
		else if (c[i] > 65280 && c[i] < 65375) c[i] = (Char)(c[i] - 65248);
	}
	return gcnew String(c);
}

String^ Krnln::文本替换(String^ 欲被替换的文本, int 起始替换位置, int 替换长度, String^ 用作替换的文本)
{
	if (欲被替换的文本 != nullptr)
	{
		int len = 欲被替换的文本->Length;
		起始替换位置--;
		if (起始替换位置 >= 0 && 起始替换位置 < len)
		{
			len -= 替换长度 + 起始替换位置;
			if (len <= 0)
			{
				替换长度 += len;
				if (起始替换位置 == 0) return 用作替换的文本;
			}
			String^ str;
			if (起始替换位置 > 0) str += 欲被替换的文本->Substring(0, 起始替换位置);
			str += 用作替换的文本;
			if (len > 0) str += 欲被替换的文本->Substring(起始替换位置 + 替换长度, len);
			return str;
		}
	}
	return 欲被替换的文本;
}

String^ Krnln::子文本替换(String^ 欲被替换的文本, String^ 欲被替换的子文本, String^ 用作替换的子文本, int 进行替换的起始位置, int 替换进行的次数, bool 是否区分大小写)
{
	if (欲被替换的文本 != nullptr && !String::IsNullOrEmpty(欲被替换的子文本))
	{
		进行替换的起始位置--;
		if (进行替换的起始位置 >= 0 && 进行替换的起始位置 < 欲被替换的文本->Length)
		{
			int len = 欲被替换的子文本->Length;
			StringComparison sc = 是否区分大小写 ? StringComparison::CurrentCulture : StringComparison::CurrentCultureIgnoreCase;
			do
			{
				进行替换的起始位置 = 欲被替换的文本->IndexOf(欲被替换的子文本, 进行替换的起始位置, sc);
				if (进行替换的起始位置 == -1) break;
				欲被替换的文本 = Krnln::文本替换(欲被替换的文本, 进行替换的起始位置 + 1, len, 用作替换的子文本);
				替换进行的次数--;
			} while (替换进行的次数 > 0);
		}
	}
	return 欲被替换的文本;
}

String^ Krnln::取空白文本(int 重复次数)
{
	String^ str = "";
	return str->PadLeft(重复次数);
}

String^ Krnln::取重复文本(int 重复次数, String^ 待重复文本)
{
	String^ str;
	while (重复次数 > 0)
	{
		str += 待重复文本;
		重复次数--;
	}
	return str;
}

array<String^>^ Krnln::分割文本(String^ 待分割文本, String^ 用作分割的文本, int 要返回的子文本数目)
{
	if (待分割文本 == nullptr) return nullptr;
	array<String^>^ arr = 待分割文本->Split(用作分割的文本->ToCharArray());
	if (要返回的子文本数目 > 0)  Array::Resize(arr, 要返回的子文本数目);
	return arr;
}

DateTime Krnln::增减时间(DateTime 时间, int 被增加部分, int 增加值)
{
	switch (被增加部分)
	{
	case 1:
		时间 = 时间.AddYears(增加值);
		break;
	case 2:
		时间 = 时间.AddMonths(增加值 * 3);
		break;
	case 3:
		时间 = 时间.AddMonths(增加值);
		break;
	case 4:
		时间 = 时间.AddDays(增加值 * 7);
		break;
	case 5:
		时间 = 时间.AddDays(增加值);
		break;
	case 6:
		时间 = 时间.AddHours(增加值);
		break;
	case 7:
		时间 = 时间.AddMinutes(增加值);
		break;
	case 8:
		时间 = 时间.AddSeconds(增加值);
		break;
	}
	return 时间;
}

double Krnln::取时间间隔(DateTime 时间1, DateTime 时间2, int 取间隔部分)
{
	TimeSpan time = 时间1 - 时间2;
	double span;
	switch (取间隔部分)
	{
	case 1:
		span = time.TotalDays / 365;
		break;
	case 2:
		span = time.TotalDays / 91.25;
		break;
	case 3:
		span = time.TotalDays / 30.4166;
		break;
	case 4:
		span = time.TotalDays / 7;
		break;
	case 5:
		span = time.TotalDays;
		break;
	case 6:
		span = time.TotalHours;
		break;
	case 7:
		span = time.TotalMinutes;
		break;
	case 8:
		span = time.TotalSeconds;
		break;
	}
	return span;
}

String^ Krnln::时间到文本(DateTime 欲转换到文本的时间, int 转换部分)
{
	String^ str;
	switch (转换部分)
	{
	case 1:
		str = 欲转换到文本的时间.ToString();
		break;
	case 2:
		str = 欲转换到文本的时间.ToLongDateString();
		break;
	case 3:
		str = 欲转换到文本的时间.ToLongTimeString();
		break;
	}
	return str;
}

int Krnln::取时间部分(DateTime 欲取其部分的时间, int 转换部分)
{
	int num;
	switch (转换部分)
	{
	case 1:
		num = 欲取其部分的时间.Year;
		break;
	case 2:
		num = Math::Ceiling(欲取其部分的时间.Month / 3);
		break;
	case 3:
		num = 欲取其部分的时间.Month;
		break;
	case 4:
		num = Math::Ceiling(欲取其部分的时间.DayOfYear / 7);
		break;
	case 5:
		num = 欲取其部分的时间.Day;
		break;
	case 6:
		num = 欲取其部分的时间.Hour;
		break;
	case 7:
		num = 欲取其部分的时间.Minute;
		break;
	case 8:
		num = 欲取其部分的时间.Second;
		break;
	case 9:
		num = (int)欲取其部分的时间.DayOfWeek + 1;
		break;
	case 10:
		num = 欲取其部分的时间.DayOfYear;
		break;
	}
	return num;
}

[DllImport("Kernel32.dll")]
extern bool SetLocalTime(SYSTEMTIME% sysTime);

bool Krnln::置现行时间(DateTime 欲设置的时间)
{
	SYSTEMTIME st;
	st.wYear = 欲设置的时间.Year;
	st.wMonth = 欲设置的时间.Month;
	st.wDay = 欲设置的时间.Day;
	st.wDayOfWeek = (WORD)欲设置的时间.DayOfWeek;
	st.wHour = 欲设置的时间.Hour;
	st.wMinute = 欲设置的时间.Minute;
	st.wSecond = 欲设置的时间.Second;
	st.wMilliseconds = 欲设置的时间.Millisecond;
	return SetLocalTime(st);
}

String^ getunit(int i, bool jt)
{

	int a = i / 9;
	int b = i % 9;
	int c = b / 5;
	int d = b % 5;
	String^ str = "";
	if (a != 0 || c != 0) d++;
	switch (d)
	{
	case 1:
		str = str->PadRight(c, (Char)19975);
		str = str->PadRight(a, (Char)20159);
		break;
	case 2:
		str = jt ? "十" : "拾";
		break;
	case 3:
		str = jt ? "百" : "佰";
		break;
	case 4:
		str = jt ? "千" : "仟";
		break;
	}
	return str;
}

String^ getint(Char c, bool jt)
{
	String^ str;
	switch (c)
	{
	case '0':
		str = "零";
		break;
	case '1':
		str = jt ? "一" : "壹";
		break;
	case '2':
		str = jt ? "二" : "贰";
		break;
	case '3':
		str = jt ? "三" : "叁";
		break;
	case '4':
		str = jt ? "四" : "肆";
		break;
	case '5':
		str = jt ? "五" : "伍";
		break;
	case '6':
		str = jt ? "六" : "陆";
		break;
	case '7':
		str = jt ? "七" : "柒";
		break;
	case '8':
		str = jt ? "八" : "捌";
		break;
	case '9':
		str = "玖";
		break;
	}
	return str;
}

String^ Krnln::数值到大写(double 欲转换形式的数值, bool 是否转换为简体)
{
	String^ str = 欲转换形式的数值.ToString();
	array<String^>^ arr = str->Split('.');
	str = arr[0];
	int len = str->Length;
	String^ prev;
	String^ ret = "";
	for (int i = str->Length - 1, n = 1; i >= 0; i--, n++)
	{
		String^ s = getint(str[i], 是否转换为简体);
		String^ ui;
		if (s == "零")
		{
			ui = "";
			if (prev == "零") s = "";
		}
		else ui = getunit(n, 是否转换为简体);
		prev = s;
		ret = s + ui + ret;
	}
	ret = ret->TrimEnd((Char)38646);
	if (ret == "") ret = "零";
	if (arr->Length == 2)
	{
		str = arr[1];
		prev = "点";
		for (int i = 0; i < str->Length; i++) prev += getint(str[i], 是否转换为简体);
		prev = prev->TrimEnd((Char)38646);
		if (prev != "点") ret += prev;
	}
	return ret;
}

String^ Krnln::数值到金额(double 欲转换形式的数值, bool 是否转换为简体)
{
	欲转换形式的数值 = Math::Round(欲转换形式的数值, 2);
	double num = Math::Truncate(欲转换形式的数值);
	String^ str = Krnln::数值到大写(num, 是否转换为简体);
	str += 是否转换为简体 ? "元" : "圆";
	欲转换形式的数值 = 欲转换形式的数值 - num;
	if (欲转换形式的数值 > 0)
	{
		String^ d = 欲转换形式的数值.ToString("F2");
		String^ j = getint(d[2], 是否转换为简体);
		String^ f = getint(d[3], 是否转换为简体);
		bool b1 = !String::IsNullOrEmpty(j);
		bool b2 = !String::IsNullOrEmpty(f);
		if (b1 || b2)
		{
			if (b1) str += j + "角";
			if (b2) str += f + "分";
		}
	}
	return str;
}

String^ Krnln::数值到格式文本(double 欲转换为文本的数值, [Optional]Nullable<int> 小数保留位数, bool 是否进行千分位分隔)
{
	String^ str;
	if (是否进行千分位分隔)
	{
		str = "N";
		if (小数保留位数.HasValue) str += 小数保留位数.Value;
		else
		{
			array<String^>^ arr = 欲转换为文本的数值.ToString()->Split('.');
			int len;
			if (arr->Length == 2) len = arr[1]->Length;
			else len = 0;
			str += len;
		}
	}
	else
	{
		if (小数保留位数.HasValue) str = "F" + 小数保留位数.Value;
	}
	return 欲转换为文本的数值.ToString(str);
}

int Krnln::取字节集长度(array<byte>^ 字节集数据)
{
	if (字节集数据 == nullptr) return 0;
	return 字节集数据->Length;
}

array<byte>^ Krnln::到字节集(String^ 欲转换为字节集的数据)
{
	if (欲转换为字节集的数据 == nullptr) return gcnew array<byte>(0);
	return Encoding::ASCII->GetBytes(欲转换为字节集的数据);
}

array<byte>^ Krnln::到字节集(DateTime 欲转换为字节集的数据)
{
	return 到字节集(欲转换为字节集的数据.Ticks);
}

array<byte>^ Krnln::到字节集(IntPtr 欲转换为字节集的数据)
{
	if (IntPtr::Size == 4) return 到字节集(欲转换为字节集的数据.ToInt32());
	else return 到字节集(欲转换为字节集的数据.ToInt64());
}

array<byte>^ Krnln::到字节集(bool 欲转换为字节集的数据)
{
	return BitConverter::GetBytes(欲转换为字节集的数据);
}

array<byte>^ Krnln::到字节集(byte 欲转换为字节集的数据)
{
	return gcnew array<byte>{ 欲转换为字节集的数据 };
}

array<byte>^ Krnln::到字节集(short 欲转换为字节集的数据)
{
	return BitConverter::GetBytes(欲转换为字节集的数据);
}

array<byte>^ Krnln::到字节集(int 欲转换为字节集的数据)
{
	return BitConverter::GetBytes(欲转换为字节集的数据);
}

array<byte>^ Krnln::到字节集(Int64 欲转换为字节集的数据)
{
	return BitConverter::GetBytes(欲转换为字节集的数据);
}

array<byte>^ Krnln::到字节集(float 欲转换为字节集的数据)
{
	return BitConverter::GetBytes(欲转换为字节集的数据);
}

array<byte>^ Krnln::到字节集(double 欲转换为字节集的数据)
{
	return BitConverter::GetBytes(欲转换为字节集的数据);
}

Object^ Krnln::取字节集数据(array<byte>^ 欲取出其中数据的字节集, int 欲取出数据的类型, int 起始索引位置)
{
	if (欲取出其中数据的字节集 == nullptr || 欲取出其中数据的字节集->Length == 0) return nullptr;
	起始索引位置--;
	if (起始索引位置 < 0) return nullptr;
	Object^ obj;
	switch (欲取出数据的类型)
	{
	case 1:
		obj = 欲取出其中数据的字节集[起始索引位置];
		break;
	case 2:
		obj = BitConverter::ToInt16(欲取出其中数据的字节集, 起始索引位置);
		break;
	case 3:
		obj = BitConverter::ToInt32(欲取出其中数据的字节集, 起始索引位置);
		break;
	case 4:
		obj = BitConverter::ToInt64(欲取出其中数据的字节集, 起始索引位置);
		break;
	case 5:
		obj = BitConverter::ToSingle(欲取出其中数据的字节集, 起始索引位置);
		break;
	case 6:
		obj = BitConverter::ToDouble(欲取出其中数据的字节集, 起始索引位置);
		break;
	case 7:
		obj = BitConverter::ToBoolean(欲取出其中数据的字节集, 起始索引位置);
		break;
	case 8:
		obj = DateTime(BitConverter::ToInt64(欲取出其中数据的字节集, 起始索引位置));
		break;
	case 9:
		if (IntPtr::Size == 4) obj = IntPtr(BitConverter::ToInt32(欲取出其中数据的字节集, 起始索引位置));
		else obj = IntPtr(BitConverter::ToInt64(欲取出其中数据的字节集, 起始索引位置));
		break;
	case 10:
		obj = Encoding::ASCII->GetString(欲取出其中数据的字节集, 起始索引位置, 欲取出其中数据的字节集->Length - 起始索引位置);
		break;
	}
	return obj;
}

array<byte>^ Krnln::取字节集左边(array<byte>^ 欲取其部分的字节集, int 欲取出字节的数目)
{
	if (欲取其部分的字节集 != nullptr && 欲取出字节的数目 > 0)
	{
		if (欲取出字节的数目 > 欲取其部分的字节集->Length) 欲取出字节的数目 = 欲取其部分的字节集->Length;
		array<byte>^ arr = gcnew array<byte>(欲取出字节的数目);
		Array::Copy(欲取其部分的字节集, arr, 欲取出字节的数目);
		return arr;
	}
	return nullptr;
}

array<byte>^ Krnln::取字节集右边(array<byte>^ 欲取其部分的字节集, int 欲取出字节的数目)
{
	if (欲取其部分的字节集 != nullptr && 欲取出字节的数目 > 0)
	{
		if (欲取出字节的数目 > 欲取其部分的字节集->Length) 欲取出字节的数目 = 欲取其部分的字节集->Length;
		int index = 欲取其部分的字节集->Length - 欲取出字节的数目;
		array<byte>^ arr = gcnew array<byte>(欲取出字节的数目);
		Array::Copy(欲取其部分的字节集, index, arr, 0, 欲取出字节的数目);
		return arr;
	}
	return nullptr;
}

array<byte>^ Krnln::取字节集中间(array<byte>^ 欲取其部分的字节集, int 起始取出位置, int 欲取出字节的数目)
{
	if (欲取其部分的字节集 != nullptr && 起始取出位置 > 0 && 欲取出字节的数目 > 0)
	{
		起始取出位置--;
		int len = 欲取其部分的字节集->Length - 起始取出位置;
		if (len >= 0)
		{
			if (欲取出字节的数目 > len) 欲取出字节的数目 = len;
			array<byte>^ arr = gcnew array<byte>(欲取出字节的数目);
			Array::Copy(欲取其部分的字节集, 起始取出位置, arr, 0, 欲取出字节的数目);
			return arr;
		}
	}
	return nullptr;
}

int Krnln::寻找字节集(array<byte>^ 被搜寻的字节集, array<byte>^ 欲寻找的字节集, int 起始搜寻位置)
{
	if (被搜寻的字节集 != nullptr && 起始搜寻位置 > 0)
	{
		if (欲寻找的字节集 != nullptr && 欲寻找的字节集->Length > 0)
		{
			if (被搜寻的字节集->Length > 0)
			{
				起始搜寻位置--;
				byte f = 欲寻找的字节集[0];
				do
				{
					起始搜寻位置 = Array::IndexOf(被搜寻的字节集, f, 起始搜寻位置);
					if (起始搜寻位置 >= 0)
					{
						int len = 被搜寻的字节集->Length - 起始搜寻位置;
						if (len < 欲寻找的字节集->Length) break;
						for (int i = 0; i < 欲寻找的字节集->Length; i++) if (被搜寻的字节集[起始搜寻位置 + i] != 欲寻找的字节集[i]) goto rt;
						return 起始搜寻位置 + 1;
					rt:;
					}
				} while (起始搜寻位置 >= 0);
			}
		}
		else return 1;
	}
	return -1;
}

int Krnln::倒找字节集(array<byte>^ 被搜寻的字节集, array<byte>^ 欲寻找的字节集, [Optional][DefaultValue(1)]int 起始搜寻位置)
{
	if (被搜寻的字节集 != nullptr && 起始搜寻位置 > 0)
	{
		if (欲寻找的字节集 != nullptr && 欲寻找的字节集->Length > 0)
		{
			if (被搜寻的字节集->Length > 0)
			{
				起始搜寻位置--;
				byte f = 欲寻找的字节集[0];
				do
				{
					起始搜寻位置 = Array::LastIndexOf(被搜寻的字节集, f, 起始搜寻位置);
					if (起始搜寻位置 >= 0)
					{
						int len = 被搜寻的字节集->Length - 起始搜寻位置;
						if (len < 欲寻找的字节集->Length) break;
						for (int i = 0; i < 欲寻找的字节集->Length; i++) if (被搜寻的字节集[起始搜寻位置 + i] != 欲寻找的字节集[i]) goto rt;
						return 起始搜寻位置 + 1;
					rt:;
					}
				} while (起始搜寻位置 >= 0);
			}
		}
		else return 1;
	}
	return -1;
}

array<byte>^ Krnln::字节集替换(array<byte>^ 欲替换其部分的字节集, int 起始替换位置, int 替换长度, array<byte>^ 用作替换的字节集)
{
	if (欲替换其部分的字节集 != nullptr)
	{
		int len = 欲替换其部分的字节集->Length;
		起始替换位置--;
		if (起始替换位置 >= 0 && 起始替换位置 < len)
		{
			len -= 替换长度 + 起始替换位置;
			if (len <= 0)
			{
				替换长度 += len;
				if (起始替换位置 == 0) return 欲替换其部分的字节集;
			}
			array<byte>^ bin;
			if (起始替换位置 > 0) bin = 取字节集左边(欲替换其部分的字节集, 起始替换位置);
			bin = 相加(bin, 用作替换的字节集);
			if (len > 0) bin = 相加(bin, 取字节集右边(欲替换其部分的字节集, len));
			return bin;
		}
	}
	return 欲替换其部分的字节集;
}

array<byte>^ Krnln::子字节集替换(array<byte>^ 欲被替换的字节集, array<byte>^ 欲被替换的子字节集, array<byte>^ 用作替换的子字节集, int 进行替换的起始位置, int 替换进行的次数)
{
	if (欲被替换的字节集 != nullptr && 欲被替换的子字节集 != nullptr && 欲被替换的子字节集->Length > 0)
	{
		if (进行替换的起始位置 > 0 && 进行替换的起始位置 < 欲被替换的子字节集->Length)
		{
			int len = 欲被替换的子字节集->Length;
			do
			{
				进行替换的起始位置 = 寻找字节集(欲被替换的字节集, 欲被替换的子字节集, 进行替换的起始位置);
				if (进行替换的起始位置 == -1) break;
				欲被替换的字节集 = 字节集替换(欲被替换的字节集, 进行替换的起始位置, len, 用作替换的子字节集);
				替换进行的次数--;
			} while (替换进行的次数 > 0);
		}
	}
	return 欲被替换的字节集;
}

array<byte>^ Krnln::取重复字节集(int 重复次数, array<byte>^ 待重复的字节集)
{
	if (待重复的字节集 != nullptr && 重复次数 > 0)
	{
		int len = 待重复的字节集->Length;
		array<byte>^ arr = gcnew array<byte>(len * 重复次数);
		for (int i = 0; i < 重复次数; i++) Array::Copy(待重复的字节集, 0, arr, i * len, len);
		return arr;
	}
	return nullptr;
}