#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInput/Public/InputActionValue.h"
#include "../Interfaces/PortalJumper.h"
#include "Components/TimelineComponent.h" 
#include "MainCharacter.generated.h"


UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Idle UMETA(DisplayName = "Idle"),
	ECS_Jog UMETA(DisplayName = "Jog"),
	ECS_Sprint UMETA(DisplayName = "Sprint"),
};


USTRUCT(BlueprintType)
struct FCharacterMovementValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxWalkSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxAcceleration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BrakingDeceleration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BrakingFrictionFactor = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BrakingFriction = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bUseSeparateBrakingFriction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundFriction = 0.f;
};


class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class APortal;
class UNiagaraSystem;
class UMaterialParameterCollection;
class UCurveFloat;
class UPhysicsHandleComponent;
class ADirectionalLight;
class UDataLayerAsset;
class UDataLayerManager;
class UUserWidget;
class UNiagaraSystem;
class ADynamicWeatherSystem;

UCLASS()
class PERCEPTION_API AMainCharacter : public ACharacter, public IPortalJumper
{
	GENERATED_BODY()

public:
	AMainCharacter();
	virtual void PossessedBy(AController* NewController) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PortalGun")
	UStaticMeshComponent* PortalGun;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PortalGun")
	USceneComponent* PortalOrbVFXSpawnPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
	UPhysicsHandleComponent* PhysicsHandle;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* PortalOneShockwave;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* PortalTwoShockwave;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* PortalDistortionShockwave;

	void UpdateCharacterState(ECharacterState CurrentCharacterState);

public:	
	UFUNCTION(BlueprintCallable, Category = "IK")
	FTransform GetLeftHandIKTransform();

	UFUNCTION(BlueprintCallable, Category = "Animation Data")
	float GetDistanceToGround();

	UFUNCTION()
	virtual void OnPortalJump_Implementation(const FVector& TargetLocation, const FRotator& TargetRotation);

private:
	// Enhanced Input:
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* MappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* IA_Sprint;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* IA_Jump;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* IA_ShootL;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* IA_ShootR;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* IA_PickupObject;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* IA_TimeTravel;


	// Input Functions
	UFUNCTION()
	void Move(const FInputActionValue& Value);

	UFUNCTION()
	void MoveCompleted(const FInputActionValue& Value);

	UFUNCTION()
	void Look(const FInputActionValue& Value);

	UFUNCTION()
	void SprintKeyStarted(const FInputActionValue& Value);

	UFUNCTION()
	void SprintKeyEnded(const FInputActionValue& Value);

	UFUNCTION()
	void StartJump(const FInputActionValue& Value);

	UFUNCTION()
	void StopJump(const FInputActionValue& Value);

	UFUNCTION()
	void ShootLeftStarted(const FInputActionValue& Value);

	UFUNCTION()
	void ShootRightStarted(const FInputActionValue& Value);

	UFUNCTION()
	void PickupObject(const FInputActionValue& Value);

	UFUNCTION()
	void TimeTravel(const FInputActionValue& Value);

private:
	// TSubclass
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess  = "true"))
	TSubclassOf<UAnimInstance> PortalGunAnimLayerClass;

	UPROPERTY(EditDefaultsOnly, Category = "User Widget", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> PlayerHUDClass;

	UPROPERTY(EditDefaultsOnly, Category = "User Widget", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> HintTextClass;

public:
	UUserWidget* PlayerHUDWidget;
	UUserWidget* HintTextWidget;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE UUserWidget* GetHintTextWidget() { return HintTextWidget; };

private:
	// Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterState", meta = (AllowPrivateAccess = "true"))
	ECharacterState CharacterState = ECharacterState::ECS_Jog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	TMap<ECharacterState, FCharacterMovementValues> MovementDataMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics", meta = (AllowPrivateAccess = "true"))
	float GrabDistance = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Color", meta = (AllowPrivateAccess = "true"))
	FLinearColor LinearOrangeColor = FLinearColor(2.0f, 0.495f, 0.104f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Color", meta = (AllowPrivateAccess = "true"))
	FLinearColor LinearBlueColor = FLinearColor(1.176f, 3.371f, 5.0f, 1.0f);

	UPROPERTY()
	APortal* PortalActor;

	UPROPERTY()
	ADynamicWeatherSystem* DynamicWeather;

	UPROPERTY()
	UPrimitiveComponent* GrabbedObject;

	// Sounds
	UPROPERTY(EditDefaultsOnly, Category = "SFX", meta = (AllowPrivateAccess = "true"))
	USoundBase* SFXTimeTravel;

	UPROPERTY(EditDefaultsOnly, Category = "SFX", meta = (AllowPrivateAccess = "true"))
	USoundBase* BluePortalSFX;

	UPROPERTY(EditDefaultsOnly, Category = "SFX", meta = (AllowPrivateAccess = "true"))
	USoundBase* OrangePortalSFX;


	//Booleans

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bSprintKeyPressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bCanSprint = true;

	bool bLastShotPortalBlue = false;
	bool bTimeTravelInProgress = false;

	void ShootPortal(const bool bLeftClick);
	void PlayShockwaveVFX(const bool in_bLastShotPortalBlue, FHitResult& in_OutHitResult);

public:
	UPROPERTY(BlueprintReadOnly);
	bool bPresentActivated = true;

private:
	//VFX
	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* PortalOrbVFX;


	// Post Processing
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PostProcessing", meta = (AllowPrivateAccess = "true"))
	UMaterialParameterCollection* WorldDeResMPC;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PostProcessing", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* PostProcessingCurve;

	FTimeline PostProcessingTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PostProcessing", meta = (AllowPrivateAccess = "true"))
	float PostProcessingCurveHalfTime = 7.5f;

	UFUNCTION()
	void PostProcessing_OnTimelineUpdate(float Amount); // Alpha is the amount in the CurveFloat, use value of 0 for min amount, 3.69 for max amount.

	UFUNCTION()
	void PostProcessing_OnTimelineFinished();

	ADirectionalLight* DirectionalLight;
	float DefaultDirectionalLightIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataLayers", meta = (AllowPrivateAccess = "true"))
	UDataLayerAsset* DL_Past;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataLayers", meta = (AllowPrivateAccess = "true"))
	UDataLayerAsset* DL_Present;

	UFUNCTION(BlueprintCallable)
	void TogglePastAndPresent();

	UDataLayerManager* DataLayerManager;

	// Private Functions
	//void HandleSprintCamera();
	//void HandleJogCamera();


};

