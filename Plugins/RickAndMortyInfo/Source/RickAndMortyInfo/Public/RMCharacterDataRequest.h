// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "RickAndMortyInfoSubsystem.h"
#include "UObject/NoExportTypes.h"

#include "RMCharacterDataRequest.generated.h"

class URMCharacterDataRequest;

DECLARE_DELEGATE_OneParam(FRMRequestIsFreeDelegate, URMCharacterDataRequest* /*Request*/);

UCLASS()
class RICKANDMORTYINFO_API URMCharacterDataRequest : public UObject
{
	GENERATED_BODY()

public:
	URMCharacterDataRequest();

	void Init(int32 Id, const FRMCharacterDataRequestDelegate& Callback);

public:
	FRMRequestIsFreeDelegate OnRequestIsFree;

protected:
	void OnMainRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnImgRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnEpisodeRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void ConstructEpisodesRequest();

	void SendResult(FHttpResponsePtr Response, bool bWasSuccessful);

protected:
	FHttpRequestPtr MainDataRequest;
	FHttpRequestPtr ImgRequest;
	FHttpRequestPtr EpisodesRequest;

	FRMCharacterData Data;
	FRMCharacterDataRequestDelegate ResultCallback;
};
