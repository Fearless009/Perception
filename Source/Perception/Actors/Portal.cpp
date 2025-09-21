#include "Portal.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/BoxComponent.h"
#include "../Interfaces/PortalJumper.h"
#include "Curves/CurveFloat.h"
#include "../Interfaces/PickupObjectInterface.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"


APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRootComponent"));
	SetRootComponent(DefaultRootComponent);

	D1 = CreateDefaultSubobject<USceneComponent>(TEXT("D1"));
	D1->SetupAttachment(GetRootComponent());

	D2 = CreateDefaultSubobject<USceneComponent>(TEXT("D2"));
	D2->SetupAttachment(GetRootComponent());

	Door1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door1"));
	Door1->SetupAttachment(D1);

	Door2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door2"));
	Door2->SetupAttachment(D2);

	CamRoot1 = CreateDefaultSubobject<USceneComponent>(TEXT("CamRoot1"));
	CamRoot1->SetupAttachment(D1);

	CamRoot2 = CreateDefaultSubobject<USceneComponent>(TEXT("CamRoot2"));
	CamRoot2->SetupAttachment(D2);

	PSim1 = CreateDefaultSubobject<USceneComponent>(TEXT("PSim1"));
	PSim1->SetupAttachment(D1);

	PSim2 = CreateDefaultSubobject<USceneComponent>(TEXT("PSim2"));
	PSim2->SetupAttachment(D2);

	P1Capture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("P1Capture"));
	P1Capture->SetupAttachment(CamRoot1);

	P2Capture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("P2Capture"));
	P2Capture->SetupAttachment(CamRoot2);

	Col1 = CreateDefaultSubobject<UBoxComponent>(TEXT("Col1"));
	Col1->SetupAttachment(D1);

	Col2 = CreateDefaultSubobject<UBoxComponent>(TEXT("Col2"));
	Col2->SetupAttachment(D2);

	ExternalCol1 = CreateDefaultSubobject<UBoxComponent>(TEXT("ExternalCol1"));
	ExternalCol1->SetupAttachment(D1);

	ExternalCol2 = CreateDefaultSubobject<UBoxComponent>(TEXT("ExternalCol2"));
	ExternalCol2->SetupAttachment(D2);

	PortalPlacementCheck_Root = CreateDefaultSubobject<USceneComponent>(TEXT("PortalPlacementCheck_Root"));
	PortalPlacementCheck_Root->SetupAttachment(GetRootComponent());

	PortalPlacementCheck_Down = CreateDefaultSubobject<USceneComponent>(TEXT("PortalPlacementCheck_Down"));
	PortalPlacementCheck_Down->SetupAttachment(PortalPlacementCheck_Root);

	PortalPlacementCheck_Up = CreateDefaultSubobject<USceneComponent>(TEXT("PortalPlacementCheck_Up"));
	PortalPlacementCheck_Up->SetupAttachment(PortalPlacementCheck_Root);

	PortalPlacementCheck_Left = CreateDefaultSubobject<USceneComponent>(TEXT("PortalPlacementCheck_Left"));
	PortalPlacementCheck_Left->SetupAttachment(PortalPlacementCheck_Root);

	PortalPlacementCheck_Right = CreateDefaultSubobject<USceneComponent>(TEXT("PortalPlacementCheck_Right"));
	PortalPlacementCheck_Right->SetupAttachment(PortalPlacementCheck_Root);

	PortalPlacementCheck_Center = CreateDefaultSubobject<USceneComponent>(TEXT("PortalPlacementCheck_Center"));
	PortalPlacementCheck_Center->SetupAttachment(PortalPlacementCheck_Root);

	TP1 = CreateDefaultSubobject<USceneComponent>(TEXT("TP1"));
	TP1->SetupAttachment(D1);

	TP2 = CreateDefaultSubobject<USceneComponent>(TEXT("TP2"));
	TP2->SetupAttachment(D2);
}

void APortal::BeginPlay()
{
	Super::BeginPlay();

	EnableExternalColliders(false);
	
	Col1->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::Col1_OnComponentBeginOverlap);
	Col2->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::Col2_OnComponentBeginOverlap);
	Col1->OnComponentEndOverlap.AddDynamic(this, &ThisClass::Col1_OnComponentEndOverlap);
	Col2->OnComponentEndOverlap.AddDynamic(this, &ThisClass::Col2_OnComponentEndOverlap);
	ExternalCol1->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::ExternalCol1_OnComponentBeginOverlap);
	ExternalCol2->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::ExternalCol2_OnComponentBeginOverlap);
	ExternalCol1->OnComponentEndOverlap.AddDynamic(this, &ThisClass::ExternalCol1_OnComponentEndOverlap);
	ExternalCol2->OnComponentEndOverlap.AddDynamic(this, &ThisClass::ExternalCol2_OnComponentEndOverlap);

	//ExternalCol1->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//ExternalCol2->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (ScaleCurve)
	{
		FOnTimelineFloat UpdateDelegate;
		UpdateDelegate.BindUFunction(this, FName("OnScaleTimelineUpdate"));
		ScaleTimeline.AddInterpFloat(ScaleCurve, UpdateDelegate);

		FOnTimelineEvent FinishedDelegate;
		FinishedDelegate.BindUFunction(this, FName("OnScaleTimelineFinished"));
		ScaleTimeline.SetTimelineFinishedFunc(FinishedDelegate);

		ScaleTimeline.SetLooping(false);
	}
}

void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateCameras();
	ScaleTimeline.TickTimeline(DeltaTime);

}

void APortal::UpdateCameras()
{
	if (!GetWorld()) return;
	const UWorld* WorldContextObject = GetWorld();
	const int PlayerIndex = 0;
	APlayerCameraManager* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(WorldContextObject, PlayerIndex);
	const FVector  CameraWorldLocation = PlayerCameraManager->GetCameraLocation();
	PSim1->SetWorldLocation(CameraWorldLocation);
	PSim2->SetWorldLocation(CameraWorldLocation);

	UpdateCaptureComponent(P1Capture, PSim2);
	UpdateCaptureComponent(P2Capture, PSim1);
}

void APortal::UpdateCaptureComponent(USceneCaptureComponent2D* CaptureComp, USceneComponent* SimComp)
{
	if (!CaptureComp || !SimComp || !Door1)
	{
		return;
	}

	const FVector SimCompRelativeLocation = SimComp->GetRelativeLocation();
	CaptureComp->SetRelativeLocation(SimCompRelativeLocation);

	const FVector CaptureCompWorldLocation = CaptureComp->GetComponentLocation();
	const USceneComponent* PortalRoot = CaptureComp->GetAttachParent();
	const FVector PortalRootWorldLocation = PortalRoot->GetComponentLocation();
	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(CaptureCompWorldLocation, PortalRootWorldLocation);
	CaptureComp->SetWorldRotation(LookAtRotation);

	const FVector CaptureCompRelativeLocation = CaptureComp->GetRelativeLocation();
	const float DistanceCaptureCompToPortal = FMath::Max(CaptureCompRelativeLocation.Size(), 1.f);
	const float PortalWidth = Door1->GetComponentScale().X * 100.f;
	const float TangentValue = PortalWidth / DistanceCaptureCompToPortal;
	const float AngleInRadians = FMath::Atan(TangentValue);
	const float AngleInDegrees = FMath::RadiansToDegrees(AngleInRadians);
	const float ClampedAngle = FMath::Clamp(AngleInDegrees, 5.f, 180.f);
	const float FOVAdjustment = 30.f;
	CaptureComp->FOVAngle = ClampedAngle + FOVAdjustment;
}

// Internal Col1 Begin Overlap
void APortal::Col1_OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FVector TargetPoint = CalculateTargetPoint(SweepResult, TP1, TP2);
	OnPortalCollision(OtherActor, Col2, P2Capture, OverlappedComp, TargetPoint);

	if (OtherActor->GetClass()->ImplementsInterface(UPickupObjectInterface::StaticClass())
		&& OtherActor->GetClass()->ImplementsInterface(UPortalJumper::StaticClass()))
	{
		// As soon as pickup object touches portal, show duplicate object relative to the other portal
		IPortalJumper::Execute_PreviewDuplicateObject(OtherActor, D1, D2); 

		// Disabling the Portal Wall collision channel on the Pickup Object, so that our object can "go through" the portal
		IPortalJumper::Execute_SetPickupObjectCollisionToPortalWall(OtherActor, false);
		GEngine->AddOnScreenDebugMessage(3002, 2.f, FColor::Cyan, "Internal Col1 Begin Overlap Called");

	}

}

// Internal Col2 Begin Overlap
void APortal::Col2_OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FVector TargetPoint = CalculateTargetPoint(SweepResult, TP2, TP1);
	OnPortalCollision(OtherActor, Col1, P1Capture, OverlappedComp, TargetPoint);

	if (OtherActor->GetClass()->ImplementsInterface(UPickupObjectInterface::StaticClass())
		&& OtherActor->GetClass()->ImplementsInterface(UPortalJumper::StaticClass()))
	{
		// As soon as pickup object touches portal, show duplicate object relative to the other portal
		IPortalJumper::Execute_PreviewDuplicateObject(OtherActor, D2, D1);

		// Disabling the Portal Wall collision channel on the Pickup Object, so that our object can "go through" the portal
		IPortalJumper::Execute_SetPickupObjectCollisionToPortalWall(OtherActor, false);
		GEngine->AddOnScreenDebugMessage(3002, 2.f, FColor::Cyan, "Internal Col2 Begin Overlap Called");
	}

}

// Internal Col1End Overlap
void APortal::Col1_OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->GetClass()->ImplementsInterface(UPickupObjectInterface::StaticClass())
		&& OtherActor->GetClass()->ImplementsInterface(UPortalJumper::StaticClass()))
	{
		IPortalJumper::Execute_SetPickupObjectMeshVisibility(OtherActor, false, false);
		IPortalJumper::Execute_SetIgnoreDuplicateDrop(OtherActor, false);

		// Enabling pickup object's collision to portal wall back to true, so that physics collisions work properly
		IPortalJumper::Execute_SetPickupObjectCollisionToPortalWall(OtherActor, true);

		// External colliders are off by default and should only be turned on once Internal Col End Overlap is called
		EnableExternalColliders(true);
	}
	//GEngine->AddOnScreenDebugMessage(3002, 2.f, FColor::Cyan, "Internal Col1 End Overlap Called");
}

// Internal Col2 End Overlap
void APortal::Col2_OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->GetClass()->ImplementsInterface(UPickupObjectInterface::StaticClass())
		&& OtherActor->GetClass()->ImplementsInterface(UPortalJumper::StaticClass()))
	{
		IPortalJumper::Execute_SetPickupObjectMeshVisibility(OtherActor, false, false);
		IPortalJumper::Execute_SetIgnoreDuplicateDrop(OtherActor, false);

		// Enabling pickup object's collision to portal wall back to true, so that physics collisions work properly
		IPortalJumper::Execute_SetPickupObjectCollisionToPortalWall(OtherActor, true);

		// External colliders are off by default and should only be turned on once Internal Col End Overlap is called
		EnableExternalColliders(true);
	}
	//GEngine->AddOnScreenDebugMessage(3002, 2.f, FColor::Cyan, "Internal Col2 End Overlap Called");
}

// External Col1 Begin Overlap
void APortal::ExternalCol1_OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//GEngine->AddOnScreenDebugMessage(3003, 2.f, FColor::Green, "External Col1 Begin Overlap Called");
	if (OtherActor->GetClass()->ImplementsInterface(UPickupObjectInterface::StaticClass())
		&& OtherActor->GetClass()->ImplementsInterface(UPortalJumper::StaticClass()))
	{
		IPortalJumper::Execute_SetIgnoreInternalPortalCollider(OtherActor, true);
		IPortalJumper::Execute_SetPickupObjectMeshVisibility(OtherActor, true, false);
	}
}

// External Col2 Begin Overlap
void APortal::ExternalCol2_OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//GEngine->AddOnScreenDebugMessage(3003, 2.f, FColor::Green, "External Col2 Begin Overlap Called");
	if (OtherActor->GetClass()->ImplementsInterface(UPickupObjectInterface::StaticClass())
		&& OtherActor->GetClass()->ImplementsInterface(UPortalJumper::StaticClass()))
	{
		IPortalJumper::Execute_SetIgnoreInternalPortalCollider(OtherActor, true);
		IPortalJumper::Execute_SetPickupObjectMeshVisibility(OtherActor, true, false);
	}
}

// External Col1 End Overlap
void APortal::ExternalCol1_OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//GEngine->AddOnScreenDebugMessage(3003, 2.f, FColor::Green, "External Col1 End Overlap Called");
	if (OtherActor->GetClass()->ImplementsInterface(UPickupObjectInterface::StaticClass())
		&& OtherActor->GetClass()->ImplementsInterface(UPortalJumper::StaticClass()))
	{
		IPortalJumper::Execute_SetIgnoreDuplicateDrop(OtherActor, true);
		IPortalJumper::Execute_SetPickupObjectMeshVisibility(OtherActor, true, false);
		IPortalJumper::Execute_SetIgnoreInternalPortalCollider(OtherActor, false);
		EnableExternalColliders(false);
		IPortalJumper::Execute_DeactivateDuplicateObject(OtherActor);
	}

}

// External Col2 End Overlap
void APortal::ExternalCol2_OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//GEngine->AddOnScreenDebugMessage(3003, 2.f, FColor::Green, "External Col2 End Overlap Called");
	if (OtherActor->GetClass()->ImplementsInterface(UPickupObjectInterface::StaticClass())
		&& OtherActor->GetClass()->ImplementsInterface(UPortalJumper::StaticClass()))
	{
		IPortalJumper::Execute_SetIgnoreDuplicateDrop(OtherActor, true);
		IPortalJumper::Execute_SetPickupObjectMeshVisibility(OtherActor, true, false);
		IPortalJumper::Execute_SetIgnoreInternalPortalCollider(OtherActor, false);
		EnableExternalColliders(false);
		IPortalJumper::Execute_DeactivateDuplicateObject(OtherActor);
	}
}

FVector APortal::CalculateTargetPoint(const FHitResult& SweepResult, USceneComponent* TPEnter, USceneComponent* TPExit)
{
	TPEnter->SetWorldLocation(SweepResult.Location);
	const FVector TPEnterRelativeLocationMirrored = TPEnter->GetRelativeLocation() * FVector(1.f, -1.f, 1.f);
	TPExit->SetRelativeLocation(TPEnterRelativeLocationMirrored);
	const FVector TPExitWorldLocation = TPExit->GetComponentLocation();
	return TPExitWorldLocation;
}

void APortal::OnPortalCollision(AActor* JumpActor, UBoxComponent* InExitCollider, const USceneComponent* TargetCapture, const UObject* TriggerCollider, FVector TargetPoint)
{
	if (!JumpActor || !InExitCollider || !TargetCapture || !TriggerCollider)
	{
		return;
	}

	if (JumpActor->GetClass()->ImplementsInterface(UPortalJumper::StaticClass()))
	{
		const FVector TargetLocation = TargetPoint;
		const FRotator TargetRotation = TargetCapture->GetComponentRotation();
		IPortalJumper::Execute_OnPortalJump(JumpActor, TargetLocation, TargetRotation);
	}
}


void APortal::OnScaleTimelineUpdate(float Alpha)
{
	const float NewPortalScale = FMath::Lerp(StartScale, EndScale, Alpha);
	if (FiringPortalRoot)
	{
		FiringPortalRoot->SetRelativeScale3D(FVector(NewPortalScale));
	}
}

void APortal::OnScaleTimelineFinished()
{
	// Play Sound effect later
}

void APortal::PlacePortal(bool bShootLeft, const FVector& NewLocation, const FRotator& NewRotator, const FVector& InNormal)
{
	FiringPortalRoot = bShootLeft ? D2 : D1;
	bBluePortalShot = bShootLeft ? true : false;

	PortalPlacementCheck_Root->SetWorldLocationAndRotation(NewLocation, NewRotator);

	bool bIsPortalUpPointValid = IsPortalPlacementPointValid(PortalPlacementCheck_Up);
	if (!bIsPortalUpPointValid)
	{
		const FVector OffsetDirection = -PortalPlacementCheck_Root->GetUpVector();
		bIsPortalUpPointValid = OffsetPortalCheckRoot(bIsPortalUpPointValid, PortalPlacementCheck_Up, OffsetDirection);
	}

	//GEngine->AddOnScreenDebugMessage(11, 3.f, FColor::Green, FString::Printf(TEXT("bIsPortalUpPointValid: %s"), bIsPortalUpPointValid ? TEXT("TRUE") : TEXT("FALSE")));

	bool bIsPortalDownPointValid = IsPortalPlacementPointValid(PortalPlacementCheck_Down);
	if (!bIsPortalDownPointValid)
	{
		const FVector OffsetDirection = PortalPlacementCheck_Root->GetUpVector();
		bIsPortalDownPointValid = OffsetPortalCheckRoot(bIsPortalDownPointValid, PortalPlacementCheck_Down, OffsetDirection);
	}
	//GEngine->AddOnScreenDebugMessage(12, 3.f, FColor::Green, FString::Printf(TEXT("bIsPortalDownPointValid: %s"), bIsPortalDownPointValid ? TEXT("TRUE") : TEXT("FALSE")));

	bIsPortalUpPointValid = IsPortalPlacementPointValid(PortalPlacementCheck_Up);

	bool bIsPortalRightPointValid = IsPortalPlacementPointValid(PortalPlacementCheck_Right);
	if (!bIsPortalRightPointValid)
	{
		const FVector OffsetDirection = PortalPlacementCheck_Root->GetRightVector();
		bIsPortalRightPointValid = OffsetPortalCheckRoot(bIsPortalRightPointValid, PortalPlacementCheck_Right, OffsetDirection);
	}
	//GEngine->AddOnScreenDebugMessage(13, 3.f, FColor::Green, FString::Printf(TEXT("bIsPortalRightPointValid: %s"), bIsPortalRightPointValid ? TEXT("TRUE") : TEXT("FALSE")));

	bool bIsPortalLeftPointValid = IsPortalPlacementPointValid(PortalPlacementCheck_Left);
	if (!bIsPortalLeftPointValid)
	{
		const FVector OffsetDirection = -PortalPlacementCheck_Root->GetRightVector();
		bIsPortalLeftPointValid = OffsetPortalCheckRoot(bIsPortalLeftPointValid, PortalPlacementCheck_Left, OffsetDirection);
	}
	//GEngine->AddOnScreenDebugMessage(14, 3.f, FColor::Green, FString::Printf(TEXT("bIsPortalLeftPointValid: %s"), bIsPortalLeftPointValid ? TEXT("TRUE") : TEXT("FALSE")));

	bIsPortalRightPointValid = IsPortalPlacementPointValid(PortalPlacementCheck_Right);

	bool bIsPortalCenterPointValid = IsPortalPlacementPointValid(PortalPlacementCheck_Center);
	//GEngine->AddOnScreenDebugMessage(15, 3.f, FColor::Green, FString::Printf(TEXT("bIsPortalCenterPointValid: %s"), bIsPortalLeftPointValid ? TEXT("TRUE") : TEXT("FALSE")));
	const bool bIsPortalPlacementValid = bIsPortalUpPointValid && bIsPortalDownPointValid && bIsPortalRightPointValid && bIsPortalLeftPointValid && bIsPortalCenterPointValid;
	

	//GEngine->AddOnScreenDebugMessage(8, 3.f, FColor::Red, FString::Printf(TEXT("Is Portal Placement Valid: %s"), bIsPortalPlacementValid ? TEXT("TRUE") : TEXT("FALSE")));

	if (FiringPortalRoot && bIsPortalPlacementValid)
	{
		FiringPortalRoot->SetRelativeScale3D(FVector(StartScale));
		FiringPortalRoot->SetWorldLocationAndRotation(PortalPlacementCheck_Root->GetComponentLocation(), NewRotator);

		if (ScaleCurve)
		{
			ScaleTimeline.PlayFromStart();
		}
		else
		{
			FiringPortalRoot->SetRelativeScale3D(FVector(EndScale));
		}
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(40, 4.f, FColor::Cyan, "Cannot place, not enough space!");
		PlayShockwaveVFX(bBluePortalShot, NewLocation, NewRotator, InNormal);
	}
}

void APortal::EnablePortalCollision(bool bEnableCollision) // Disabling portals during time travel transition. Calling in MainCharacter, TimeTravel Input
{
	if (bEnableCollision)
	{
		Col1->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Col2->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		Col1->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Col2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void APortal::EnableExternalColliders(bool bEnableCollision)
{
	if (bEnableCollision)
	{
		ExternalCol1->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ExternalCol2->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		ExternalCol1->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ExternalCol2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

bool APortal::IsPortalPlacementPointValid(USceneComponent* PortalPlacementPoint)
{
	bool bIsPortalPlacementPointValid = false;

	if (!GetWorld() || !PortalPlacementPoint)
	{
		return bIsPortalPlacementPointValid;
	}
	const UObject* WorldContextObject = GetWorld();
	const FVector StartLocation = PortalPlacementPoint->GetComponentLocation();
	const float TraceRange = 100.f;
	const FVector EndLocation = (PortalPlacementPoint->GetForwardVector() * TraceRange) + PortalPlacementPoint->GetComponentLocation();
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	const bool bTraceComplexCollisions = false;
	const TArray<AActor*> ActorsToIgnore = { };
	FHitResult OutHitResult;
	const bool bIgnoreSelf = false;

	const bool bHitSuccess = UKismetSystemLibrary::LineTraceSingle(WorldContextObject, StartLocation, EndLocation, TraceChannel, bTraceComplexCollisions,
		ActorsToIgnore, EDrawDebugTrace::None, OutHitResult, bIgnoreSelf);

	if (bHitSuccess)
	{
		EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(OutHitResult);

		switch (SurfaceType)
		{
		case SurfaceType_Default:
			bIsPortalPlacementPointValid = false;
			break;

		case SurfaceType1: // Portal Wall Surface
			bIsPortalPlacementPointValid = true;
			break;

		case SurfaceType2: // Portal One Surface
			if (!bBluePortalShot)
			{
				bIsPortalPlacementPointValid = true;
			}
			break;

		case SurfaceType3: // Portal Two Surface
			if (bBluePortalShot)
			{
				bIsPortalPlacementPointValid = true;
			}
			break;
		}
	}
	else
	{
		bIsPortalPlacementPointValid = false;
	}

	return bIsPortalPlacementPointValid;
}

bool APortal::OffsetPortalCheckRoot(bool BooleanToCheck, USceneComponent* PortalCheckPoint, FVector OffsetDirection)
{
	if (!PortalCheckPoint)
	{
		return false;
	}

	OffsetDirection.Normalize();
	const float OffsetAmount = 20.f;
	const FVector OffsetVector = OffsetDirection * OffsetAmount;
	const int MaxIterations = 10;
	for (int i = 1; (i <= MaxIterations); i++)
	{
		const FVector PlacementPortalWorldLocation = PortalPlacementCheck_Root->GetComponentLocation();
		const FVector PlacementPortalNewLocation = PlacementPortalWorldLocation + OffsetVector;
		PortalPlacementCheck_Root->SetWorldLocation(PlacementPortalNewLocation);

		BooleanToCheck = IsPortalPlacementPointValid(PortalCheckPoint);

		if (BooleanToCheck)
		{
			break;
		}
	}

	return BooleanToCheck;
}

void APortal::PlayShockwaveVFX(bool in_bLastShotPortalBlue, const FVector& NewLocation, const FRotator& NewRotator, const FVector& InNormal)
{
	if (!PortalOneShockwave || !PortalTwoShockwave)
	{
		return;
	}

	if (in_bLastShotPortalBlue)
	{
		FVector ShockwaveSpawnLocation = NewLocation + InNormal * 10.f;
		FRotator RotationOffset = FRotator(90.f, 0.f, 0.f);
		FRotator ShockwaveSpawnRotation = UKismetMathLibrary::MakeRotFromX(InNormal) + RotationOffset;
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PortalTwoShockwave, ShockwaveSpawnLocation, ShockwaveSpawnRotation);
	}
	else
	{
		FVector ShockwaveSpawnLocation = NewLocation + InNormal * 10.f;
		FRotator RotationOffset = FRotator(90.f, 0.f, 0.f);
		FRotator ShockwaveSpawnRotation = UKismetMathLibrary::MakeRotFromX(InNormal) + RotationOffset;
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PortalOneShockwave, ShockwaveSpawnLocation, ShockwaveSpawnRotation);
	}
}
