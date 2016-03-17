#ifndef __WORLD_H__
#define __WORLD_H__

#include <random>

#include "gl/glew.h"
#include "gl/freeglut.h" 
#include "engine/utils/types_3d.h"
#include "cube.h"
#include "chunk.h"

// access variable from main
extern std::mt19937 random_engine;

typedef uint8 NYAxis;
#define NY_AXIS_X 0x01
#define NY_AXIS_Y 0x02
#define NY_AXIS_Z 0x04

#define MAT_SIZE 2 //en nombre de chunks
#define MAT_HEIGHT 2 //en nombre de chunks
#define MAT_SIZE_CUBES (MAT_SIZE * NYChunk::CHUNK_SIZE)
#define MAT_HEIGHT_CUBES (MAT_HEIGHT * NYChunk::CHUNK_SIZE)

class NYWorld
{
public :
	NYChunk * _Chunks[MAT_SIZE][MAT_SIZE][MAT_HEIGHT];
	int _MatriceHeights[MAT_SIZE_CUBES][MAT_SIZE_CUBES];
	float _FacteurGeneration;

	NYColor cubeColors[4];

	NYWorld()
	{
		cubeColors[NYCubeType::CUBE_HERBE] = NYColor(0.f / 255.f, 123.f / 255.f, 12.f / 255.f, 1.f);
		cubeColors[NYCubeType::CUBE_EAU] = NYColor(98.f / 255.f, 146.f / 255.f, 134.f / 255.f, 0.8f);
		cubeColors[NYCubeType::CUBE_TERRE] = NYColor(148.f / 255.f, 62.f / 255.f, 15.f / 255.f, 1.f);
		cubeColors[NYCubeType::CUBE_AIR] = NYColor(255.f / 255.f, 255.f / 255.f, 255.f / 255.f, 0.0f);

		_FacteurGeneration = 1.0;

		//On crée les chunks
		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
					_Chunks[x][y][z] = new NYChunk();

		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
				{
					NYChunk * cxPrev = NULL;
					if(x > 0)
						cxPrev = _Chunks[x-1][y][z];
					NYChunk * cxNext = NULL;
					if(x < MAT_SIZE-1)
						cxNext = _Chunks[x+1][y][z];

					NYChunk * cyPrev = NULL;
					if(y > 0)
						cyPrev = _Chunks[x][y-1][z];
					NYChunk * cyNext = NULL;
					if(y < MAT_SIZE-1)
						cyNext = _Chunks[x][y+1][z];

					NYChunk * czPrev = NULL;
					if(z > 0)
						czPrev = _Chunks[x][y][z-1];
					NYChunk * czNext = NULL;
					if(z < MAT_HEIGHT-1)
						czNext = _Chunks[x][y][z+1];

					_Chunks[x][y][z]->setVoisins(cxPrev,cxNext,cyPrev,cyNext,czPrev,czNext);
				}

					
	}

	inline NYCube * getCube(int x, int y, int z)
	{	
		if(x < 0)x = 0;
		if(y < 0)y = 0;
		if(z < 0)z = 0;
		if(x >= MAT_SIZE * NYChunk::CHUNK_SIZE) x = (MAT_SIZE * NYChunk::CHUNK_SIZE)-1;
		if(y >= MAT_SIZE * NYChunk::CHUNK_SIZE) y = (MAT_SIZE * NYChunk::CHUNK_SIZE)-1;
		if(z >= MAT_HEIGHT * NYChunk::CHUNK_SIZE) z = (MAT_HEIGHT * NYChunk::CHUNK_SIZE)-1;

		return &(_Chunks[x / NYChunk::CHUNK_SIZE][y / NYChunk::CHUNK_SIZE][z / NYChunk::CHUNK_SIZE]->_Cubes[x % NYChunk::CHUNK_SIZE][y % NYChunk::CHUNK_SIZE][z % NYChunk::CHUNK_SIZE]);
	}

	void updateCube(int x, int y, int z)
	{	
		if(x < 0)x = 0;
		if(y < 0)y = 0;
		if(z < 0)z = 0;
		if(x >= MAT_SIZE * NYChunk::CHUNK_SIZE)x = (MAT_SIZE * NYChunk::CHUNK_SIZE)-1;
		if(y >= MAT_SIZE * NYChunk::CHUNK_SIZE)y = (MAT_SIZE * NYChunk::CHUNK_SIZE)-1;
		if(z >= MAT_HEIGHT * NYChunk::CHUNK_SIZE)z = (MAT_HEIGHT * NYChunk::CHUNK_SIZE)-1;
		_Chunks[x / NYChunk::CHUNK_SIZE][y / NYChunk::CHUNK_SIZE][z / NYChunk::CHUNK_SIZE]->toVbo();
	}

	void deleteCube(int x, int y, int z)
	{
		NYCube * cube = getCube(x,y,z);
		cube->_Draw = false;
		cube = getCube(x-1,y,z);
		updateCube(x,y,z);	
	}

	//Création d'une pile de cubes
	//only if zero permet de ne générer la pile que si sa hauteur actuelle est de 0 (et ainsi de ne pas regénérer de piles existantes)
	void load_pile(int x, int y, int height, bool onlyIfZero = true)
	{
		if (!onlyIfZero || _MatriceHeights[x][y] == 0)
		{
			_MatriceHeights[x][y] = height;
			NYCube * cube = getCube(x, y, 0);
			cube->_Type = NYCubeType::CUBE_EAU;
			for (int z = 1; z < height - 1; ++z)
			{
				cube = getCube(x, y, z);
				cube->_Type = NYCubeType::CUBE_TERRE;
			}
			if (height > 1)
			{
				cube = getCube(x, y, height - 1);  // for a height of 3, setup 3 cubes at z = 0, 1 and 2 (therefore a height if 0 is impossible as we need at least water)
				cube->_Type = NYCubeType::CUBE_HERBE;
			}
		}
	}

	//Creation du monde entier, en utilisant le mouvement brownien fractionnaire
	// prof: profondeur de la recursion
	// profMax: profondeur max, s'arreter au-dela
	void generate_piles(int x1, int y1,
		int x2, int y2, 
		int x3, int y3,
		int x4, int y4, int prof, int profMax = -1)
	{
		if (prof > 500)
			Log::log(Log::USER_ERROR, ("generate_piles depth: " + to_string(prof)).c_str());

		bool isWideX = x2 - x1 > 1;
		bool isWideY = y4 - y1 > 1;

		// STOP condition
		if (!isWideX && !isWideY)
		{
			// patch reduced to 2x2 square, stop
			return;
		}

		// max recursion limit
		if (profMax != -1 && prof > profMax)
		{
			Log::log(Log::USER_ERROR, "Reached max recursion depth in generate_piles");
			return;
		}

		// normal recursion (with many branches)

		// reduce variation around average proportionally to distance between interpolated tiles (20% of distance)
		// (can also use 100 / pow(2, prof) since we split in half each time)
		int maxVariationX = 0.2f * (x2 - x1);
		std::uniform_int_distribution<int> distX(-maxVariationX, maxVariationX);
		int maxVariationY = 0.2f * (y4 - y1);
		std::uniform_int_distribution<int> distY(-maxVariationY, maxVariationY);
		int maxVariationXY = (int) floor(0.2f * ((x2 - x1) + (y4 - y1)) / 2);  // theoretically dist is a sqrt but expensive
		std::uniform_int_distribution<int> distXY(-maxVariationXY, maxVariationXY);

		// use some wrong index values to detect errors if those values are used but should not
		int midX = -1;
		int midY = -1;
		if (isWideX) midX = (x1 + x2) / 2;
		if (isWideY) midY = (y1 + y4) / 2;

		if (isWideX && isWideY)
		{
			// compute height at 5 pillar positions

			// left
			load_pile(x1, midY, computeAverageHeight(x1, y1, x1, y4, distY(random_engine)));
			// bottom (if we consider bottom-left coords, although matrix is probably top-left but in 3D quite arbitrary)
			load_pile(midX, y1, computeAverageHeight(x1, y1, x2, y1, distX(random_engine)));
			// right
			load_pile(x2, midY, computeAverageHeight(x2, y1, x2, y4, distY(random_engine)));
			// top
			load_pile(midX, y4, computeAverageHeight(x1, y4, x2, y4, distX(random_engine)));
			// center
			load_pile(midX, midY, computeAverageHeight(x1, y1, x2, y1, x2, y4, x1, y4, distXY(random_engine)));
			
			// bottom-left quarter
			generate_piles(x1, y1, midX, y1, midX, midY, x1, midY, prof + 1, profMax);

			// bottom-right quarter
			generate_piles(midX, y1, x2, y1, x2, midY, midX, midY, prof + 1, profMax);

			// top-right quarter
			generate_piles(midX, midY, x2, midY, x2, y4, midX, y4, prof + 1, profMax);

			// top-left quarter
			generate_piles(x1, midY, midX, midY, midX, y4, x1, y4, prof + 1, profMax);
		}
		else if (isWideX)
		{
			// Y size must be 2 only, so only compute 2 midX pillars and 2 halves

			// bottom
			load_pile(midX, y1, computeAverageHeight(x1, y1, x2, y1, distX(random_engine)));
			// top
			load_pile(midX, y4, computeAverageHeight(x1, y4, x2, y4, distX(random_engine)));

			// left half
			generate_piles(x1, y1, midX, y1, midX, y4, x1, y4, prof + 1, profMax);
			// right half
			generate_piles(midX, y1, x2, y1, x2, y4, midX, y4, prof + 1, profMax);
		}
		else
		{
			// X size must be 2 only, so only compute 2 midY pillars and 2 halves

			// left
			load_pile(x1, midY, computeAverageHeight(x1, y1, x1, y4, distY(random_engine)));
			// right
			load_pile(x2, midY, computeAverageHeight(x2, y1, x2, y4, distY(random_engine)));

			// bottom half
			generate_piles(x1, y1, x2, y1, x2, midY, x1, midY, prof + 1, profMax);
			// top half
			generate_piles(x1, midY, x2, midY, x2, y4, x1, y4, prof + 1, profMax);
		}
	}

	void lisse(void)
	{

	}

	void init_world(int profmax = -1)
	{
		_cprintf("Creation du monde %f \n",_FacteurGeneration);

		srand(6665);

		//Reset du monde
		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
					_Chunks[x][y][z]->reset();
		memset(_MatriceHeights,0x00,MAT_SIZE_CUBES*MAT_SIZE_CUBES*sizeof(int));

		//On charge les 4 coins
		load_pile(0,0,MAT_HEIGHT_CUBES/2);
		load_pile(MAT_SIZE_CUBES-1,0,MAT_HEIGHT_CUBES/2);
		load_pile(MAT_SIZE_CUBES-1,MAT_SIZE_CUBES-1,MAT_HEIGHT_CUBES/2);	
		load_pile(0,MAT_SIZE_CUBES-1,MAT_HEIGHT_CUBES/2);

		//On génère a partir des 4 coins
		generate_piles(0,0,
			MAT_SIZE_CUBES-1,0,
			MAT_SIZE_CUBES-1,MAT_SIZE_CUBES-1,
			0,MAT_SIZE_CUBES-1,1,profmax);	

		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
					_Chunks[x][y][z]->disableHiddenCubes();
	}

	NYCube * pick(NYVert3Df  pos, NYVert3Df  dir, NYPoint3D * point)
	{
		return NULL;
	}

	//Boites de collisions plus petites que deux cubes
	NYAxis getMinCol(NYVert3Df pos, float width, float height, float & valueColMin, int i)
	{
		NYAxis axis = 0x00;
		return axis;
	}


	void render_world_vbo(void)
	{
		for(int x=0;x<MAT_SIZE_CUBES;x++)
			for(int y=0;y<MAT_SIZE_CUBES;y++)
				for(int z=0;z<MAT_HEIGHT_CUBES;z++)
				{
					glPushMatrix();
					glTranslatef((float)(x*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE),(float)(y*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE),(float)(z*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE));
					_Chunks[x][y][z]->render();
					glPopMatrix();
				}
	}

	void add_world_to_vbo(void)
	{
		int totalNbVertices = 0;
		
		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
				{
					_Chunks[x][y][z]->toVbo();
					totalNbVertices += _Chunks[x][y][z]->_NbVertices;
				}

		Log::log(Log::ENGINE_INFO,(toString(totalNbVertices) + " vertices in VBO").c_str());
	}

	void render_world_old_school(void)
	{
		glEnable(GL_COLOR_MATERIAL);

		// iterate on world 3D tile matrix
		for (int x = 0; x<MAT_SIZE_CUBES; x++)
			for (int y = 0; y<MAT_SIZE_CUBES; y++)
				for (int z = 0; z<MAT_HEIGHT_CUBES; z++)
				{
					NYCube * cube = getCube(x, y, z);

					if (cube->_Draw && cube->_Type != NYCubeType::CUBE_AIR)
					{
						glPushMatrix();
						glTranslatef(NYCube::CUBE_SIZE * x, NYCube::CUBE_SIZE * y, NYCube::CUBE_SIZE * z);
						NYColor color = cubeColors[cube->_Type];
						glColor4f(color.R, color.V, color.B, color.A);
						glutSolidCube(NYCube::CUBE_SIZE);
						glPopMatrix();
					}
				}

		glDisable(GL_COLOR_MATERIAL);

	}

private:
	// Compute average height between 2 XY coords, adding an offset and clamping
	int computeAverageHeight(int x1, int y1, int x2, int y2, int offset)
	{
		int height = (_MatriceHeights[x1][y1] + _MatriceHeights[x2][y2]) / 2 + offset;
		if (height < 1) return 1;  // at least one cube per pile
		if (height > MAT_HEIGHT_CUBES) return MAT_HEIGHT_CUBES;
		return height;
	}

	// Same with average of 4 positions
	int computeAverageHeight(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int offset)
	{
		int height = (_MatriceHeights[x1][y1] + _MatriceHeights[x2][y2] + _MatriceHeights[x3][y3] + _MatriceHeights[x4][y4]) / 4 + offset;
		if (height < 1) return 1;  // at least one cube per pile
		if (height > MAT_HEIGHT_CUBES) return MAT_HEIGHT_CUBES;
		return height;
	}

};

#endif