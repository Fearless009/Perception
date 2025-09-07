#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BaseAnimInstance.generated.h"

class AMainCharacter;

UCLASS()
class PERCEPTION_API UBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	FTransform EffectorTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	float DistanceToGround;

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
private:
	AMainCharacter* MainCharacter;
};
