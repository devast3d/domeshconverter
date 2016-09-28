#include "Model.h"

using domeshconverter::Model;

int main(int argc, char *argv[])
{
	Model model;
	
	Model::LoadFromM3d(std::string("D:/Projects/DarkOmenReborn/github/domeshconverter/Test/BASE.M3D"), model);
	//model.SaveToJson(std::string("D:/Projects/DarkOmenReborn/github/domeshconverter/Test/BASE.json"));
	model.SaveToObj(std::string("D:/Projects/DarkOmenReborn/github/domeshconverter/Test/BASE.obj"));

	//Model::LoadFromJson(std::string("D:/Projects/DarkOmenReborn/github/domeshconverter/Test/BASE.json"), model);
	//model.SaveToM3d(std::string("D:/Projects/DarkOmenReborn/github/domeshconverter/Test/BASE1.M3D"));
}