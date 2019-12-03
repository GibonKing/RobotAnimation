#ifndef APPLICATION_H
#define APPLICATION_H

#define WIN32_LEAN_AND_MEAN

#include <assert.h>

#include <stdio.h>
#include <windows.h>
#include <d3d11.h>

#include "CommonApp.h"
#include "CommonMesh.h"
#include "Timer.h"

class Aeroplane;
class HeightMap;
class Animation;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class Application : public CommonApp
{
  public:
	static Application* s_pApp;

  protected:
	bool HandleStart();
	void HandleStop();
	void HandleUpdate();
	void HandleRender();

  private:
	Timer timer;
	bool slowMo = false;
	float m_frameCount;
	bool m_reload;
	float m_rotationAngle;
	float m_cameraZ;
	bool m_bWireframe;

	int m_cameraState;

	Aeroplane* m_pAeroplane;
	HeightMap* m_pHeightMap;
	Animation* m_pAnimation;

	void ReloadShaders();
};

#endif
