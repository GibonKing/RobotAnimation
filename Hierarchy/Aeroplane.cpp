//*********************************************************************************************
// File:			Aeroplane.cpp
// Description:		A very simple class to represent an aeroplane as one object with all the
//					hierarchical components stored internally within the class.
// Module:			Real-Time 3D Techniques for Games
// Created:			Jake - 2010-2011
// Notes:
//*********************************************************************************************

#include "Aeroplane.h"

// Initialise static class variables.
CommonMesh* Aeroplane::s_pPlaneMesh = NULL;
CommonMesh* Aeroplane::s_pPropMesh = NULL;
CommonMesh* Aeroplane::s_pTurretMesh = NULL;
CommonMesh* Aeroplane::s_pGunMesh = NULL;

bool Aeroplane::s_bResourcesReady = false;

Aeroplane::Aeroplane(float fX, float fY, float fZ, float fRotY)
{
	m_mWorldMatrix = XMMatrixIdentity();
	m_mPropWorldMatrix = XMMatrixIdentity();
	m_mTurretWorldMatrix = XMMatrixIdentity();
	m_mGunWorldMatrix = XMMatrixIdentity();
	m_mCamWorldMatrix = XMMatrixIdentity();

	m_v4Rot = XMFLOAT4(0.0f, fRotY, 0.0f, 0.0f);
	m_v4Pos = XMFLOAT4(fX, fY, fZ, 0.0f);

	m_v4PropOff = XMFLOAT4(0.0f, 0.0f, 1.9f, 0.0f);
	m_v4PropRot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	m_v4TurretOff = XMFLOAT4(0.0f, 1.05f, -1.3f, 0.0f);
	m_v4TurretRot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	m_v4GunOff = XMFLOAT4(0.0f, 0.5f, 0.0f, 0.0f);
	m_v4GunRot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	m_v4CamOff = XMFLOAT4(0.0f, 4.5f, -15.0f, 0.0f);
	m_v4CamRot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	m_vCamWorldPos = XMVectorZero();
	m_vForwardVector = XMVectorZero();

	m_fSpeed = 0.0f;

	m_bGunCam = false;
}

Aeroplane::~Aeroplane(void)
{
}

void Aeroplane::SetWorldPosition(float fX, float fY, float fZ)
{
	m_v4Pos = XMFLOAT4(fX, fY, fZ, 0.0f);
	UpdateMatrices();
}

void Aeroplane::UpdateMatrices(void)
{
	XMMATRIX mRotX, mRotY, mRotZ, mTrans;
	XMMATRIX mPlaneCameraRot, mPlaneGunRot, mPlaneTurretRot, mForwardMatrix;

	// [START HERE]

	// Calculate m_mWorldMatrix for plane based on Euler rotation angles and position data.
	// i.e. Use XMMatrixRotationX(), XMMatrixRotationY(), XMMatrixRotationZ() and XMMatrixTranslationFromVector to calculate mRotX, mRotY, mRotZ and mTrans from m_v4Rot
	// Then concatenate the matrices to calculate m_mWorldMatrix

	mRotX = XMMatrixRotationX(XMConvertToRadians(m_v4Rot.x));
	mRotY = XMMatrixRotationY(XMConvertToRadians(m_v4Rot.y));
	mRotZ = XMMatrixRotationZ(XMConvertToRadians(m_v4Rot.z));
	mTrans = XMMatrixTranslationFromVector(XMLoadFloat4(&m_v4Pos));
	m_mWorldMatrix = mRotZ * mRotX * mRotY * mTrans;

	// [Skip this step first time through] Also calculate mPlaneCameraRot which ignores rotations in Z and X for the camera to parent to

	mPlaneCameraRot = mRotY * mTrans;

	// [Skip this step first time through] Get the forward vector out of the world matrix and put it in m_vForwardVector

	m_vForwardVector = m_mWorldMatrix.r[2];

	// Calculate m_mPropWorldMatrix for propeller based on Euler rotation angles and position data.
	// Parent the propeller to the plane

	mRotX = XMMatrixRotationX(XMConvertToRadians(m_v4PropRot.x));
	mRotY = XMMatrixRotationY(XMConvertToRadians(m_v4PropRot.y));
	mRotZ = XMMatrixRotationZ(XMConvertToRadians(m_v4PropRot.z));
	mTrans = XMMatrixTranslationFromVector(XMLoadFloat4(&m_v4PropOff));
	m_mPropWorldMatrix = mRotZ * mRotX * mRotY * mTrans * m_mWorldMatrix;

	// Calculate m_mTurretWorldMatrix for propeller based on Euler rotation angles and position data.
	// Parent the turret to the plane

	mRotX = XMMatrixRotationX(XMConvertToRadians(m_v4TurretRot.x));
	mRotY = XMMatrixRotationY(XMConvertToRadians(m_v4TurretRot.y));
	mRotZ = XMMatrixRotationZ(XMConvertToRadians(m_v4TurretRot.z));
	mTrans = XMMatrixTranslationFromVector(XMLoadFloat4(&m_v4TurretOff));
	m_mTurretWorldMatrix = mRotZ * mRotX * mRotY * mTrans * m_mWorldMatrix;
	mPlaneTurretRot = mRotY * mTrans * mPlaneCameraRot;

	// Calculate m_mGunWorldMatrix for gun based on Euler rotation angles and position data.
	// Parent the gun to the turret

	mRotX = XMMatrixRotationX(XMConvertToRadians(m_v4GunRot.x));
	mRotY = XMMatrixRotationY(XMConvertToRadians(m_v4GunRot.y));
	mRotZ = XMMatrixRotationZ(XMConvertToRadians(m_v4GunRot.z));
	mTrans = XMMatrixTranslationFromVector(XMLoadFloat4(&m_v4GunOff));
	m_mGunWorldMatrix = mRotZ * mRotX * mRotY * mTrans * m_mTurretWorldMatrix;
	mPlaneGunRot = mRotY * mTrans * mPlaneTurretRot;

	// Calculate m_mCameraWorldMatrix for camera based on Euler rotation angles and position data.
	
	mRotX = XMMatrixRotationX(XMConvertToRadians(m_v4CamRot.x));
	mRotY = XMMatrixRotationY(XMConvertToRadians(m_v4CamRot.y));
	mRotZ = XMMatrixRotationZ(XMConvertToRadians(m_v4CamRot.z));
	mTrans = XMMatrixTranslationFromVector(XMLoadFloat4(&m_v4CamOff));
	m_mCamWorldMatrix = mRotZ * mRotX * mRotY * mTrans;

	// [Skip this step first time through] Switch between parenting the camera to the plane (without X and Z rotations) and the gun based on m_bGunCam

	if (m_bGunCam) {
		m_mCamWorldMatrix = m_mCamWorldMatrix * mPlaneGunRot;
	}
	else {
		m_mCamWorldMatrix = m_mCamWorldMatrix * mPlaneCameraRot;
	}

	// Get the camera's world position (m_vCamWorldPos) out of m_mCameraWorldMatrix

	m_vCamWorldPos = m_mCamWorldMatrix.r[3];

}

void Aeroplane::Update(bool bPlayerControl)
{
	// DON'T DO THIS UNTIL YOu HAVE COMPLETED THE FUNCTION ABOVE
	if(bPlayerControl)
	{
		// Step 1: Make the plane pitch upwards when you press "Q" and return to level when released
		// Maximum pitch = 60 degrees

		if (Application::s_pApp->IsKeyPressed('Q') && m_v4Rot.x < 60) {
			m_v4Rot.x += 1.f;
		}
		else if (m_v4Rot.x > 0) {
			m_v4Rot.x -= 1.f;
		}

		// Step 2: Make the plane pitch downwards when you press "A" and return to level when released
		// You can also impose a take off speed of 0.5 if you like
		// Minimum pitch = -60 degrees

		if (Application::s_pApp->IsKeyPressed('A') && m_v4Rot.x > -60) {
			m_v4Rot.x -= 1.f;
		}
		else if (m_v4Rot.x < 0) {
			m_v4Rot.x += 1.f;
		}

		// Step 3: Make the plane yaw and roll left when you press "O" and return to level when released
		// Maximum roll = 20 degrees

		if (Application::s_pApp->IsKeyPressed('O')) {
			if(m_v4Rot.z < 20)
				m_v4Rot.z += 1.f;
			m_v4Rot.y -= 1.f;
		}
		else if (m_v4Rot.z > 0) {
			m_v4Rot.z -= 1.f;
		}

		// Step 4: Make the plane yaw and roll right when you press "P" and return to level when released
		// Minimum roll = -20 degrees

		if (Application::s_pApp->IsKeyPressed('P')) {
			if (m_v4Rot.z > -20)
				m_v4Rot.z -= 1.f;
			m_v4Rot.y += 1.f;
		}
		else if (m_v4Rot.z < 0) {
			m_v4Rot.z += 1.f;
		}

		static bool dbM = false;
		if (Application::s_pApp->IsKeyPressed('M')) {
			if (!dbM) {
				if (m_fSpeed != 0) {
					oldSpeed = m_fSpeed;
					m_fSpeed = 0;
					move = false;
				}
				else {
					m_fSpeed = oldSpeed;
					move = true;
				}
				dbM = true;
			}
		}
		else {
			dbM = false;
		}

		static bool dbSpace = false;
		if (Application::s_pApp->IsKeyPressed(' ')) {
			if (!dbSpace) {
				dbSpace = true;
			}
		}
		else {
			dbSpace = false;
		}
	} // End of if player control

	if (move) {
		// Apply a forward thrust and limit to a maximum speed of 1
		m_fSpeed += 0.001f;

		if (m_fSpeed > 1)
			m_fSpeed = 1;
	}

	// Rotate propeller and turret
	m_v4PropRot.z += 100 * m_fSpeed;
	m_v4TurretRot.y += 0.1f;

	// Tilt gun up and down as turret rotates
	m_v4GunRot.x = (sin((float)XMConvertToRadians(m_v4TurretRot.y * 4.0f)) * 10.0f) - 10.0f;

	UpdateMatrices();

	// Move Forward
	XMVECTOR vCurrPos = XMLoadFloat4(&m_v4Pos);
	vCurrPos += m_vForwardVector * m_fSpeed;
	XMStoreFloat4(&m_v4Pos, vCurrPos);
}

void Aeroplane::LoadResources(void)
{
	s_pPlaneMesh = CommonMesh::LoadFromXFile(Application::s_pApp, "Resources/Plane/plane.x");
	s_pPropMesh = CommonMesh::LoadFromXFile(Application::s_pApp, "Resources/Plane/prop.x");
	s_pTurretMesh = CommonMesh::LoadFromXFile(Application::s_pApp, "Resources/Plane/turret.x");
	s_pGunMesh = CommonMesh::LoadFromXFile(Application::s_pApp, "Resources/Plane/gun.x");
}

void Aeroplane::ReleaseResources(void)
{
	delete s_pPlaneMesh;
	delete s_pPropMesh;
	delete s_pTurretMesh;
	delete s_pGunMesh;
}

void Aeroplane::Draw(void)
{
	Application::s_pApp->SetWorldMatrix(m_mWorldMatrix);
	s_pPlaneMesh->Draw();

	Application::s_pApp->SetWorldMatrix(m_mPropWorldMatrix);
	s_pPropMesh->Draw();

	Application::s_pApp->SetWorldMatrix(m_mTurretWorldMatrix);
	s_pTurretMesh->Draw();

	Application::s_pApp->SetWorldMatrix(m_mGunWorldMatrix);
	s_pGunMesh->Draw();
}
