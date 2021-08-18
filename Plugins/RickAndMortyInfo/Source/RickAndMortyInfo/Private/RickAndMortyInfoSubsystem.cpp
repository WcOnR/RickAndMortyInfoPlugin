// Fill out your copyright notice in the Description page of Project Settings.

#include "RickAndMortyInfoSubsystem.h"

#include "RMCharacterDataRequest.h"

DEFINE_LOG_CATEGORY_STATIC(LogRickAndMortyInfo, All, All)

void URickAndMortyInfoSubsystem::GetCharacterById(int32 Id, const FRMCharacterDataRequestDelegate& Callback)
{
	if (ProcessedRequests.Num() < MaxRequestPoolSize)
	{
		URMCharacterDataRequest* Request = NewObject<URMCharacterDataRequest>();
		ProcessedRequests.Add(Request);
		Request->OnRequestIsFree.BindUObject(this, &URickAndMortyInfoSubsystem::ReturnRequestToPool);
		Request->Init(Id, Callback);
		return;
	}
	UE_LOG(LogRickAndMortyInfo, Error, TEXT("Request limit exceeded!"));
	Callback.ExecuteIfBound(FRMCharacterData(), false);
}

void URickAndMortyInfoSubsystem::ReturnRequestToPool(URMCharacterDataRequest* Request)
{
	Request->OnRequestIsFree.Unbind();
	ProcessedRequests.Remove(Request);
}
