// Fill out your copyright notice in the Description page of Project Settings.


#include "RCLoadingScreenSubsystem.h"

#include "Blueprint/UserWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCLoadingScreenSubsystem)

class UUserWidget;

//////////////////////////////////////////////////////////////////////
// URCLoadingScreenSubsystem

URCLoadingScreenSubsystem::URCLoadingScreenSubsystem()
{
}

void URCLoadingScreenSubsystem::SetLoadingScreenContentWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
	if (LoadingScreenWidgetClass != NewWidgetClass)
	{
		LoadingScreenWidgetClass = NewWidgetClass;

		OnLoadingScreenWidgetChanged.Broadcast(LoadingScreenWidgetClass);
	}
}

TSubclassOf<UUserWidget> URCLoadingScreenSubsystem::GetLoadingScreenContentWidget() const
{
	return LoadingScreenWidgetClass;
}
