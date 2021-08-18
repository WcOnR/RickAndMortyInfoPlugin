// Fill out your copyright notice in the Description page of Project Settings.

#include "RMCharacterDataRequest.h"

#include "Containers/UnrealString.h"
#include "ImageUtils.h"
#include "JsonObjectConverter.h"

DEFINE_LOG_CATEGORY_STATIC(LogRMCharacterRequest, All, All)

namespace
{
static const FString REST_API_ADDRESS("https://rickandmortyapi.com/api");
}

URMCharacterDataRequest::URMCharacterDataRequest()
{
	MainDataRequest = FHttpModule::Get().CreateRequest();
	MainDataRequest->OnProcessRequestComplete().BindUObject(this, &URMCharacterDataRequest::OnMainRequestComplete);
	ImgRequest = FHttpModule::Get().CreateRequest();
	ImgRequest->OnProcessRequestComplete().BindUObject(this, &URMCharacterDataRequest::OnImgRequestComplete);
	EpisodesRequest = FHttpModule::Get().CreateRequest();
	EpisodesRequest->OnProcessRequestComplete().BindUObject(this, &URMCharacterDataRequest::OnEpisodeRequestComplete);
}

void URMCharacterDataRequest::Init(int32 Id, const FRMCharacterDataRequestDelegate& Callback)
{
	ResultCallback = Callback;
	MainDataRequest->SetURL(FString::Printf(TEXT("%s/character/%d"), *REST_API_ADDRESS, Id));
	MainDataRequest->ProcessRequest();
}

void URMCharacterDataRequest::OnMainRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		SendResult(Response, false);
		return;
	}
	FString Content = Response->GetContentAsString();
	FJsonObjectConverter::JsonObjectStringToUStruct(Content, &Data, false, false);

	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Content);
	if (FJsonSerializer::Deserialize(JsonReader, Json))
	{
		FString AvatarUrl;
		if (Json->TryGetStringField(FString("image"), AvatarUrl))
		{
			ImgRequest->SetURL(AvatarUrl);
			ImgRequest->ProcessRequest();
		}
		else
		{
			SendResult(Response, false);
		}

		const TSharedPtr<FJsonObject>* LocationData;
		if (Json->TryGetObjectField(FString("origin"), LocationData))
			(*LocationData)->TryGetStringField(FString("name"), Data.OriginLocationName);
		if (Json->TryGetObjectField(FString("location"), LocationData))
			(*LocationData)->TryGetStringField(FString("name"), Data.LocationName);
	}
	ConstructEpisodesRequest();
}

void URMCharacterDataRequest::OnImgRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		SendResult(Response, false);
		return;
	}
	Data.Image = FImageUtils::ImportBufferAsTexture2D(Response->GetContent());
	if (EpisodesRequest->GetStatus() == EHttpRequestStatus::Succeeded)
		SendResult(Response, true);
}

void URMCharacterDataRequest::OnEpisodeRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		SendResult(Response, false);
		return;
	}
	FString ArrayKey("wrap");
	FString JsonWrap = FString::Printf(TEXT("{ \"%s\": %s }"), *ArrayKey, *Response->GetContentAsString());

	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonWrap);
	if (FJsonSerializer::Deserialize(JsonReader, Json))
	{
		const TArray<TSharedPtr<FJsonValue>>* Episodes;
		if (Json->TryGetArrayField(ArrayKey, Episodes))
		{
			Data.Episode.SetNum(Episodes->Num());
			for (int32 i = 0; i < Episodes->Num(); ++i)
			{
				if (auto EpisodeObj = (*Episodes)[i]->AsObject())
				{
					EpisodeObj->TryGetStringField(FString("name"), Data.Episode[i]);
				}
				else
				{
					Data.Episode[i].Empty();
				}
			}
		}
	}
	if (ImgRequest->GetStatus() == EHttpRequestStatus::Succeeded)
		SendResult(Response, true);
}

void URMCharacterDataRequest::ConstructEpisodesRequest()
{
	FString EpisodesRequestUrl = FString::Printf(TEXT("%s/episode/"), *REST_API_ADDRESS);
	int32 PrefixLength = EpisodesRequestUrl.Len();
	for (const auto& EpisodeUrl : Data.Episode)
	{
		EpisodesRequestUrl += EpisodeUrl.Mid(PrefixLength) + ",";
	}
	EpisodesRequest->SetURL(EpisodesRequestUrl);
	EpisodesRequest->ProcessRequest();
}

void URMCharacterDataRequest::SendResult(FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		if (Response.IsValid())
		{
			int32 Code = Response->GetResponseCode();
			UE_LOG(LogRMCharacterRequest, Error, TEXT("Request failed! ResponseCode:%d Url:%s"), Code, *Response->GetURL());
		}
		else
		{
			UE_LOG(LogRMCharacterRequest, Error, TEXT("Request failed! Unknown reason."));
		}
	}
	ResultCallback.ExecuteIfBound(Data, bWasSuccessful);
	Data = FRMCharacterData();
	OnRequestIsFree.ExecuteIfBound(this);
}
