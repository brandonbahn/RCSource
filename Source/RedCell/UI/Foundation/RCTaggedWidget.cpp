// Fill out your copyright notice in the Description page of Project Settings.


#include "RCTaggedWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCTaggedWidget)

//@TODO: The other TODOs in this file are all related to tag-based showing/hiding of widgets, see UE-142237

URCTaggedWidget::URCTaggedWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void URCTaggedWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!IsDesignTime())
    {
        // Listen for tag changes on our hidden tags
        //@TODO: That thing I said

        // Set our initial visibility value (checking the tags, etc...)
        SetVisibility(GetVisibility());
    }
}

void URCTaggedWidget::NativeDestruct()
{
    if (!IsDesignTime())
    {
        //@TODO: Stop listening for tag changes
    }

    Super::NativeDestruct();
}

void URCTaggedWidget::SetVisibility(ESlateVisibility InVisibility)
{
#if WITH_EDITORONLY_DATA
    if (IsDesignTime())
    {
        Super::SetVisibility(InVisibility);
        return;
    }
#endif

    // Remember what the caller requested; even if we're currently being
    // suppressed by a tag we should respect this call when we're done
    bWantsToBeVisible = ConvertSerializedVisibilityToRuntime(InVisibility).IsVisible();
    if (bWantsToBeVisible)
    {
        ShownVisibility = InVisibility;
    }
    else
    {
        HiddenVisibility = InVisibility;
    }

    const bool bHasHiddenTags = false;//@TODO: Foo->HasAnyTags(HiddenByTags);

    // Actually apply the visibility
    const ESlateVisibility DesiredVisibility = (bWantsToBeVisible && !bHasHiddenTags) ? ShownVisibility : HiddenVisibility;
    if (GetVisibility() != DesiredVisibility)
    {
        Super::SetVisibility(DesiredVisibility);
    }
}

void URCTaggedWidget::OnWatchedTagsChanged()
{
    const bool bHasHiddenTags = false;//@TODO: Foo->HasAnyTags(HiddenByTags);

    // Actually apply the visibility
    const ESlateVisibility DesiredVisibility = (bWantsToBeVisible && !bHasHiddenTags) ? ShownVisibility : HiddenVisibility;
    if (GetVisibility() != DesiredVisibility)
    {
        Super::SetVisibility(DesiredVisibility);
    }
}

