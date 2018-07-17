// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "BitStream.h"
#include "RealTimeSrvHelper.h"
#include <memory>
#include "RealTimeSrvEntity.generated.h"






UCLASS()
class ARealTimeSrvEntity : public APawn
{
	GENERATED_BODY()

public:
	// 'GOBJ' = 1196376650;
	virtual uint32_t GetClassId() const /*{ return 'GOBJ'; }*/ ;

	enum EReplicationState
	{
		EPS_Pose = 1 << 0,
		EPS_PlayerId = 1 << 1,
		//ECRS_Health = 1 << 2,

		EPS_AllState = EPS_Pose
		| EPS_PlayerId
		//| ECRS_Health
	};

public:
	virtual void	Update() {}
	virtual void	UpdateTargetState() {}

	virtual uint32_t	Write( OutputBitStream& inOutputStream, uint32_t inDirtyState ) const { return 0; }
	virtual void		Read( InputBitStream& inInputStream ) {}

	void			SetLocalVelocity( const FVector& inVelocity ) { mLocalVelocity = inVelocity; }
	const FVector&	GetLocalVelocity()						const { return mLocalVelocity; }

	const FVector&	GetServerVelocity()						const { return mVelocity; }


	UFUNCTION( BlueprintCallable, Category = "RealTimeSrv" )
		virtual FVector GetVelocity() const override { return GetLocalVelocity(); }

	
	void SetVelocity(const FVector& inVelocity) { mVelocity = inVelocity; }

	void	SetIndexInWorld( int inIndex ) { mIndexInWorld = inIndex; }
	int		GetIndexInWorld()				const { return mIndexInWorld; }


	bool		DoesWantToDie()				const				{ return mDoesWantToDie; }
	void		SetDoesWantToDie( bool inWants )				{ mDoesWantToDie = inWants; }

	virtual void	HandleDying() {  Destroy(); }

	const FVector&		GetLocation()				const { return mLocation; }
	void		SetLocation( const FVector& inLocation ) { mLocation = inLocation; }

	void	SetRotation( const FRotator& inRotation ) { mRotation = inRotation; };
	FRotator	GetRotation()					const { return mRotation; }


	int			GetNetworkId()				const { return mNetworkId; }
	void		SetNetworkId( int inNetworkId ) { mNetworkId = inNetworkId;  }

protected:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ActionPawnMovement )
		FVector mLocalVelocity;


	FRotator mLocalRotation;

	FVector mLocalLocation;

protected:

	FVector											mVelocity;
	FVector											mLocation;
	FVector											mColor;

	FRotator										mRotation;

	float											mScale;
	int												mIndexInWorld;

	bool											mDoesWantToDie;

	int												mNetworkId;

protected:

	///move down here for padding reasons...

	float				mLastMoveTimestamp;

	//float				mThrustDir;
	int					mHealth;

	bool				mIsShooting;

	float mIsTimeToStartSimulateMovementForRemotePawn;
};

typedef ARealTimeSrvEntity*	RealTimeSrvEntityPtr;