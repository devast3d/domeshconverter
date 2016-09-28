#include "Model.h"
#include "jsoncpp\json.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <cmath>
#include <map>

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
			object.Spare1 = nativeObject.Spare1;
			object.Spare2 = nativeObject.Spare2;

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

	bool Model::LoadFromJson(std::string& filePath, Model& out_model)
	{
		cout << "Started reading json file\n";

		ifstream file(filePath);

		if (!file.is_open())
		{
			cout << "Input file failed to open";
			return false;
		}

		Json::Value root;
		file >> root;
		
		out_model.ID = root["ID"].asInt();
		out_model.Magic = root["Magic"].asInt();
		out_model.Version = root["Version"].asInt();
		out_model.CRC = root["CRC"].asInt();
		out_model.NOT_CRC = root["NOT_CRC"].asInt();

		out_model.NumTextures = static_cast<int16_t>(root["NumTextures"].asInt());
		out_model.NumObjects = static_cast<int16_t>(root["NumObjects"].asInt());

		for (auto& textureValue : root["Textures"])
		{
			Texture texture;
			texture.Path = textureValue["Path"].asString();
			texture.Name = textureValue["Name"].asString();

			out_model.Textures.push_back(texture);
		}

		for (auto& objectValue : root["Objects"])
		{
			Object object;
			object.Name = objectValue["Name"].asString();
			object.Parent = static_cast<int16_t>(objectValue["Parent"].asInt());

			Json::Value pivotValue = objectValue["Pivot"];
			object.Pivot.X = pivotValue["X"].asFloat();
			object.Pivot.Y = pivotValue["Y"].asFloat();
			object.Pivot.Z = pivotValue["Z"].asFloat();

			object.NumVertices = static_cast<int16_t>(objectValue["NumVertices"].asInt());
			object.NumFaces = static_cast<int16_t>(objectValue["NumFaces"].asInt());
			object.Flags = objectValue["Flags"].asInt();
			object.Spare1 = objectValue["Spare1"].asInt();
			object.Spare2 = objectValue["Spare2"].asInt();

			for (auto& faceValue : objectValue["Faces"])
			{
				Face face;

				Json::Value vertexValue = faceValue["Vertex"];
				face.Vertex[0] = static_cast<int16_t>(vertexValue["V0"].asInt());
				face.Vertex[1] = static_cast<int16_t>(vertexValue["V1"].asInt());
				face.Vertex[2] = static_cast<int16_t>(vertexValue["V2"].asInt());

				face.Texture = static_cast<int16_t>(faceValue["Texture"].asInt());

				Json::Value normalValue = faceValue["Normal"];
				face.Normal.X = normalValue["X"].asFloat();
				face.Normal.Y = normalValue["Y"].asFloat();
				face.Normal.Z = normalValue["Z"].asFloat();

				face.Spare1 = faceValue["Spare1"].asInt();
				face.Spare2 = faceValue["Spare2"].asInt();

				object.Faces.push_back(face);
			}

			for (auto& vertexValue : objectValue["Vertices"])
			{
				Vertex vertex;

				Json::Value pointValue = vertexValue["Point"];
				vertex.Point.X = pointValue["X"].asFloat();
				vertex.Point.Y = pointValue["Y"].asFloat();
				vertex.Point.Z = pointValue["Z"].asFloat();

				Json::Value normalValue = vertexValue["Normal"];
				vertex.Normal.X = normalValue["X"].asFloat();
				vertex.Normal.Y = normalValue["Y"].asFloat();
				vertex.Normal.Z = normalValue["Z"].asFloat();

				Json::Value rgbaValue = vertexValue["Rgba"];
				vertex.Rgba[0] = static_cast<uint8_t>(rgbaValue["R"].asInt());
				vertex.Rgba[1] = static_cast<uint8_t>(rgbaValue["G"].asInt());
				vertex.Rgba[2] = static_cast<uint8_t>(rgbaValue["B"].asInt());
				vertex.Rgba[3] = static_cast<uint8_t>(rgbaValue["A"].asInt());

				Json::Value uvValue = vertexValue["Uv"];
				vertex.Uv.X = uvValue["U"].asFloat();
				vertex.Uv.Y = uvValue["V"].asFloat();

				vertex.Spare1 = vertexValue["Spare1"].asInt();
				vertex.Spare2 = vertexValue["Spare2"].asInt();

				object.Vertices.push_back(vertex);
			}
			
			out_model.Objects.push_back(object);
		}

		file.close();

		cout << "Finished reading json file\n";

		return true;
	}

	bool Model::SaveToM3d(std::string& filePath)
	{
		cout << "Started writing m3d\n";

		ofstream file(filePath, ios::binary);

		if (!file.is_open())
		{
			return false;
		}

		file.write((char*)&ID, sizeof(ID));
		file.write((char*)&Magic, sizeof(Magic));
		file.write((char*)&Version, sizeof(Version));
		file.write((char*)&CRC, sizeof(CRC));
		file.write((char*)&NOT_CRC, sizeof(NOT_CRC));

		file.write((char*)&NumTextures, sizeof(NumTextures));
		file.write((char*)&NumObjects, sizeof(NumObjects));

		for (auto& texture : Textures)
		{
			char path[64] = { 0 };
			memcpy_s(path, texture.Path.size(), texture.Path.c_str(), texture.Path.size());
			file.write(path, sizeof(path));

			char name[32] = { 0 };
			memcpy_s(name, texture.Name.size(), texture.Name.c_str(), texture.Name.size());
			file.write(name, sizeof(name));
		}

		for (auto& object : Objects)
		{
			char name[32] = { 0 };
			memcpy_s(name, object.Name.size(), object.Name.c_str(), object.Name.size());
			file.write(name, sizeof(name));

			file.write((char*)&object.Parent, sizeof(object.Parent));
			
			uint16_t zero = 0;
			file.write((char*)&zero, 2); // Since Pivot is padded in the struct, we have to add 2 bytes of zero data
			
			file.write((char*)&object.Pivot, sizeof(object.Pivot));
			file.write((char*)&object.NumVertices, sizeof(object.NumVertices));
			file.write((char*)&object.NumFaces, sizeof(object.NumFaces));
			file.write((char*)&object.Flags, sizeof(object.Flags));
			file.write((char*)&object.Spare1, sizeof(object.Spare1));
			file.write((char*)&object.Spare2, sizeof(object.Spare2));

			file.write((char*)&object.Faces.front(), sizeof(Face) * (object.NumFaces));
			file.write((char*)&object.Vertices.front(), sizeof(Vertex) * object.NumVertices);
		}

		file.close();

		cout << "Finished writing m3d\n";

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

	bool Model::SaveToObj(std::string& filePath)
	{
		cout << "Started writing obj\n";

		ofstream file(filePath, ofstream::out);

		if (!file.is_open())
		{
			return false;
		}

		string mtlFilePath = filePath.substr(0, filePath.find_last_of(".")) + ".mtl";
		string matlFileName = mtlFilePath.substr(mtlFilePath.find_last_of("\\/") + 1);

		file << "mtllib " << matlFileName << endl;

		map<uint16_t, map<uint32_t, vector<uint32_t>>> renderMap;
		vector<uint32_t> vertexOffsets;
		uint32_t vertexOffset = 0;
		
		for (uint32_t objectIndex = 0; objectIndex < Objects.size(); ++objectIndex)
		{
			Object& object = Objects[objectIndex];

			//if (object.Name != "n1") continue;
			//file << "g " << object.Name << endl;
			
			float pivotX = object.Pivot.X;
			float pivotY = object.Pivot.Y;
			float pivotZ = object.Pivot.Z;

			for (auto& vertex : object.Vertices)
			{
				float invNormalLength = 1.0f / sqrtf(vertex.Normal.X * vertex.Normal.X + vertex.Normal.Y * vertex.Normal.Y + vertex.Normal.Z * vertex.Normal.Z);

				float px = vertex.Point.X + pivotX;
				float py = vertex.Point.Y + pivotY;
				float pz = vertex.Point.Z + pivotZ;

				float u = vertex.Uv.X;
				float v = 1.0f - vertex.Uv.Y;

				float nx = vertex.Normal.X * invNormalLength;
				float ny = vertex.Normal.Y * invNormalLength;
				float nz = vertex.Normal.Z * invNormalLength;

				file << "v " << px << " " << pz << " " << py << endl;
				file << "vt " << u << " " << v << endl;
				file << "vn " << nx << " " << nz << " " << ny  << endl;
			}

			//map<uint16_t, vector<uint32_t>> textureToFaceMap;
			for (uint32_t faceIndex = 0; faceIndex < object.Faces.size(); ++faceIndex)
			{
				uint16_t textureIndex = object.Faces[faceIndex].Texture;
				//textureToFaceMap[textureIndex].push_back(faceIndex);
				renderMap[textureIndex][objectIndex].push_back(faceIndex);
			}

			//for (auto& pair : textureToFaceMap)
			//{
			//	//++group;
			//	//file << "g faces" << group << endl;

			//	string originalTextureName = Textures[pair.first].Name;
			//	string textureName = originalTextureName.substr(0, originalTextureName.find_last_of("."));
			//	file << "usemtl " << "M_" << textureName << endl;
			//	
			//	for (auto& faceIndex : pair.second)
			//	{
			//		Face& face = object.Faces[faceIndex];
			//		
			//		int v0 = face.Vertex[0] + 1 + vertexOffset;
			//		int v1 = face.Vertex[2] + 1 + vertexOffset;
			//		int v2 = face.Vertex[1] + 1 + vertexOffset;

			//		file << "f ";
			//		file << v0 << "/" << v0 << "/" << v0 << " ";
			//		file << v1 << "/" << v1 << "/" << v1 << " ";
			//		file << v2 << "/" << v2 << "/" << v2 << endl;
			//	}
			//}

			vertexOffsets.push_back(vertexOffset);
			vertexOffset += object.Vertices.size();
		}

		uint32_t faceGroupIndex = 0;
		uint32_t objectIndex = 0;
		for (auto& pair : renderMap)
		{
			uint16_t textureIndex = pair.first;

			++faceGroupIndex;
			file << "g facegroup" << faceGroupIndex << endl;

			for (auto& objectPair : pair.second)
			{
				uint32_t objectIndex = objectPair.first;
				Object& object = Objects[objectIndex];
				uint32_t vertexOffset = vertexOffsets[objectIndex];

				string originalTextureName = Textures[textureIndex].Name;
				string textureName = originalTextureName.substr(0, originalTextureName.find_last_of("."));
				file << "usemtl " << "M_" << textureName << endl;

				for (auto& faceIndex : objectPair.second)
				{
					Face& face = object.Faces[faceIndex];

					int v0 = face.Vertex[0] + 1 + vertexOffset;
					int v1 = face.Vertex[2] + 1 + vertexOffset;
					int v2 = face.Vertex[1] + 1 + vertexOffset;

					file << "f ";
					file << v0 << "/" << v0 << "/" << v0 << " ";
					file << v1 << "/" << v1 << "/" << v1 << " ";
					file << v2 << "/" << v2 << "/" << v2 << endl;
				}
			}
		}

		file.close();

		
		ofstream mtlFile(mtlFilePath, ofstream::out);

		if (!mtlFile.is_open())
		{
			return false;
		}

		for (auto& texture : Textures)
		{
			string textureName = texture.Name.substr(0, texture.Name.find_last_of("."));
			mtlFile << "newmtl " << "M_" << textureName << endl;
			mtlFile << "map_Kd " << "./TEXTURE/" << texture.Name << endl;
		}

		mtlFile.close();

		cout << "Finished writing obj\n";

		return true;
	}
}