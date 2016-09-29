#include "cxxopts\cxxopts.hpp"
#include "Model.h"
#include <string>

using domeshconverter::Model;

int main(int argc, char* argv[])
{
	cxxopts::Options options("domeshconverter", "Dark Omen mesh converter.");
	options.add_options()
		("i,infile", "Input file path", cxxopts::value<std::string>())
		("o,outfile", "Output file path", cxxopts::value<std::string>())
		("m,mode", "Mode. Available options: m3d_to_json, json_to_m3d, m3d_to_obj, json_to_obj", cxxopts::value<std::string>())
		;
	options.parse(argc, argv);

	Model model;
	bool result = true;

	std::string inputFile = options["infile"].as<std::string>();
	std::string outputFile = options["outfile"].as<std::string>();

	std::string mode = options["mode"].as<std::string>();
	if (mode == std::string("m3d_to_json"))
	{
		result = Model::LoadFromM3d(inputFile, model);
		if (result)
		{
			result = model.SaveToJson(outputFile);
		}
	}
	else if (mode == std::string("m3d_to_json"))
	{
		result = Model::LoadFromJson(inputFile, model);
		if (result)
		{
			result = model.SaveToM3d(outputFile);
		}
	}
	else if (mode == std::string("m3d_to_obj"))
	{
		result = Model::LoadFromM3d(inputFile, model);
		if (result)
		{
			result = model.SaveToObj(outputFile);
		}
	}
	else if (mode == std::string("json_to_obj"))
	{
		result = Model::LoadFromJson(inputFile, model);
		if (result)
		{
			result = model.SaveToObj(outputFile);
		}
	}

	return result ? 0 : 1;
}