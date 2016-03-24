#ifndef __WORLD_H__
#define __WORLD_H__

#include <random>

#include "gl/glew.h"
#include "gl/freeglut.h" 
#include "engine/utils/types_3d.h"
#include "engine/render/graph/tex_manager.h"
#include "Game.h" // split in cpp and h to avoid this stuff
#include "cube.h"
#include "chunk.h"

// access variable from main
extern std::mt19937 random_engine;

typedef uint8 NYAxis;
#define NY_AXIS_X 0x01
#define NY_AXIS_Y 0x02
#define NY_AXIS_Z 0x04

#define MAT_SIZE 3 //en nombre de chunks
#define MAT_HEIGHT 2 //en nombre de chunks
#define MAT_SIZE_CUBES (MAT_SIZE * NYChunk::CHUNK_SIZE)
#define MAT_HEIGHT_CUBES (MAT_HEIGHT * NYChunk::CHUNK_SIZE)

class NYWorld
{
public :
	NYChunk * _Chunks[MAT_SIZE][MAT_SIZE][MAT_HEIGHT];
	int _MatriceHeights[MAT_SIZE_CUBES][MAT_SIZE_CUBES];
	int _MatriceHeightsTmp[MAT_SIZE_CUBES][MAT_SIZE_CUBES];
	float _FacteurGeneration;
	float _HeightDiffFactor = 0.2f;  // the higher, the bigger ground height variations
	int _WaterLevel = 10;

	NYColor cubeColors[4];

	NYWorld()
	{
		cubeColors[CUBE_GRASS] = NYColor(0.f / 255.f, 123.f / 255.f, 12.f / 255.f, 1.f);
		cubeColors[CUBE_WATER] = NYColor(20.f / 255.f, 70.f / 255.f, 180.f / 255.f, 0.5f);  // if you make it transparent, adapt Cube::isSolid
		cubeColors[CUBE_EARTH] = NYColor(148.f / 255.f, 62.f / 255.f, 15.f / 255.f, 1.f);
		cubeColors[CUBE_AIR] = NYColor(255.f / 255.f, 255.f / 255.f, 255.f / 255.f, 0.0f);

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
		// OPTIMIZE: if we knew what that cube *was*, we could just update the VBOs corresponding to the previous and new types of the cube
		_Chunks[x / NYChunk::CHUNK_SIZE][y / NYChunk::CHUNK_SIZE][z / NYChunk::CHUNK_SIZE]->toVbos();
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
			// interval safety
			if (height < 1)
				height = 1;
			if (height > MAT_HEIGHT_CUBES)
				height = MAT_HEIGHT_CUBES;

			_MatriceHeights[x][y] = height;
			NYCube * cube;
			for (int z = 0; z < height - 1; ++z)
			{
				cube = getCube(x, y, z);
				cube->_Draw = true;
				cube->_Type = NYCubeType::CUBE_EARTH;
			}
			cube = getCube(x, y, height - 1);  // for a height of 3, setup 3 cubes at z = 0, 1 and 2 (therefore a height if 0 is impossible as we need at least water)
			cube->_Draw = true;
			cube->_Type = NYCubeType::CUBE_GRASS;
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

		// reduce variation around average proportionally to distance between interpolated tiles (X% of distance)
		// (can also use 100 / pow(2, prof) since we split in half each time)
		int maxVariationX = _HeightDiffFactor * (x2 - x1);
		std::uniform_int_distribution<int> distX(-maxVariationX, maxVariationX);
		int maxVariationY = _HeightDiffFactor * (y4 - y1);
		std::uniform_int_distribution<int> distY(-maxVariationY, maxVariationY);
		int maxVariationXY = (int) floor(_HeightDiffFactor * ((x2 - x1) + (y4 - y1)) / 2);  // theoretically dist is a sqrt but expensive so use Manhattan dist
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

	//On utilise un matrice temporaire _MatriceHeightsTmp à déclarer
	//Penser à appeler la fonction a la fin de la génération (plusieurs fois si besoin)
	void lisse(void)
	{
		int sizeWindow = 4;
		memset(_MatriceHeightsTmp, 0x00, sizeof(int)*MAT_SIZE_CUBES*MAT_SIZE_CUBES);
		for (int x = 0; x<MAT_SIZE_CUBES; x++)
		{
			for (int y = 0; y<MAT_SIZE_CUBES; y++)
			{
				//on moyenne sur une distance
				int nb = 0;
				for (int i = (x - sizeWindow < 0 ? 0 : x - sizeWindow);
				i < (x + sizeWindow >= MAT_SIZE_CUBES ? MAT_SIZE_CUBES - 1 : x + sizeWindow); i++)
				{
					for (int j = (y - sizeWindow < 0 ? 0 : y - sizeWindow);
					j <(y + sizeWindow >= MAT_SIZE_CUBES ? MAT_SIZE_CUBES - 1 : y + sizeWindow); j++)
					{
						_MatriceHeightsTmp[x][y] += _MatriceHeights[i][j];
						nb++;
					}
				}
				if (nb)
					_MatriceHeightsTmp[x][y] /= nb;
			}
		}

		//On reset les piles
		for (int x = 0; x<MAT_SIZE_CUBES; x++)
		{
			for (int y = 0; y<MAT_SIZE_CUBES; y++)
			{
				load_pile(x, y, _MatriceHeightsTmp[x][y], false);
			}
		}
	}

	/// Fill world with water until given level (0 for no water)
	void fill_water(int level)
	{
		for (int x = 0; x < MAT_SIZE_CUBES; x++)
		{
			for (int y = 0; y < MAT_SIZE_CUBES; y++)
			{
				// only fill columns lower or at water level with water
				if (_MatriceHeights[x][y] < _WaterLevel)
				{
					_MatriceHeights[x][y] = _WaterLevel;
					for (int z = 0; z < _WaterLevel; z++)
					{
						getCube(x, y, z)->_Draw = true;
						getCube(x, y, z)->_Type = CUBE_WATER;
					}
				}
			}
		}
	}

	void init_world(int profmax = -1)
	{
		// chargement texture
		_TexGrass = NYTexManager::getInstance()->loadTexture(std::string("textures/grass.png"));

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

//		lisse();
		fill_water(_WaterLevel);

		// disable cubes completely surrounded by other cubes
		// do this before adding to VBO so that only visible cubes are added to the VBOs
		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
					_Chunks[x][y][z]->disableHiddenCubes();

		add_world_to_vbo();

		// set camera et acceptable position
		// do not look completely downward, deactivated for now (for an editor and not a character, maybe acceptable)
		Game::Instance().g_renderer->_Camera->setPosition(NYVert3Df(12, 12, 50) * NYCube::CUBE_SIZE);
		Game::Instance().g_renderer->_Camera->setLookAt(NYVert3Df(16, 16, 0) * NYCube::CUBE_SIZE);
		
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
		glDisable(GL_TEXTURE_2D);

//		glEnable(GL_TEXTURE_2D);
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, _TexGrass->Texture);

		// iterate on chunks NOT cubes
		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
				{
					glPushMatrix();
					// set whole chunk origin to bottom-left-close corner in the world
					glTranslatef((float)(x*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE),(float)(y*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE),(float)(z*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE));
					_Chunks[x][y][z]->render();
					glPopMatrix();
				}

//		glDisable(GL_TEXTURE_2D);
	}

	void add_world_to_vbo(void)
	{
		int totalNbVertices = 0;
		
		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
				{
					_Chunks[x][y][z]->toVbos();
					totalNbVertices += _Chunks[x][y][z]->_TotalNbVertices;
				}

		Log::log(Log::ENGINE_INFO,(toString(totalNbVertices) + " vertices in VBO").c_str());
	}

	void render_world_old_school(void)
	{
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

	}

private:

	// texture files
	NYTexFile * _TexGrass;

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