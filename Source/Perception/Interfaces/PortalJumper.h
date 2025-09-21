#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PortalJumper.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPortalJumper : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PERCEPTION_API IPortalJumper
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Portal")
	void OnPortalJump(const FVector& TargetLocation, const FRotator& TargetRotation);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Portal")
	void PreviewDuplicateObject(USceneComponent* EntryPortal, USceneComponent* ExitPortal);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Portal")
	void DeactivateDuplicateObject();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Portal")
	void SetIgnoreDuplicateDrop(bool Enable);

	//need to delete this maybe
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Portal")
	void SetPickupObjectMeshVisibility(bool Enable, bool ForceEnable);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Portal")
	void SetPickupObjectHiddenIngame(bool Hidden);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Portal")
	void SetIgnoreInternalPortalCollider(bool bIgnore);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Portal")
	void SetPickupObjectCollisionToPortalWall(bool Enable);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Portal")
	void SetPickupObjectOverlapWithPortal(bool Enable);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Portal")
	void PlayerJumpedPortalWithGrabbedObject();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Portal")
	void SetIsGrabbed(bool InIsGrabbed);
};
