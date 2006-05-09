#include "Fuji.h"
#include "F3D.h"
#include "MFMatrix.h"

#include "tinyxml/tinyxml.h"

#include <string>
#include <vector>

extern F3DFile *pModel;

TiXmlElement* pRoot;

MFMatrix transformMatrix;
bool flipWinding = false;

void startElement(void *userData, const char *name, const char **atts)
{
  int i;
  int *depthPtr = (int*)userData;
  for (i = 0; i < *depthPtr; i++)
    putchar('\t');
  puts(name);
  *depthPtr += 1;
}

void endElement(void *userData, const char *name)
{
  int *depthPtr = (int*)userData;
  *depthPtr -= 1;
}

bool TestID(TiXmlElement *pLib, const char *pLibName)
{
	const char *pName = pLib->Attribute("id");
	if(pName && !stricmp(pName, pLibName))
		return true;
	return false;
}

TiXmlElement* FindObjectInLibrary(const char *pLibName, const char *pLibrary, const char *pLibContent)
{
	if(pLibName[0] == '#')
		++pLibName;

	TiXmlElement *pLib = pRoot->FirstChildElement(pLibrary);

	while(pLib)
	{
		TiXmlElement *pObject = pLib->FirstChildElement(pLibContent);

		while(pObject)
		{
			if(TestID(pObject, pLibName))
				return pObject;

			pObject = pObject->NextSiblingElement(pLibContent);
		}

		pLib = pLib->NextSiblingElement(pLibrary);
	}

	return NULL;
}

TiXmlElement* GetSceneLib(const char *pLibName)
{
	return FindObjectInLibrary(pLibName, "library_visual_scenes", "visual_scene");
}

TiXmlElement* GetGeometryLib(const char *pLibName)
{
	return FindObjectInLibrary(pLibName, "library_geometries", "geometry");
}

TiXmlElement* GetMaterialLib(const char *pLibName)
{
	return FindObjectInLibrary(pLibName, "library_materials", "material");
}

void ParseDAEAsset(TiXmlElement *pAsset)
{
	TiXmlElement *pContributor = pAsset->FirstChildElement("contributor");

	TiXmlElement *pContrib = pContributor ? pContributor : pAsset;
	
	TiXmlElement *pAuthor = pContrib->FirstChildElement("author");
	TiXmlElement *pAuthoringTool = pContrib->FirstChildElement("authoring_tool");
	TiXmlElement *pSourceData = pContrib->FirstChildElement("source_data");

	TiXmlElement *pUp = pAsset->FirstChildElement("up_axis");
	TiXmlElement *pUnit = pAsset->FirstChildElement("unit");
	TiXmlElement *pCopyright = pAsset->FirstChildElement("copyright");
	TiXmlElement *pCreated = pAsset->FirstChildElement("created");
	TiXmlElement *pLastModified = pAsset->FirstChildElement("modified");
	TiXmlElement *pRevision = pAsset->FirstChildElement("revision");

	if(pUp)
	{
		// the axiis need to be swapped about into ingame axiis
		const char *pAxis = pUp->GetText();

		MFVector x, y, z;
		x = transformMatrix.GetXAxis();
		y = transformMatrix.GetYAxis();
		z = transformMatrix.GetZAxis();

		if(!stricmp(pAxis, "Y_UP"))
		{
			transformMatrix.SetXAxis4(x);
			transformMatrix.SetYAxis4(y);
			transformMatrix.SetZAxis4(z);
		}
		else if(!stricmp(pAxis, "Z_UP"))
		{
			transformMatrix.SetXAxis4(x);
			transformMatrix.SetYAxis4(z);
			transformMatrix.SetZAxis4(y);

			flipWinding = true;
		}
	}

	if(pUnit)
	{
		// we need to scale the model into metres..
		const char *pMeter = pUnit->Attribute("meter");
		float scale = (float)atof(pMeter);
		transformMatrix.Scale(MakeVector(scale));
	}

	if(pAuthor)
	{
		// get the author (for kicks)
		const char *pAuth = pAuthor->GetText();
		strcpy(pModel->author, pAuth);
	}

	if(pCopyright)
	{
		// get the author (for kicks)
		const char *pCopyrightString = pCopyright->GetText();
		strcpy(pModel->copyrightString, pCopyrightString);
	}

	if(pAuthoringTool)
	{
		// get the authoring tool (for kicks)
		const char *pAuthTool = pAuthoringTool->GetText();
		strcpy(pModel->authoringTool, pAuthTool);
	}
}

int ParseDAEMaterial(TiXmlElement *pMaterialNode)
{
	const char *pName = pMaterialNode->Attribute("name");

	if(!pName)
		pName = "Untitled collada material";

	F3DMaterialChunk *pMatChunk = pModel->GetMaterialChunk();

	int matIndex = pMatChunk->materials.size();

	F3DMaterial &mat = pMatChunk->materials.push();
	strcpy(mat.name, pName);

	// TODO: parse additional data from material?

	return matIndex;
}

enum ComponentType
{
	CT_Unknown = -1,

	CT_Position = 0,
	CT_Normal,
	CT_UV1,
	CT_Colour,
	CT_Weights,
	CT_Indices,
	CT_Binormal,
	CT_Tangent
};

ComponentType GetComponentType(const char *pType)
{
	if(!stricmp(pType, "POSITION"))
	{
		return CT_Position;
	}
	else if(!stricmp(pType, "NORMAL"))
	{
		return CT_Normal;
	}
	else if(!stricmp(pType, "TEXCOORD"))
	{
		return CT_UV1;
	}

	MFDebug_Assert(false, MFStr("Unknown semantic: '%s'", pType));
	return CT_Unknown;
}

class SourceData
{
public:
	SourceData() {}
	~SourceData() {}
	std::string id;
	std::vector<std::vector<float> > data;
	int validComponents;
};

std::vector<SourceData> sourceData;

SourceData* GetSourceData(const char *pSourceData)
{
	if(*pSourceData == '#')
		++pSourceData;

	for(int a=0; a<(int)sourceData.size(); a++)
	{
		if(!stricmp(pSourceData, sourceData[a].id.c_str()))
		{
			return &sourceData[a];
		}
	}

	return NULL;
}

void ReadSourceData(TiXmlElement *pSource, SourceData &data)
{
	TiXmlElement *pTechnique = pSource->FirstChildElement("technique_common");

	if(pTechnique)
	{
		TiXmlElement *pAccessor = pTechnique->FirstChildElement("accessor");

		int count, stride;
		pAccessor->Attribute("count", &count);
		pAccessor->Attribute("stride", &stride);

		data.validComponents = stride;
		data.data.resize(count);
	}

	TiXmlElement *pArray;
	pArray = pSource->FirstChildElement("array");
	MFDebug_Assert(!pArray, "'array' not supported for vertex data.\n");
	pArray = pSource->FirstChildElement("int_array");
	MFDebug_Assert(!pArray, "'int_array' not supported for vertex data.\n");
	pArray = pSource->FirstChildElement("Name_array");
	MFDebug_Assert(!pArray, "'Name_array' not supported for vertex data.\n");

	pArray = pSource->FirstChildElement("float_array");
	MFDebug_Assert(pArray, "source has no array data.\n");

	int dataCount;
	pArray->Attribute("count", &dataCount);
	MFDebug_Assert(data.data.size() * data.validComponents == dataCount, "Not enough data in array for all data specified by the technique.\n");

	const char *pText = pArray->GetText();

	int i=0;
	int j=0;

	while(*pText)
	{
		data.data[j].push_back((float)atof(pText));

		while(*pText && !MFIsWhite(*pText))
			++pText;
		pText = MFSkipWhite(pText);

		++i;
		if(i >= data.validComponents)
		{
			i=0;
			++j;
		}
	}

	MFDebug_Assert(j == data.data.size(), "Inforect amount of data in array.\n");
}

MFArray<MFVector>* GetSemanticArray(F3DSubObject &sub, ComponentType ct)
{
	switch(ct)
	{
		case CT_Position:
			return &sub.positions;
		case CT_Normal:
			return &sub.normals;
		case CT_UV1:
			return &sub.uvs;
		case CT_Colour:
			return &sub.colours;
		case CT_Binormal:
			return &sub.biNormals;
		case CT_Tangent:
			return &sub.tangents;
		case CT_Weights:
		case CT_Indices:
			return NULL;
	}

	return NULL;
}

void ParseDAEGeometry(TiXmlElement *pGeometryNode, const MFMatrix &worldTransform)
{
	int a,b;

	const char *pName = pGeometryNode->Attribute("name");

	F3DMeshChunk *pMeshChunk = pModel->GetMeshChunk();
	F3DSubObject &subObject = pMeshChunk->subObjects.push();

	strcpy(subObject.name, pName);

	TiXmlElement *pMesh = pGeometryNode->FirstChildElement("mesh");

	while(pMesh)
	{
		TiXmlElement *pMeshElement = pMesh->FirstChildElement();

		typedef std::pair<ComponentType, std::string> DataSource;
		typedef std::vector<DataSource> DataSources;
		std::vector<DataSources> components;
		components.resize(1);
		sourceData.clear();

		while(pMeshElement)
		{
			const char *pValue = pMeshElement->Value();

			if(!stricmp(pValue, "source"))
			{
				const char *pName = pMeshElement->Attribute("id");

				SourceData x;
				sourceData.push_back(x);
				SourceData &data = sourceData.back();
				data.id = pName;

				ReadSourceData(pMeshElement, data);
			}
			else if(!stricmp(pValue, "vertices"))
			{
				TiXmlElement *pInputs = pMeshElement->FirstChildElement("input");

				while(pInputs)
				{
					const char *pSemantic = pInputs->Attribute("semantic");
					const char *pSource = pInputs->Attribute("source");

					components[0].push_back(DataSource(GetComponentType(pSemantic), pSource));

					pInputs = pInputs->NextSiblingElement("input");
				}
			}
			else if(!stricmp(pValue, "polylist") || !stricmp(pValue, "triangles"))
			{
				TiXmlElement *pInputs = pMeshElement->FirstChildElement("input");

				MFDebug_Assert(pInputs, "No inputs for polygons.\n");

				const char *pSemantic = pInputs->Attribute("semantic");
				const char *pSource = pInputs->Attribute("source");

				MFDebug_Assert(!stricmp(pSemantic, "VERTEX"), "First input must be VERTEX.\n");

				// get mat sub structure
				F3DMaterialSubobject &matSub = subObject.matSubobjects.push();

				const char *pMaterial = pMeshElement->Attribute("material");

				if(pMaterial)
				{
					// find material in the library
					TiXmlElement *pMat = GetMaterialLib(pMaterial);

					if(pMat)
					{
						// get material name
						const char *pMaterialName = pMat->Attribute("name");

						if(!pMaterialName)
							pMaterialName = pMat->Attribute("id");

						// get the F3D material index
						matSub.materialIndex = pModel->GetMaterialChunk()->GetMaterialIndexByName(pMaterialName);

						// if the material doesnt exist in the F3D yet
						if(matSub.materialIndex == -1)
						{
							// add material to F3D
							matSub.materialIndex = ParseDAEMaterial(pMat);
						}
					}
					else
					{
						// not found :/
						MFDebug_Assert(false, MFStr("Material '%s' not found in library", pMaterial));
						matSub.materialIndex = -1;
					}
				}

				pInputs = pInputs->NextSiblingElement("input");

				while(pInputs)
				{
					// need to skip the first item...
					const char *pSemantic = pInputs->Attribute("semantic");
					const char *pSource = pInputs->Attribute("source");

					DataSources t;
					t.push_back(DataSource(GetComponentType(pSemantic), pSource));
					components.push_back(t);

					pInputs = pInputs->NextSiblingElement("input");
				}

				int numIndices = (int)components.size();
				int c,d;

				// copy the data into the arrays
				for(a=0; a<numIndices; a++)
				{
					DataSources sources = components[a];

					for(b=0; b<(int)sources.size(); b++)
					{
						switch(sources[b].first)
						{
							case CT_Position:
							case CT_Normal:
							case CT_UV1:
							case CT_Colour:
							case CT_Binormal:
							case CT_Tangent:
							{
								SourceData *pData = GetSourceData(sources[b].second.c_str());

								if(pData)
								{
									MFArray<MFVector> *pDataArray = GetSemanticArray(subObject, sources[b].first);

									for(c=0; c<(int)pData->data.size(); c++)
									{
										MFVector &v = pDataArray->push();

										for(d=0; d<MFMin(pData->validComponents, 4); d++)
										{
											v[d] = pData->data[c][d];
										}
									}
								}
								break;
							}
							case CT_Weights:
							case CT_Indices:
							default:
								// do nothing
								break;
						}
					}
				}

				// get num polys
				const char *pNumPolys = pMeshElement->Attribute("count");
				MFDebug_Assert(pNumPolys, "<polylist> has no count attribute.");

				int numPolys = atoi(pNumPolys);

				// get vertcounts for each poly
				MFArray<int> vertCounts;

				if(!stricmp(pValue, "polylist"))
				{

					TiXmlElement *pVCount = pMeshElement->FirstChildElement("vcount");
					MFDebug_Assert(pVCount, "No <vcount> in <polylist>");

					const char *pVCountString = pVCount->GetText();
					pVCountString = MFSkipWhite(pVCountString);

					while(*pVCountString)
					{
						int vcount = atoi(pVCountString);
						vertCounts.push(vcount);

						while(*pVCountString && !MFIsWhite(*pVCountString))
							++pVCountString;

						pVCountString = MFSkipWhite(pVCountString);
					}
				}
				else if(!stricmp(pValue, "triangles"))
				{
					vertCounts.resize(numPolys);

					for(int a=0; a<numPolys; a++)
					{
						vertCounts[a] = 3;
					}
				}
				else
				{
					MFDebug_Assert(false, "Unknown primitive type..");
				}

				// read polygons
				TiXmlElement *pPolygon = pMeshElement->FirstChildElement("p");
				MFDebug_Assert(pPolygon, "No <p> in <polylist>");

				const char *pText = pPolygon->GetText();

				// build the vertex and face lists
				int tri = 0;
				int poly = 0;

				int vert = 0;
				int firstVert = matSub.vertices.size();

				while(*pText)
				{
					if(vert > 2)
					{
						++tri;
						matSub.triangles[tri].v[0] = firstVert;
						matSub.triangles[tri].v[1] = matSub.vertices.size()-1;
						matSub.triangles[tri].v[2] = matSub.vertices.size();
					}
					else
					{
						matSub.triangles[tri].v[vert] = matSub.vertices.size();
					}

					F3DVertex &v = matSub.vertices.push();

					for(a=0; a<numIndices; a++)
					{
						int index = atoi(pText);

						DataSources sources = components[a];

						for(b=0; b<(int)sources.size(); b++)
						{
							switch(sources[b].first)
							{
								case CT_Position:
									v.position = index;
									break;
								case CT_Normal:
									v.normal = index;
									break;
								case CT_UV1:
									v.uv1 = index;
									break;
								case CT_Colour:
									v.colour = index;
									break;
								case CT_Weights:
								{
									SourceData *pData = GetSourceData(sources[b].second.c_str());

									if(pData)
									{
										for(c=0; c<MFMin(pData->validComponents, 4); c++)
										{
											v.weight[c] = pData->data[index][c];
										}
									}
									break;
								}
								case CT_Indices:
								{
									SourceData *pData = GetSourceData(sources[b].second.c_str());

									if(pData)
									{
										for(c=0; c<MFMin(pData->validComponents, 4); c++)
										{
											v.bone[c] = (int)pData->data[index][c];
										}
									}
									break;
								}
								case CT_Binormal:
									v.biNormal = index;
									break;
								case CT_Tangent:
									v.tangent = index;
									break;
								default:
									// do nothing
									break;
							}
						}

						while(*pText && !MFIsWhite(*pText))
							++pText;
						pText = MFSkipWhite(pText);
					}

					++vert;

					if(vert >= vertCounts[poly])
					{
						++poly;
						++tri;
						vert = 0;
						firstVert = matSub.vertices.size();
					}
				}
			}

			pMeshElement = pMeshElement->NextSiblingElement();
		}

		pMesh = pMesh->NextSiblingElement("mesh");
	}

	// apply coordinate system correction matrix
	MFMatrix transform;
	transform.Multiply4x4(worldTransform, transformMatrix);

	for(a=0; a<subObject.positions.size(); a++)
	{
		subObject.positions[a] = ApplyMatrixH(subObject.positions[a], transform);
	}

	// flip the triangle winding if coordinate systems have changed 'handedness'
	if(flipWinding)
	{
		for(a=0; a<subObject.matSubobjects.size(); a++)
		{
			for(b=0; b<subObject.matSubobjects[a].triangles.size(); b++)
			{
				int i = subObject.matSubobjects[a].triangles[b].v[1];
				subObject.matSubobjects[a].triangles[b].v[1] = subObject.matSubobjects[a].triangles[b].v[2];
				subObject.matSubobjects[a].triangles[b].v[2] = i;
			}
		}
	}
}

void ParseDAEAnimation(TiXmlElement *pSceneNode)
{
	
}

void ParseDAELight(TiXmlElement *pLightNode)
{

}

void ParseDAECamera(TiXmlElement *pCameraNode)
{

}

void FindAndAddGeometryToScene(TiXmlElement *pInstanceNode, TiXmlElement *pParentNode, const MFMatrix &matrix)
{
	// get object name
	const char *pObjectName = pInstanceNode->Attribute("url");

	// scan library for object
	TiXmlElement *pObject = GetGeometryLib(pObjectName);

	if(pObject)
	{
		ParseDAEGeometry(pObject, matrix);
	}
	else
	{
		printf(MFStr("Object '%s' not found in library...\n", pObjectName));
	}
}

void ParseSceneNode(TiXmlElement *pSceneNode, const MFMatrix &parentMatrix)
{
	TiXmlElement *pTransform;
	MFMatrix localMat = MFMatrix::identity;

	// build the transform
	pTransform = pSceneNode->FirstChildElement();

	while(pTransform)
	{
		const char *pTransformValue = pTransform->Value();

		if(!stricmp(pTransformValue, "matrix"))
		{
			// apply this node's transformation matrix to the current transform
			const char *pMat = pTransform->GetText();

			int c = 0;
			while(*pMat && c<16)
			{
				localMat.m[c%4][c>>2] =  (float)atof(pMat);
				++c;

				while(*pMat && !MFIsWhite(*pMat))
					pMat++;
				while(MFIsWhite(*pMat))
					pMat++;
			}
		}
		else if(!stricmp(pTransformValue, "translate"))
		{
			// just translate the current transform
			const char *pTrans = pTransform->GetText();
			MFVector translation = MFVector::identity;

			int c = 0;
			while(*pTrans && c<4)
			{
				translation[c] = (float)atof(pTrans);
				++c;

				while(*pTrans && !MFIsWhite(*pTrans))
					pTrans++;
				while(MFIsWhite(*pTrans))
					pTrans++;
			}

			localMat.Translate(translation);
		}
		else if(!stricmp(pTransformValue, "scale"))
		{
			// scale the current transform
			const char *pScale = pTransform->GetText();
			MFVector scale = MFVector::identity;

			int c = 0;
			while(*pScale && c<4)
			{
				scale[c] = (float)atof(pScale);
				++c;

				while(*pScale && !MFIsWhite(*pScale))
					pScale++;
				while(MFIsWhite(*pScale))
					pScale++;
			}

			localMat.Scale(scale);
		}
		else if(!stricmp(pTransformValue, "rotate"))
		{
			// rotate the current transform
			const char *pRot = pTransform->GetText();
			MFVector rot = MFVector::zero;

			int c = 0;
			while(*pRot && c<4)
			{
				rot[c] = (float)atof(pRot);
				++c;

				while(*pRot && !MFIsWhite(*pRot))
					pRot++;
				while(MFIsWhite(*pRot))
					pRot++;
			}

			localMat.Rotate(rot, MFDEGREES(rot.w));
		}

		pTransform = pTransform->NextSiblingElement();
	}

	// TODO: this may need to be the other way around :)
	localMat.Multiply(localMat, parentMatrix);

	// check for a geometry node
	TiXmlElement *pGeometry = pSceneNode->FirstChildElement("instance_geometry");
	if(pGeometry)
	{
		// node is an instance node
		// find and add instance to scene
		FindAndAddGeometryToScene(pGeometry, pSceneNode, localMat);
	}

	// all the other stuff
//	TiXmlElement *pInstance = pSceneNode->FirstChildElement("instance_camera");

	// recurse child nodes
	TiXmlElement *pNode = pSceneNode->FirstChildElement("node");
	while(pNode)
	{
		// and recurse for child nodes
		ParseSceneNode(pNode, localMat);

		pNode = pNode->NextSiblingElement("node");
	}
}

void ParseDAEScene(TiXmlElement *pSceneNode)
{
	TiXmlElement *pScene;
	pScene = pSceneNode->FirstChildElement();

	while(pScene)
	{
		const char *pValue = pScene->Value();

		if(!stricmp(pValue, "instance_visual_scene"))
		{
			const char *pURL = pScene->Attribute("url");

			TiXmlElement *pSceneRoot = GetSceneLib(pURL);

			if(pSceneRoot)
			{
				const char *pName = pSceneRoot->Attribute("name");
				strcpy(pModel->name, pName);

				ParseSceneNode(pSceneRoot, MFMatrix::identity);
			}
			else
			{
				// scene not found..
				printf("Scene '%s' not found!\n", pURL);
			}
		}

		pScene = pScene->NextSiblingElement();
	}
}

void ParseDAERootElement(TiXmlElement *pRoot)
{
	TiXmlElement *pElement = pRoot->FirstChildElement();

	while(pElement)
	{
		const char *pValue = pElement->Value();

		if(!stricmp(pValue, "asset"))
		{
			ParseDAEAsset(pElement);
		}
		// i should probably build the library here
/*
		else if(!stricmp(pValue, "library"))
		{
			ParseDAELibrary(pElement);
		}
*/
		// but for the time being i'll just parse the scene graph and refer to the library the slow way (manually)
		else if(!stricmp(pValue, "scene"))
		{
			ParseDAEScene(pElement);
		}


		pElement = pElement->NextSiblingElement();
	}
}

int F3DFile::ReadDAE(char *pFilename)
{
	transformMatrix = MFMatrix::identity;

	TiXmlDocument doc(pFilename);
	bool loadOkay = doc.LoadFile();

	if(!loadOkay)
	{
		printf("Failed to load collada file '%s' for conversion..\n", pFilename);
		return -1;
	}

	pRoot = doc.FirstChildElement("COLLADA");

	if(!pRoot)
	{
		printf("Document has no root node..\n");
		return 1;
	}

	strcpy(pModel->name, "Untitled collada file");

	ParseDAERootElement(pRoot);

	return 0;
}
