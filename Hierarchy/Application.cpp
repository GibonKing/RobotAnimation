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
const int CAMERA_BOMB = 5;

static const int RENDER_TARGET_WIDTH = 512;
static const int RENDER_TARGET_HEIGHT = 512;

static const float AEROPLANE_RADIUS = 6.f;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool Application::HandleStart()
{
	s_pApp = this;
	m_frameCount = 0.0f;
	this->SetWindowTitle("Hierarchy");

	font = NULL;
	style = CommonFont::Style(VertexColour(255, 255, 255, 255), XMFLOAT2(1.0, 1.0));
	m_pHeightMap = NULL;
	m_pAeroplane = NULL;
	m_pAeroplaneDefaultMeshes = NULL;
	m_pAeroplaneShadowMeshes = NULL;
	m_pAnimation = NULL;

	m_pRenderTargetColourTexture = NULL;
	m_pRenderTargetDepthStencilTexture = NULL;
	m_pRenderTargetColourTargetView = NULL;
	m_pRenderTargetColourTextureView = NULL;
	m_pRenderTargetDepthStencilTargetView = NULL;
	m_drawHeightMapShaderVSConstants.pCB = NULL;
	m_drawHeightMapShaderPSConstants.pCB = NULL;
	m_pRenderTargetDebugDisplayBuffer = NULL;
	m_shadowCastingLightPosition = XMFLOAT3(4.0f, 10.f, 0.f);
	m_shadowColour = XMFLOAT4(0.f, 0.f, 0.f, .25f);
	m_pShadowSamplerState = NULL;

	if (!this->CommonApp::HandleStart())
		return false;

	char aMaxNumLightsStr[100];
	_snprintf_s(aMaxNumLightsStr, sizeof aMaxNumLightsStr, _TRUNCATE, "%d", MAX_NUM_LIGHTS);

	const D3D_SHADER_MACRO aMacros[] = {
		{"MAX_NUM_LIGHTS", aMaxNumLightsStr, },
		{NULL},
	};

	if (!this->CompileShaderFromFile(&m_drawShadowCasterShader, "./Resources/DrawShadowCaster.hlsl", aMacros, g_aVertexDesc_Pos3fColour4ubNormal3f, g_vertexDescSize_Pos3fColour4ubNormal3f))
		return false;
	
	// DrawHeightMap.hlsl
	{
		ID3D11VertexShader *pVS = NULL;
		ID3D11PixelShader *pPS = NULL;
		ID3D11InputLayout *pIL = NULL;
		ShaderDescription vs, ps;

		if (!CompileShadersFromFile(m_pD3DDevice, "./Resources/DrawHeightMap.hlsl",
			"VSMain", &pVS, &vs,
			g_aVertexDesc_Pos3fColour4ubNormal3f, g_vertexDescSize_Pos3fColour4ubNormal3f, &pIL,
			"PSMain", &pPS, &ps,
			aMacros))
		{
			return false;
		}

		this->CreateShaderFromCompiledShader(&m_drawHeightMapShader, pVS, &vs, pIL, pPS, &ps);

		vs.FindCBuffer("DrawHeightMap", &m_drawHeightMapShaderVSConstants.slot);
		vs.FindFloat4x4(m_drawHeightMapShaderVSConstants.slot, "g_shadowMatrix", &m_drawHeightMapShaderVSConstants.shadowMatrix);
		vs.FindFloat4(m_drawHeightMapShaderVSConstants.slot, "g_shadowColour", &m_drawHeightMapShaderVSConstants.shadowColour);
		m_drawHeightMapShaderVSConstants.pCB = CreateBuffer(m_pD3DDevice, vs.GetCBufferSizeBytes(m_drawHeightMapShaderVSConstants.slot), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, NULL);

		ps.FindCBuffer("DrawHeightMap", &m_drawHeightMapShaderPSConstants.slot);
		ps.FindFloat4x4(m_drawHeightMapShaderPSConstants.slot, "g_shadowMatrix", &m_drawHeightMapShaderPSConstants.shadowMatrix);
		ps.FindFloat4(m_drawHeightMapShaderPSConstants.slot, "g_shadowColour", &m_drawHeightMapShaderPSConstants.shadowColour);
		m_drawHeightMapShaderPSConstants.pCB = CreateBuffer(m_pD3DDevice, ps.GetCBufferSizeBytes(m_drawHeightMapShaderPSConstants.slot), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, NULL);

		ps.FindTexture("g_shadowTexture", &m_heightMapShaderShadowTexture);
		ps.FindSamplerState("g_shadowSampler", &m_heightMapShaderShadowSampler);
	}

	if (!this->CreateRenderTarget())
		return false;

	m_bWireframe = false;

	m_pHeightMap = new HeightMap("Resources/heightmap.bmp", 2.0f, &m_drawHeightMapShader);
	m_pAeroplane = new Aeroplane(0.0f, 3.5f, 0.0f, 105.0f);
	m_pAnimation = new Animation(0.0f, 2.4f, 0.0f, 0.0f);

	m_pAeroplaneDefaultMeshes = AeroplaneMeshes::Load();
	if (!m_pAeroplaneDefaultMeshes)
		return false;
	m_pAeroplaneShadowMeshes = AeroplaneMeshes::Load();
	if (!m_pAeroplaneShadowMeshes)
		return false;

	m_pAeroplaneShadowMeshes->pPlaneMesh->SetShaderForAllSubsets(&m_drawShadowCasterShader);
	m_pAeroplaneShadowMeshes->pPropMesh->SetShaderForAllSubsets(&m_drawShadowCasterShader);
	m_pAeroplaneShadowMeshes->pTurretMesh->SetShaderForAllSubsets(&m_drawShadowCasterShader);
	m_pAeroplaneShadowMeshes->pGunMesh->SetShaderForAllSubsets(&m_drawShadowCasterShader);
	m_pAeroplaneShadowMeshes->pSphereMesh->SetShaderForAllSubsets(&m_drawShadowCasterShader);

	m_cameraZ = 50.0f;
	m_rotationAngle = 0.f;

	m_reload = false;
	ReloadShaders();

	font = CommonFont::CreateByName("Arial", 10, 0, this);
	
	this->SetRasterizerState(false, m_bWireframe);

	m_cameraState = CAMERA_MAP;

	m_pShadowCastingLightMesh = CommonMesh::NewSphereMesh(this, 1.0f, 16, 16);

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Application::HandleStop()
{
	Release(m_pRenderTargetDebugDisplayBuffer);
	Release(m_pRenderTargetColourTexture);
	Release(m_pRenderTargetDepthStencilTexture);
	Release(m_pRenderTargetColourTargetView);
	Release(m_pRenderTargetColourTextureView);
	Release(m_pRenderTargetDepthStencilTargetView);
	Release(m_pShadowSamplerState);
	Release(m_drawHeightMapShaderVSConstants.pCB);
	Release(m_drawHeightMapShaderPSConstants.pCB);

	delete m_pHeightMap;
	delete m_pAeroplane;
	delete m_pAeroplaneDefaultMeshes;
	delete m_pAeroplaneShadowMeshes;
	m_drawShadowCasterShader.Reset();
	delete m_pShadowCastingLightMesh;
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

	static bool dbH = false;
	if (this->IsKeyPressed('H'))
	{
		if (!dbH)
		{
			help = !help;
			dbH = true;
		}
	}
	else
	{
		dbH = false;
	}

	if (m_cameraState != CAMERA_PLANE && m_cameraState != CAMERA_GUN)
	{
		if (this->IsKeyPressed(VK_LEFT))
			m_shadowCastingLightPosition.x += .2f;

		if (this->IsKeyPressed(VK_RIGHT))
			m_shadowCastingLightPosition.x -= .2f;

		if (this->IsKeyPressed(VK_UP))
			m_shadowCastingLightPosition.z += .2f;

		if (this->IsKeyPressed(VK_DOWN))
			m_shadowCastingLightPosition.z -= .2f;

		if (this->IsKeyPressed(VK_PRIOR))
			m_shadowCastingLightPosition.y -= .2f;

		if (this->IsKeyPressed(VK_NEXT))
			m_shadowCastingLightPosition.y += .2f;
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

	static bool dbS = false; //Slow Mo
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

	//Plane Controls
	static bool dbB = false;
	if (Application::s_pApp->IsKeyPressed('B')) {
		if (!dbB && !m_pAeroplane->GetBomb()) {
			m_pAeroplane->SetBomb(true);
			m_pAeroplane->MakeBomb();
			m_cameraState = CAMERA_BOMB;
			dbB = true;
		}
	}
	else {
		dbB = false;
	}

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
	this->RenderShadow();
	this->Render3D();
	this->Render2D();
	m_frameCount++;
}

void Application::RenderShadow()
{
	// Only the alpha channel is relevant, but clear the RGB channels to white
	// so that it's easy to see what's going on.
	float clearColour[4] = { 0.f, 0.f, 0.f, 0.f };
	m_pD3DDeviceContext->ClearRenderTargetView(m_pRenderTargetColourTargetView, clearColour);
	m_pD3DDeviceContext->ClearDepthStencilView(m_pRenderTargetDepthStencilTargetView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	m_pD3DDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetColourTargetView, m_pRenderTargetDepthStencilTargetView);

	D3D11_VIEWPORT viewport = { 0.f, 0.f, RENDER_TARGET_WIDTH, RENDER_TARGET_HEIGHT, 0.f, 1.f };
	m_pD3DDeviceContext->RSSetViewports(1, &viewport);

	XMFLOAT4 vTemp = m_pAeroplane->GetPosition();
	XMVECTOR vPlanePos = XMLoadFloat4(&vTemp);

	//*************************************************************************
	// Your code to adjust the perspective projection of the light goes here
	// You will need to calculate fovy, zn and zf instead of using these default values:

	float disToPlane = sqrt(pow(vTemp.x - m_shadowCastingLightPosition.x, 2) + pow(vTemp.y - m_shadowCastingLightPosition.y, 2) + pow(vTemp.z - m_shadowCastingLightPosition.z, 2));
	float zn = abs(disToPlane - AEROPLANE_RADIUS);
	float zf = abs(disToPlane + AEROPLANE_RADIUS);
	float fovy = atan(AEROPLANE_RADIUS / disToPlane) * 2;
	float aspect = RENDER_TARGET_WIDTH / RENDER_TARGET_HEIGHT;

	// You will find the following constants (defined above) useful:
	// RENDER_TARGET_WIDTH, RENDER_TARGET_HEIGHT, AEROPLANE_RADIUS
	//*************************************************************************

	XMMATRIX projMtx;
	projMtx = XMMatrixPerspectiveFovLH(fovy, aspect, zn, zf);
	this->SetProjectionMatrix(projMtx);

	XMMATRIX viewMtx;
	viewMtx = XMMatrixLookAtLH(XMLoadFloat3(&m_shadowCastingLightPosition), vPlanePos, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	this->SetViewMatrix(viewMtx);

	m_shadowMtx = XMMatrixMultiply(viewMtx, projMtx);

	XMMATRIX worldMtx = XMMatrixIdentity();
	this->SetWorldMatrix(worldMtx);

	this->SetDepthStencilState(true);
	this->SetRasterizerState(false, false);

	m_pAeroplane->Draw(m_pAeroplaneShadowMeshes);

	this->SetDefaultRenderTarget();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void Application::Render3D()
{
	this->SetRasterizerState(false, m_bWireframe);

	this->SetDepthStencilState(true);

	XMVECTOR vCamera = XMVectorZero();
	XMVECTOR vLookat = XMVectorZero();;
	XMVECTOR vUpVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMFLOAT4 vPlanePos = m_pAeroplane->GetPosition();
	XMFLOAT4 vCamPos = m_pAeroplane->GetCameraPosition();
	XMFLOAT4 vFocusPos = m_pAeroplane->GetPosition();
	XMFLOAT4 vBombPos = m_pAeroplane->GetBombPosition();

	switch (m_cameraState)
	{
	case CAMERA_MAP:
		vCamera = XMVectorSet(sin(m_rotationAngle)*m_cameraZ, m_cameraZ / 3 + 4, cos(m_rotationAngle)*m_cameraZ, 0.0f);
		vLookat = XMVectorSet(0.0f, 2.0f, 0.0f, 0.0f);
		break;
	case CAMERA_ROTATE:
		vCamera = XMVectorSet(sin(m_rotationAngle)*m_cameraZ, m_cameraZ / 3 + 4, cos(m_rotationAngle)*m_cameraZ, 0.0f);
		vLookat = XMVectorSet(0.0f, 2.0f, 0.0f, 0.0f);
		break;
	case CAMERA_PLANE:
		m_pAeroplane->SetGunCamera(false);
		vCamera = XMLoadFloat4(&vCamPos);
		vLookat = XMLoadFloat4(&vFocusPos);
		break;
	case CAMERA_GUN:
		m_pAeroplane->SetGunCamera(true);
		vCamera = XMLoadFloat4(&vCamPos);
		vLookat = XMLoadFloat4(&vFocusPos);
		break;
	case CAMERA_BOMB:
		m_pAeroplane->SetGunCamera(false);
		vCamera = XMLoadFloat4(&vCamPos);
		vLookat = XMLoadFloat4(&vBombPos);
		break;
	}

	XMMATRIX  matView;
	matView = XMMatrixLookAtLH(vCamera, vLookat, vUpVector);

	XMMATRIX matProj;
	matProj = XMMatrixPerspectiveFovLH(kMath_PI / 4.f, 2, 1.5f, 5000.0f);

	this->SetViewMatrix(matView);
	this->SetProjectionMatrix(matProj);

	this->EnablePointLight(0, m_shadowCastingLightPosition, XMFLOAT3(1.0f, 1.0f, 1.0f));
	this->SetLightAttenuation(0, 200.f, 2.f, 2.f, 2.f);
	this->EnableDirectionalLight(1, XMFLOAT3(-1.f, -1.f, -1.f), XMFLOAT3(0.65f, 0.55f, 0.65f));

	this->Clear(XMFLOAT4(.2f, .2f, .6f, 1.f));

	this->SetWorldMatrix(XMMatrixIdentity());

	{
		D3D11_MAPPED_SUBRESOURCE vsMap;
		if (!m_drawHeightMapShaderVSConstants.pCB || FAILED(m_pD3DDeviceContext->Map(m_drawHeightMapShaderVSConstants.pCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &vsMap)))
			vsMap.pData = NULL;

		D3D11_MAPPED_SUBRESOURCE psMap;
		if (!m_drawHeightMapShaderPSConstants.pCB || FAILED(m_pD3DDeviceContext->Map(m_drawHeightMapShaderPSConstants.pCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &psMap)))
			psMap.pData = NULL;

		SetCBufferFloat4x4(vsMap, m_drawHeightMapShaderVSConstants.shadowMatrix, m_shadowMtx);
		SetCBufferFloat4x4(psMap, m_drawHeightMapShaderPSConstants.shadowMatrix, m_shadowMtx);

		SetCBufferFloat4(vsMap, m_drawHeightMapShaderVSConstants.shadowColour, m_shadowColour);
		SetCBufferFloat4(psMap, m_drawHeightMapShaderPSConstants.shadowColour, m_shadowColour);

		if (psMap.pData)
			m_pD3DDeviceContext->Unmap(m_drawHeightMapShaderPSConstants.pCB, 0);

		if (vsMap.pData)
			m_pD3DDeviceContext->Unmap(m_drawHeightMapShaderVSConstants.pCB, 0);

		if (m_drawHeightMapShaderVSConstants.slot >= 0)
			m_pD3DDeviceContext->VSSetConstantBuffers(m_drawHeightMapShaderVSConstants.slot, 1, &m_drawHeightMapShaderVSConstants.pCB);

		if (m_drawHeightMapShaderPSConstants.slot >= 0)
			m_pD3DDeviceContext->PSSetConstantBuffers(m_drawHeightMapShaderPSConstants.slot, 1, &m_drawHeightMapShaderPSConstants.pCB);

		if (m_heightMapShaderShadowSampler >= 0)
			m_pD3DDeviceContext->PSSetSamplers(m_heightMapShaderShadowSampler, 1, &m_pShadowSamplerState);

		if (m_heightMapShaderShadowTexture >= 0)
			m_pD3DDeviceContext->PSSetShaderResources(m_heightMapShaderShadowTexture, 1, &m_pRenderTargetColourTextureView);

		m_pHeightMap->Draw(m_frameCount);
	}

	m_pAeroplane->Draw(m_pAeroplaneDefaultMeshes);
	m_pAnimation->Draw();

	XMMATRIX worldMtx = XMMatrixTranslation(m_shadowCastingLightPosition.x, m_shadowCastingLightPosition.y, m_shadowCastingLightPosition.z);
	this->SetWorldMatrix(worldMtx);
	m_pShadowCastingLightMesh->Draw();
}

void Application::Render2D() {
	float windowWidth, windowHeight;
	this->GetWindowSize(&windowWidth, &windowHeight);

	XMMATRIX projMtx = XMMatrixOrthographicOffCenterLH(0.f, windowWidth, 0.f, windowHeight, 1.f, 250.f);
	this->SetProjectionMatrix(projMtx);

	XMMATRIX viewMtx = XMMatrixTranslation(0.f, 0.f, 2.f);
	this->SetViewMatrix(viewMtx);

	this->SetWorldMatrix(XMMatrixIdentity());

	this->SetDepthStencilState(false, false);
	this->SetRasterizerState(false, false);
	this->SetBlendState(false);

	this->DrawTextured(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, m_pRenderTargetDebugDisplayBuffer, NULL, 4, m_pRenderTargetColourTextureView, this->GetSamplerState());

	if (help) {
		font->DrawString(XMFLOAT3(2.0f, windowHeight - 020.0f, 0.0f), &style, "Controls:");
		font->DrawString(XMFLOAT3(2.0f, windowHeight - 040.0f, 0.0f), &style, "C: Cycle Cameras");
		font->DrawString(XMFLOAT3(2.0f, windowHeight - 060.0f, 0.0f), &style, "F1: Robot Camera Toggle Rotate");
		font->DrawString(XMFLOAT3(2.0f, windowHeight - 080.0f, 0.0f), &style, "F2: Plane Camera");
		font->DrawString(XMFLOAT3(2.0f, windowHeight - 100.0f, 0.0f), &style, "Q/A/O/P: Plane Controls");
		font->DrawString(XMFLOAT3(2.0f, windowHeight - 120.0f, 0.0f), &style, "M: Toggle Plane Movement");
		font->DrawString(XMFLOAT3(2.0f, windowHeight - 140.0f, 0.0f), &style, "B: Drop Bomb");
		font->DrawString(XMFLOAT3(2.0f, windowHeight - 160.0f, 0.0f), &style, "1/2/3: Idle/Attack/Death");
		font->DrawString(XMFLOAT3(2.0f, windowHeight - 180.0f, 0.0f), &style, "0: Pause Animation");
		font->DrawString(XMFLOAT3(2.0f, windowHeight - 200.0f, 0.0f), &style, "S: Slow Animation");
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool Application::CreateRenderTarget()
{
	// Create render target texture, and two views for it - one for using it as
	// a render target, and one for using it as a texture.
	{
		HRESULT hr;

		D3D11_TEXTURE2D_DESC td;

		td.Width = RENDER_TARGET_WIDTH;
		td.Height = RENDER_TARGET_HEIGHT;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;

		hr = m_pD3DDevice->CreateTexture2D(&td, NULL, &m_pRenderTargetColourTexture);
		if (FAILED(hr))
		{
			this->SetStartErrorMessage("Failed to create render target colour texture.");
			return false;
		}

		D3D11_RENDER_TARGET_VIEW_DESC rtvd;

		rtvd.Format = td.Format;
		rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvd.Texture2D.MipSlice = 0;

		hr = m_pD3DDevice->CreateRenderTargetView(m_pRenderTargetColourTexture, &rtvd, &m_pRenderTargetColourTargetView);
		if (FAILED(hr))
		{
			this->SetStartErrorMessage("Failed to create render target colour target view.");
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC svd;

		svd.Format = td.Format;
		svd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		svd.Texture2D.MostDetailedMip = 0;
		svd.Texture2D.MipLevels = 1;

		hr = m_pD3DDevice->CreateShaderResourceView(m_pRenderTargetColourTexture, &svd, &m_pRenderTargetColourTextureView);
		if (FAILED(hr))
		{
			this->SetStartErrorMessage("Failed to create render target colour texture view.");
			return false;
		}
	}

	// Do the same again, roughly, for its matching depth buffer. (There is a
	// default depth buffer, but it can't really be re-used for this, because it
	// tracks the window size and the window could end up too small.)
	{
		HRESULT hr;

		D3D11_TEXTURE2D_DESC td;

		td.Width = RENDER_TARGET_WIDTH;
		td.Height = RENDER_TARGET_HEIGHT;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_D16_UNORM;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;

		hr = m_pD3DDevice->CreateTexture2D(&td, NULL, &m_pRenderTargetDepthStencilTexture);
		if (FAILED(hr))
		{
			this->SetStartErrorMessage("Failed to create render target depth/stencil texture.");
			return false;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;

		dsvd.Format = td.Format;
		dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvd.Flags = 0;
		dsvd.Texture2D.MipSlice = 0;

		hr = m_pD3DDevice->CreateDepthStencilView(m_pRenderTargetDepthStencilTexture, &dsvd, &m_pRenderTargetDepthStencilTargetView);
		if (FAILED(hr))
		{
			this->SetStartErrorMessage("Failed to create render target depth/stencil view.");
			return false;
		}
	}

	// VB for drawing render target on screen.
	static const Vertex_Pos3fColour4ubTex2f aRenderTargetDebugDisplayBufferVtxs[] = {
		Vertex_Pos3fColour4ubTex2f(XMFLOAT3(0.f, 0.f, 0.f), VertexColour(255, 255, 255, 255), XMFLOAT2(0.f, 1.f)),
		Vertex_Pos3fColour4ubTex2f(XMFLOAT3(256.f, 0.f, 0.f), VertexColour(255, 255, 255, 255), XMFLOAT2(1.f, 1.f)),
		Vertex_Pos3fColour4ubTex2f(XMFLOAT3(0.f, 256.f, 0.f), VertexColour(255, 255, 255, 255), XMFLOAT2(0.f, 0.f)),
		Vertex_Pos3fColour4ubTex2f(XMFLOAT3(256.f, 256.f, 0.f), VertexColour(255, 255, 255, 255), XMFLOAT2(1.f, 0.f)),
	};

	m_pRenderTargetDebugDisplayBuffer = CreateImmutableVertexBuffer(m_pD3DDevice, sizeof aRenderTargetDebugDisplayBufferVtxs, aRenderTargetDebugDisplayBufferVtxs);

	// Sampler state for using the shadow texture to draw with.
	{
		D3D11_SAMPLER_DESC sd;

		sd.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;

		// Use BORDER addressing, so that anything outside the area the shadow
		// texture casts on can be given a specific fixed colour.
		sd.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sd.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sd.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;

		sd.MipLODBias = 0.f;
		sd.MaxAnisotropy = 16;
		sd.ComparisonFunc = D3D11_COMPARISON_NEVER;

		// Set the border colour to transparent, corresponding to unshadowed.
		sd.BorderColor[0] = 0.f;
		sd.BorderColor[1] = 0.f;
		sd.BorderColor[2] = 0.f;
		sd.BorderColor[3] = 0.f;

		sd.MinLOD = 0.f;
		sd.MaxLOD = D3D11_FLOAT32_MAX;

		if (FAILED(m_pD3DDevice->CreateSamplerState(&sd, &m_pShadowSamplerState)))
			return false;
	}

	return true;
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
