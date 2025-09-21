#include "BaseAnimInstance.h"
#include "MainCharacter.h"

void UBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MainCharacter = Cast<AMainCharacter>(TryGetPawnOwner());
}

void UBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!MainCharacter)
	{
		MainCharacter = Cast<AMainCharacter>(TryGetPawnOwner());
	}

	if (MainCharacter)
	{
		EffectorTransform = MainCharacter->GetLeftHandIKTransform();
		DistanceToGround = MainCharacter->GetDistanceToGround();
	}
}
