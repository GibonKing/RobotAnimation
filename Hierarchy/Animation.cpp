#include "Animation.h"

Animation::Animation(float fX, float fY, float fZ, float fRotY) {
	SetRotation(fRotY);
	SetupModel();
	SetupAnimations({ "Resources/Model/RobotIdleAnim.dae", "Resources/Model/RobotAttackAnim.dae", "Resources/Model/RobotDieAnimDAE.txt" });
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
		if (ModelParts[i].rotKeyframe + 1 >= ModelParts[i].ModelAnimations[animation].inputRotationValues.size()) {
			ModelParts[i].rotKeyframe = -1;
			ModelParts[i].rotFinish = true;
		}
		if (!ModelParts[i].rotFinish && ModelParts[i].ModelAnimations[animation].inputRotationValues[ModelParts[i].rotKeyframe + 1] < timeElapsed.count()) {
			ModelParts[i].rotation = ModelParts[i].ModelAnimations[animation].outputRotationValues[ModelParts[i].rotKeyframe + 1];
			ModelParts[i].rotKeyframe++;
		}			

		//Check Translation
		if (ModelParts[i].tranKeyframe + 1 >= ModelParts[i].ModelAnimations[animation].inputTranslationValues.size()) {
			ModelParts[i].tranKeyframe = -1;
			ModelParts[i].tranFinish = true;
		}
		if (!ModelParts[i].tranFinish && ModelParts[i].ModelAnimations[animation].inputTranslationValues[ModelParts[i].tranKeyframe + 1] < timeElapsed.count()) {
			ModelParts[i].offset = ModelParts[i].ModelAnimations[animation].outputTranslationValues[ModelParts[i].tranKeyframe + 1];
			ModelParts[i].tranKeyframe++;
		}
	}

	if (allModelFinish()) {
		for (int i(0); i < ModelParts.size(); i++) {
			time = 0;
			startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
			ModelParts[i].tranFinish = false;
			ModelParts[i].rotFinish = false;
		}
	}
}

bool Animation::allModelFinish() {
	for (int i(0); i < ModelParts.size(); i++) {
		if (!ModelParts[i].tranFinish || !ModelParts[i].rotFinish) {
			return false;
		}
	}
	return true;
}

void Animation::Update() {
	if (Application::s_pApp->IsKeyPressed('1')) {
		animation = 0;
	}
	if (Application::s_pApp->IsKeyPressed('2')) {
		animation = 1;
	}
	if (Application::s_pApp->IsKeyPressed('3')) {
		animation = 2;
	}

	if (Application::s_pApp->IsKeyPressed('L')) {
		time += 0.001;
		CheckKeyframes();
	}
	if (Application::s_pApp->IsKeyPressed('K')) {
		time -= 0.001;
		CheckKeyframes();
	}

	timeElapsed = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) - startTime) / 1000;
	CheckKeyframes();

	UpdateMatrices();
}