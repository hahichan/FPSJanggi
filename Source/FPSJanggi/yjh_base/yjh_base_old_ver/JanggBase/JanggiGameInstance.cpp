// Fill out your copyright notice in the Description page of Project Settings.

#include "JanggiGameInstance.h"

UJanggiGameInstance::UJanggiGameInstance()
{
	ClearAllDuelData();
}

void UJanggiGameInstance::Init()
{
	Super::Init();

	// 인스턴스 초기화 시 캐시 비우기
	ClearAllDuelData();
}

void UJanggiGameInstance::SetDuelRequestData(const FDuelRequestData& InRequestData)
{
	CachedRequestData = InRequestData;
}

FDuelRequestData UJanggiGameInstance::GetDuelRequestData() const
{
	return CachedRequestData;
}

bool UJanggiGameInstance::HasValidDuelRequest() const
{
	return CachedRequestData.IsValid();
}

void UJanggiGameInstance::ClearDuelRequestData()
{
	CachedRequestData = FDuelRequestData();
}

void UJanggiGameInstance::SetDuelResultData(const FDuelResultData& InResultData)
{
	CachedResultData = InResultData;
}

FDuelResultData UJanggiGameInstance::GetDuelResultData() const
{
	return CachedResultData;
}

void UJanggiGameInstance::ClearDuelResultData()
{
	CachedResultData = FDuelResultData();
}

void UJanggiGameInstance::ClearAllDuelData()
{
	ClearDuelRequestData();
	ClearDuelResultData();
}
