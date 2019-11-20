#include "Animation.h"

Animation::Animation(float fX, float fY, float fZ, float fRotY) {
	setRotation(fRotY);
	setupModel();
	setWorldPosition(fX, fY, fZ);
}

Animation::~Animation() {}

void Animation::setWorldPosition(float fX, float fY, float fZ) {
	worldPosition = XMFLOAT4(fX, fY, fZ, 0.0f);
	updateMatrices();
}

void Animation::setRotation(float fRotY) {
	rotation = XMFLOAT4(0.0f, fRotY, 0.0f, 0.0f);
}

void Animation::setupModel() {
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

				modelPart.offset = XMFLOAT4(std::stof(x), std::stof(y), std::stof(z), 0.0f);
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

void Animation::updateMatrices() {
	XMMATRIX mRotX, mRotY, mRotZ, mTrans, parentMatrix;
	
	mRotX = XMMatrixRotationX(XMConvertToRadians(rotation.x));
	mRotY = XMMatrixRotationY(XMConvertToRadians(rotation.y));
	mRotZ = XMMatrixRotationZ(XMConvertToRadians(rotation.z));
	mTrans = XMMatrixTranslationFromVector(XMLoadFloat4(&worldPosition));
	worldMatrix = mRotZ * mRotX * mRotY * mTrans;

	for each (ModelPart modelPart in ModelParts)
	{
		if (modelPart.parent != "")
			parentMatrix = ModelParts[modelPart.parentIterator].modelMatrix;
		else
			parentMatrix = worldMatrix;

		mRotX = XMMatrixRotationX(XMConvertToRadians(modelPart.rotation.x));
		mRotY = XMMatrixRotationY(XMConvertToRadians(modelPart.rotation.y));
		mRotZ = XMMatrixRotationZ(XMConvertToRadians(modelPart.rotation.z));
		mTrans = XMMatrixTranslationFromVector(XMLoadFloat4(&modelPart.offset));
		modelPart.modelMatrix = mRotZ * mRotX * mRotY * mTrans * parentMatrix;
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