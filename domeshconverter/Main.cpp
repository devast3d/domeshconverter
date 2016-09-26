#include "Model.h"

using domeshconverter::Model;

int main(int argc, char *argv[])
{
	Model model;
	bool result = Model::LoadM3d(std::string("D:/DarkOmenReborn/github/domeshconverter/Test/BASE.M3D"), model);
	return result ? 0 : 1;
}