#include "Door.h"
#include "Curves/CurveFloat.h"

ADoor::ADoor()
{
	PrimaryActorTick.bCanEverTick = true;

	DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
	SetRootComponent(DoorFrame);

	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Door->SetupAttachment(GetRootComponent());
}

void ADoor::BeginPlay()
{
	Super::BeginPlay();

	if (DoorCurve)
	{
		FOnTimelineFloat UpdateDelegate;
		UpdateDelegate.BindUFunction(this, FName("DoorOnTimelineUpdate"));
		DoorTimeline.AddInterpFloat(DoorCurve, UpdateDelegate);

		DoorTimeline.SetLooping(false);
	}
	
}

void ADoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DoorTimeline.TickTimeline(DeltaTime);
}

void ADoor::OpenDoor()
{
	if (DoorCurve)
	{
		DoorTimeline.Play();
	}
}

void ADoor::CloseDoor()
{
	if (DoorCurve)
	{
		DoorTimeline.Reverse();
	}
}


void ADoor::DoorOnTimelineUpdate(float Alpha)
{
	const float DoorRotationZ = FMath::Lerp(0.f, 90.f, Alpha);
	FRotator NewRotation = FRotator(0.f, DoorRotationZ, 0.f);
	Door->SetRelativeRotation(NewRotation);
}


