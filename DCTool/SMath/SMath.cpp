#include "../App.h"

SGlobalMath::SGlobalMath()
{
	// Init base angle table.
	for (INT i = 0; i < NUM_ANGLES; i++) {
		TrigFLOAT[i] = appSin((float)i * 2.f * PI / (float)NUM_ANGLES);
	}
}

INT GSRandSeed;
SGlobalMath g_SGlobalMath;

/** Utility to ensure angle is between +/- 180 degrees by unwinding. */
void UnwindDegreeComponent(float& A)
{
	while (A > 180.f)
	{
		A -= 360.f;
	}

	while (A < -180.f)
	{
		A += 360.f;
	}
}
void SVector::UnwindEuler()
{
	UnwindDegreeComponent(X);
	UnwindDegreeComponent(Y);
	UnwindDegreeComponent(Z);
}

SVector VRandCone(SVector const & Dir, float ConeHalfAngleRad)
{
	if (ConeHalfAngleRad > 0.f)
	{
		float const RandU = appFrand();
		float const RandV = appFrand();

		// Get spherical coords that have an even distribution over the unit sphere
		// Method described at http://mathworld.wolfram.com/SpherePointPicking.html	
		float Theta = 2.f * PI * RandU;
		float Phi = appAcos((2.f * RandV) - 1.f);

		// restrict phi to [0, ConeHalfAngleRad]
		// this gives an even distribution of points on the surface of the cone
		// centered at the origin, pointing upward (z), with the desired angle
		Phi = appFmod(Phi, ConeHalfAngleRad);

		// get axes we need to rotate around
		SMatrix const DirMat = SRotationMatrix(SVectorToRotation(Dir));
		// note the axis translation, since we want the variation to be around X
		SVector const DirZ = DirMat.GetAxis(0);
		SVector const DirY = DirMat.GetAxis(1);

		// convert to unreal rot units, to satisfy RotateAngleAxis
		float const Rad2Unr = 65536.f / (2.f*PI);
		SVector Result = Dir.RotateAngleAxis(appTrunc(Rad2Unr*Phi), DirY);
		Result = Result.RotateAngleAxis(appTrunc(Rad2Unr*Theta), DirZ);

		// ensure it's a unit vector (might not have been passed in that way)
		Result = Result.SafeNormal();

		return Result;
	}
	else
	{
		return Dir.SafeNormal();
	}
}

SVector VRandCone(SVector const & Dir, float HorizontalConeHalfAngleRad, float VerticalConeHalfAngleRad)
{
	if ((VerticalConeHalfAngleRad > 0.f) && (HorizontalConeHalfAngleRad > 0.f))
	{
		float const RandU = appFrand();
		float const RandV = appFrand();

		// Get spherical coords that have an even distribution over the unit sphere
		// Method described at http://mathworld.wolfram.com/SpherePointPicking.html	
		float Theta = 2.f * PI * RandU;
		float Phi = appAcos((2.f * RandV) - 1.f);

		// restrict phi to [0, ConeHalfAngleRad]
		// where ConeHalfAngleRad is now a function of Theta
		// (specifically, radius of an ellipse as a function of angle)
		// function is ellipse function (x/a)^2 + (y/b)^2 = 1, converted to polar coords
		float ConeHalfAngleRad = Square(appCos(Theta) / HorizontalConeHalfAngleRad) + Square(appSin(Theta) / VerticalConeHalfAngleRad);
		ConeHalfAngleRad = appSqrt(1.f / ConeHalfAngleRad);

		// clamp to make a cone instead of a sphere
		Phi = appFmod(Phi, ConeHalfAngleRad);

		// get axes we need to rotate around
		SMatrix const DirMat = SRotationMatrix(SVectorToRotation(Dir));
		// note the axis translation, since we want the variation to be around X
		SVector const DirZ = DirMat.GetAxis(0);
		SVector const DirY = DirMat.GetAxis(1);

		// convert to unreal rot units, to satisfy RotateAngleAxis
		float const Rad2Unr = 65536.f / (2.f*PI);
		SVector Result = Dir.RotateAngleAxis(appTrunc(Rad2Unr*Phi), DirY);
		Result = Result.RotateAngleAxis(appTrunc(Rad2Unr*Theta), DirZ);

		// ensure it's a unit vector (might not have been passed in that way)
		Result = Result.SafeNormal();

		return Result;
	}
	else
	{
		return Dir.SafeNormal();
	}
}

SVector ClosestPointOnLine(const SVector & LineStart, const SVector & LineEnd, const SVector & Point)
{
	// Solve to find alpha along line that is closest point
	// Weisstein, Eric W. "Point-Line Distance--3-Dimensional." From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html 
	const float A = (LineStart - Point) | (LineEnd - LineStart);
	const float B = (LineEnd - LineStart).SizeSquared();
	const float T = ::Clamp(-A / B, 0.f, 1.f);

	// Generate closest point
	SVector ClosestPoint = LineStart + ((LineEnd - LineStart) * T);

	return ClosestPoint;
}

SVector CalcAngularVelocity(SRotator const& OldRot, SRotator const& NewRot, float DeltaTime)
{
	SVector RetAngVel(0.f);

	if (OldRot != NewRot)
	{
		float const InvDeltaTime = 1.f / DeltaTime;
		SQuat const DeltaQRot = SRotatorToSQuat(NewRot - OldRot);

		SVector Axis;
		float Angle;
		DeltaQRot.ToAxisAndAngle(Axis, Angle);

		RetAngVel = Axis * Angle * InvDeltaTime;
	}

	return RetAngVel;
}

SQuat SQuatFindBetween(const SVector& vec1, const SVector& vec2)
{
	const SVector cross = vec1 ^ vec2;
	const float crossMag = cross.Size();

	// See if vectors are parallel or anti-parallel
	if (crossMag < KINDA_SMALL_NUMBER)
	{
		// If these vectors are parallel - just return identity quaternion (ie no rotation).
		const float Dot = vec1 | vec2;
		if (Dot > -KINDA_SMALL_NUMBER)
		{
			return SQuat::Identity; // no rotation
		}
		// Exactly opposite..
		else
		{
			// ..rotation by 180 degrees around a vector orthogonal to vec1 & vec2
			SVector Vec = vec1.SizeSquared() > vec2.SizeSquared() ? vec1 : vec2;
			Vec.Normalize();

			SVector AxisA, AxisB;
			Vec.FindBestAxisVectors(AxisA, AxisB);

			return SQuat(AxisA.X, AxisA.Y, AxisA.Z, 0.f); // (axis*sin(pi/2), cos(pi/2)) = (axis, 0)
		}
	}

	// Not parallel, so use normal code
	float angle = appAsin(crossMag);

	const float dot = vec1 | vec2;
	if (dot < 0.0f)
	{
		angle = PI - angle;
	}

	const float sinHalfAng = appSin(0.5f * angle);
	const float cosHalfAng = appCos(0.5f * angle);
	const SVector axis = cross / crossMag;

	return SQuat(
		sinHalfAng * axis.X,
		sinHalfAng * axis.Y,
		sinHalfAng * axis.Z,
		cosHalfAng);
}

/** Interpolate a normal vector Current to Target, by interpolating the angle between those vectors with constant step. */
SVector VInterpNormalRotationTo(const SVector& Current, const SVector& Target, float DeltaTime, float RotationSpeedDegrees)
{
	// Find delta rotation between both normals.
	SQuat DeltaQuat = SQuatFindBetween(Current, Target);

	// Decompose into an axis and angle for rotation
	SVector DeltaAxis(0.f);
	float DeltaAngle = 0.f;
	DeltaQuat.ToAxisAndAngle(DeltaAxis, DeltaAngle);

	// Find rotation step for this frame
	const float RotationStepRadians = RotationSpeedDegrees * (PI / 180) * DeltaTime;

	if (Abs(DeltaAngle) > RotationStepRadians)
	{
		DeltaAngle = Clamp(DeltaAngle, -RotationStepRadians, RotationStepRadians);
		DeltaQuat = SQuat(DeltaAxis, DeltaAngle);
		return SQuatRotationTranslationMatrix(DeltaQuat, SVector(0.f)).TransformNormal(Current).ToVector();
	}

	return Target;
}

/** Interpolate a normal vector from Current to Target with constant step */
SVector VInterpNormalConstantTo(const SVector Current, const SVector & Target, float DeltaTime, float InterpSpeed)
{
	const SVector Delta = Target - Current;
	const float DeltaM = Delta.Size();
	const float MaxStep = InterpSpeed * DeltaTime;

	if (DeltaM > MaxStep)
	{
		if (MaxStep > 0.f)
		{
			const SVector DeltaN = Delta / DeltaM;
			return (Current + DeltaN * MaxStep).SafeNormal();
		}
		else
		{
			return Current;
		}
	}

	return Target;
}

/** Interpolate vector from Current to Target with constant step */
SVector VInterpConstantTo(const SVector Current, const SVector & Target, float DeltaTime, float InterpSpeed)
{
	const SVector Delta = Target - Current;
	const float DeltaM = Delta.Size();
	const float MaxStep = InterpSpeed * DeltaTime;

	if (DeltaM > MaxStep)
	{
		if (MaxStep > 0.f)
		{
			const SVector DeltaN = Delta / DeltaM;
			return Current + DeltaN * MaxStep;
		}
		else
		{
			return Current;
		}
	}

	return Target;
}

/** Interpolate vector from Current to Target. Scaled by distance to Target, so it has a strong start speed and ease out. */
SVector VInterpTo(const SVector & Current, const SVector & Target, float & DeltaTime, float InterpSpeed)
{
	// If no interp speed, jump to target value
	if (InterpSpeed <= 0.f)
	{
		return Target;
	}

	// Distance to reach
	const SVector Dist = Target - Current;

	// If distance is too small, just set the desired location
	if (Dist.SizeSquared() < KINDA_SMALL_NUMBER)
	{
		return Target;
	}

	// Delta Move, Clamp so we do not over shoot.
	const SVector	DeltaMove = Dist * Clamp<float>(DeltaTime * InterpSpeed, 0.f, 1.f);

	return Current + DeltaMove;
}
/** Interpolate vector from Current to Target. Scaled by distance to Target, so it has a strong start speed and ease out. */

SVector VClamp(SVector A, SVector Min, SVector Max)
{
	A.X = Clamp<float>(A.X, Min.X, Max.X);
	A.Y = Clamp<float>(A.Y, Min.Y, Max.Y);
	A.Z = Clamp<float>(A.Z, Min.Z, Max.Z);
	return A;
}

SVector SRotator::Euler() const
{
	return SVector(Roll * (180.f / 32768.f), Pitch * (180.f / 32768.f), Yaw * (180.f / 32768.f));
}

SVector SRotator::Vector() const
{
	return SRotationMatrix(*this).GetAxis(0);
}

SVector SQuat::Euler() const
{
	return SQuatRotationTranslationMatrix(*this, SVector(0.f)).Rotator().Euler();
}

//Zero angle.	
const SAngle SAngle::Zero;

//Zero vector.
const SVector SVector::Zero;

//Zero vector.
const SVector SVector::ZeroVector;

//Unit vector.
const SVector SVector::Unit = SVector(1.0f, 1.0f, 1.0f);

//Unit quarternion.
const SQuat	SQuat::Identity = SQuat(1.0f, 1.0f, 1.0f, 1.0f);

/*-----------------------------------------------------------------------------
	SRotator.
-----------------------------------------------------------------------------*/
SRotator SMatrix::Rotator() const
{
	const SVector		XAxis = GetAxis(0);
	const SVector		YAxis = GetAxis(1);
	const SVector		ZAxis = GetAxis(2);

	SRotator	Rotator = SRotator(
		appRound(appAtan2(XAxis.Z, appSqrt(Square(XAxis.X) + Square(XAxis.Y))) * 32768.f / PI),
		appRound(appAtan2(XAxis.Y, XAxis.X) * 32768.f / PI),
		0
	);

	const SVector		SYAxis = SRotationMatrix(Rotator).GetAxis(1);
	Rotator.Roll = appRound(appAtan2(ZAxis | SYAxis, YAxis | SYAxis) * 32768.f / PI);
	return Rotator;
}

SRotator SRotator::MakeFromEuler(const SVector & Euler)
{
	return SRotator(appTrunc(Euler.Y * (32768.f / 180.f)), appTrunc(Euler.Z * (32768.f / 180.f)), appTrunc(Euler.X * (32768.f / 180.f)));
}

SRotator SQuat::Rotator() const
{
	return SQuatRotationTranslationMatrix(*this, SVector(0.f)).Rotator();
}

void SRotator::MakeShortestRoute()
{
	//// YAW

	// Clip rotation to [0,65535] range
	Yaw = Yaw & 65535;

	// Then ensure it takes the 'shortest' route.
	if (Yaw >= 32768)
		Yaw -= 65536;

	//// PITCH
	Pitch = Pitch & 65535;

	if (Pitch >= 32768)
		Pitch -= 65536;

	//// ROLL
	Roll = Roll & 65535;

	if (Roll >= 32768)
		Roll -= 65536;
}

void SRotator::GetWindingAndRemainder(SRotator & Winding, SRotator & Remainder) const
{
	//// YAW
	Remainder.Yaw = Yaw & 65535;
	if (Remainder.Yaw >= 32768)
	{
		Remainder.Yaw -= 65536;
	}

	Winding.Yaw = Yaw - Remainder.Yaw;

	//// PITCH
	Remainder.Pitch = Pitch & 65535;
	if (Remainder.Pitch >= 32768)
	{
		Remainder.Pitch -= 65536;
	}

	Winding.Pitch = Pitch - Remainder.Pitch;

	//// ROLL
	Remainder.Roll = Roll & 65535;
	if (Remainder.Roll >= 32768)
	{
		Remainder.Roll -= 65536;
	}

	Winding.Roll = Roll - Remainder.Roll;
}

SRotator RInterpTo(const SRotator & Current, const SRotator & Target, float & DeltaTime, float InterpSpeed, BOOL bConstantInterpSpeed)
{
	// if DeltaTime is 0, do not perform any interpolation (Location was already calculated for that frame)
	if (DeltaTime == 0.f || Current == Target)
	{
		return Current;
	}

	// If no interp speed, jump to target value
	if (InterpSpeed <= 0.f)
	{
		return Target;
	}

	const float DeltaInterpSpeed = InterpSpeed * DeltaTime;
	if (!bConstantInterpSpeed)
	{
		// Delta Move, Clamp so we do not over shoot.
		const SRotator DeltaMove = (Target - Current).GetNormalized() * Clamp<float>(DeltaInterpSpeed, 0.f, 1.f);

		// If steps are too small, just return Target and consider we've reached our destination.
		if (DeltaMove.IsZero())
		{
			return Target;
		}

		return (Current + DeltaMove).GetNormalized();
	}
	else
	{
		const SRotator DeltaMove = (Target - Current).GetNormalized();
		SRotator Result = Current;
		INT DeltaInterpSpeedInt = appTrunc(DeltaInterpSpeed);
		Result.Pitch += Clamp(DeltaMove.Pitch, -DeltaInterpSpeedInt, DeltaInterpSpeedInt);
		Result.Yaw += Clamp(DeltaMove.Yaw, -DeltaInterpSpeedInt, DeltaInterpSpeedInt);
		Result.Roll += Clamp(DeltaMove.Roll, -DeltaInterpSpeedInt, DeltaInterpSpeedInt);
		return Result.GetNormalized();
	}
}


/*-----------------------------------------------------------------------------
	SBox.
-----------------------------------------------------------------------------*/
SBox::SBox(const SVector * Points, INT Count)
	: Min(0, 0, 0), Max(0, 0, 0), IsValid(0)
{
	for (INT i = 0; i < Count; i++)
	{
		*this += Points[i];
	}
}

//SBox SBox::TransformBy(const SMatrix & M) const
//{
//	// If we are not valid, return another invalid box.
//	if (!IsValid)
//	{
//		return SBox(0);
//	}
//
//	VectorRegister Vertices[8];
//	VectorRegister m0 = VectorLoadAligned(M.M[0]);
//	VectorRegister m1 = VectorLoadAligned(M.M[1]);
//	VectorRegister m2 = VectorLoadAligned(M.M[2]);
//	VectorRegister m3 = VectorLoadAligned(M.M[3]);
//	Vertices[0] = VectorLoadFloat3(&Min);
//	Vertices[1] = VectorSetFloat3(Min.X, Min.Y, Max.Z);
//	Vertices[2] = VectorSetFloat3(Min.X, Max.Y, Min.Z);
//	Vertices[3] = VectorSetFloat3(Max.X, Min.Y, Min.Z);
//	Vertices[4] = VectorSetFloat3(Max.X, Max.Y, Min.Z);
//	Vertices[5] = VectorSetFloat3(Max.X, Min.Y, Max.Z);
//	Vertices[6] = VectorSetFloat3(Min.X, Max.Y, Max.Z);
//	Vertices[7] = VectorLoadFloat3(&Max);
//	VectorRegister r0 = VectorMultiply(VectorReplicate(Vertices[0], 0), m0);
//	VectorRegister r1 = VectorMultiply(VectorReplicate(Vertices[1], 0), m0);
//	VectorRegister r2 = VectorMultiply(VectorReplicate(Vertices[2], 0), m0);
//	VectorRegister r3 = VectorMultiply(VectorReplicate(Vertices[3], 0), m0);
//	VectorRegister r4 = VectorMultiply(VectorReplicate(Vertices[4], 0), m0);
//	VectorRegister r5 = VectorMultiply(VectorReplicate(Vertices[5], 0), m0);
//	VectorRegister r6 = VectorMultiply(VectorReplicate(Vertices[6], 0), m0);
//	VectorRegister r7 = VectorMultiply(VectorReplicate(Vertices[7], 0), m0);
//
//	r0 = VectorMultiplyAdd(VectorReplicate(Vertices[0], 1), m1, r0);
//	r1 = VectorMultiplyAdd(VectorReplicate(Vertices[1], 1), m1, r1);
//	r2 = VectorMultiplyAdd(VectorReplicate(Vertices[2], 1), m1, r2);
//	r3 = VectorMultiplyAdd(VectorReplicate(Vertices[3], 1), m1, r3);
//	r4 = VectorMultiplyAdd(VectorReplicate(Vertices[4], 1), m1, r4);
//	r5 = VectorMultiplyAdd(VectorReplicate(Vertices[5], 1), m1, r5);
//	r6 = VectorMultiplyAdd(VectorReplicate(Vertices[6], 1), m1, r6);
//	r7 = VectorMultiplyAdd(VectorReplicate(Vertices[7], 1), m1, r7);
//
//	r0 = VectorMultiplyAdd(VectorReplicate(Vertices[0], 2), m2, r0);
//	r1 = VectorMultiplyAdd(VectorReplicate(Vertices[1], 2), m2, r1);
//	r2 = VectorMultiplyAdd(VectorReplicate(Vertices[2], 2), m2, r2);
//	r3 = VectorMultiplyAdd(VectorReplicate(Vertices[3], 2), m2, r3);
//	r4 = VectorMultiplyAdd(VectorReplicate(Vertices[4], 2), m2, r4);
//	r5 = VectorMultiplyAdd(VectorReplicate(Vertices[5], 2), m2, r5);
//	r6 = VectorMultiplyAdd(VectorReplicate(Vertices[6], 2), m2, r6);
//	r7 = VectorMultiplyAdd(VectorReplicate(Vertices[7], 2), m2, r7);
//
//	r0 = VectorAdd(r0, m3);
//	r1 = VectorAdd(r1, m3);
//	r2 = VectorAdd(r2, m3);
//	r3 = VectorAdd(r3, m3);
//	r4 = VectorAdd(r4, m3);
//	r5 = VectorAdd(r5, m3);
//	r6 = VectorAdd(r6, m3);
//	r7 = VectorAdd(r7, m3);
//
//	SBox NewBox;
//	VectorRegister min0 = VectorMin(r0, r1);
//	VectorRegister min1 = VectorMin(r2, r3);
//	VectorRegister min2 = VectorMin(r4, r5);
//	VectorRegister min3 = VectorMin(r6, r7);
//	VectorRegister max0 = VectorMax(r0, r1);
//	VectorRegister max1 = VectorMax(r2, r3);
//	VectorRegister max2 = VectorMax(r4, r5);
//	VectorRegister max3 = VectorMax(r6, r7);
//	min0 = VectorMin(min0, min1);
//	min1 = VectorMin(min2, min3);
//	max0 = VectorMax(max0, max1);
//	max1 = VectorMax(max2, max3);
//	min0 = VectorMin(min0, min1);
//	max0 = VectorMax(max0, max1);
//	VectorStoreFloat3(min0, &NewBox.Min);
//	VectorStoreFloat3(max0, &NewBox.Max);
//	NewBox.IsValid = 1;
//
//	return NewBox;
//}


/*-----------------------------------------------------------------------------
	SQuat.
-----------------------------------------------------------------------------*/
//SQuat::SQuat(const SMatrix & M)
//{
//	// If Matrix is NULL, return Identity quaternion.
//	if (M.GetAxis(0).IsNearlyZero() && M.GetAxis(1).IsNearlyZero() && M.GetAxis(2).IsNearlyZero())
//	{
//		*this = SQuat::Identity;
//		return;
//	}
//
//	//const MeReal *const t = (MeReal *) tm;
//	float	s;
//
//	// Check diagonal (trace)
//	const float tr = M.M[0][0] + M.M[1][1] + M.M[2][2];
//
//	if (tr > 0.0f)
//	{
//		float InvS = appInvSqrt(tr + 1.f);
//		this->W = 0.5f * (1.f / InvS);
//		s = 0.5f * InvS;
//
//		this->X = (M.M[1][2] - M.M[2][1]) * s;
//		this->Y = (M.M[2][0] - M.M[0][2]) * s;
//		this->Z = (M.M[0][1] - M.M[1][0]) * s;
//	}
//	else
//	{
//		// diagonal is negative
//		INT i = 0;
//
//		if (M.M[1][1] > M.M[0][0])
//			i = 1;
//
//		if (M.M[2][2] > M.M[i][i])
//			i = 2;
//
//		static const INT nxt[3] = { 1, 2, 0 };
//		const INT j = nxt[i];
//		const INT k = nxt[j];
//
//		s = M.M[i][i] - M.M[j][j] - M.M[k][k] + 1.0f;
//
//		float InvS = appInvSqrt(s);
//
//		float qt[4];
//		qt[i] = 0.5f * (1.f / InvS);
//
//		s = 0.5f * InvS;
//
//		qt[3] = (M.M[j][k] - M.M[k][j]) * s;
//		qt[j] = (M.M[i][j] + M.M[j][i]) * s;
//		qt[k] = (M.M[i][k] + M.M[k][i]) * s;
//
//		this->X = qt[0];
//		this->Y = qt[1];
//		this->Z = qt[2];
//		this->W = qt[3];
//	}
//}

//SQuat::SQuat(const SRotator & R)
//{
//	*this = MakeFromRotator(R);
//}

//SQuat SQuat::MakeFromEuler(const SVector & Euler)
//{
//	return SQuat(SRotationTranslationMatrix(SRotator::MakeFromEuler(Euler), SVector(0.f)));
//}

SQuat SQuat::Log() const
{
	SQuat Result;
	Result.W = 0.f;

	if (Abs(W) < 1.f)
	{
		const float Angle = appAcos(W);
		const float SinAngle = appSin(Angle);

		if (Abs(SinAngle) >= SMALL_NUMBER)
		{
			const float Scale = Angle / SinAngle;
			Result.X = Scale*X;
			Result.Y = Scale*Y;
			Result.Z = Scale*Z;

			return Result;
		}
	}

	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;

	return Result;
}

//SQuat SMatrix::ToQuat() const
//{
//	SQuat Result(*this);
//	return Result;
//}

//SQuat SRotator::Quaternion() const
//{
//	return SQuat(SRotationMatrix(*this));
//}

// Assumes a quaternion with W=0 and V=theta*v (where |v| = 1).
// Exp(q) = (sin(theta)*v, cos(theta))
SQuat SQuat::Exp() const
{
	const float Angle = appSqrt(X*X + Y*Y + Z*Z);
	const float SinAngle = appSin(Angle);

	SQuat Result;
	Result.W = appCos(Angle);

	if (Abs(SinAngle) >= SMALL_NUMBER)
	{
		const float Scale = SinAngle / Angle;
		Result.X = Scale*X;
		Result.Y = Scale*Y;
		Result.Z = Scale*Z;
	}
	else
	{
		Result.X = X;
		Result.Y = Y;
		Result.Z = Z;
	}

	return Result;
}

//SQuat SQuat::MakeFromRotator(const SRotator & rotator) const
//{
//	return SQuat(SRotationMatrix(rotator));
//}

/*-----------------------------------------------------------------------------
SSphere.
-----------------------------------------------------------------------------*/
SSphere::SSphere(const SVector * Pts, INT Count)
	: Center(0, 0, 0), W(0)
{
	if (Count)
	{
		const SBox Box(Pts, Count);
		*this = SSphere((Box.Min + Box.Max) / 2, 0);
		for (INT i = 0; i < Count; i++)
		{
			const float Dist = SDistSquared(Pts[i], Center);
			if (Dist > W)
			{
				W = Dist;
			}
		}
		W = appSqrt(W) * 1.001f;
	}
}



BOOL SPlaneAABBIsect(const SPlane & P, const SBox & AABB)
{
	// find diagonal most closely aligned with normal of plane
	SVector Vmin, Vmax;

	// Bypass the slow FVector[] operator. Not RESTRICT because it won't update Vmin, Vmax
	float* VminPtr = (float*)&Vmin;
	float* VmaxPtr = (float*)&Vmax;

	// Use restrict to get better instruction scheduling and to bypass the slow FVector[] operator
	const float* __restrict AABBMinPtr = (const float*)&AABB.Min;
	const float* __restrict AABBMaxPtr = (const float*)&AABB.Max;
	const float* __restrict PlanePtr = (const float*)&P;

	for (INT Idx = 0; Idx < 3; ++Idx)
	{
		if (PlanePtr[Idx] >= 0.f)
		{
			VminPtr[Idx] = AABBMinPtr[Idx];
			VmaxPtr[Idx] = AABBMaxPtr[Idx];
		}
		else
		{
			VminPtr[Idx] = AABBMaxPtr[Idx];
			VmaxPtr[Idx] = AABBMinPtr[Idx];
		}
	}

	// if either diagonal is right on the plane, or one is on either side we have an interesection
	float dMax = P.PlaneDot(Vmax);
	float dMin = P.PlaneDot(Vmin);

	// if Max is below plane, or Min is above we know there is no intersection.. otherwise there must be one
	return (dMax >= 0.f && dMin <= 0.f);
}

BOOL SLineExtentBoxIntersection(const SBox & inBox, const SVector & Start, const SVector & End, const SVector & Extent, SVector & HitLocation, SVector & HitNormal, float & HitTime)
{
	SBox box = inBox;
	box.Max.X += Extent.X;
	box.Max.Y += Extent.Y;
	box.Max.Z += Extent.Z;

	box.Min.X -= Extent.X;
	box.Min.Y -= Extent.Y;
	box.Min.Z -= Extent.Z;

	const SVector Dir = (End - Start);

	SVector	Time;
	BOOL	Inside = 1;
	float   faceDir[3] = { 1, 1, 1 };

	/////////////// X
	if (Start.X < box.Min.X)
	{
		if (Dir.X <= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			faceDir[0] = -1;
			Time.X = (box.Min.X - Start.X) / Dir.X;
		}
	}
	else if (Start.X > box.Max.X)
	{
		if (Dir.X >= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			Time.X = (box.Max.X - Start.X) / Dir.X;
		}
	}
	else
		Time.X = 0.0f;

	/////////////// Y
	if (Start.Y < box.Min.Y)
	{
		if (Dir.Y <= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			faceDir[1] = -1;
			Time.Y = (box.Min.Y - Start.Y) / Dir.Y;
		}
	}
	else if (Start.Y > box.Max.Y)
	{
		if (Dir.Y >= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			Time.Y = (box.Max.Y - Start.Y) / Dir.Y;
		}
	}
	else
		Time.Y = 0.0f;

	/////////////// Z
	if (Start.Z < box.Min.Z)
	{
		if (Dir.Z <= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			faceDir[2] = -1;
			Time.Z = (box.Min.Z - Start.Z) / Dir.Z;
		}
	}
	else if (Start.Z > box.Max.Z)
	{
		if (Dir.Z >= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			Time.Z = (box.Max.Z - Start.Z) / Dir.Z;
		}
	}
	else
		Time.Z = 0.0f;

	// If the line started inside the box (ie. player started in contact with the fluid)
	if (Inside)
	{
		HitLocation = Start;
		HitNormal = SVector(0, 0, 1);
		HitTime = 0;
		return 1;
	}
	// Otherwise, calculate when hit occured
	else
	{
		if (Time.Y > Time.Z)
		{
			HitTime = Time.Y;
			HitNormal = SVector(0, faceDir[1], 0);
		}
		else
		{
			HitTime = Time.Z;
			HitNormal = SVector(0, 0, faceDir[2]);
		}

		if (Time.X > HitTime)
		{
			HitTime = Time.X;
			HitNormal = SVector(faceDir[0], 0, 0);
		}

		if (HitTime >= 0.0f && HitTime <= 1.0f)
		{
			HitLocation = Start + Dir * HitTime;
			const float BOX_SIDE_THRESHOLD = 0.1f;
			if (HitLocation.X > box.Min.X - BOX_SIDE_THRESHOLD && HitLocation.X < box.Max.X + BOX_SIDE_THRESHOLD &&
				HitLocation.Y > box.Min.Y - BOX_SIDE_THRESHOLD && HitLocation.Y < box.Max.Y + BOX_SIDE_THRESHOLD &&
				HitLocation.Z > box.Min.Z - BOX_SIDE_THRESHOLD && HitLocation.Z < box.Max.Z + BOX_SIDE_THRESHOLD)
			{
				return 1;
			}
		}

		return 0;
	}
}

SRotator SVectorToRotation(const SVector & vec)
{
	SRotator R;

	// Find yaw.
	R.Yaw = appRound(appAtan2(vec.Y, vec.X) * (float)INT16_MAX / (2.f*PI));

	// Find pitch.
	R.Pitch = appRound(appAtan2(vec.Z, appSqrt(vec.X*vec.X + vec.Y*vec.Y)) * (float)INT16_MAX / (2.f*PI));

	// Find roll.
	R.Roll = 0;

	return R;
}

SQuat SRotatorToSQuat(const SRotator & rot)
{
	const auto M = SRotationMatrix(rot);

	// If Matrix is NULL, return Identity quaternion.
	if (M.GetAxis(0).IsNearlyZero() && M.GetAxis(1).IsNearlyZero() && M.GetAxis(2).IsNearlyZero())
	{
		return SQuat::Identity;
	}

	//const MeReal *const t = (MeReal *) tm;
	float	s;
	SQuat result = SQuat();

	// Check diagonal (trace)
	const float tr = M.M[0][0] + M.M[1][1] + M.M[2][2];

	if (tr > 0.0f)
	{
		float InvS = appInvSqrt(tr + 1.f);
		result.W = 0.5f * (1.f / InvS);
		s = 0.5f * InvS;

		result.X = (M.M[1][2] - M.M[2][1]) * s;
		result.Y = (M.M[2][0] - M.M[0][2]) * s;
		result.Z = (M.M[0][1] - M.M[1][0]) * s;
	}
	else
	{
		// diagonal is negative
		INT i = 0;

		if (M.M[1][1] > M.M[0][0])
			i = 1;

		if (M.M[2][2] > M.M[i][i])
			i = 2;

		static const INT nxt[3] = { 1, 2, 0 };
		const INT j = nxt[i];
		const INT k = nxt[j];

		s = M.M[i][i] - M.M[j][j] - M.M[k][k] + 1.0f;

		float InvS = appInvSqrt(s);

		float qt[4];
		qt[i] = 0.5f * (1.f / InvS);

		s = 0.5f * InvS;

		qt[3] = (M.M[j][k] - M.M[k][j]) * s;
		qt[j] = (M.M[i][j] + M.M[j][i]) * s;
		qt[k] = (M.M[i][k] + M.M[k][i]) * s;

		result.X = qt[0];
		result.Y = qt[1];
		result.Z = qt[2];
		result.W = qt[3];
	}

	return result;
}

float SInterpTo(float Current, float Target, float DeltaTime, float InterpSpeed)
{
	// If no interp speed, jump to target value
	if (InterpSpeed == 0.f)
	{
		return Target;
	}

	// Distance to reach
	const float Dist = Target - Current;

	// If distance is too small, just set the desired location
	if (Square(Dist) < SMALL_NUMBER)
	{
		return Target;
	}

	// Delta Move, Clamp so we do not over shoot.
	const float DeltaMove = Dist * Clamp<float>(DeltaTime * InterpSpeed, 0.f, 1.f);

	return Current + DeltaMove;
}

float SInterpConstantTo(float Current, float Target, float DeltaTime, float InterpSpeed)
{
	const float Dist = Target - Current;

	// If distance is too small, just set the desired location
	if (Square(Dist) < SMALL_NUMBER)
	{
		return Target;
	}

	const float Step = InterpSpeed * DeltaTime;
	return Current + Clamp<float>(Dist, -Step, Step);
}

float SInterpEaseInOut(float A, float B, float Alpha, float Exp)
{
	float ModifiedAlpha;

	if (Alpha < 0.5f)
	{
		ModifiedAlpha = 0.5f * appPow(2.f * Alpha, Exp);
	}
	else
	{
		ModifiedAlpha = 1.f - 0.5f * appPow(2.f * (1.f - Alpha), Exp);
	}

	return LerpF(A, B, ModifiedAlpha);
}
