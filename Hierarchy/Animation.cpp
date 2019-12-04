#include "Animation.h"

Animation::Animation(float fX, float fY, float fZ, float fRotY) {
	SetRotation(fRotY);
	SetupModel();
	SetupAnimations({ "Resources/Anim/RobotIdleAnim.dae", "Resources/Anim/RobotAttackAnim.dae", "Resources/Anim/RobotDieAnim.dae" });
	SetWorldPosition(fX, fY, fZ);
}

Animation::~Animation() {}

void Animation::SetWorldPosition(float fX, float fY, float fZ) {
	worldPosition = XMFLOAT4(fX, fY, fZ, 0.0f);
	UpdateMatrices();
}

void Animation::SetRotation(float fRotY) {
	rotation = XMFLOAT4(0.0f, fRotY, 0.0f, 0.0f);
}

void Animation::SetupModel() {
	int lineCounter(0);
	std::string info = "", line;
	std::ifstream inFile("Resources/Model/hierarchy.txt");

	ModelPart modelPart;
	while (std::getline(inFile, line)) {
		switch (lineCounter) {
			case 0:
				line.erase(std::remove(line.begin(), line.end(), '\"'), line.end());
				modelPart.name = line;
				break;
			case 1:
				line.erase(std::remove(line.begin(), line.end(), '\"'), line.end());
				modelPart.parent = line;
				break;
			case 2:
				std::string x,y,z;
				std::stringstream ss(line);
				std::getline(ss, x, ',');
				std::getline(ss, y, ',');
				std::getline(ss, z, ',');

				modelPart.offset = XMFLOAT4(std::stof(x)/10, std::stof(y)/10, std::stof(z)/10, 0.0f);
				break;
		}

		lineCounter++;
		if (lineCounter > 2) {
			std::string path = "Resources/Model/" + modelPart.name + ".x";
			modelPart.modelMesh = CommonMesh::LoadFromXFile(Application::s_pApp, path.c_str());
			ModelParts.push_back(modelPart);
			lineCounter = 0;
		}
	}

	for (int x(0); x < ModelParts.size(); x++) {
		for (int y(0); y < ModelParts.size(); y++) {
			if (ModelParts[x].parent == ModelParts[y].name) {
				ModelParts[x].parentIterator = y;
				break;
			}
		}
	}
}

void Animation::SetupAnimations(std::vector<std::string> filePaths) {
	for each(std::string filePath in filePaths) {
		SetupAnimation(filePath);
	}
}

void Animation::SetupAnimation(std::string filePath) {
	std::ifstream idleAnim(filePath);
	std::string line, temp1, part, prevPart, type;
	int pos, pos2, modelPartsIterator = -1, modelAnimationsIterator = 0;
	bool input;
	std::vector<float> inputTranslations, inputRotations, inputRotX, inputRotY, inputRotZ, outputRotX, outputRotY, outputRotZ;
	std::vector<XMFLOAT4> outputTranslations, outputRotations;

	std::string text = "";

	while (std::getline(idleAnim, line)) {
		//Find Part and Type Start
			pos = line.find("animation id");
			if (pos != -1) {
				temp1 = line.substr(pos);
				pos = temp1.find('"');
				if (pos != -1) {
					temp1 = temp1.substr(pos);
					pos = temp1.find(".");
					if (pos != -1) {

						//Get Part
						part = temp1.substr(1, pos - 1);
						if (part != prevPart) {
							if (modelPartsIterator != -1) {
								for (int valueCount(0); valueCount < outputRotX.size(); valueCount++) {
									outputRotations.push_back(XMFLOAT4(outputRotX[valueCount], outputRotY[valueCount], outputRotZ[valueCount], 0.0f));
								}
							}
							for (int models(0); models < ModelParts.size(); models++) {
								if (part == ModelParts[models].name) {
									if (modelPartsIterator != -1) {
										ModelParts[modelPartsIterator].ModelAnimations.push_back({ inputTranslations, inputRotations, outputTranslations, outputRotations });
										modelAnimationsIterator++;
										outputRotations.clear();
									}
									modelPartsIterator = models;
									break;
								}
							}
							prevPart = part;
						}

						//Get Type
						pos2 = temp1.find('>');
						type = temp1.substr(pos + 1, (pos2 - 1) - (pos + 1));
					}
				}
			}
		//Find Part and Type End
			else {
		//Find Values Start
				pos = line.find("float_array");
				if (pos != -1) {
					pos = line.find("output");
					if (pos != -1) {
						pos = line.find(">");
						temp1 = line.substr(pos + 1);
						pos2 = temp1.find("<");
						temp1 = temp1.substr(0, pos2);

						if (type == "translate") {
							outputTranslations = GetTranslationValues(temp1);
						}
						else if (type == "rotateX") {
							outputRotX = GetRotationValues(temp1);
						}
						else if (type == "rotateY") {
							outputRotY = GetRotationValues(temp1);
						}
						else if (type == "rotateZ") {
							outputRotZ = GetRotationValues(temp1);
						}
					}
					else {
						pos = line.find("input");
						if (pos != -1) {
							pos = line.find(">");
							temp1 = line.substr(pos + 1);
							pos2 = temp1.find("<");
							temp1 = temp1.substr(0, pos2);

							if (type == "translate") {
								inputTranslations = GetRotationValues(temp1);
							}
							else if (type == "rotateX") {
								inputRotations = GetRotationValues(temp1);
							}
						}
					}
				}
			}
		//Find Values End
	}

	for (int valueCount(0); valueCount < outputRotX.size(); valueCount++) {
		outputRotations.push_back(XMFLOAT4(outputRotX[valueCount], outputRotY[valueCount], outputRotZ[valueCount], 0.0f));
	}
	for (int models(0); models < ModelParts.size(); models++) {
		if (part == ModelParts[models].name) {
			if (modelPartsIterator != -1) {
				ModelParts[modelPartsIterator].ModelAnimations.push_back({ inputTranslations, inputRotations, outputTranslations, outputRotations });
				modelAnimationsIterator++;
				outputTranslations.clear();
				outputRotations.clear();
			}
			modelPartsIterator = models;
			break;
		}
	}

	animationCount++;
}

std::vector<float> Animation::GetRotationValues(std::string temp1) {
	std::string value;
	std::istringstream ss(temp1);
	std::vector<float> values;

	do {
		ss >> value;
		values.push_back(std::stof(value));
	} while (ss);
	values.pop_back();

	return values;
}

std::vector<XMFLOAT4> Animation::GetTranslationValues(std::string temp1) {
	std::string value;
	XMFLOAT4 vectorValue;
	std::vector<XMFLOAT4> vectorValues;
	int count(0);
	std::istringstream ss(temp1);

	do {
		ss >> value;

		switch (count) {
		case 0:
			vectorValue.x = std::stof(value)/10;
			break;
		case 1:
			vectorValue.y = std::stof(value)/10;
			break;
		case 2:
			vectorValue.z = std::stof(value)/10;
			vectorValue.w = 0.0f;
			vectorValues.push_back(vectorValue);
			count = -1;
			break;
		}
		count++;
	} while (ss);

	return vectorValues;
}

void Animation::UpdateMatrices() {
	XMMATRIX mRotX, mRotY, mRotZ, mTrans, parentMatrix;
	
	mRotX = XMMatrixRotationX(XMConvertToRadians(rotation.x));
	mRotY = XMMatrixRotationY(XMConvertToRadians(rotation.y));
	mRotZ = XMMatrixRotationZ(XMConvertToRadians(rotation.z));
	mTrans = XMMatrixTranslationFromVector(XMLoadFloat4(&worldPosition));
	worldMatrix = mRotX * mRotY * mRotZ * mTrans;

	for (int x(0); x < ModelParts.size(); x++)
	{
		if (ModelParts[x].parent != "")
			parentMatrix = ModelParts[ModelParts[x].parentIterator].modelMatrix;
		else
			parentMatrix = worldMatrix;

		mRotX = XMMatrixRotationX(XMConvertToRadians(ModelParts[x].rotation.x));
		mRotY = XMMatrixRotationY(XMConvertToRadians(ModelParts[x].rotation.y));
		mRotZ = XMMatrixRotationZ(XMConvertToRadians(ModelParts[x].rotation.z));
		mTrans = XMMatrixTranslationFromVector(XMLoadFloat4(&ModelParts[x].offset));
		ModelParts[x].modelMatrix = mRotX * mRotY * mRotZ * mTrans * parentMatrix;
	}
}

void Animation::Draw(void) {
	for each (ModelPart modelPart in ModelParts)
	{
		Application::s_pApp->SetWorldMatrix(modelPart.modelMatrix);
		if(modelPart.modelMesh != NULL)
			modelPart.modelMesh->Draw();
	}
}

void Animation::CheckKeyframes() {
	for (int i(0); i < ModelParts.size(); i++) {

		//Check Rotation
		if (ModelParts[i].rotKeyframe >= ModelParts[i].ModelAnimations[animation].inputRotationValues.size()) {
			ModelParts[i].rotKeyframe = 0;
			ModelParts[i].rotFinish = true;
		}	
		if (ModelParts[i].ModelAnimations[animation].inputRotationValues[ModelParts[i].rotKeyframe] <= timeElapsed) {
			ModelParts[i].rotKeyframe++;
		}
		if (ModelParts[i].rotKeyframe >= ModelParts[i].ModelAnimations[animation].inputRotationValues.size()) {
			ModelParts[i].rotKeyframe = 0;
			ModelParts[i].rotFinish = true;
		}

		//Check Translation
		if (ModelParts[i].tranKeyframe >= ModelParts[i].ModelAnimations[animation].inputTranslationValues.size()) {
			ModelParts[i].tranKeyframe = 0;
			ModelParts[i].tranFinish = true;
		}
		if (ModelParts[i].ModelAnimations[animation].inputTranslationValues[ModelParts[i].tranKeyframe] <= timeElapsed) {
			ModelParts[i].tranKeyframe++;
		}
		if (ModelParts[i].tranKeyframe >= ModelParts[i].ModelAnimations[animation].inputTranslationValues.size()) {
			ModelParts[i].tranKeyframe = 0;
			ModelParts[i].tranFinish = true;
		}
	}

	if (AllModelFinish()) {
		for (int i(0); i < ModelParts.size(); i++) {
			timeElapsed = 0;
			ModelParts[i].tranFinish = false;
			ModelParts[i].rotFinish = false;
		}
	}
}

bool Animation::AllModelFinish() {
	for (int i(0); i < ModelParts.size(); i++) {
		if (!ModelParts[i].tranFinish || !ModelParts[i].rotFinish) {
			return false;
		}
	}
	return true;
}

void Animation::Animate() {
	XMFLOAT4 rot, tran;
	for (int i(0); i < ModelParts.size(); i++) {
		ModelParts[i].anim1Rotation = AnimateRotations(animation, ModelParts[i]);
		ModelParts[i].anim1Offset = AnimateTranslations(animation, ModelParts[i]);
		if (blendTime < 1.0f) {
			ModelParts[i].anim2Rotation = AnimateRotations(prevAnimation, ModelParts[i]);
			ModelParts[i].anim2Offset = AnimateTranslations(prevAnimation, ModelParts[i]);

			XMStoreFloat4(&rot, XMVectorLerp(ModelParts[i].anim2Rotation, ModelParts[i].anim1Rotation, blendTime));
			XMStoreFloat4(&tran, XMVectorLerp(ModelParts[i].anim2Offset, ModelParts[i].anim1Offset, blendTime));
		}
		else {
			XMStoreFloat4(&rot, ModelParts[i].anim1Rotation);
			XMStoreFloat4(&tran, ModelParts[i].anim1Offset);
		}
		ModelParts[i].rotation = rot;
		ModelParts[i].offset = tran;
	}
}

XMVECTOR Animation::AnimateRotations(int anim, ModelPart modelPart) {
	int prevKeyframe, keyFrame;
	float localStartTime, localEndTime, framelength, frameDuration, framePercent;
	if (modelPart.rotKeyframe >= modelPart.ModelAnimations[anim].inputRotationValues.size()) {
		keyFrame = 0;
	}
	else {
		keyFrame = modelPart.rotKeyframe;
	}

	localEndTime = modelPart.ModelAnimations[anim].inputRotationValues[keyFrame];
	if (keyFrame == 0) {
		localStartTime = 0;
	}
	else {
		localStartTime = modelPart.ModelAnimations[anim].inputRotationValues[keyFrame - 1];
	}
	framelength = localEndTime - localStartTime;
	frameDuration = timeElapsed - localStartTime;
	framePercent = frameDuration / framelength;
	if (framePercent < 0)
		framePercent = 0;
	else if (framePercent > 1)
		framePercent = 1;

	if (keyFrame == 0) {
		prevKeyframe = modelPart.ModelAnimations[anim].outputRotationValues.size() - 1;
	}
	else {
		prevKeyframe = keyFrame - 1;
	}

	XMVECTOR currentRotVector = XMVECTOR(XMLoadFloat3(&XMFLOAT3(modelPart.ModelAnimations[anim].outputRotationValues[prevKeyframe].x,
																modelPart.ModelAnimations[anim].outputRotationValues[prevKeyframe].y,
																modelPart.ModelAnimations[anim].outputRotationValues[prevKeyframe].z)));
	XMVECTOR targetRotVector = XMVECTOR(XMLoadFloat3(&XMFLOAT3(modelPart.ModelAnimations[anim].outputRotationValues[keyFrame].x,
																modelPart.ModelAnimations[anim].outputRotationValues[keyFrame].y,
																modelPart.ModelAnimations[anim].outputRotationValues[keyFrame].z)));
	return XMVectorLerp(currentRotVector, targetRotVector, framePercent);
}

XMVECTOR Animation::AnimateTranslations(int anim, ModelPart modelPart) {
	int prevKeyframe, keyFrame;
	float localStartTime, localEndTime, framelength, frameDuration, framePercent;
	if (modelPart.tranKeyframe >= modelPart.ModelAnimations[anim].inputTranslationValues.size()) {
		keyFrame = 0;
	}
	else {
		keyFrame = modelPart.tranKeyframe;
	}

	localEndTime = modelPart.ModelAnimations[anim].inputTranslationValues[keyFrame];
	if (keyFrame == 0) {
		localStartTime = 0;
	}
	else {
		localStartTime = modelPart.ModelAnimations[anim].inputTranslationValues[keyFrame - 1];
	}
	framelength = localEndTime - localStartTime;
	frameDuration = timeElapsed - localStartTime;
	framePercent = frameDuration / framelength;
	if (framePercent < 0)
		framePercent = 0;
	else if (framePercent > 1)
		framePercent = 1;

	if (keyFrame == 0) {
		prevKeyframe = modelPart.ModelAnimations[anim].outputTranslationValues.size() - 1;
	}
	else {
		prevKeyframe = modelPart.tranKeyframe - 1;
	}

	XMVECTOR currentTranVector = XMVECTOR(XMLoadFloat3(&XMFLOAT3(modelPart.ModelAnimations[anim].outputTranslationValues[prevKeyframe].x,
																modelPart.ModelAnimations[anim].outputTranslationValues[prevKeyframe].y,
																modelPart.ModelAnimations[anim].outputTranslationValues[prevKeyframe].z)));
	XMVECTOR targetTranVector = XMVECTOR(XMLoadFloat3(&XMFLOAT3(modelPart.ModelAnimations[anim].outputTranslationValues[keyFrame].x,
																modelPart.ModelAnimations[anim].outputTranslationValues[keyFrame].y,
																modelPart.ModelAnimations[anim].outputTranslationValues[keyFrame].z)));
	return XMVectorLerp(currentTranVector, targetTranVector, framePercent);
}

void Animation::ChangeAnimation(int anim) {
	if (animation != anim) {
		prevAnimation = animation;
		animation = anim;
		blendTime = 0;
	}
}
void Animation::PauseAnimation() {
	animate = !animate;
}

void Animation::Update(float deltaTime) {
	if (animate) {
		timeElapsed += deltaTime;
		if (blendTime < 1.1)
			blendTime += deltaTime;
		CheckKeyframes();
		Animate();
		UpdateMatrices();
	}
}