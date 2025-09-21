#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "WeightButton.generated.h"

class USphereComponent;
class UCurveFloat;
class ADoor;

UCLASS()
class PERCEPTION_API AWeightButton : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeightButton();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ButtonPedastal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Button;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* OverlapCollider;

	UFUNCTION()
	void Collider_OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void Collider_OnComponentEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UPROPERTY(EditAnywhere, Category = "Door")
	ADoor* DoorToOpen;

	UFUNCTION(BlueprintImplementableEvent)
	void SetButtonEmissiveColor(bool Enable);

	bool bAvoidTimelineTick = false;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetAvoidTimelineTick() { return bAvoidTimelineTick; }

	bool WeightOverlapped = false;


private:
	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* ButtonCurve;

	FTimeline ButtonTimeline;

	UFUNCTION()
	void OnButtonTimelineUpdate(float Alpha);

	UFUNCTION()
	void OnButtonTimelineFinished();
};
