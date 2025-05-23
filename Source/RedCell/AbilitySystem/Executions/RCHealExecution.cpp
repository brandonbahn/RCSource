// Fill out your copyright notice in the Description page of Project Settings.


#include "RCHealExecution.h"
#include "AbilitySystem/Attributes/RCHealthSet.h"
#include "AbilitySystem/Attributes/RCCombatSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RCHealExecution)


struct FHealStatics

{
	FGameplayEffectAttributeCaptureDefinition BaseHealDef;

	FHealStatics()
	{
		BaseHealDef = FGameplayEffectAttributeCaptureDefinition(URCCombatSet::GetBaseHealAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

static FHealStatics& HealStatics()
{
	static FHealStatics Statics;
	return Statics;
}


URCHealExecution::URCHealExecution()
{
	RelevantAttributesToCapture.Add(HealStatics().BaseHealDef);
}

void URCHealExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float BaseHeal = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(HealStatics().BaseHealDef, EvaluateParameters, BaseHeal);

	const float HealingDone = FMath::Max(0.0f, BaseHeal);

	if (HealingDone > 0.0f)
	{
		// Apply a healing modifier, this gets turned into + health on the target
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(URCHealthSet::GetHealingAttribute(), EGameplayModOp::Additive, HealingDone));
	}
#endif // #if WITH_SERVER_CODE
}

