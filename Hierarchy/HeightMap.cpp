#include "HeightMap.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

HeightMap::HeightMap(char* filename, float gridSize)
{
	for (size_t i = 0; i < NUM_TEXTURE_FILES; ++i)
	{
		m_pTextures[i] = NULL;
		m_pTextureViews[i] = NULL;
	}
	m_pSamplerState = NULL;
	m_pMyAppCBuffer = NULL;

	LoadHeightMap(filename, gridSize);
	m_pHeightMapBuffer = NULL;

	static const VertexColour MAP_COLOUR(200, 255, 255, 255);

	m_HeightMapVtxCount = (m_HeightMapLength - 1) * (m_HeightMapWidth - 1) * 6;
	m_pMapVtxs = new Vertex_Pos3fColour4ubNormal3fTex2f[m_HeightMapVtxCount];

	int mapIndex = 0;
	int vertex(0);
	int normal(0);
	int width = m_HeightMapWidth - 1;
	int height = m_HeightMapLength - 1;
	bool Even = true;
	int useGridSqrX;
	int normalCount = width * height * 3;
	std::vector<XMFLOAT3> topNormals(normalCount);
	std::vector<XMFLOAT3> botNormals(normalCount);
	XMFLOAT3 zero(0.0f, 0.0f, 0.0f);
	XMFLOAT3 v512 = XMFLOAT3(512.0f, 0.0f, 512.0f);
	XMVECTOR vOffset = XMLoadFloat3(&v512);

	int topLeft;
	int top;
	int topRight;
	int left;
	int centre;
	int right;
	int botLeft;
	int bot;
	int botRight;
	int miTopLeft;
	int miTop;
	int miTopRight;
	int miLeft;
	int miCentre;
	int miRight;
	int miBotLeft;
	int miBot;
	int miBotRight;


	for (int gridSqreZ(0); gridSqreZ < height; gridSqreZ++) {
		for (int gridSqrX(0); gridSqrX < width; gridSqrX++) {

			if (Even)
				useGridSqrX = gridSqrX;
			else
				useGridSqrX = (width - 1) - gridSqrX;

			mapIndex = (gridSqreZ * m_HeightMapWidth) + useGridSqrX;

			//Vertices
			XMFLOAT3 V0 = m_pHeightMap[mapIndex + m_HeightMapWidth];		//BottomLeft
			XMFLOAT3 V1 = m_pHeightMap[mapIndex];							//TopLeft
			XMFLOAT3 V2 = m_pHeightMap[mapIndex + m_HeightMapWidth + 1];	//BottomRight
			XMFLOAT3 V3 = m_pHeightMap[mapIndex + 1];						//TopRight

			//Vectors between points
			XMVECTOR V0V1 = XMLoadFloat3(&V1) - XMLoadFloat3(&V0);
			XMVECTOR V0V2 = XMLoadFloat3(&V2) - XMLoadFloat3(&V0);
			XMVECTOR V3V1 = XMLoadFloat3(&V1) - XMLoadFloat3(&V3);
			XMVECTOR V3V2 = XMLoadFloat3(&V2) - XMLoadFloat3(&V3);

			//Normal Vectors
			XMVECTOR TNV = XMVector3Cross(-V3V1, V3V2);
			XMVECTOR BNV = XMVector3Cross(V0V1, V0V2);

			//Normal Floats
			XMFLOAT3 TNF;
			XMStoreFloat3(&TNF, TNV);
			XMFLOAT3 BNF;
			XMStoreFloat3(&BNF, BNV);

			//Add normals to array
			topNormals[mapIndex] = TNF;
			botNormals[mapIndex] = BNF;
		}
		Even = !Even;
	}

	normal = 0;

	for (int gridSqreZ(0); gridSqreZ < height; gridSqreZ++) {
		for (int gridSqrX(0); gridSqrX < width; gridSqrX++) {

			if (Even)
				useGridSqrX = gridSqrX;
			else
				useGridSqrX = (width - 1) - gridSqrX;

			//Map Indexes
			mapIndex = (gridSqreZ * m_HeightMapWidth) + useGridSqrX;
			miTopLeft = mapIndex - m_HeightMapWidth - 1;
			miTop = mapIndex - m_HeightMapWidth;
			miTopRight = mapIndex - m_HeightMapWidth + 1;
			miLeft = mapIndex - 1;
			miCentre = mapIndex;
			miRight = mapIndex + 1;
			miBotLeft = mapIndex + m_HeightMapWidth - 1;
			miBot = mapIndex + m_HeightMapWidth;
			miBotRight = mapIndex + m_HeightMapWidth + 1;

			//Vertices
			XMVECTOR V0 = XMLoadFloat3(&m_pHeightMap[mapIndex + m_HeightMapWidth]);		//BottomLeft
			XMVECTOR V1 = XMLoadFloat3(&m_pHeightMap[mapIndex]);							//TopLeft
			XMVECTOR V2 = XMLoadFloat3(&m_pHeightMap[mapIndex + m_HeightMapWidth + 1]);	//BottomRight
			XMVECTOR V3 = XMLoadFloat3(&m_pHeightMap[mapIndex + 1]);						//TopRight

			// Spread textures evenly across landscape
			XMVECTOR t0 = (V0 + vOffset) / 32.0f;
			XMVECTOR t1 = (V1 + vOffset) / 32.0f;
			XMVECTOR t2 = (V2 + vOffset) / 32.0f;
			XMVECTOR t3 = (V3 + vOffset) / 32.0f;

			t0 = XMVectorSwizzle(t0, 0, 2, 1, 3);
			t1 = XMVectorSwizzle(t1, 0, 2, 1, 3);
			t2 = XMVectorSwizzle(t2, 0, 2, 1, 3);
			t3 = XMVectorSwizzle(t3, 0, 2, 1, 3);

			//Check Array Values
			topLeft = miTopLeft >= 0 ? miTopLeft < normalCount ? miTopLeft : -1 : -1;
			top = miTop >= 0 ? miTop < normalCount ? miTop : -1 : -1;
			topRight = miTopRight >= 0 ? miTopRight < normalCount ? miTopRight : -1 : -1;
			left = miLeft >= 0 ? miLeft < normalCount ? miLeft : -1 : -1;
			centre = miCentre >= 0 ? miCentre < normalCount ? miCentre : -1 : -1;
			right = miRight >= 0 ? miRight < normalCount ? miRight : -1 : -1;
			botLeft = miBotLeft >= 0 ? miBotLeft < normalCount ? miBotLeft : -1 : -1;
			bot = miBot >= 0 ? miBot < normalCount ? miBot : -1 : -1;
			botRight = miBotRight >= 0 ? miBotRight < normalCount ? miBotRight : -1 : -1;

			XMVECTOR avgV2 = ( //Average Normal Bottom Right
				(centre == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[centre]) + XMLoadFloat3(&topNormals[centre]))) +
				(right == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[right]))) +
				(bot == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&topNormals[bot]))) +
				(botRight == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[botRight]) + XMLoadFloat3(&topNormals[botRight])))
				) / 6;

			//Put plots into array
			if (Even) {

				XMVECTOR avgV3 = ( //Average Normal Top Right
					(top == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[top]) + XMLoadFloat3(&topNormals[top]))) +
					(topRight == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[topRight]))) +
					(centre == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&topNormals[centre]))) +
					(right == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[right]) + XMLoadFloat3(&topNormals[right])))
					) / 6;

				if (useGridSqrX == 0) {

					XMVECTOR avgV0 = ( //Average Normal Bottom Left
						(left == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[left]) + XMLoadFloat3(&topNormals[left]))) +
						(centre == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[centre]))) +
						(botLeft == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&topNormals[botLeft]))) +
						(bot == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[bot]) + XMLoadFloat3(&topNormals[bot])))
						) / 6;

					XMVECTOR avgV1 = ( //Average Normal Top Left
						(topLeft == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[topLeft]) + XMLoadFloat3(&topNormals[topLeft]))) +
						(top == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[top]))) +
						(left == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&topNormals[left]))) +
						(centre == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[centre]) + XMLoadFloat3(&topNormals[centre])))
						) / 6;

					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3fTex2f(V0, MAP_COLOUR, avgV0, t0);
					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3fTex2f(V1, MAP_COLOUR, avgV1, t1);
				}

				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3fTex2f(V2, MAP_COLOUR, avgV2, t2);
				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3fTex2f(V3, MAP_COLOUR, avgV3, t3);

				if (useGridSqrX == width - 1) {
					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3fTex2f(V2, MAP_COLOUR, avgV2, t2);
				}
			}
			else {

				XMVECTOR avgV1 = ( //Average Normal Top Left
					(topLeft == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[topLeft]) + XMLoadFloat3(&topNormals[topLeft]))) +
					(top == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[top]))) +
					(left == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&topNormals[left]))) +
					(centre == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[centre]) + XMLoadFloat3(&topNormals[centre])))
					) / 6;

				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3fTex2f(V2, MAP_COLOUR, avgV2, t2);
				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3fTex2f(V1, MAP_COLOUR, avgV1, t1);

				if (useGridSqrX == 0) {

					XMVECTOR avgV0 = ( //Average Normal Bottom Left
						(left == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[left]) + XMLoadFloat3(&topNormals[left]))) +
						(centre == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[centre]))) +
						(botLeft == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&topNormals[botLeft]))) +
						(bot == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[bot]) + XMLoadFloat3(&topNormals[bot])))
						) / 6;

					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3fTex2f(V0, MAP_COLOUR, avgV0, t0);
				}
			}
		}
		Even = !Even;
	}

	m_pHeightMapBuffer = CreateImmutableVertexBuffer(Application::s_pApp->GetDevice(), sizeof Vertex_Pos3fColour4ubNormal3fTex2f * m_HeightMapVtxCount, m_pMapVtxs);

	for (size_t i = 0; i < NUM_TEXTURE_FILES; ++i)
	{
		LoadTextureFromFile(Application::s_pApp->GetDevice(), g_aTextureFileNames[i], &m_pTextures[i], &m_pTextureViews[i], &m_pSamplerState);
	}

	ReloadShader(); // This compiles the shader
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

HeightMap::~HeightMap()
{
	Release(m_pHeightMapBuffer);

	DeleteShader();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void HeightMap::Draw(float frameCount)
{
	XMMATRIX worldMtx = XMMatrixIdentity();

	ID3D11DeviceContext* pContext = Application::s_pApp->GetDeviceContext();

	Application::s_pApp->SetWorldMatrix(worldMtx);

	// Update the data in our `MyApp' cbuffer.
	// The D3D11_MAP_WRITE_DISCARD flag is best for performance,
	// However, when mapping, the previous buffer contents are indeterminate. So the entire buffer
	// must be written.

	if (m_pMyAppCBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE map;
		if (SUCCEEDED(pContext->Map(m_pMyAppCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
		{
			// Set the buffer contents. There is only one variable to set in this case.
			// This method relies on the offset which has been found through Shader Reflection.
			SetCBufferFloat(map, m_frameCountOffset, frameCount);
			pContext->Unmap(m_pMyAppCBuffer, 0);
		}
	}

	// Bind the same constant buffer to any stages that use it.
	if (m_psMyAppCBufferSlot != -1)
	{
		pContext->PSSetConstantBuffers(m_psMyAppCBufferSlot, 1, &m_pMyAppCBuffer);
	}
	if (m_vsMyAppCBufferSlot != -1)
	{
		pContext->VSSetConstantBuffers(m_vsMyAppCBufferSlot, 1, &m_pMyAppCBuffer);
	}

	if (m_psTexture0 >= 0)
		pContext->PSSetShaderResources(m_psTexture0, 1, &m_pTextureViews[0]);

	if (m_psTexture1 >= 0)
		pContext->PSSetShaderResources(m_psTexture1, 1, &m_pTextureViews[1]);

	if (m_psTexture2 >= 0)
		pContext->PSSetShaderResources(m_psTexture2, 1, &m_pTextureViews[2]);

	if (m_vsMaterialMap >= 0)
		pContext->VSSetShaderResources(m_vsMaterialMap, 1, &m_pTextureViews[3]);

	m_pSamplerState = Application::s_pApp->GetSamplerState(true, true, true);

	Application::s_pApp->DrawWithShader(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, m_pHeightMapBuffer, sizeof(Vertex_Pos3fColour4ubNormal3fTex2f),
										NULL, 0, m_HeightMapVtxCount, NULL, m_pSamplerState, &m_shader);
}

bool HeightMap::ReloadShader(void)
{
	DeleteShader();

	ID3D11VertexShader* pVS = NULL;
	ID3D11PixelShader* pPS = NULL;
	ID3D11InputLayout* pIL = NULL;
	ShaderDescription vs, ps;

	ID3D11Device* pDevice = Application::s_pApp->GetDevice();

	// When the CommonApp draw functions set any of the light arrays,
	// they assume that the arrays are of CommonApp::MAX_NUM_LIGHTS
	// in size. Using a shader compiler #define is an easy way to
	// get this value to the shader.

	char maxNumLightsValue[100];
	_snprintf_s(maxNumLightsValue, sizeof maxNumLightsValue, _TRUNCATE, "%d", CommonApp::MAX_NUM_LIGHTS);

	D3D_SHADER_MACRO aMacros[] = {
		{
			"MAX_NUM_LIGHTS",
			maxNumLightsValue,
		},
		{NULL},
	};

	if (!CompileShadersFromFile(pDevice, "./Resources/ExampleShader.hlsl", "VSMain", &pVS, &vs, g_aVertexDesc_Pos3fColour4ubNormal3fTex2f,
								g_vertexDescSize_Pos3fColour4ubNormal3fTex2f, &pIL, "PSMain", &pPS, &ps, aMacros))
	{
		return false; // false;
	}

	Application::s_pApp->CreateShaderFromCompiledShader(&m_shader, pVS, &vs, pIL, pPS, &ps);

	// Find cbuffer(s) for the globals that won't get set by the CommonApp
	// code. These are shader-specific; you have to know what you are
	// looking for, if you're going to set it...

	// Find the constant buffer which might in either shader or both.
	// However, it should be the same size and parameter offsets in either.
	uint32_t cBufferSize = 0;
	if (ps.FindCBuffer("MyApp", &m_psMyAppCBufferSlot))
	{
		cBufferSize = ps.GetCBufferSizeBytes(m_psMyAppCBufferSlot);
		ps.FindFloat(m_psMyAppCBufferSlot, "g_frameCount", &m_frameCountOffset);
	}

	// We have to find the constant buffer slot in the vertex shader too
	if (vs.FindCBuffer("MyApp", &m_vsMyAppCBufferSlot))
	{
		cBufferSize = vs.GetCBufferSizeBytes(m_vsMyAppCBufferSlot);
		vs.FindFloat(m_vsMyAppCBufferSlot, "g_frameCount", &m_frameCountOffset);
	}

	ps.FindTexture("g_texture0", &m_psTexture0);
	ps.FindTexture("g_texture1", &m_psTexture1);
	ps.FindTexture("g_texture2", &m_psTexture2);

	vs.FindTexture("g_materialMap", &m_vsMaterialMap);

	// Create a cbuffer, using the shader description to find out how
	// large it needs to be.
	if (cBufferSize > 0)
	{
		m_pMyAppCBuffer = CreateBuffer(pDevice, cBufferSize, D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, NULL);
	}

	// In this example we are sharing the constant buffer between both vertex and pixel shaders.
	// This is efficient since we only update one buffer. However we could define separate constant buffers for each stage.
	// Generally constant buffers should represent groups of variables that must be updated at the same rate.
	// So : we might have 'per execution' 'per frame', 'per draw' constant buffers.

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void HeightMap::DeleteShader()
{
	Release(m_pMyAppCBuffer);
	m_shader.Reset();
}

//////////////////////////////////////////////////////////////////////
// LoadHeightMap
// Original code sourced from rastertek.com
//////////////////////////////////////////////////////////////////////
bool HeightMap::LoadHeightMap(char* filename, float gridSize)
{
	FILE* filePtr;
	int error;
	unsigned int count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	int imageSize, i, j, k, index;
	unsigned char* bitmapImage;
	unsigned char height;

	// Open the height map file in binary.
	error = fopen_s(&filePtr, filename, "rb");
	if(error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Read in the bitmap info header.
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Save the dimensions of the terrain.
	m_HeightMapWidth = bitmapInfoHeader.biWidth;
	m_HeightMapLength = bitmapInfoHeader.biHeight;

	// Calculate the size of the bitmap image data.
	imageSize = m_HeightMapWidth * m_HeightMapLength * 3;

	// Allocate memory for the bitmap image data.
	bitmapImage = new unsigned char[imageSize];
	if(!bitmapImage)
	{
		return false;
	}

	// Move to the beginning of the bitmap data.
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Read in the bitmap image data.
	count = fread(bitmapImage, 1, imageSize, filePtr);
	if(count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if(error != 0)
	{
		return false;
	}

	// Create the structure to hold the height map data.
	XMFLOAT3* pUnsmoothedMap = new XMFLOAT3[m_HeightMapWidth * m_HeightMapLength];
	m_pHeightMap = new XMFLOAT3[m_HeightMapWidth * m_HeightMapLength];

	if(!m_pHeightMap)
	{
		return false;
	}

	// Initialize the position in the image data buffer.
	k = 0;

	// Read the image data into the height map.
	// Bitmaps store the data bottom to top so it needs to be flipped!
	for(j = m_HeightMapLength-1; j>=0; j--)
	{
		for(i = 0; i < m_HeightMapWidth; i++)
		{
			height = bitmapImage[k];
			index = (m_HeightMapWidth * j) + i;
			m_pHeightMap[index].x = (float)(i - (m_HeightMapWidth / 2)) * gridSize;
			m_pHeightMap[index].y = (float)height / 6 * gridSize;
			m_pHeightMap[index].z = (float)((m_HeightMapLength / 2) - j) * gridSize;
			pUnsmoothedMap[index].y = (float)height / 6 * gridSize;
			k += 3;
		}
	}

	// Smoothing the landscape mesh makes a difference to the look of the shading (separate to smoothing the normals)
	for(int s = 0; s < 2; ++s)
	{
		for(j = 0; j < m_HeightMapLength; j++)
		{
			for(i = 0; i < m_HeightMapWidth; i++)
			{
				index = (m_HeightMapWidth * j) + i;

				if(j > 0 && j < m_HeightMapLength - 1 && i > 0 && i < m_HeightMapWidth - 1)
				{
					m_pHeightMap[index].y = 0.0f;
					m_pHeightMap[index].y += pUnsmoothedMap[index - m_HeightMapWidth - 1].y + pUnsmoothedMap[index - m_HeightMapWidth].y + pUnsmoothedMap[index - m_HeightMapWidth + 1].y;
					m_pHeightMap[index].y += pUnsmoothedMap[index - 1].y + pUnsmoothedMap[index].y + pUnsmoothedMap[index + 1].y;
					m_pHeightMap[index].y += pUnsmoothedMap[index + m_HeightMapWidth - 1].y + pUnsmoothedMap[index + m_HeightMapWidth].y + pUnsmoothedMap[index + m_HeightMapWidth + 1].y;
					m_pHeightMap[index].y /= 9;
				}
			}
		}

		for(j = 0; j < m_HeightMapLength; j++)
		{
			for(i = 0; i < m_HeightMapWidth; i++)
			{
				index = (m_HeightMapWidth * j) + i;
				pUnsmoothedMap[index].y = m_pHeightMap[index].y;
			}
		}
	}

	// Release the bitmap image data.
	delete[] bitmapImage;
	delete[] pUnsmoothedMap;
	bitmapImage = 0;

	return true;
}

//////////////////////////////////////////////////////////////////////
////////////////////  COLLISION DETECTION  ///////////////////////////
//////////////////////////////////////////////////////////////////////

bool HeightMap::RayCollision(XMVECTOR& rayPos, XMVECTOR rayDir, float raySpeed, XMVECTOR& colPos, XMVECTOR& colNormN)
{

	XMVECTOR v0, v1, v2, v3;
	int i0, i1, i2, i3;
	float colDist = 0.0f;

	// This is a brute force solution that checks against every triangle in the heightmap
	for (int l = 0; l < m_HeightMapLength - 1; ++l)
	{
		for (int w = 0; w < m_HeightMapWidth - 1; ++w)
		{
			int mapIndex = (l*m_HeightMapWidth) + w;
	
			i0 = mapIndex;
			i1 = mapIndex + m_HeightMapWidth;
			i2 = mapIndex + 1;
			i3 = mapIndex + m_HeightMapWidth + 1;
	
			v0 = XMLoadFloat3(&m_pHeightMap[i0]);
			v1 = XMLoadFloat3(&m_pHeightMap[i1]);
			v2 = XMLoadFloat3(&m_pHeightMap[i2]);
			v3 = XMLoadFloat3(&m_pHeightMap[i3]);
	
			//012 213
			if (RayTriangle(v0, v2, v1, rayPos, rayDir, colPos, colNormN, colDist))
			{
				// Needs to be >=0 
				if (colDist <= raySpeed && colDist >= 0.0f)
				{
					return true;
				}	
			}
			// 213
			if (RayTriangle(v1, v2, v3, rayPos, rayDir, colPos, colNormN, colDist))
			{
				// Needs to be >=0 
				if (colDist <= raySpeed && colDist >= 0.0f)
				{	
					return true;
				}
			}
		}	
	}	
	return false;
}

bool HeightMap::RayTriangle(const XMVECTOR& vert0, const XMVECTOR& vert1, const XMVECTOR& vert2, const XMVECTOR& rayPos, const XMVECTOR& rayDir, XMVECTOR& colPos, XMVECTOR& colNormN, float& colDist)
{
	// Part 1: Calculate the collision point between the ray and the plane on which the triangle lies

	// Step 1: Calculate |COLNORM| 
	colNormN = XMVector3Normalize(XMVector3Cross((vert2 - vert1), (vert0 - vert1)));

	// Step 2: Use |COLNORM| and any vertex on the triangle to calculate D
	float D = -XMVectorGetX(XMVector3Dot(colNormN, vert0)); // D = -(|COLNORM| dot |ANYVERT|)

	// Step 3: Calculate the demoninator of the COLDIST equation: (|COLNORM| dot |RAYDIR|) and "early out" (return false) if it is 0
	float colDistDenom = XMVectorGetX(XMVector3Dot(colNormN, XMVector3Normalize(rayDir)));
	if (colDistDenom == 0.0f)
		return false;

	// Step 4: Calculate the numerator of the COLDIST equation: -(D+(|COLNORM| dot RAYPOS))
	float colDistNumerator = -(D + XMVectorGetX(XMVector3Dot(colNormN, rayPos)));

	// Step 5: Calculate COLDIST and "early out" again if COLDIST is behind RAYDIR
	colDist = (colDistNumerator / colDistDenom);
	if (colDist < 0.0f)
		return false;

	// Step 6: Use COLDIST to calculate COLPOS
	colPos = rayPos + colDist * XMVector3Normalize(rayDir); //COLPOS = RAYPOS + COLDIST * | RAYDIR |

	// Part 2: Work out if the intersection point falls within the triangle

	// Move the ray backwards by a tiny bit (one unit) in case the ray is already on the plane
	XMVECTOR RAYPOS = XMVectorSubtract(rayPos, XMVector3Normalize(rayDir));

	// Step 1: Test against plane 1 and return false if behind plane
	if (!PointPlane(RAYPOS, vert0, vert1, colPos))
		return false;

	// Step 2: Test against plane 2 and return false if behind plane
	if (!PointPlane(RAYPOS, vert1, vert2, colPos))
		return false;

	// Step 3: Test against plane 3 and return false if behind plane
	if (!PointPlane(RAYPOS, vert2, vert0, colPos))
		return false;

	// Step 4: Return true! (on triangle)
	return true;
}

bool HeightMap::PointPlane(const XMVECTOR& vert0, const XMVECTOR& vert1, const XMVECTOR& vert2, const XMVECTOR& pointPos)
{
	XMVECTOR sVec0, sVec1, sNormN;
	float sD, sNumer;

	// Step 1: Calculate PNORM
	sVec0 = vert1 - vert0;
	sVec1 = vert2 - vert0;
	sNormN = XMVector3Normalize(XMVector3Cross(sVec1, sVec0));

	// Step 2: Calculate D
	sD = -XMVectorGetX(XMVector3Dot(sNormN, vert0)); //D = -(| PNORM | dot VERT0)

	// Step 3: Calculate full equation
	sNumer = XMVectorGetX(XMVector3Dot(sNormN, pointPos)) + sD; //(|PNORM| dot POINTPOS) - (|PNORM| dot VERT0)

	// Step 4: Return false if < 0 (behind plane)
	if (sNumer < 0.0f)
		return false;

	// Step 5: Return true! (in front of plane)
	return true;
}