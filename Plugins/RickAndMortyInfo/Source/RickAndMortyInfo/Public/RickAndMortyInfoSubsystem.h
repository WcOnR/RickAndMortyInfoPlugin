// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "RickAndMortyInfoSubsystem.generated.h"

class URMCharacterDataRequest;

USTRUCT(BlueprintType)
struct FRMCharacterData
{
	GENERATED_BODY();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Id;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Name;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Status;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Species;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Type;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Gender;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString OriginLocationName;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString LocationName;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTexture2D* Image;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FString> Episode;
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FRMCharacterDataRequestDelegate, const FRMCharacterData&, Data, bool, Successful);

UCLASS()
class RICKANDMORTYINFO_API URickAndMortyInfoSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void GetCharacterById(int32 Id, const FRMCharacterDataRequestDelegate& Callback);

protected:
	void ReturnRequestToPool(URMCharacterDataRequest * Request);

protected:
	TArray<URMCharacterDataRequest*> ProcessedRequests;

	static const int32 MaxRequestPoolSize = 10;
};
