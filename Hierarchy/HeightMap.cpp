#include "HeightMap.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

HeightMap::HeightMap(char* filename, float gridSize)
{
	LoadHeightMap(filename, gridSize);

	m_pHeightMapBuffer = NULL;

	static const VertexColour MAP_COLOUR(200, 255, 255, 255);

	m_HeightMapVtxCount = (m_HeightMapLength - 1) * (m_HeightMapWidth - 1) * 6;
	m_pMapVtxs = new Vertex_Pos3fColour4ubNormal3f[m_HeightMapVtxCount];

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
			XMFLOAT3 V0 = m_pHeightMap[mapIndex + m_HeightMapWidth];		//BottomLeft
			XMFLOAT3 V1 = m_pHeightMap[mapIndex];							//TopLeft
			XMFLOAT3 V2 = m_pHeightMap[mapIndex + m_HeightMapWidth + 1];	//BottomRight
			XMFLOAT3 V3 = m_pHeightMap[mapIndex + 1];						//TopRight

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

			XMVECTOR avgV2V = ( //Average Normal Bottom Right
				(centre == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[centre]) + XMLoadFloat3(&topNormals[centre]))) +
				(right == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[right]))) +
				(bot == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&topNormals[bot]))) +
				(botRight == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[botRight]) + XMLoadFloat3(&topNormals[botRight])))
				) / 6;
			XMFLOAT3 avgV2;
			XMStoreFloat3(&avgV2, avgV2V);

			//Put plots into array
			if (Even) {

				XMVECTOR avgV3V = ( //Average Normal Top Right
					(top == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[top]) + XMLoadFloat3(&topNormals[top]))) +
					(topRight == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[topRight]))) +
					(centre == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&topNormals[centre]))) +
					(right == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[right]) + XMLoadFloat3(&topNormals[right])))
					) / 6;
				XMFLOAT3 avgV3;
				XMStoreFloat3(&avgV3, avgV3V);

				if (useGridSqrX == 0) {

					XMVECTOR avgV0V = ( //Average Normal Bottom Left
						(left == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[left]) + XMLoadFloat3(&topNormals[left]))) +
						(centre == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[centre]))) +
						(botLeft == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&topNormals[botLeft]))) +
						(bot == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[bot]) + XMLoadFloat3(&topNormals[bot])))
						) / 6;
					XMFLOAT3 avgV0;
					XMStoreFloat3(&avgV0, avgV0V);

					XMVECTOR avgV1V = ( //Average Normal Top Left
						(topLeft == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[topLeft]) + XMLoadFloat3(&topNormals[topLeft]))) +
						(top == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[top]))) +
						(left == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&topNormals[left]))) +
						(centre == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[centre]) + XMLoadFloat3(&topNormals[centre])))
						) / 6;
					XMFLOAT3 avgV1;
					XMStoreFloat3(&avgV1, avgV1V);

					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V0, MAP_COLOUR, avgV0);
					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V1, MAP_COLOUR, avgV1);
				}

				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V2, MAP_COLOUR, avgV2);
				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V3, MAP_COLOUR, avgV3);

				if (useGridSqrX == width - 1) {
					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V2, MAP_COLOUR, avgV2);
				}
			}
			else {

				XMVECTOR avgV1V = ( //Average Normal Top Left
					(topLeft == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[topLeft]) + XMLoadFloat3(&topNormals[topLeft]))) +
					(top == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[top]))) +
					(left == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&topNormals[left]))) +
					(centre == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[centre]) + XMLoadFloat3(&topNormals[centre])))
					) / 6;
				XMFLOAT3 avgV1;
				XMStoreFloat3(&avgV1, avgV1V);

				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V2, MAP_COLOUR, avgV2);
				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V1, MAP_COLOUR, avgV1);

				if (useGridSqrX == 0) {

					XMVECTOR avgV0V = ( //Average Normal Bottom Left
						(left == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[left]) + XMLoadFloat3(&topNormals[left]))) +
						(centre == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[centre]))) +
						(botLeft == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&topNormals[botLeft]))) +
						(bot == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[bot]) + XMLoadFloat3(&topNormals[bot])))
						) / 6;
					XMFLOAT3 avgV0;
					XMStoreFloat3(&avgV0, avgV0V);

					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V0, MAP_COLOUR, avgV0);
				}
			}
		}
		Even = !Even;
	}

	m_pHeightMapBuffer = CreateImmutableVertexBuffer(Application::s_pApp->GetDevice(), sizeof Vertex_Pos3fColour4ubNormal3f * m_HeightMapVtxCount, m_pMapVtxs);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

HeightMap::~HeightMap()
{
	Release(m_pHeightMapBuffer);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void HeightMap::Draw(void)
{
	Application::s_pApp->DrawUntexturedLit(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, m_pHeightMapBuffer, NULL, m_HeightMapVtxCount);
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
//////////////////////////////////////////////////////////////////////
