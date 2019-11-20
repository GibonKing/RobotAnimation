#ifndef ANIMATION_H
#define ANIMATION_H

#include "Application.h"
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

__declspec(align(16)) class Animation
{
public:
	Animation(float fX, float fY, float fZ, float fRotY);
	~Animation();

	void setWorldPosition(float fX, float fY, float fZ);
	void setRotation(float fRotY);
	void setupModel();
	void updateMatrices();
	void Draw();
private:
	XMFLOAT4 worldPosition;
	XMFLOAT4 rotation;
	XMMATRIX worldMatrix;

	struct ModelPart{
		std::string name;
		std::string parent = "";
		int parentIterator;
		XMFLOAT4 offset;
		XMFLOAT4 rotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		CommonMesh* modelMesh;
		XMMATRIX modelMatrix;
	};
	std::vector<ModelPart> ModelParts;
};

#endif