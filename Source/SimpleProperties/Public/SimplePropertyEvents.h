// Copyright Matt Chapman. All Rights Reserved.

#pragma once

#include "Delegates/Delegate.h"
#include "Delegates/DelegateCombinations.h"
#include "SimplePropertyTemplates.h"
#include "Templates/UnrealTypeTraits.h"

struct FSimplePropertyOnChange
{
	DECLARE_DELEGATE(FDelegate)

	FDelegate Callback;

	FSimplePropertyOnChange() = delete;

	FSimplePropertyOnChange(FDelegate InCallback)
		: Callback(InCallback)
	{
	}

	template<typename... InArgsType, typename InFunctionType = void(*)(InArgsType&...)
		UE_REQUIRES(TModels<CIsLambdaFunction, InFunctionType>::Value)>
	FSimplePropertyOnChange(InFunctionType InFunction, InArgsType&&... InArgs)
		: FSimplePropertyOnChange(FDelegate::CreateLambda(InFunction, Forward<InArgsType>(InArgs)...))
	{
	}

	template<typename... InArgsType, typename InFunctionType = void(*)(InArgsType&...)
		UE_REQUIRES(TIsStaticClassFunction<InFunctionType>::Value)>
	FSimplePropertyOnChange(InFunctionType InFunction, InArgsType&&... InArgs)
		: FSimplePropertyOnChange(FDelegate::CreateStatic(InFunction, Forward<InArgsType>(InArgs)...))
	{
	}

	template<typename InClassType, typename... InArgsType,
		typename InFunctionType = void(InClassType::*)(InArgsType&&...)
		UE_REQUIRES(TIsInstanceClassFunction<InFunctionType>::Value)>
	FSimplePropertyOnChange(InClassType* InObject, InFunctionType InFunction, InArgsType&&... InArgs)
		: FSimplePropertyOnChange(FDelegate::CreateRaw(InObject, InFunction, Forward<InArgsType>(InArgs)...))
	{
	}
};
