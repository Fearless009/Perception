#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h" 
#include "Portal.generated.h"

class USceneCaptureComponent2D;
class UBoxComponent;
class UCurveFloat;
class UNiagaraSystem;

UCLASS()
class PERCEPTION_API APortal : public AActor
{
	GENERATED_BODY()
	
public:	
	APortal();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* DefaultRootComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* D1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* D2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Door1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Door2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* CamRoot1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* CamRoot2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* PSim1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* PSim2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneCaptureComponent2D* P1Capture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneCaptureComponent2D* P2Capture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* Col1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* Col2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* ExternalCol1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* ExternalCol2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* PortalPlacementCheck_Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* PortalPlacementCheck_Down;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* PortalPlacementCheck_Up;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* PortalPlacementCheck_Left;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* PortalPlacementCheck_Right;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* PortalPlacementCheck_Center;

	USceneComponent* FiringPortalRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* TP1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* TP2;


protected:
	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* ScaleCurve;

	FTimeline ScaleTimeline;

	UFUNCTION()
	void OnScaleTimelineUpdate(float Alpha);

	UFUNCTION()
	void OnScaleTimelineFinished();

	UPROPERTY(EditAnywhere, Category = "Timeline")
	float StartScale = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	float EndScale = 0.75f;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* PortalOneShockwave;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* PortalTwoShockwave;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* PortalDistortionShockwave;



public:	
	void PlacePortal(bool bShootLeft, const FVector& NewLocation, const FRotator& NewRotator, const FVector& InNormal);
	void EnablePortalCollision(bool bEnableCollision);
	void EnableExternalColliders(bool bEnableCollision);

private:
	void UpdateCameras();
	void UpdateCaptureComponent(USceneCaptureComponent2D* CaptureComp, USceneComponent* SimComp);

	UFUNCTION()
	void Col1_OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void Col2_OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void Col1_OnComponentEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UFUNCTION()
	void Col2_OnComponentEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UFUNCTION()
	void ExternalCol1_OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void ExternalCol2_OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void ExternalCol1_OnComponentEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UFUNCTION()
	void ExternalCol2_OnComponentEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);


	FVector CalculateTargetPoint(const FHitResult& SweepResult, USceneComponent* TPEnter, USceneComponent* TPExit);
	void OnPortalCollision(AActor* JumpActor, UBoxComponent* InExitCollider, const USceneComponent* TargetCapture, const UObject* TriggerCollider, FVector TargetPoint);
	bool IsPortalPlacementPointValid(USceneComponent* PortalPlacementPoint);
	bool OffsetPortalCheckRoot(bool BooleanToCheck, USceneComponent* PortalCheckPoint, FVector OffsetDirection);




private:
	bool bBluePortalShot = false;
	void PlayShockwaveVFX(bool in_bLastShotPortalBlue, const FVector& NewLocation, const FRotator& NewRotator, const FVector& InNormal);

};
