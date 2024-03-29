#ifndef AEROPLANE_H
#define AEROPLANE_H

//*********************************************************************************************
// File:			Aeroplane.h
// Description:		A very simple class to represent an aeroplane as one object with all the
//					hierarchical components stored internally within the class.
// Module:			Real-Time 3D Techniques for Games
// Created:			Jake - 2010-2011
// Notes:
//*********************************************************************************************

#include "Application.h"
#include "HeightMap.h"

// Holds meshes needed for drawing the aeroplane so that there can be more than one set (used for shadows)
class AeroplaneMeshes
{
public:
	static AeroplaneMeshes *Load();

	CommonMesh*	pPlaneMesh;
	CommonMesh*	pPropMesh;
	CommonMesh*	pTurretMesh;
	CommonMesh*	pGunMesh;
	CommonMesh* pSphereMesh;

	~AeroplaneMeshes();
protected:
	AeroplaneMeshes();
private:
	// It's not safe to copy this kind of object.
	AeroplaneMeshes(const AeroplaneMeshes &);
	AeroplaneMeshes &operator=(const AeroplaneMeshes &);
};

__declspec(align(16)) class Aeroplane
{
  public:
	Aeroplane(float fX = 0.0f, float fY = 0.0f, float fZ = 0.0f, float fRotY = 0.0f);
	~Aeroplane(void);

	void Update(bool bPlayerControl); // Player only has control of plane when flag is set
	void Draw(const AeroplaneMeshes *pMeshes);

	void SetWorldPosition(float fX, float fY, float fZ);

  private:
	void UpdateMatrices(void);

	XMFLOAT4 m_v4Rot; // Euler rotation angles
	XMFLOAT4 m_v4Pos; // World position

	XMVECTOR m_vForwardVector; // Forward Vector for Plane
	float m_fSpeed, oldSpeed; // Forward speed

	XMMATRIX m_mWorldTrans; // World translation matrix
	XMMATRIX m_mWorldMatrix; // World transformation matrix

	XMFLOAT4 m_v4PropRot; // Local rotation angles
	XMFLOAT4 m_v4PropOff; // Local offset
	XMMATRIX m_mPropWorldMatrix; // Propeller's world transformation matrix

	XMFLOAT4 m_v4TurretRot; // Local rotation angles
	XMFLOAT4 m_v4TurretOff; // Local offset
	XMMATRIX m_mTurretWorldMatrix; // Turret's world transformation matrix

	XMFLOAT4 m_v4GunRot; // Local rotation angles
	XMFLOAT4 m_v4GunOff; // Local offset
	XMMATRIX m_mGunWorldMatrix; // Gun's world transformation matrix

	XMFLOAT4 m_v4CamRot; // Local rotation angles
	XMFLOAT4 m_v4CamOff; // Local offset

	XMVECTOR m_vCamWorldPos; // World position
	XMMATRIX m_mCamWorldMatrix; // Camera's world transformation matrix
	
	XMFLOAT4 m_v4BombRot; // Local rotation angles
	XMFLOAT4 m_v4BombPos; // Local position
	XMMATRIX m_mBombWorldMatrix; // Bomb's world transformation matrix
	XMFLOAT4 bombVel, Gravity;
	bool bombCollided;
	float bombSpeed;
	int bombBounceCount;

	bool m_bGunCam, move, shoot, bomb = false;

  public:
	float GetXPosition(void) { return m_v4Pos.x; }
	float GetYPosition(void) { return m_v4Pos.y; }
	float GetZPosition(void) { return m_v4Pos.z; }
	XMFLOAT4 GetFocusPosition(void) { return GetPosition(); }
	XMFLOAT4 GetCameraPosition(void)
	{
		XMFLOAT4 v4Pos;
		XMStoreFloat4(&v4Pos, m_vCamWorldPos);
		return v4Pos;
	}
	XMFLOAT4 GetPosition(void) { return m_v4Pos; }
	void SetGunCamera(bool value) { m_bGunCam = value; }

	void SetBomb(bool Bomb) { bomb = Bomb; }
	bool GetBomb() { return bomb; }
	XMFLOAT4 GetBombPosition() { return m_v4BombPos; }
	void MakeBomb() {
		m_v4BombPos = m_v4Pos;
		m_v4BombRot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		Gravity = XMFLOAT4(0.0f, -0.05f, 0.0f, 0.0f);
		bombVel = XMFLOAT4(XMVectorGetX(m_vForwardVector), -0.2f, XMVectorGetZ(m_vForwardVector), XMVectorGetW(m_vForwardVector));
		bombCollided = false;
		bombBounceCount = 0;
	}

	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}
};

#endif