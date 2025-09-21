#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInput/Public/EnhancedInputSubsystems.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "../Actors/Portal.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Curves/CurveFloat.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "EngineUtils.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "WorldPartition/DataLayer/DataLayerAsset.h"
#include "WorldPartition/DataLayer/DataLayerManager.h"
#include "Blueprint/UserWidget.h"
#include "../Actors/DynamicWeatherSystem.h"


AMainCharacter::AMainCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	PortalGun = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalGun"));
	PortalGun->SetupAttachment(GetMesh());

	PortalOrbVFXSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PortalOrbVFXSpawnPoint"));
	PortalOrbVFXSpawnPoint->SetupAttachment(PortalGun);

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
}

void AMainCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	APlayerController* PlayerController = Cast<APlayerController>(NewController);

	if (PlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = 
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

		if (EnhancedInputSubsystem)
		{
			EnhancedInputSubsystem->AddMappingContext(MappingContext, 0);
		}
	}
}

void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerHUDClass)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			PlayerHUDWidget = CreateWidget<UUserWidget>(PC, PlayerHUDClass);
			if (PlayerHUDWidget)
			{
				PlayerHUDWidget->AddToViewport();
			}

			HintTextWidget = CreateWidget<UUserWidget>(PC, HintTextClass);
			if (HintTextWidget)
			{
				HintTextWidget->AddToViewport();
			}
		}
	}

	DataLayerManager = UDataLayerManager::GetDataLayerManager(GetWorld());
	DynamicWeather = Cast<ADynamicWeatherSystem>(UGameplayStatics::GetActorOfClass(this, ADynamicWeatherSystem::StaticClass()));
	PortalActor = Cast<APortal>(UGameplayStatics::GetActorOfClass(this, APortal::StaticClass()));
	DirectionalLight = Cast<ADirectionalLight>(UGameplayStatics::GetActorOfClass(this, ADirectionalLight::StaticClass()));
	if (DirectionalLight)
	{
		DefaultDirectionalLightIntensity = DirectionalLight->GetLightComponent()->Intensity;
	}

	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (CameraManager)
	{
		CameraManager->ViewPitchMax = 60.f;
		CameraManager->ViewPitchMin = -60.f;
	}

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (CharacterMesh && PortalGunAnimLayerClass)
	{
		CharacterMesh->LinkAnimClassLayers(PortalGunAnimLayerClass);
	}

	USkeletalMeshComponent* PortalGun_AttachmentParentMesh = GetMesh();
	const FAttachmentTransformRules PortalGun_AttachmentRules = FAttachmentTransformRules::SnapToTargetIncludingScale;
	const FName PortalGun_AttachmentSocketName = FName("WeaponSocket");
	if (PortalGun && PortalGun_AttachmentParentMesh)
	{
		PortalGun->AttachToComponent(PortalGun_AttachmentParentMesh, PortalGun_AttachmentRules, PortalGun_AttachmentSocketName);
	}

	UpdateCharacterState(ECharacterState::ECS_Idle);

	FCharacterMovementValues JogValues;
	JogValues.MaxWalkSpeed = 400.f;
	JogValues.MaxAcceleration = 400.f;
	JogValues.BrakingDeceleration = 1200.f;
	JogValues.BrakingFrictionFactor = 1.f;
	JogValues.BrakingFriction = 0.f;
	JogValues.bUseSeparateBrakingFriction = true;
	JogValues.GroundFriction = 8.f;

	FCharacterMovementValues SprintValues;
	SprintValues.MaxWalkSpeed = 800.f;
	SprintValues.MaxAcceleration = 800.f;
	SprintValues.BrakingDeceleration = 1400.f;
	SprintValues.BrakingFrictionFactor = 1.f;
	SprintValues.BrakingFriction = 0.f;
	SprintValues.bUseSeparateBrakingFriction = true;
	SprintValues.GroundFriction = 8.f;

	MovementDataMap.Add(ECharacterState::ECS_Jog, JogValues);
	MovementDataMap.Add(ECharacterState::ECS_Sprint, SprintValues);

	if (PostProcessingCurve)
	{
		FOnTimelineFloat UpdateDelegate;
		UpdateDelegate.BindUFunction(this, FName("PostProcessing_OnTimelineUpdate"));
		PostProcessingTimeline.AddInterpFloat(PostProcessingCurve, UpdateDelegate);

		FOnTimelineEvent FinishedDelegate;
		FinishedDelegate.BindUFunction(this, FName("PostProcessing_OnTimelineFinished"));
		PostProcessingTimeline.SetTimelineFinishedFunc(FinishedDelegate);

		PostProcessingTimeline.SetLooping(false);
	}
}

void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PostProcessingTimeline.TickTimeline(DeltaTime);

	if (PhysicsHandle && GrabbedObject)
	{
		FVector GrabLocation = (Camera->GetForwardVector() * GrabDistance) + Camera->GetComponentLocation();
		PhysicsHandle->SetTargetLocationAndRotation(GrabLocation, GetActorRotation());
	}
}

void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Completed, this, &ThisClass::MoveCompleted);
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ThisClass::Look);
		EnhancedInputComponent->BindAction(IA_Sprint, ETriggerEvent::Started, this, &ThisClass::SprintKeyStarted);
		EnhancedInputComponent->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &ThisClass::SprintKeyEnded);
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &ThisClass::StartJump);
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ThisClass::StopJump);
		EnhancedInputComponent->BindAction(IA_ShootL, ETriggerEvent::Started, this, &ThisClass::ShootLeftStarted);
		EnhancedInputComponent->BindAction(IA_ShootR, ETriggerEvent::Started, this, &ThisClass::ShootRightStarted);
		EnhancedInputComponent->BindAction(IA_PickupObject, ETriggerEvent::Started, this, &ThisClass::PickupObject);
		EnhancedInputComponent->BindAction(IA_TimeTravel, ETriggerEvent::Started, this, &ThisClass::TimeTravel);
	}
}

void AMainCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D DirectionVector = Value.Get<FVector2D>();
	const float ForwardDirectionScaler = DirectionVector.Y;
	float RightDirectionScaler = DirectionVector.X;

	const bool bMovingForward = ForwardDirectionScaler > 0.2f;
	if (bSprintKeyPressed && bMovingForward)
	{
		RightDirectionScaler = 0.f;
		UpdateCharacterState(ECharacterState::ECS_Sprint);
	}
	else
	{
		UpdateCharacterState(ECharacterState::ECS_Jog);
	}

	if (GetController())
	{
		const FRotator ControlRotation = GetController()->GetControlRotation();
		const FRotator YawRotation = FRotator(0.f, ControlRotation.Yaw, 0.f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, ForwardDirectionScaler);
		AddMovementInput(RightDirection, RightDirectionScaler);
	}
}

void AMainCharacter::MoveCompleted(const FInputActionValue& Value)
{

}

void AMainCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D DirectionVector = Value.Get<FVector2D>();
	const float MouseXDirection = DirectionVector.X;
	const float MouseYDirection = DirectionVector.Y;

	if (GetController())
	{
		AddControllerYawInput(MouseXDirection * MouseSensitivityMultiplier);
		AddControllerPitchInput(MouseYDirection * MouseSensitivityMultiplier);
	}
}

void AMainCharacter::SprintKeyStarted(const FInputActionValue& Value)
{
	if (bCanSprint)
	{
		bSprintKeyPressed = true;
	}
}

void AMainCharacter::SprintKeyEnded(const FInputActionValue& Value)
{
	bSprintKeyPressed = false;
}

void AMainCharacter::StartJump(const FInputActionValue& Value)
{
	Jump();
}

void AMainCharacter::StopJump(const FInputActionValue& Value)
{
	StopJumping();
}

void AMainCharacter::ShootLeftStarted(const FInputActionValue& Value)
{
	if (PortalOrbVFX)
	{
		const FVector SpawnLocation = PortalOrbVFXSpawnPoint->GetComponentLocation();
		UNiagaraComponent* NSPortalOrb = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PortalOrbVFX, SpawnLocation, FRotator::ZeroRotator, FVector(1.0f), true, true, ENCPoolMethod::AutoRelease);
		FVector4 LinearColorToVector4 = FVector4(LinearBlueColor.R, LinearBlueColor.G, LinearBlueColor.B, 1.0f);
		NSPortalOrb->SetNiagaraVariableVec3(FString("Velocity"), Camera->GetForwardVector());
		NSPortalOrb->SetNiagaraVariableVec4(FString("Color"), LinearColorToVector4);
	}

	if (BluePortalSFX)
	{
		UGameplayStatics::PlaySound2D(this, BluePortalSFX, 1.f, 1.f, 0.3f);
	}
	const bool bLeftClickShoot = true;
	bLastShotPortalBlue = true;
	ShootPortal(bLeftClickShoot);
}

void AMainCharacter::ShootRightStarted(const FInputActionValue& Value)
{
	if (PortalOrbVFX)
	{
		const FVector SpawnLocation = PortalOrbVFXSpawnPoint->GetComponentLocation();
		UNiagaraComponent* NSPortalOrb = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PortalOrbVFX, SpawnLocation, FRotator::ZeroRotator, FVector(1.0f), true, true, ENCPoolMethod::AutoRelease);
		FVector4 LinearColorToVector4 = FVector4(LinearOrangeColor.R, LinearOrangeColor.G, LinearOrangeColor.B, 1.0f);
		NSPortalOrb->SetNiagaraVariableVec3(FString("Velocity"), Camera->GetForwardVector());
		NSPortalOrb->SetNiagaraVariableVec4(FString("Color"), LinearColorToVector4);
	}

	if (OrangePortalSFX)
	{
		UGameplayStatics::PlaySound2D(this, OrangePortalSFX, 1.f, 1.f, 0.3f);
	}
	const bool bLeftClickShoot = true;
	bLastShotPortalBlue = false;
	ShootPortal(!bLeftClickShoot);
}

void AMainCharacter::PickupObject(const FInputActionValue& Value)
{
	// Grab object
	if (!GrabbedObject && PhysicsHandle)
	{
		const UObject* WorldContextObject = this;
		const FVector StartLocation = Camera->GetComponentLocation();
		const float TraceRange = 500.f;
		const FVector EndLocation = (Camera->GetForwardVector() * TraceRange) + Camera->GetComponentLocation();
		const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
		const bool bTraceComplexCollisions = false;
		const TArray<AActor*> ActorsToIgnore = { this };
		FHitResult OutHitResult;
		const bool bIgnoreSelf = true;

		const bool bHitSuccess = UKismetSystemLibrary::LineTraceSingle(WorldContextObject, StartLocation, EndLocation, TraceChannel, bTraceComplexCollisions,
			ActorsToIgnore, EDrawDebugTrace::None, OutHitResult, bIgnoreSelf);

		if (bHitSuccess)
		{
			FVector GrabLocation = (Camera->GetForwardVector() * GrabDistance) + Camera->GetComponentLocation();
			UPrimitiveComponent* HitComponent = OutHitResult.GetComponent();
			AActor* HitActor = OutHitResult.GetActor();
			if (HitComponent->IsSimulatingPhysics())
			{
				PhysicsHandle->GrabComponentAtLocationWithRotation(HitComponent, NAME_None, GrabLocation, FRotator::ZeroRotator);
				GrabbedObject = HitComponent;
				// Enabling overlap with portal for pickup object once grabbed
				IPortalJumper::Execute_SetPickupObjectOverlapWithPortal(HitActor, true);
				// Resetting important pickup object variables once grabbed, putting in default state
				IPortalJumper::Execute_SetIgnoreDuplicateDrop(HitActor, true);
				IPortalJumper::Execute_SetIgnoreInternalPortalCollider(HitActor, false);
				IPortalJumper::Execute_SetIsGrabbed(HitActor, true);
				bCanSprint = false;

				if (HintTextWidget)
				{
					HintTextWidget->RemoveFromParent();
				}
			}
		}
	}

	// Release object
	else
	{
		IPortalJumper::Execute_SetPickupObjectOverlapWithPortal(GrabbedObject->GetOwner(), false);
		IPortalJumper::Execute_SetPickupObjectCollisionToPortalWall(GrabbedObject->GetOwner(), true);
		IPortalJumper::Execute_DeactivateDuplicateObject(GrabbedObject->GetOwner());
		IPortalJumper::Execute_SetPickupObjectMeshVisibility(GrabbedObject->GetOwner(), true, true);
		IPortalJumper::Execute_SetIgnoreInternalPortalCollider(GrabbedObject->GetOwner(), false);
		IPortalJumper::Execute_SetIsGrabbed(GrabbedObject->GetOwner(), false);

		PhysicsHandle->ReleaseComponent();
		GrabbedObject = nullptr;
		bCanSprint = true;
	}

	if (PortalActor)
	{
		PortalActor->EnableExternalColliders(false);
	}
}

void AMainCharacter::TimeTravel(const FInputActionValue& Value)
{
	if (bTimeTravelInProgress)
	{
		return;
	}

	if (!DirectionalLight)
	{
		DirectionalLight = Cast<ADirectionalLight>(UGameplayStatics::GetActorOfClass(this, ADirectionalLight::StaticClass()));
	}

	const FVector PlayerLocation = GetActorLocation();
	const FVector4 MaterialLocationInput = FVector4(PlayerLocation.X, PlayerLocation.Y, PlayerLocation.Z, 0.f);

	if (GetWorld() && WorldDeResMPC)
	{
		UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(WorldDeResMPC);
		if (MPCInstance)
		{
			MPCInstance->SetVectorParameterValue(FName("Location"), MaterialLocationInput);

			if (PostProcessingCurve && PortalActor)
			{
				PortalActor->EnablePortalCollision(false);
				PostProcessingTimeline.PlayFromStart();
				bTimeTravelInProgress = true;
				if (SFXTimeTravel)
				{
					UGameplayStatics::PlaySound2D(this, SFXTimeTravel);
				}
				FTimerHandle DLTimerHandle;
				GetWorld()->GetTimerManager().SetTimer(DLTimerHandle, this, &ThisClass::TogglePastAndPresent, PostProcessingCurveHalfTime, false);
			}
		}
	}
}

void AMainCharacter::PostProcessing_OnTimelineUpdate(float Amount)
{
	float CurrentTime = PostProcessingTimeline.GetPlaybackPosition();
	if (GetWorld() && WorldDeResMPC)
	{
		UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(WorldDeResMPC);
		if (MPCInstance)
		{
			MPCInstance->SetScalarParameterValue(FName("Amount"), Amount);

			if (CurrentTime <= PostProcessingCurveHalfTime)
			{
				FVector4 OrangeColor(LinearOrangeColor.R, LinearOrangeColor.G, LinearOrangeColor.B, LinearOrangeColor.A);
				MPCInstance->SetVectorParameterValue(FName("OverlayColor"), OrangeColor);
			}
			else
			{
				FVector4 BlueColor(LinearBlueColor.R, LinearBlueColor.G, LinearBlueColor.B, LinearBlueColor.A);
				MPCInstance->SetVectorParameterValue(FName("OverlayColor"), BlueColor);
			}

			if (DirectionalLight)
			{
				float NormalizedAlpha = Amount / 3.69f; //3.69f is the max amount on the PostProcessCurve
				float LightIntensity = FMath::Lerp(DefaultDirectionalLightIntensity, 0.1f, NormalizedAlpha);
				DirectionalLight->GetLightComponent()->SetIntensity(LightIntensity);

			}
		}
	}
}

void AMainCharacter::PostProcessing_OnTimelineFinished()
{
	if (PortalActor)
	{
		PortalActor->EnablePortalCollision(true);
	}
	bTimeTravelInProgress = false;
}

void AMainCharacter::PlayShockwaveVFX(const bool in_bLastShotPortalBlue, FHitResult& in_OutHitResult)
{
	if (!PortalOneShockwave || !PortalTwoShockwave)
	{
		return; 
	}

	if (in_bLastShotPortalBlue)
	{
		FVector ShockwaveSpawnLocation = in_OutHitResult.Location + in_OutHitResult.Normal * 10.f;
		FRotator RotationOffset = FRotator(90.f, 0.f, 0.f);
		FRotator ShockwaveSpawnRotation = UKismetMathLibrary::MakeRotFromX(in_OutHitResult.Normal) + RotationOffset;
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PortalTwoShockwave, ShockwaveSpawnLocation, ShockwaveSpawnRotation);
	}
	else
	{
		FVector ShockwaveSpawnLocation = in_OutHitResult.Location + in_OutHitResult.Normal * 10.f;
		FRotator RotationOffset = FRotator(90.f, 0.f, 0.f);
		FRotator ShockwaveSpawnRotation = UKismetMathLibrary::MakeRotFromX(in_OutHitResult.Normal) + RotationOffset;
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PortalOneShockwave, ShockwaveSpawnLocation, ShockwaveSpawnRotation);
	}
}

void AMainCharacter::ShootPortal(const bool bLeftClick)
{
	if (!PortalActor)
	{
		return;
	}

	const UObject* WorldContextObject = this;
	const FVector StartLocation = Camera->GetComponentLocation();
	const float TraceRange = 10000.f;
	const FVector EndLocation = (Camera->GetForwardVector() * TraceRange) + Camera->GetComponentLocation();
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	const bool bTraceComplexCollisions = false;
	const TArray<AActor*> ActorsToIgnore = { this };
	FHitResult OutHitResult;
	const bool bIgnoreSelf = true;

	const bool bHitSuccess = UKismetSystemLibrary::LineTraceSingle(WorldContextObject, StartLocation, EndLocation, TraceChannel, bTraceComplexCollisions,
		ActorsToIgnore, EDrawDebugTrace::None, OutHitResult, bIgnoreSelf);


	if (bHitSuccess)
	{
		EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(OutHitResult);
		//GEngine->AddOnScreenDebugMessage(5, 60.f, FColor::Red, FString::Printf(TEXT("Surface Hit: %d"), SurfaceType));
		FVector NewLocation;
		FRotator NewRotation;
		switch (SurfaceType)
		{
		case SurfaceType_Default:
			PlayShockwaveVFX(bLastShotPortalBlue, OutHitResult);
			break;

		case SurfaceType1: // Portal Wall Surface
			NewLocation = OutHitResult.Location + OutHitResult.Normal;
			NewRotation = UKismetMathLibrary::MakeRotFromX(OutHitResult.Normal);
			PortalActor->PlacePortal(bLeftClick, NewLocation, NewRotation, OutHitResult.Normal);
			break;

		case SurfaceType2: // Portal One Surface - Orange Portal
			if (!bLastShotPortalBlue)
			{
				NewLocation = OutHitResult.Location + OutHitResult.Normal;
				NewRotation = UKismetMathLibrary::MakeRotFromX(OutHitResult.Normal);
				PortalActor->PlacePortal(bLeftClick, NewLocation, NewRotation, OutHitResult.Normal);
			}
			else
			{
				PlayShockwaveVFX(bLastShotPortalBlue, OutHitResult);
			}
			break;

		case SurfaceType3: // Portal Two Surface - Blue Portal
			if (bLastShotPortalBlue)
			{
				NewLocation = OutHitResult.Location + OutHitResult.Normal;
				NewRotation = UKismetMathLibrary::MakeRotFromX(OutHitResult.Normal);
				PortalActor->PlacePortal(bLeftClick, NewLocation, NewRotation, OutHitResult.Normal);
			}
			else
			{
				PlayShockwaveVFX(bLastShotPortalBlue, OutHitResult);
			}
			break;
		}
	}
}

void AMainCharacter::UpdateCharacterState(ECharacterState InCharacterState)
{
	CharacterState = InCharacterState;
	const FCharacterMovementValues* MovementValues = MovementDataMap.Find(CharacterState);

	if (MovementValues && GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = MovementValues->MaxWalkSpeed;
		GetCharacterMovement()->MaxAcceleration = MovementValues->MaxAcceleration;
		GetCharacterMovement()->BrakingDecelerationWalking = MovementValues->BrakingDeceleration;
		GetCharacterMovement()->BrakingFrictionFactor = MovementValues->BrakingFrictionFactor;
		GetCharacterMovement()->BrakingFriction = MovementValues->BrakingFriction;
		GetCharacterMovement()->bUseSeparateBrakingFriction = MovementValues->bUseSeparateBrakingFriction;
		GetCharacterMovement()->GroundFriction = MovementValues->GroundFriction;
	}
}

FTransform AMainCharacter::GetLeftHandIKTransform()
{
	const FName LHIK_SocketName = FName("LHIK");
	const FTransform LHIK_SocketTransform = PortalGun->GetSocketTransform(LHIK_SocketName);

	const FName TransformToBoneSpace_BoneName = FName("hand_r");
	const FVector TransformToBoneSpace_InLocation = LHIK_SocketTransform.GetLocation();
	const FRotator TransformToBoneSpace_InRotation = LHIK_SocketTransform.GetRotation().Rotator(); 

	FVector TransformToBoneSpace_OutPosition;
	FRotator TransformToBoneSpace_OutRotation;

	GetMesh()->TransformToBoneSpace(
		TransformToBoneSpace_BoneName, 
		TransformToBoneSpace_InLocation, 
		TransformToBoneSpace_InRotation,
		TransformToBoneSpace_OutPosition, 
		TransformToBoneSpace_OutRotation
	);

	FTransform LHIK_BoneSpaceTransform = FTransform(TransformToBoneSpace_OutRotation, TransformToBoneSpace_OutPosition, LHIK_SocketTransform.GetScale3D());
	
	return LHIK_BoneSpaceTransform;
}

float AMainCharacter::GetDistanceToGround()
{
	float DistanceToGround = 0.f;
	if (!GetCapsuleComponent())
	{
		return DistanceToGround;
	}

	const float CapsuleComponentHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector CharacterCenterLocation = GetActorLocation();
	const FVector CharacterFeetLocation = CharacterCenterLocation - FVector(0.f, 0.f, CapsuleComponentHalfHeight);

	const FVector TraceStartLocation = CharacterFeetLocation;
	const FVector TraceEndLocation = CharacterFeetLocation + FVector(0.f, 0.f, -1000.f);
	const float TraceSphereRadius = 20.f;
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	const bool bTestAgainstComplexCollision = false;
	const TArray<AActor*> ActorsToIgnore = { this };
	FHitResult OutHitResult;
	const bool bIgnoreSelf = true;

	const bool bHitSuccess = UKismetSystemLibrary::SphereTraceSingle(this, TraceStartLocation, TraceEndLocation, TraceSphereRadius,
		TraceChannel, bTestAgainstComplexCollision, ActorsToIgnore, EDrawDebugTrace::None, OutHitResult, bIgnoreSelf);

	if (bHitSuccess)
	{
		DistanceToGround = OutHitResult.Distance;
		//GEngine->AddOnScreenDebugMessage(2, 10.f, FColor::Red, FString::Printf(TEXT("Distance to ground: %f"), DistanceToGround));
	}

	return DistanceToGround;
}

void AMainCharacter::OnPortalJump_Implementation(const FVector& TargetLocation, const FRotator& TargetRotation)
{

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent)
	{
		const float SpeedMultiplier = 1.f;
		const float PortalJumpVelocity = GetVelocity().Size();
		//const FVector ExitForward = TargetRotation.Vector();
		const float ExitOffset = 10.f;
		const FVector ExitForward = FRotationMatrix(TargetRotation).GetUnitAxis(EAxis::X);
		const FVector SafeLocation = TargetLocation + ExitForward * ExitOffset;
		const FRotator CharacterRotation = FRotator(0.f, TargetRotation.Yaw, 0.f);
		SetActorLocationAndRotation(SafeLocation, CharacterRotation, false, (FHitResult *)nullptr, ETeleportType::TeleportPhysics);
		GetController()->SetControlRotation(TargetRotation);
		MovementComponent->Velocity = GetActorForwardVector() * PortalJumpVelocity * SpeedMultiplier;

		if (GrabbedObject)
		{
			IPortalJumper::Execute_SetIgnoreDuplicateDrop(GrabbedObject->GetOwner(), true);
			IPortalJumper::Execute_PlayerJumpedPortalWithGrabbedObject(GrabbedObject->GetOwner());
			IPortalJumper::Execute_DeactivateDuplicateObject(GrabbedObject->GetOwner());
			IPortalJumper::Execute_SetPickupObjectMeshVisibility(GrabbedObject->GetOwner(), true, false);
			IPortalJumper::Execute_SetIgnoreInternalPortalCollider(GrabbedObject->GetOwner(), false);
		}
	}
}

void AMainCharacter::TogglePastAndPresent()
{
	if (!DataLayerManager)
	{
		return;
	}

	if (bPresentActivated)
	{
		DataLayerManager->SetDataLayerRuntimeState(DL_Past, EDataLayerRuntimeState::Activated);
		DataLayerManager->SetDataLayerRuntimeState(DL_Present, EDataLayerRuntimeState::Loaded);
		bPresentActivated = false;
		GEngine->AddOnScreenDebugMessage(1200, 1.f, FColor::Red, "Past - activated");
		if (DynamicWeather)
		{
			DynamicWeather->EnablePastWeather();
		}
	}
	else
	{
		DataLayerManager->SetDataLayerRuntimeState(DL_Present, EDataLayerRuntimeState::Activated);
		DataLayerManager->SetDataLayerRuntimeState(DL_Past, EDataLayerRuntimeState::Loaded);
		bPresentActivated = true;
		GEngine->AddOnScreenDebugMessage(1200, 1.f, FColor::Cyan, "Present - activated");
		if (DynamicWeather)
		{
			DynamicWeather->EnablePresentWeather();
		}
	}
}


