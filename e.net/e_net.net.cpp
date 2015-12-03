#include "stdafx.h"
#include "Plugins.h"
#include "common.net.h"
#include "e_net.net.h"

MethodDefinition^ E_Net::GetType(ModuleDefinition^ module)
{
	MethodDefinition^ method = CreateMethod("取类型", module->ImportReference(typeof(Type)), ToList(CreateParameter("类型", module->ImportReference(typeof(RuntimeTypeHandle)))), STATICMETHOD);
	ILProcessor^ ILProcessor = method->Body->GetILProcessor();
	AddILCode(ILProcessor, OpCodes::Call, module->ImportReference(GetStaticMethod(Type, "GetTypeFromHandle", typeof(RuntimeTypeHandle))));
	return method;
}

Object^ E_Net::实例化(RuntimeTypeHandle 类型, ...array<Object^>^ 参数)
{
	return Activator::CreateInstance(Type::GetTypeFromHandle(类型), 参数);
}

Delegate^ E_Net::取子程序指针(Object^ 对象, String^ 名称, ...array<Object^>^ 参数类型)
{

}