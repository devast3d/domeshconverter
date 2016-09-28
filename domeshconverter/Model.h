#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace domeshconverter
{
	struct Texture
	{
		std::string Path;
		std::string Name;
	};

	struct Point2
	{
		float X;
		float Y;
	};

	struct Point3
	{
		float X;
		float Y;
		float Z;
	};

	struct Vertex
	{
		Point3			Point;
		Point3			Normal;
		std::uint8_t	Rgba[4];
		Point2			Uv;
		std::int32_t	Spare1;
		std::int32_t	Spare2;
	};

	struct Face
	{
		std::int16_t	Vertex[3];
		std::int16_t	Texture;
		Point3			Normal;
		std::int32_t	Spare1;
		std::int32_t	Spare2;
	};

	struct Object
	{
		std::string			Name;
		std::int16_t		Parent;
		Point3				Pivot;
		std::int16_t		NumVertices;
		std::int16_t		NumFaces;
		std::int32_t		Flags;
		std::int32_t		Spare1;
		std::int32_t		Spare2;
		std::vector<Face>	Faces;
		std::vector<Vertex>	Vertices;
	};

	class Model
	{
	public:
		std::int32_t			ID;
		std::int32_t			Magic;
		std::int32_t			Version;
		std::int32_t			CRC;
		std::int32_t			NOT_CRC;
		std::int16_t			NumTextures;
		std::int16_t			NumObjects;

		std::vector<Texture>	Textures;
		std::vector<Object>		Objects;

	public:
		Model();

	public:
		static bool LoadFromM3d(std::string& filePath, Model& out_model);
		static bool LoadFromJson(std::string& filePath, Model& out_model);

		bool SaveToM3d(std::string& filePath);
		bool SaveToJson(std::string& filePath);
		bool SaveToObj(std::string& filePath);
	};
}