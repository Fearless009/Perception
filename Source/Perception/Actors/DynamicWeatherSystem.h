#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DynamicWeatherSystem.generated.h"

UCLASS()
class PERCEPTION_API ADynamicWeatherSystem : public AActor
{
	GENERATED_BODY()
	
public:	
	ADynamicWeatherSystem();

	UFUNCTION(BlueprintImplementableEvent, Category = "Weather")
	void EnablePresentWeather();

	UFUNCTION(BlueprintImplementableEvent, Category = "Weather")
	void EnablePastWeather();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
