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

	/** Attempts to start a managed transaction. Returns the transaction index if successful, invalid index if not. */
	int32 StartManagedTransaction(const FText& InTransactionDescription, UObject* InPrimaryObject = nullptr);

	/** Cancels the managed transaction if this is the currently managed one. */
	bool CancelManagedTransaction(int32 InTransactionId);

	/** Ends the managed transaction if this is the currently managed one. */
	bool EndManagedTransaction(int32 InTransactionId);

	/** Cancels the unmanaged transaction if this is not the currently managed one. */
	bool CancelUnmanagedTransaction(int32 InTransactionId);

	/** Ends the unmanaged transaction if this is not the currently managed one. */
	bool EndUnmanagedTransaction(int32 InTransactionId);

protected:
	int32 ManagedTransactionId;

	FSimplePropertyTransactionManager();
};

#endif
