#include "Model.h"

using domeshconverter::Model;

int main(int argc, char *argv[])
{
	Model model;
	bool result = Model::LoadFromM3d(std::string("D:/DarkOmenReborn/github/domeshconverter/Test/BASE.M3D"), model);
	result = model.SaveToJson(std::string("D:/DarkOmenReborn/github/domeshconverter/Test/BASE.json"));
	return result ? 0 : 1;
}