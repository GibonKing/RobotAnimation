#ifndef ANIMATION_H
#define ANIMATION_H

#include "Application.h"
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>

__declspec(align(16)) class Animation
{
public:
	Animation(float fX, float fY, float fZ, float fRotY);
	~Animation();

	//Initial Setup
	void SetWorldPosition(float fX, float fY, float fZ);
	void SetRotation(float fRotY);
	void SetupModel();
	void SetupAnimations(std::vector<std::string> filePaths);
	void SetupAnimation(std::string filePath);

	//Update Functions
	void Update();
	void UpdateMatrices();
	void UpdateKeyframe();
	void Draw();
	void Animate();

	//Extra Functions
	std::vector<float> GetRotationValues(std::string temp1);
	std::vector<XMFLOAT3> GetTranslationValues(std::string temp1);
private:
	XMFLOAT4 worldPosition;
	XMFLOAT4 rotation;
	XMMATRIX worldMatrix;
	int animation = 0;
	float time = 0;

	struct ModelPart{
		std::string name;
		std::string parent = "";
		int parentIterator, rotKeyframe = 0, tranKeyframe = 0;
		XMFLOAT4 offset;
		XMFLOAT4 rotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		CommonMesh* modelMesh;
		XMMATRIX modelMatrix;

		struct ModelAnimation {
			std::vector<float> inputTranslationValues;
			std::vector<float> inputRotationValues;
			std::vector<XMFLOAT3> outputTranslationValues;
			std::vector<XMFLOAT3> outputRotationValues;
		};
		std::vector<ModelAnimation> ModelAnimations;
	};
	std::vector<ModelPart> ModelParts;
};

#endif