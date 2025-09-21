#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalWall.generated.h"

UCLASS()
class PERCEPTION_API APortalWall : public AActor
{
	GENERATED_BODY()
	
public:	
	APortalWall();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PortalWallMesh;

public:	
};
