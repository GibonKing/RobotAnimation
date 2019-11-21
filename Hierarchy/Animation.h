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

	//Initial Setup
	void SetWorldPosition(float fX, float fY, float fZ);
	void SetRotation(float fRotY);
	void SetupModel();
	void SetupAnimations();

	//Update Functions
	void Update();
	void UpdateMatrices();
	void Draw();
private:
	XMFLOAT4 worldPosition;
	XMFLOAT4 rotation;
	XMMATRIX worldMatrix;
	int animation = 1;

	struct ModelPart{
		std::string name;
		std::string parent = "";
		int parentIterator;
		XMFLOAT4 offset;
		XMFLOAT4 rotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		CommonMesh* modelMesh;
		XMMATRIX modelMatrix;

		struct ModelAnimation {
			std::vector<XMFLOAT3> translationValues;
			std::vector<XMFLOAT3> rotationValues;
		};
		std::vector<ModelAnimation> ModelAnimations;
	};
	std::vector<ModelPart> ModelParts;

};

#endif