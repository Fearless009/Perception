#include "DynamicWeatherSystem.h"

ADynamicWeatherSystem::ADynamicWeatherSystem()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ADynamicWeatherSystem::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADynamicWeatherSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

