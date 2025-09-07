#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h" 
#include "Door.generated.h"

class UCurveFloat;

UCLASS()
class PERCEPTION_API ADoor : public AActor
{
	GENERATED_BODY()
	
public:	
	ADoor();
	virtual void Tick(float DeltaTime) override;

	void OpenDoor();
	void CloseDoor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DoorFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Door;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* DoorCurve;

	FTimeline DoorTimeline;

	UFUNCTION()
	void DoorOnTimelineUpdate(float Alpha);

public:	


};
