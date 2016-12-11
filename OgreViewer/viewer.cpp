#include "OgreMain.h"
#include <iostream>

int main(int argc, char* argv[])
{
	char filePath[MAX_PATH];
	char meshName[MAX_PATH];
	char textureName[MAX_PATH];

	if (argc == 3)
	{
		strcpy(filePath, argv[0]);
		strcpy(meshName, argv[1]);
		strcpy(textureName, argv[2]);
	}
	else
	{
		strcpy(filePath, argv[1]);
		strcpy(meshName, argv[2]);
		strcpy(textureName, argv[3]);
	}

	std::cout << meshName << std::endl;
	std::cout << textureName << std::endl;

	Ogre::String ofilePath(filePath);
	Ogre::String omeshName(meshName);
	Ogre::String otextureName(textureName);

	COgreMain om(ofilePath, omeshName, otextureName);
	om.run();
	return 0;
}
