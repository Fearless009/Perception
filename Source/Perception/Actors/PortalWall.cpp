#include "PortalWall.h"

APortalWall::APortalWall()
{
	PrimaryActorTick.bCanEverTick = false;

	PortalWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalWallMesh"));
	SetRootComponent(PortalWallMesh);
}

void APortalWall::BeginPlay()
{
	Super::BeginPlay();
	
}

void APortalWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
