#pragma once

#include <stdio.h>
#include <engine/utils/types_3d.h>

enum NYCubeType
{
	CUBE_EARTH = 0,
	CUBE_GRASS = 1,
	CUBE_WATER = 2,
	CUBE_AIR
};

class NYCube
{

	public :
		bool _Draw;
		NYCubeType _Type;
		static const int CUBE_SIZE = 10;

		static NYColor cubeAmbientColors[3];
		static NYColor cubeDiffuseColors[3];
		static NYColor cubeSpecularColors[3];

		NYCube()
		{
			_Draw = true;
			_Type = CUBE_AIR;
		}

		bool isSolid(void)
		{
			return (_Type != CUBE_AIR && _Type != CUBE_WATER);
		}

		void saveToFile(FILE * fs)
		{
			fputc(_Draw ? 1 : 0,fs);
			fputc(_Type,fs);
		}

		void loadFromFile(FILE * fe)
		{
			_Draw = fgetc(fe) ? true : false;
			_Type = (NYCubeType)fgetc(fe);
		}
};