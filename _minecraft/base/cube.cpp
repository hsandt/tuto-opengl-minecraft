#include "cube.h"

#include <engine/utils/types_3d.h>

// define colors in order for EARTH, GRASS, WATER

NYColor NYCube::cubeAmbientColors[3] = {
	NYColor(148.f / 255.f, 62.f / 255.f, 15.f / 255.f, 1.f),
	NYColor(0.f / 255.f, 123.f / 255.f, 12.f / 255.f, 1.f),
	NYColor(20.f / 255.f, 70.f / 255.f, 180.f / 255.f, 0.8f)
};

NYColor NYCube::cubeDiffuseColors[3] = {
	NYColor(148.f / 255.f, 62.f / 255.f, 15.f / 255.f, 1.f),
	NYColor(0.f / 255.f, 123.f / 255.f, 12.f / 255.f, 1.f),
	NYColor(20.f / 255.f, 70.f / 255.f, 180.f / 255.f, 1.f)
};

NYColor NYCube::cubeSpecularColors[3] = {
	NYColor(0.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1.f),
	NYColor(0.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1.f),
	NYColor(255.f / 255.f, 255.f / 255.f, 255.f / 255.f, 1.f)
};
