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
		cout << "Started reading m3d file\n";

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
				cout << "Skipping empty object " << string(nativeObject.Name) << "\n";
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

		file.close();

		cout << "Finished reading m3d file\n";

		return true;
	}

	bool Model::SaveToJson(std::string& filePath)
	{
		cout << "Started writing json\n";

		ofstream file(filePath, ofstream::out);

		if (!file.is_open())
		{
			return false;
		}

		Json::Value root(Json::objectValue);
		root["ID"] = ID;
		root["Magic"] = Magic;
		root["Version"] = Version;
		root["CRC"] = CRC;
		root["NOT_CRC"] = NOT_CRC;
		root["NumTextures"] = NumTextures;
		root["NumObjects"] = NumObjects;

		Json::Value textures(Json::arrayValue);
		for (auto& texture : Textures)
		{
			Json::Value value(Json::objectValue);
			value["Path"] = texture.Path;
			value["Name"] = texture.Name;
			textures.append(value);
		}
		root["Textures"] = textures;

		Json::Value objects(Json::arrayValue);
		for (auto& object : Objects)
		{
			Json::Value objectValue(Json::objectValue);
			objectValue["Name"] = object.Name;
			objectValue["Parent"] = object.Parent;
			Json::Value pivotValue(Json::objectValue);
			pivotValue["X"] = object.Pivot.X;
			pivotValue["Y"] = object.Pivot.Y;
			pivotValue["Z"] = object.Pivot.Z;
			objectValue["Pivot"] = pivotValue;
			objectValue["NumVertices"] = object.NumVertices;
			objectValue["NumFaces"] = object.NumFaces;
			objectValue["Flags"] = object.Flags;
			objectValue["Spare1"] = object.Spare1;
			objectValue["Spare2"] = object.Spare2;

			Json::Value faces(Json::arrayValue);
			for (auto& face : object.Faces)
			{
				Json::Value faceValue(Json::objectValue);
				Json::Value vertexValue(Json::objectValue);
				vertexValue["V0"] = face.Vertex[0];
				vertexValue["V1"] = face.Vertex[1];
				vertexValue["V2"] = face.Vertex[2];
				faceValue["Vertex"] = vertexValue;
				faceValue["Texture"] = face.Texture;
				Json::Value normalValue(Json::objectValue);
				normalValue["X"] = face.Normal.X;
				normalValue["Y"] = face.Normal.Y;
				normalValue["Z"] = face.Normal.Z;
				faceValue["Normal"] = normalValue;
				faceValue["Spare1"] = face.Spare1;
				faceValue["Spare2"] = face.Spare2;

				faces.append(faceValue);
			}
			objectValue["Faces"] = faces;

			Json::Value vertices(Json::arrayValue);
			for (auto& vertex : object.Vertices)
			{
				Json::Value vertexValue(Json::objectValue);
				Json::Value pointValue(Json::objectValue);
				pointValue["X"] = vertex.Point.X;
				pointValue["Y"] = vertex.Point.Y;
				pointValue["Z"] = vertex.Point.Z;
				vertexValue["Point"] = pointValue;
				Json::Value normalValue(Json::objectValue);
				normalValue["X"] = vertex.Normal.X;
				normalValue["Y"] = vertex.Normal.Y;
				normalValue["Z"] = vertex.Normal.Z;
				vertexValue["Normal"] = normalValue;
				Json::Value rgbaValue(Json::objectValue);
				rgbaValue["R"] = vertex.Rgba[0];
				rgbaValue["G"] = vertex.Rgba[1];
				rgbaValue["B"] = vertex.Rgba[2];
				rgbaValue["A"] = vertex.Rgba[3];
				vertexValue["Rgba"] = rgbaValue;
				Json::Value uvValue(Json::objectValue);
				uvValue["U"] = vertex.Uv.X;
				uvValue["V"] = vertex.Uv.Y;
				vertexValue["Uv"] = uvValue;
				vertexValue["Spare1"] = vertex.Spare1;
				vertexValue["Spare2"] = vertex.Spare2;

				vertices.append(vertexValue);
			}
			objectValue["Vertices"] = vertices;

			objects.append(objectValue);
		}
		root["Objects"] = objects;

		file << root;
		file.close();

		cout << "Finished writing json\n";

		return true;
	}
}