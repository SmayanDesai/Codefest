#include "HealthComponent.h"

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentHP = MaxHP;
}

bool UHealthComponent::EvaluateDamage(float Damage)
{
    if (Damage <= 0.f || CurrentHP <= 0.f)
        return false;

    CurrentHP -= Damage;

    if (CurrentHP <= 0.f)
    {
        CurrentHP = 0.f;
        return true;
    }

    return false;
}
