#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interfaces/PickupObjectInterface.h"
#include "../Interfaces/PortalJumper.h"
#include "PickupObjectBase.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;

UCLASS()
class PERCEPTION_API APickupObjectBase : public AActor, public IPickupObjectInterface, public IPortalJumper
{
	GENERATED_BODY()
	
public:	
	APickupObjectBase();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetIsGrabbed() { return bIsGrabbed; };

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshCopy;

	bool bIsGrabbed = false;

	UFUNCTION()
	virtual void PreviewDuplicateObject_Implementation(USceneComponent* EntryPortal, USceneComponent* ExitPortal);

	UFUNCTION()
	virtual void DeactivateDuplicateObject_Implementation();

	UFUNCTION()
	virtual void SetIgnoreDuplicateDrop_Implementation(bool Enable);

	UFUNCTION()
	virtual void SetPickupObjectMeshVisibility_Implementation(bool Enable, bool ForceEnable);

	UFUNCTION()
	virtual void SetPickupObjectHiddenIngame_Implementation(bool Hidden);

	UFUNCTION()
	virtual	void SetIgnoreInternalPortalCollider_Implementation(bool bIgnore);

	UFUNCTION()
	virtual void SetPickupObjectCollisionToPortalWall_Implementation(bool Enable);

	UFUNCTION()
	virtual void SetPickupObjectOverlapWithPortal_Implementation(bool Enable);

	UFUNCTION()
	virtual void PlayerJumpedPortalWithGrabbedObject_Implementation();

	UFUNCTION()
	virtual void SetIsGrabbed_Implementation(bool InIsGrabbed);

private:
	bool bDuplicatedActivated = false;
	USceneComponent* EntryPortalDoor;
	USceneComponent* ExitPortalDoor;
	bool bIgnoreDuplicateDrop = false;
	bool bIgnoreInternalPortalCollider = false;
};
