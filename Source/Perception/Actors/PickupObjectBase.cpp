#include "PickupObjectBase.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

APickupObjectBase::APickupObjectBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	MeshCopy = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshCopy"));
	MeshCopy->SetupAttachment(GetRootComponent());
}

void APickupObjectBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void APickupObjectBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDuplicatedActivated && EntryPortalDoor)
	{
		FTransform MainObjectWorldTransform = Mesh->GetComponentTransform();
		FTransform EntryDoorWorldTransform = EntryPortalDoor->GetComponentTransform();
		FTransform RelativeTransform = UKismetMathLibrary::MakeRelativeTransform(MainObjectWorldTransform, EntryDoorWorldTransform);
		FVector RotatedVector = UKismetMathLibrary::RotateAngleAxis(RelativeTransform.GetLocation(), 180.f, EntryPortalDoor->GetUpVector());
		FRotator FixedRotation = UKismetMathLibrary::ComposeRotators(RelativeTransform.GetRotation().Rotator(), FRotator(0.f, 180.f, 0.f));
		FTransform NewRelativeTransform = FTransform(FixedRotation, RotatedVector, RelativeTransform.GetScale3D());
		MeshCopy->SetRelativeTransform(NewRelativeTransform);
	}

	//GEngine->AddOnScreenDebugMessage(3000, 1.f, FColor::Red, FString::Printf(TEXT("bIgnoreDuplicateDrop: %s"), bIgnoreDuplicateDrop ? TEXT("True") : TEXT("False")));
	//GEngine->AddOnScreenDebugMessage(3001, 1.f, FColor::Red, FString::Printf(TEXT("bIgnoreInternalPortalCollider: %s"), bIgnoreInternalPortalCollider ? TEXT("True") : TEXT("False")));
}

void APickupObjectBase::PreviewDuplicateObject_Implementation(USceneComponent* EntryPortal, USceneComponent* ExitPortal)
{
	if (bIgnoreInternalPortalCollider)
	{
		return;
	}
	//GEngine->AddOnScreenDebugMessage(0, 2.f, FColor::Cyan, "PreviewDuplicateObject_Implementation called");
	EntryPortalDoor = EntryPortal;
	ExitPortalDoor = ExitPortal;
	MeshCopy->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	bDuplicatedActivated = true;
	MeshCopy->SetHiddenInGame(false);
	MeshCopy->AttachToComponent(ExitPortalDoor, FAttachmentTransformRules::KeepWorldTransform);
}

void APickupObjectBase::DeactivateDuplicateObject_Implementation()
{
	//GEngine->AddOnScreenDebugMessage(0, 2.f, FColor::Cyan, "DeactivateDuplicateObject_Implementation called");
	Mesh->SetHiddenInGame(false);
	bIgnoreInternalPortalCollider = false;
	Mesh->SetHiddenInGame(false);
	if (bIgnoreDuplicateDrop)
	{
		bDuplicatedActivated = false;
		MeshCopy->SetHiddenInGame(true);
		MeshCopy->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
		MeshCopy->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
		return;
	}

	if (bDuplicatedActivated)
	{
		bDuplicatedActivated = false;
		MeshCopy->SetHiddenInGame(true);
		FTransform DuplicateObjectWorldTransform = MeshCopy->GetComponentTransform();
		Mesh->SetWorldTransform(DuplicateObjectWorldTransform);
		MeshCopy->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
		MeshCopy->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	}
}

void APickupObjectBase::SetPickupObjectMeshVisibility_Implementation(bool Enable, bool ForceEnable)
{
	if (ForceEnable)
	{
		Mesh->SetHiddenInGame(false);
		return;
	}

	if (!Enable && !bIgnoreInternalPortalCollider) // Hide visibility + don't ignore internal colliders
	{
		Mesh->SetHiddenInGame(true);
		bIgnoreInternalPortalCollider = true;
	}
	else // Show visibility
	{
		Mesh->SetHiddenInGame(false);
	}
}

void APickupObjectBase::SetPickupObjectHiddenIngame_Implementation(bool Hidden)
{
	Mesh->SetHiddenInGame(Hidden);
}

void APickupObjectBase::SetIgnoreDuplicateDrop_Implementation(bool Enable)
{
	bIgnoreDuplicateDrop = Enable;
}

void APickupObjectBase::SetIgnoreInternalPortalCollider_Implementation(bool bIgnore)
{
	bIgnoreInternalPortalCollider = bIgnore;
}

void APickupObjectBase::SetPickupObjectCollisionToPortalWall_Implementation(bool Enable)
{
	// ECC_GameTraceChannel3 = PortalWall
	if (Enable)
	{
		Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECollisionResponse::ECR_Block);
	}
	else
	{
		Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECollisionResponse::ECR_Ignore);
	}
}

void APickupObjectBase::SetPickupObjectOverlapWithPortal_Implementation(bool Enable)
{
	//ECC_GameTraceChannel2 = InternalPortalColliders
	//ECC_GameTraceChannel4 = ExternalPortalColliders
	if (Enable)
	{
		Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Overlap);
		Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel4, ECollisionResponse::ECR_Overlap);
	}
	else
	{
		Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Block);
		Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel4, ECollisionResponse::ECR_Block);
	}
}

void APickupObjectBase::PlayerJumpedPortalWithGrabbedObject_Implementation()
{
	if (!GetWorld())
	{
		return;
	}
	Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Mesh->SetHiddenInGame(false);
	FTimerHandle TempHandle;
	auto EnableCollision = [this, &TempHandle]() 
		{	
			Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
			Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Overlap);
			Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		};
	const float Delay = 0.2f;
	GetWorld()->GetTimerManager().SetTimer(TempHandle, EnableCollision, Delay, false);
}

void APickupObjectBase::SetIsGrabbed_Implementation(bool InIsGrabbed)
{
	bIsGrabbed = InIsGrabbed;
}
