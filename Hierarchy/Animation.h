#ifndef ANIMATION_H
#define ANIMATION_H

#include "Application.h"
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <cmath>

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
	void Draw();
	void CheckKeyframes();
	void Animate();

	//Extra Functions
	std::vector<float> GetRotationValues(std::string temp1);
	std::vector<XMFLOAT4> GetTranslationValues(std::string temp1);
	void SetTargets();
	bool AllModelFinish();
private:
	XMFLOAT4 worldPosition, rotation;
	XMMATRIX worldMatrix;
	int animationCount = 0, animation = 0;
	std::chrono::milliseconds timeElapsed, startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

	struct ModelPart{
		std::string name, parent = "";
		int parentIterator, rotKeyframe = 0, tranKeyframe = 0;
		XMFLOAT4 offset, rotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), targetOffset, targetRotation;
		CommonMesh* modelMesh;
		XMMATRIX modelMatrix;
		bool tranFinish, rotFinish;

		struct ModelAnimation {
			std::vector<float> inputTranslationValues, inputRotationValues;
			std::vector<XMFLOAT4> outputTranslationValues, outputRotationValues;
		};
		std::vector<ModelAnimation> ModelAnimations;
	};
	std::vector<ModelPart> ModelParts;
};

#endif