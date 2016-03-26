#include "chunk.h"

#include <random>

// access variable from main
extern std::mt19937 random_engine;

// for each of the 3 main cube types
// we need 6 faces, 4 vertices per face since quads, 3 coords per vertex, hence 3*4*6
// 8 vertices are not enough, we need redundancy to 
float NYChunk::_WorldVert[NB_VBO][CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * NB_FLOAT_VECTOR3 * 4 * 6];
//float NYChunk::_WorldCols[NB_VBO][CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * NB_FLOAT_COLOR4 * 4 * 6];
float NYChunk::_WorldNorm[NB_VBO][CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * NB_FLOAT_VECTOR3 * 4 * 6];
float NYChunk::_WorldUV[NB_VBO][CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * NB_FLOAT_UV * 4 * 6];
float NYChunk::_WorldAttr[NB_VBO][CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * NB_FLOAT_ATTR1 * 4 * 6];

namespace
{
	// Update vertex, color and normal data for vertex at x, y, z (+1 to reach different points in the cube)
	void SetVertNormUVAttr(float * &ptVert, float *& ptNorm, float *& ptUV, float*& ptAttr,
	                    int x, int y, int z, const std::uniform_real_distribution<float> & dist,
	                    NYVert3Df normal, float u, float v, float attr)
	{
		*ptVert = x * (float) NYCube::CUBE_SIZE;
		*(ptVert + 1) = y * (float) NYCube::CUBE_SIZE;
		*(ptVert + 2) = z * (float) NYCube::CUBE_SIZE;
		ptVert += NB_FLOAT_VECTOR3;

		// currently we pass 3 values per color; if you want to unlock alpha channel, switch to 4 in buffer
//		*ptCols = color.R + dist(random_engine);
//		*(ptCols + 1) = color.V + dist(random_engine);
//		*(ptCols + 2) = color.B + dist(random_engine);
//		*(ptCols + 3) = color.A + dist(random_engine);
//		ptCols += NB_FLOAT_COLOR4;

		*ptNorm = normal.X;
		*(ptNorm + 1) = normal.Y;
		*(ptNorm + 2) = normal.Z;
		ptNorm += NB_FLOAT_VECTOR3;

		*ptUV = u;
		*(ptUV + 1) = v;
		ptUV += NB_FLOAT_UV;

		*ptAttr = attr;
		ptAttr += NB_FLOAT_ATTR1;

	}
}

void NYChunk::toVbos()
{
	// detruit le buffer s'il existe deja (verifie le premier no de buffer, qui devrait etre different de 0,
	// et suppose que les autres aussi ont ete definis)
	if (_WorldBuffers[0] != 0)
		glDeleteBuffers(NB_VBO, _WorldBuffers);

	// genere un ID
	// an array of 1 elt is theoretically what we want, although a pointer to the var is technically the same
	glGenBuffers(NB_VBO, _WorldBuffers);

	_TotalNbVertices = 0;

	for (NYCubeType cubeType : {CUBE_EARTH, CUBE_GRASS, CUBE_WATER})
//	for (NYCubeType cubeType : {CUBE_EARTH})
	{
		toVbo(cubeType);
		_TotalNbVertices += _NbVertices[cubeType];
	}
}

/// Store VBO for all cubes of given type in this chunk
void NYChunk::toVbo(NYCubeType cubeType)
{
	//On utilise les buffers temporaires pour préparer nos datas
	float * ptVert = _WorldVert[cubeType];
//	float * ptCols = _WorldCols[cubeType];
	float * ptNorm = _WorldNorm[cubeType];
	float * ptUV = _WorldUV[cubeType];
	float * ptAttr = _WorldAttr[cubeType];
	int nbVertices = 0;

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_SIZE; y++)
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				NYCube cube = _Cubes[x][y][z];

				// only register cubes of the passed type (earth, grass, water)
				if (cube._Draw && cube._Type == cubeType)
				{
					// Position du cube (coin bas gauche face avant)
					float xPos = x * (float) NYCube::CUBE_SIZE;
					float yPos = y * (float) NYCube::CUBE_SIZE;
					float zPos = z * (float) NYCube::CUBE_SIZE;

					NYCube * cubeXPrev = nullptr;
					NYCube * cubeXNext = nullptr;
					NYCube * cubeYPrev = nullptr;
					NYCube * cubeYNext = nullptr;
					NYCube * cubeZPrev = nullptr;
					NYCube * cubeZNext = nullptr;

					get_surrounding_cubes(x, y, z, cubeXPrev, cubeXNext, cubeYPrev, cubeYNext, cubeZPrev, cubeZNext);

					std::uniform_real_distribution<float> dist(-0.05f, 0.05f);

					float wave_factor = 0.f;

					// only apply wave effect for water surface, and on concerned vertices
					if (cube._Type == CUBE_WATER && (cubeZNext == nullptr || cubeZNext->_Type == CUBE_AIR))
					{
						// parametered by slider, but since chunks are created at init, the slider alone does not work without reset/update chunk
						wave_factor = 1.f;
					}

					// ne dessiner une face que si un cube transparent ou semi-transparent est devant
					// ne pas dessiner une face adjacente a un cube semi-transparent de meme nature (evite "grille" de tiles d'eau
					// qui ne donnent pas du tout un Beer-Lambert visuellement plaisant)

					// Premier QUAD (Z-)
					if (cubeZPrev == nullptr || (cube.isSolid() && !cubeZPrev->isSolid()) || (!cube.isSolid() && !cubeZPrev->isSolid() && cube._Type != cubeZPrev->_Type))
					{
						NYVert3Df normal = NYVert3Df(0, 0, -1);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x, y, z, dist, normal, 0, 0, 0);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x, y + 1, z, dist, normal, 0, 1, 0);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x + 1, y + 1, z, dist, normal, 1, 1, 0);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x + 1, y, z, dist, normal, 1, 0, 0);
						nbVertices += 4;  // increment inside condition, so that array size is correct is some faces are skipped
						// NOTE: if you don't, the expected buffer size will be longer than the actual size of data
						// which will render garbage such as 2x the same level at different heights...
					}

					//Second QUAD (X+)
					// because of water wave, do not hide side quads for water, except if neighbor is also water
					// we assume lower water cubes are already hidden
					if (cubeXNext == nullptr || (cube.isSolid()&& !cubeXNext->isSolid()) || (!cube.isSolid()&& !cubeXNext->isSolid()&& cube._Type != cubeXNext->_Type) || (wave_factor == 1.f && cubeXNext->_Type != CUBE_WATER))
					{
						NYVert3Df normal = NYVert3Df(1, 0, 0);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x + 1, y, z, dist, normal, 0, 0, 0);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x + 1, y + 1, z, dist, normal, 1, 0, 0);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x + 1, y + 1, z + 1, dist, normal, 1, 1, wave_factor);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x + 1, y, z + 1, dist, normal, 0, 1, wave_factor);
						nbVertices += 4;
					}

					//Troisieme QUAD (X-)
					if (cubeXPrev == nullptr || (cube.isSolid()&& !cubeXPrev->isSolid()) || (!cube.isSolid()&& !cubeXPrev->isSolid()&& cube._Type != cubeXPrev->_Type) || (wave_factor == 1.f && cubeXPrev->_Type != CUBE_WATER))
					{
						NYVert3Df normal = NYVert3Df(-1, 0, 0);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x, y, z, dist, normal, 0, 0, 0);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x, y, z + 1, dist, normal, 1, 0, wave_factor);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x, y + 1, z + 1, dist, normal, 1, 1, wave_factor);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x, y + 1, z, dist, normal, 0, 1, 0);
						nbVertices += 4;
					}

					//Quatrieme QUAD (Y+)
					if (cubeYNext == nullptr || (cube.isSolid()&& !cubeYNext->isSolid()) || (!cube.isSolid()&& !cubeYNext->isSolid()&& cube._Type != cubeYNext->_Type) || (wave_factor == 1.f && cubeYNext->_Type != CUBE_WATER))
					{
						NYVert3Df normal = NYVert3Df(0, 1, 0);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x, y + 1, z, dist, normal, 0, 0, 0);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x, y + 1, z + 1, dist, normal, 1, 0, wave_factor);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x + 1, y + 1, z + 1, dist, normal, 1, 1, wave_factor);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x + 1, y + 1, z, dist, normal, 0, 1, 0);
						nbVertices += 4;
					}

					//Cinquieme QUAD (Y-)
					if (cubeYPrev == nullptr || (cube.isSolid()&& !cubeYPrev->isSolid()) || (!cube.isSolid()&& !cubeYPrev->isSolid()&& cube._Type != cubeYPrev->_Type) || (wave_factor == 1.f && cubeYPrev->_Type != CUBE_WATER))
					{
						NYVert3Df normal = NYVert3Df(0, -1, 0);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x, y, z, dist, normal, 0, 0, 0);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x + 1, y, z, dist, normal, 1, 0, 0);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x + 1, y, z + 1, dist, normal, 1, 1, wave_factor);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x, y, z + 1, dist, normal, 0, 1, wave_factor);
						nbVertices += 4;
					}

					if (cubeZNext == nullptr || (cube.isSolid()&& !cubeZNext->isSolid()) || (!cube.isSolid()&& !cubeZNext->isSolid()&& cube._Type != cubeZNext->_Type))
					{
						//Sixieme QUAD (Z+)
						NYVert3Df normal = NYVert3Df(0, 0, 1);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x, y, z + 1, dist, normal, 0, 0, wave_factor);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x + 1, y, z + 1, dist, normal, 1, 0, wave_factor);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x + 1, y + 1, z + 1, dist, normal, 1, 1, wave_factor);
						SetVertNormUVAttr(ptVert, ptNorm, ptUV, ptAttr, x, y + 1, z + 1, dist, normal, 0, 1, wave_factor);
						nbVertices += 4;
					}

				}
			}

	//On attache le VBO pour pouvoir le modifier
	glBindBuffer(GL_ARRAY_BUFFER, _WorldBuffers[cubeType]);

	//On reserve la quantite totale de datas (creation de la zone memoire, mais sans passer les données)
	//Les tailles g_size* sont en octets, à vous de les calculer
	glBufferData(GL_ARRAY_BUFFER,
		nbVertices * SIZE_VERTEX +
		nbVertices * SIZE_NORMAL +
		nbVertices * SIZE_UV +
		nbVertices * SIZE_ATTR,
		nullptr,
		GL_STREAM_DRAW);

	//Check error (la tester ensuite...)
	error = glGetError();

	//On copie les vertices
	glBufferSubData(GL_ARRAY_BUFFER,
		0, //Offset 0, on part du debut                        
		nbVertices * SIZE_VERTEX, //Taille en octets des datas copiés
		_WorldVert[cubeType]);  //Datas          

						//Check error (la tester ensuite...)
	error = glGetError();

//	//On copie les couleurs
//	glBufferSubData(GL_ARRAY_BUFFER,
//		nbVertices * SIZE_VERTEX, //Offset : on se place après les vertices
//		nbVertices * SIZE_COLOR, //On copie tout le buffer couleur : on donne donc sa taille
//		_WorldCols[cubeType]);  //Pt sur le buffer couleur       
//
//	//Check error (la tester ensuite...)
//	error = glGetError();

	//On copie les normales (a vous de déduire les params)
	glBufferSubData(GL_ARRAY_BUFFER,
//		nbVertices * SIZE_VERTEX + nbVertices * SIZE_COLOR,
		nbVertices * SIZE_VERTEX,
		nbVertices * SIZE_NORMAL,
		_WorldNorm[cubeType]);

	//Check error (la tester ensuite...)
	error = glGetError();

	//On copie les normales (a vous de déduire les params)
	glBufferSubData(GL_ARRAY_BUFFER,
//		nbVertices * SIZE_VERTEX + nbVertices * SIZE_COLOR + nbVertices * SIZE_NORMAL,
		nbVertices * SIZE_VERTEX + nbVertices * SIZE_NORMAL,
		nbVertices * SIZE_UV,
		_WorldUV[cubeType]);

	error = glGetError();

	//On copie les attributs de vertex
	glBufferSubData(GL_ARRAY_BUFFER,
//		nbVertices * SIZE_VERTEX + nbVertices * SIZE_COLOR + nbVertices * SIZE_NORMAL + nbVertices * SIZE_UV,
		nbVertices * SIZE_VERTEX + nbVertices * SIZE_NORMAL + nbVertices * SIZE_UV,
		nbVertices * SIZE_ATTR,
		_WorldAttr[cubeType]);

	error = glGetError();

	// record number of vertices for this cube type, so that when rendering VBO we have the correct sub-buffer size for each type
	_NbVertices[cubeType] = nbVertices;

	//On debind le buffer pour eviter une modif accidentelle par le reste du code
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
