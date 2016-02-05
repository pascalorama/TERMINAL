#include <Camera.h>
#include <Vector3.h>
#include <mathf.h>

static float g_SH4Vector[ 4 ];
static float g_Result[ 4 ];

void CAM_CalculateViewMatrix( PMATRIX4X4 p_pMatrix, const PCAMERA p_pCamera )
{
	VECTOR3 Up, Right, Direction;

	MAT44_SetIdentity( p_pMatrix );

	VEC3_Subtract( &Direction, &p_pCamera->LookAt, &p_pCamera->Position );
	VEC3_Normalise( &Direction );

	VEC3_Cross( &Right, &p_pCamera->WorldUp, &Direction );
	VEC3_Normalise( &Right );

	VEC3_Cross( &Up, &Direction, &Right );
	VEC3_Normalise( &Up );

	/* R U D P
	 * R U D P
	 * R U D P
	 * 0 0 0 1 */
	p_pMatrix->M00 = Right.X;
	p_pMatrix->M01 = Up.X;
	p_pMatrix->M02 = Direction.X;
	p_pMatrix->M03 = -VEC3_Dot( &Right, &p_pCamera->Position );

	p_pMatrix->M10 = Right.Y;
	p_pMatrix->M11 = Up.Y;
	p_pMatrix->M12 = Direction.Y;
	p_pMatrix->M13 = -VEC3_Dot( &Up, &p_pCamera->Position );

	p_pMatrix->M20 = Right.Z;
	p_pMatrix->M21 = Up.Z;
	p_pMatrix->M22 = Direction.Z;
	p_pMatrix->M23 = -VEC3_Dot( &Direction, &p_pCamera->Position );
}

void CAM_CalculateProjectionMatrix( PMATRIX4X4 p_pMatrix,
	const PCAMERA p_pCamera )
{
	MATRIX4X4 ProjectionMatrix, ScreenMatrix;
	register float HalfWidth, HalfHeight, H;

	HalfWidth = p_pCamera->GateWidth / 2.0f;
	HalfHeight = p_pCamera->GateHeight / 2.0f;

	MAT44_SetIdentity( p_pMatrix );

	H = tanf( p_pCamera->FieldOfView / 2.0f );

	MAT44_SetIdentity( &ProjectionMatrix );
	ProjectionMatrix.M22 = H * p_pCamera->FarPlane /
		( p_pCamera->FarPlane - p_pCamera->NearPlane );
	ProjectionMatrix.M23 = H;
	ProjectionMatrix.M32 = -H * p_pCamera->FarPlane * p_pCamera->NearPlane /
		( p_pCamera->FarPlane - p_pCamera->NearPlane );
	
	ProjectionMatrix.M33 = 0.0f;
	
	MAT44_SetIdentity( &ScreenMatrix );
	ScreenMatrix.M00 = HalfWidth;
	ScreenMatrix.M11 = -HalfWidth;
	ScreenMatrix.M20 = HalfWidth;
	ScreenMatrix.M21 = HalfHeight;

	MAT44_Multiply( p_pMatrix, &ProjectionMatrix, &ScreenMatrix );
}

void CAM_TransformNonClipPerspective( float *p_pTransformedVertices,
	const float *p_pVertices, const size_t p_VertexCount,
	const size_t p_TransformedStride,const size_t p_UntransformedStride,
	const PMATRIX4X4 p_pMatrix, const CAMERA *p_pCamera )
{
	register float H, *pDestVector, HalfWidth, HalfHeight, RHW, SpeedUp;
	register int Index;
	size_t UStride, TStride;
	MATRIX4X4 Transform, Projection;

#if defined ( DEBUG )
	if( ( p_UntransformedStride < ( sizeof( float ) * 3 ) ) ||
		( p_TransformedStride < ( sizeof( float ) * 3 ) ) )
	{
		return;
	}
#endif /* DEBUG */

	HalfWidth = p_pCamera->GateWidth / 2.0f;
	HalfHeight = p_pCamera->GateHeight / 2.0f;

	UStride = ( p_UntransformedStride - ( sizeof( float ) * 3 ) ) /
		sizeof( float );

	TStride = ( p_TransformedStride - ( sizeof( float ) * 3 ) ) /
		sizeof( float );

	H = tanf( p_pCamera->FieldOfView / 2.0f );

	MAT44_SetIdentity( &Projection );

	Projection.M22 = H * p_pCamera->FarPlane /
		( p_pCamera->FarPlane - p_pCamera->NearPlane );
	Projection.M23 = H;
	Projection.M32 = -H * p_pCamera->FarPlane * p_pCamera->NearPlane /
		( p_pCamera->FarPlane - p_pCamera->NearPlane );
	Projection.M33 = 0.0f;

	MAT44_Multiply( &Transform, p_pMatrix, &Projection );

	ld_ext( ( void * )&Transform );

	pDestVector = p_pTransformedVertices;

	g_SH4Vector[ 3 ] = 1.0f;

	for( Index = p_VertexCount; Index > 0; --Index )
	{
		g_SH4Vector[ 0 ] = *( p_pVertices++ );
		g_SH4Vector[ 1 ] = *( p_pVertices++ );
		g_SH4Vector[ 2 ] = *( p_pVertices++ );

		p_pVertices += UStride;

		ftrv( ( float * )g_SH4Vector, ( float * )g_Result );

		RHW = 1.0f / g_Result[ 2 ];

		SpeedUp = RHW * HalfWidth;

		*( pDestVector++ ) = SpeedUp * g_Result[ 0 ] + HalfWidth;
		*( pDestVector++ ) = -SpeedUp * g_Result[ 1 ] + HalfHeight;
		*( pDestVector++ ) = RHW;

		pDestVector += TStride;
	}
}

