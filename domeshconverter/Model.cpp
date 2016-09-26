#include "Model.h"
#include "jsoncpp\json.h"
#include <fstream>
#include <iostream>

using namespace std;

namespace domeshconverter
{
	Model::Model()
	{
	}

	bool Model::LoadFromM3d(std::string& filePath, Model& out_model)
	{
		ifstream file(filePath,  ios::binary);

		if (!file.is_open())
		{
			cout << "Input file failed to open";
			return false;
		}

		struct M3DFILEHEADER
		{
			long ID;
			long Magic;
			long Version;
			long CRC;
			long NOT_CRC;
		};

		M3DFILEHEADER fileHeader;
		file.read((char*)&fileHeader, sizeof(M3DFILEHEADER));

		out_model.ID = fileHeader.ID;
		out_model.Magic = fileHeader.Magic;
		out_model.Version = fileHeader.Version;
		out_model.CRC = fileHeader.CRC;
		out_model.NOT_CRC = fileHeader.NOT_CRC;

		struct M3DHEADER
		{
			short NumTextures;
			short NumObjects;
		};

		M3DHEADER header;
		file.read((char*)&header, sizeof(header));

		uint32_t numTextures = header.NumTextures;
		uint32_t numObjects = header.NumObjects;

		out_model.NumTextures = numTextures;
		out_model.NumObjects = numObjects;
		out_model.Textures.resize(numTextures);

		struct M3DTEXTURE
		{
			char Path[64];	//  path to texture.
			char Name[32];	//  filename of texture.
		};

		vector<M3DTEXTURE> textures(numTextures);
		file.read((char*)&textures.front(), sizeof(M3DTEXTURE) * numTextures);

		for (uint32_t i = 0; i < numTextures; ++i)
		{
			out_model.Textures[i].Path = string(textures[i].Path);
			out_model.Textures[i].Name = string(textures[i].Name);
		}

		struct M3DOBJECT
		{
			char		Name[32];	// object name
			short		Parent;		// parent index
			Point3		Pivot;		// origin for heirarchy
			short		Vertices;	// number of verts.
			short		Faces;		// number of faces
			long		Flags;
			long		Spare1;
			long		Spare2;
		};

		for (uint32_t i = 0; i < numObjects; ++i)
		{
			M3DOBJECT nativeObject;
			file.read((char*)&nativeObject, sizeof(M3DOBJECT));

			uint32_t numFaces = nativeObject.Faces;
			uint32_t numVertices = nativeObject.Vertices;

			if (numFaces == 0 || numVertices == 0)
			{
				cout << "Skipping empty object " << string(nativeObject.Name);
				continue;
			}

			Object object;
			object.Name = string(nativeObject.Name);
			object.Parent = nativeObject.Parent;
			object.Pivot = nativeObject.Pivot;
			object.NumVertices = numVertices;
			object.NumFaces = numFaces;
			object.Flags = nativeObject.Flags;

			object.Faces.resize(object.NumFaces);
			file.read((char*)&object.Faces.front(), sizeof(Face) * numFaces);
			
			object.Vertices.resize(object.NumVertices);
			file.read((char*)&object.Vertices.front(), sizeof(Vertex) * numVertices);
			
			out_model.Objects.push_back(object);
		}

		return true;
	}

	bool Model::SaveToJson(std::string& filePath)
	{

	}
}