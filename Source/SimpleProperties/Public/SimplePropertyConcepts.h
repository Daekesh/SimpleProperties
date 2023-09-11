// Copyright Matt Chapman. All Rights Reserved.

#pragma once

class FName;
class FProperty;
class FReferenceCollector;

// FString
struct CEqualityEquals
{
	template<typename InValueType, typename InCompareType>
	auto Requires(bool& Result, const InValueType& InValue, const InCompareType& InOther) -> decltype(
		Result = InValue.Equals(InOther)
	);
};

// FText
struct CEqualityEqualTo
{
	template<typename InValueType, typename InCompareType>
	auto Requires(bool& Result, const InValueType& InValue, const InCompareType& InOther) -> decltype(
		Result = InValue.EqualTo(InOther)
	);
};

struct CCanCollectReferences
{
	template<typename InValueType>
	auto Requires(bool& Result, const InValueType& InValue, FReferenceCollector& InCollector) -> decltype(
		Result = InValue.AddStructReferencedObjects(InCollector)
	);
};

struct CCanFindInnerPropertyInstance
{
	template<typename InValueType>
	auto Requires(bool& Result, const InValueType& InValue, FName InPropertyName, const FProperty*& OutProp, const void*& OutData) -> decltype(
		Result = InValue.FindInnerPropertyInstance(InPropertyName, OutProp, OutData)
	);
};

struct CIsLambdaFunction
{
	template<typename InFunctionType>
	auto Requires(bool& Result, InFunctionType InFunction) -> decltype(
		Result = InFunction.operator()
	);
};
