// Fill out your copyright notice in the Description page of Project Settings.


#include "MyFireWeapon.h"

AMyFireWeapon::AMyFireWeapon()
{
	WeaponType = EWeaponType::WT_Fire;
	BaseSocketName = TEXT("FirePosition");
}