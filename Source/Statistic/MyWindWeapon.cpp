// Fill out your copyright notice in the Description page of Project Settings.


#include "MyWindWeapon.h"

AMyWindWeapon::AMyWindWeapon()
{
    WeaponType = EWeaponType::WT_Wind;
    BaseSocketName = TEXT("WindPosition");
}