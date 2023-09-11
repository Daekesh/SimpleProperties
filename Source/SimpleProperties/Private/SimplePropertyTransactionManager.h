// Copyright Matt Chapman. All Rights Reserved.

#pragma once

#if WITH_EDITOR

#include "Templates/SharedPointer.h"

class FScopedTransaction;
class FText;

class FSimplePropertyTransactionManager
{
public:
	static FSimplePropertyTransactionManager& Get();

	static int32 GetInvalidTransactionId();

	static bool IsValidTransactionId(int32 InTransactionId);

	UE_NODISCARD static int32 StartUnmanagedTransaction(const FText& InTransactionDescription, UObject* InPrimaryObject = nullptr);

	UE_NODISCARD static TSharedPtr<FScopedTransaction> StartUnmanagedScopedTransaction(const FText& InTransactionDescription,
		UObject* InPrimaryObject = nullptr);

	bool IsManagedTransaction(int32 InTransactionId) const;

	bool IsUnmanagedTransaction(int32 InTransactionId) const;

	bool HasManagedTransaction() const;

	/** Attempts to start a transaction. Returns it if successful. */
	int32 StartManagedTransaction(const FText& InTransactionDescription, UObject* InPrimaryObject = nullptr);

	/** Cancels the managed transaction if this is the currently managed one. */
	bool CancelManagedTransaction(int32 InTransactionId);

	/** Ends the managed transaction if this is the currently managed one. */
	bool EndManagedTransaction(int32 InTransactionId);

	/** Cancels the managed transaction if this is the currently managed one. */
	bool CancelUnmanagedTransaction(int32 InTransactionId);

	/** Ends the managed transaction if this is the currently managed one. */
	bool EndUnmanagedTransaction(int32 InTransactionId);

protected:
	int32 ManagedTransactionId;

	FSimplePropertyTransactionManager();
};

#endif
