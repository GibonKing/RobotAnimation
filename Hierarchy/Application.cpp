#include "Application.h"
#include "Heightmap.h"
#include "Aeroplane.h"
#include "Animation.h"

Application* Application::s_pApp = NULL;

const int CAMERA_MAP = 0;
const int CAMERA_ROTATE = 1;
const int CAMERA_PLANE = 2;
const int CAMERA_GUN = 3;
const int CAMERA_MAX = 4;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool Application::HandleStart()
{
	s_pApp = this;

	m_frameCount = 0.0f;

	this->SetWindowTitle("Hierarchy");

	m_bWireframe = false;

	m_pHeightMap = new HeightMap("Resources/heightmap.bmp", 2.0f);
	m_pAeroplane = new Aeroplane(0.0f, 3.5f, 0.0f, 105.0f);
	m_pAnimation = new Animation(0.0f, 2.4f, 0.0f, 0.0f);

	m_pAeroplane->LoadResources();

	m_cameraZ = 50.0f;
	m_rotationAngle = 0.f;

	m_reload = false;
	ReloadShaders();

	if(!this->CommonApp::HandleStart())
		return false;

	this->SetRasterizerState(false, m_bWireframe);

	m_cameraState = CAMERA_MAP;

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Application::HandleStop()
{
	delete m_pHeightMap;
	Aeroplane::ReleaseResources();
	delete m_pAeroplane;
	delete m_pAnimation;

	this->CommonApp::HandleStop();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Application::ReloadShaders()
{
	if (m_pHeightMap->ReloadShader() == false)
		this->SetWindowTitle("Reload Failed - see Visual Studio output window. Press F5 to try again.");
	else
		this->SetWindowTitle("Your Shader Here. Press F5 to reload shader file.");
}

void Application::HandleUpdate()
{
	timer.Update();
	static float delay = 0.0f;
	delay += timer.GetDeltaTime();

	if (m_cameraState == CAMERA_ROTATE)
		m_rotationAngle += .01f;

	if(m_cameraState == CAMERA_MAP || m_cameraState == CAMERA_ROTATE)
	{
		if(this->IsKeyPressed('Q'))
			m_cameraZ -= 2.0f;

		if(this->IsKeyPressed('A'))
			m_cameraZ += 2.0f;
	}

	static bool dbC = false;

	if(this->IsKeyPressed('C'))
	{
		if(!dbC)
		{
			if(++m_cameraState == CAMERA_MAX)
				m_cameraState = CAMERA_MAP;
			dbC = true;
		}
	}
	else
	{
		dbC = false;
	}

	static bool dbW = false;
	if(this->IsKeyPressed('W'))
	{
		if(!dbW)
		{
			m_bWireframe = !m_bWireframe;
			this->SetRasterizerState(false, m_bWireframe);
			dbW = true;
		}
	}
	else
	{
		dbW = false;
	}

	if (this->IsKeyPressed(VK_F5))
	{
		if (!m_reload)
		{
			ReloadShaders();
			m_reload = true;
		}
	}
	else
		m_reload = false;

	static bool dbS = false;
	if (this->IsKeyPressed('S')) {
		if (!dbS)
		{
			slowMo = !slowMo;
			dbS = true;
		}
	}
	else
	{
		dbS = false;
	}

	static bool dbCamera = false;
	if (this->IsKeyPressed(VK_F1)) {
		if (!dbCamera)
		{
			if(m_cameraState == CAMERA_MAP)
				m_cameraState = CAMERA_ROTATE;
			else
				m_cameraState = CAMERA_MAP;
			dbCamera = true;
		}
	}
	else if (this->IsKeyPressed(VK_F2)) {
		if (!dbCamera)
		{
			m_cameraState = CAMERA_PLANE;
			dbCamera = true;
		}
	}
	else if (this->IsKeyPressed(VK_F3)) {
		//if (!dbCamera)
		//{
		//	m_cameraState = 
		//	dbCamera = true;
		//}
	}
	else if(this->IsKeyPressed(VK_F4)) {
		//if (!dbCamera)
		//{
		//	m_cameraState =
		//	dbCamera = true;
		//}
	}
	else
	{
		dbCamera = false;
	}

	//Animation Controls
	static bool dbAnimate = false;
	if (this->IsKeyPressed('1')) { //Idle
		if (!dbAnimate) {
			m_pAnimation->ChangeAnimation(0);
			dbAnimate = true;
		}
	}else if (this->IsKeyPressed('2')) { //Attack
		if (!dbAnimate) {
			m_pAnimation->ChangeAnimation(1);
			dbAnimate = true;
		}
	}
	else if (this->IsKeyPressed('3')) { //Death
		if (!dbAnimate) {
			m_pAnimation->ChangeAnimation(2);
			dbAnimate = true;
		}
	}else if (this->IsKeyPressed('0')) { //Play/Pause
		if (!dbAnimate) {
			m_pAnimation->PauseAnimation();
			dbAnimate = true;
		}
	}else
		dbAnimate = false;

	m_pAeroplane->Update(m_cameraState != CAMERA_MAP && m_cameraState != CAMERA_ROTATE);
	if (!slowMo || delay > 1) {
		m_pAnimation->Update(timer.GetDeltaTime());
		delay = 0;
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Application::HandleRender()
{
	XMFLOAT3 vUpVector(0.0f, 1.0f, 0.0f);
	XMFLOAT3 vCamera, vLookat;

	switch(m_cameraState)
	{
		case CAMERA_MAP:
			vCamera = XMFLOAT3(sin(m_rotationAngle) * m_cameraZ, m_cameraZ / 4, cos(m_rotationAngle) * m_cameraZ);
			vLookat = XMFLOAT3(0.0f, 4.0f, 0.0f);
			break;
		case CAMERA_ROTATE:
			vCamera = XMFLOAT3(sin(m_rotationAngle) * m_cameraZ, m_cameraZ / 4, cos(m_rotationAngle) * m_cameraZ);
			vLookat = XMFLOAT3(0.0f, 4.0f, 0.0f);
			break;
		case CAMERA_PLANE:
			m_pAeroplane->SetGunCamera(false);
			vCamera = XMFLOAT3(m_pAeroplane->GetCameraPosition().x, m_pAeroplane->GetCameraPosition().y, m_pAeroplane->GetCameraPosition().z);
			vLookat = XMFLOAT3(m_pAeroplane->GetFocusPosition().x, m_pAeroplane->GetFocusPosition().y, m_pAeroplane->GetFocusPosition().z);
			break;
		case CAMERA_GUN:
			m_pAeroplane->SetGunCamera(true);
			vCamera = XMFLOAT3(m_pAeroplane->GetCameraPosition().x, m_pAeroplane->GetCameraPosition().y, m_pAeroplane->GetCameraPosition().z);
			vLookat = XMFLOAT3(m_pAeroplane->GetFocusPosition().x, m_pAeroplane->GetFocusPosition().y, m_pAeroplane->GetFocusPosition().z);
			break;
	}

	XMMATRIX matView;
	matView = XMMatrixLookAtLH(XMLoadFloat3(&vCamera), XMLoadFloat3(&vLookat), XMLoadFloat3(&vUpVector));

	XMMATRIX matProj;
	matProj = XMMatrixPerspectiveFovLH(float(XM_PI / 4), 2, 1.5f, 5000.0f);

	this->EnableDirectionalLight(1, XMFLOAT3(-1.f, -1.f, -1.f), XMFLOAT3(0.65f, 0.55f, 0.65f));
	this->EnablePointLight(0, XMFLOAT3(100.0f, 100.f, -100.f), XMFLOAT3(1.f, 1.f, 1.f));
	this->SetLightAttenuation(0, 200.f, 2.f, 2.f, 2.f);

	this->SetViewMatrix(matView);
	this->SetProjectionMatrix(matProj);

	this->Clear(XMFLOAT4(.2f, .2f, .6f, 1.f));

	XMMATRIX matWorld;
	matWorld = XMMatrixIdentity();
	this->SetWorldMatrix(matWorld);

	m_pHeightMap->Draw(m_frameCount);
	m_pAeroplane->Draw();
	m_pAnimation->Draw();

	m_frameCount++;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	Application application;

	Run(&application);

	return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
