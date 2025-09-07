#include "WeightButton.h"
#include "Components/SphereComponent.h"
#include "Curves/CurveFloat.h"
#include "../Interfaces/PickupObjectInterface.h"
#include "../Actors/Door.h"

AWeightButton::AWeightButton()
{
	PrimaryActorTick.bCanEverTick = true;

	ButtonPedastal = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonPedastal"));
	SetRootComponent(ButtonPedastal);

	Button = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Button"));
	Button->SetupAttachment(GetRootComponent());

	OverlapCollider = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapCollider"));
	OverlapCollider->SetupAttachment(GetRootComponent());

}

void AWeightButton::BeginPlay()
{
	Super::BeginPlay();

	OverlapCollider->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::Collider_OnComponentBeginOverlap);
	OverlapCollider->OnComponentEndOverlap.AddDynamic(this, &ThisClass::Collider_OnComponentEndOverlap);

	if (ButtonCurve)
	{
		FOnTimelineFloat UpdateDelegate;
		UpdateDelegate.BindUFunction(this, FName("OnButtonTimelineUpdate"));
		ButtonTimeline.AddInterpFloat(ButtonCurve, UpdateDelegate);

		FOnTimelineEvent FinishedDelegate;
		FinishedDelegate.BindUFunction(this, FName("OnButtonTimelineFinished"));
		ButtonTimeline.SetTimelineFinishedFunc(FinishedDelegate);

		ButtonTimeline.SetLooping(false);
	}
}



void AWeightButton::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ButtonTimeline.TickTimeline(DeltaTime);

}

void AWeightButton::Collider_OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->GetClass()->ImplementsInterface(UPickupObjectInterface::StaticClass()))
	{
		if (ButtonCurve)
		{
			ButtonTimeline.Play();
		}

		WeightOverlapped = true;
	}
}

void AWeightButton::Collider_OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->GetClass()->ImplementsInterface(UPickupObjectInterface::StaticClass()))
	{
		if (ButtonCurve)
		{
			ButtonTimeline.Reverse();
		}

		WeightOverlapped = false;
	}
}

void AWeightButton::OnButtonTimelineUpdate(float Alpha)
{
	float ButtonZLocation = FMath::Lerp(0.f, -11.f, Alpha);
	Button->SetRelativeLocation(FVector(0.f, 0.f, ButtonZLocation));

	if (!bAvoidTimelineTick)
	{
		bAvoidTimelineTick = true;
		SetButtonEmissiveColor(false);
		if (DoorToOpen)
		{
			DoorToOpen->CloseDoor();
		}
	}
}

void AWeightButton::OnButtonTimelineFinished()
{
	bAvoidTimelineTick = false;

	if (WeightOverlapped)
	{
		SetButtonEmissiveColor(true);
		if (DoorToOpen)
		{
			DoorToOpen->OpenDoor();
		}
	}
}

