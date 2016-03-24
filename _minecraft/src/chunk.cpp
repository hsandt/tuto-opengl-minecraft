#include "chunk.h"

#include "world.h"

// for each of the 3 main cube types
// we need 6 faces, 4 vertices per face since quads, 3 coords per vertex, hence 3*4*6
// 8 vertices are not enough, we need redundancy to 
float NYChunk::_WorldVert[3][CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * NB_FLOAT_VECTOR3 * 4 * 6];
float NYChunk::_WorldCols[3][CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * NB_FLOAT_COLOR4 * 4 * 6];
float NYChunk::_WorldNorm[3][CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * NB_FLOAT_VECTOR3 * 4 * 6];
float NYChunk::_WorldUV[3][CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * NB_FLOAT_UV * 4 * 6];
float NYChunk::_WorldAttr[3][CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * NB_FLOAT_ATTR1 * 4 * 6];

namespace
{
	// Update vertex, color and normal data for vertex at x, y, z (+1 to reach different points in the cube)
	void SetVertColNorm(float * &ptVert, float * &ptCols, float * &ptNorm, float * &ptUV, float*& ptAttr,
	                    int x, int y, int z, NYColor color, const std::uniform_real_distribution<float> & dist,
	                    NYVert3Df normal, float u, float v, float attr)
	{
		*ptVert = x * (float)NYCube::CUBE_SIZE;
		*(ptVert + 1) = y * (float)NYCube::CUBE_SIZE;
		*(ptVert + 2) = z * (float)NYCube::CUBE_SIZE;
		ptVert += NB_FLOAT_VECTOR3;

		// currently we pass 3 values per color; if you want to unlock alpha channel, switch to 4 in buffer
		*ptCols = color.R + dist(random_engine);
		*(ptCols + 1) = color.V + dist(random_engine);
		*(ptCols + 2) = color.B + dist(random_engine);
		*(ptCols + 3) = color.A + dist(random_engine);
		ptCols += NB_FLOAT_COLOR4;

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
	{
		toVbo(cubeType);
		_TotalNbVertices += _NbVertices;
	}
}

/// Store VBO for all cubes of given type in this chunk
void NYChunk::toVbo(NYCubeType cubeType)
{
	//On utilise les buffers temporaires pour préparer nos datas
	float * ptVert = _WorldVert[cubeType];
	float * ptCols = _WorldCols[cubeType];
	float * ptNorm = _WorldNorm[cubeType];
	float * ptUV = _WorldUV[cubeType];
	float * ptAttr = _WorldAttr[cubeType];
	_NbVertices = 0;

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_SIZE; y++)
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				NYCube cube = _Cubes[x][y][z];

				// only register cubes of the passed type (earth, grass, water)
				if (cube._Draw && cube._Type == cubeType)
				{
					// Position du cube (coin bas gauche face avant)
					float xPos = x * (float)NYCube::CUBE_SIZE;
					float yPos = y * (float)NYCube::CUBE_SIZE;
					float zPos = z * (float)NYCube::CUBE_SIZE;

					NYCube * cubeXPrev = nullptr;
					NYCube * cubeXNext = nullptr;
					NYCube * cubeYPrev = nullptr;
					NYCube * cubeYNext = nullptr;
					NYCube * cubeZPrev = nullptr;
					NYCube * cubeZNext = nullptr;

					get_surrounding_cubes(x, y, z, cubeXPrev, cubeXNext, cubeYPrev, cubeYNext, cubeZPrev, cubeZNext);

					NYColor color = Game::Instance().g_world->cubeColors[cube._Type];
					std::uniform_real_distribution<float> dist(-0.05f, 0.05f);

					float wave_factor = 0.f;

					// only apply wave effect for water surface, and on concerned vertices
					if (cube._Type == CUBE_WATER && (cubeZNext == nullptr || cubeZNext->_Type == CUBE_AIR))
					{
						// parametered by slider, but since chunks are created at init, the slider alone does not work without reset/update chunk
						wave_factor = 1.f;
					}

					// ne dessiner une face que si un cube transparent ou semi-transparent est devant

					// Premier QUAD (Z-)
					if (cubeZPrev == nullptr || cubeZPrev->_Type == CUBE_AIR)
					{
						NYVert3Df normal = NYVert3Df(0, 0, -1);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x, y, z, color, dist, normal, 0, 0, 0);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x, y + 1, z, color, dist, normal, 0, 1, 0);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x + 1, y + 1, z, color, dist, normal, 1, 1, 0);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x + 1, y, z, color, dist, normal, 1, 0, 0);
						_NbVertices += 4;  // increment inside condition, so that array size is correct is some faces are skipped
						// NOTE: if you don't, the expected buffer size will be longer than the actual size of data
						// which will render garbage such as 2x the same level at different heights...
					}

					//Second QUAD (X+)
					// because of water wave, do not hide side quads for water, except if neighbor is also water
					// we assume lower water cubes are already hidden
					if (cubeXNext == nullptr || cubeXNext->_Type == CUBE_AIR || (wave_factor == 1.f && cubeXNext->_Type != CUBE_WATER))
					{
						NYVert3Df normal = NYVert3Df(1, 0, 0);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x + 1, y, z, color, dist, normal, 0, 0, 0);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x + 1, y + 1, z, color, dist, normal, 1, 0, 0);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x + 1, y + 1, z + 1, color, dist, normal, 1, 1, wave_factor);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x + 1, y, z + 1, color, dist, normal, 0, 1, wave_factor);
						_NbVertices += 4;
					}

					//Troisieme QUAD (X-)
					if (cubeXPrev == nullptr || cubeXPrev->_Type == CUBE_AIR || (wave_factor == 1.f && cubeXPrev->_Type != CUBE_WATER))
					{
						NYVert3Df normal = NYVert3Df(-1, 0, 0);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x, y, z, color, dist, normal, 0, 0, 0);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x, y, z + 1, color, dist, normal, 1, 0, wave_factor);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x, y + 1, z + 1, color, dist, normal, 1, 1, wave_factor);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x, y + 1, z, color, dist, normal, 0, 1, 0);
						_NbVertices += 4;
					}

					//Quatrieme QUAD (Y+)
					if (cubeYNext == nullptr || cubeYNext->_Type == CUBE_AIR || (wave_factor == 1.f && cubeYNext->_Type != CUBE_WATER))
					{
						NYVert3Df normal = NYVert3Df(0, 1, 0);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x, y + 1, z, color, dist, normal, 0, 0, 0);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x, y + 1, z + 1, color, dist, normal, 1, 0, wave_factor);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x + 1, y + 1, z + 1, color, dist, normal, 1, 1, wave_factor);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x + 1, y + 1, z, color, dist, normal, 0, 1, 0);
						_NbVertices += 4;
					}

					//Cinquieme QUAD (Y-)
					if (cubeYPrev == nullptr || cubeYPrev->_Type == CUBE_AIR || (wave_factor == 1.f && cubeYPrev->_Type != CUBE_WATER))
					{
						NYVert3Df normal = NYVert3Df(0, -1, 0);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x, y, z, color, dist, normal, 0, 0, 0);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x + 1, y, z, color, dist, normal, 1, 0, 0);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x + 1, y, z + 1, color, dist, normal, 1, 1, wave_factor);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x, y, z + 1, color, dist, normal, 0, 1, wave_factor);
						_NbVertices += 4;
					}

					if (cubeZNext == nullptr || cubeZNext->_Type == CUBE_AIR)
					{
						//Sixieme QUAD (Z+)
						NYVert3Df normal = NYVert3Df(0, 0, 1);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x, y, z + 1, color, dist, normal, 0, 0, wave_factor);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x + 1, y, z + 1, color, dist, normal, 1, 0, wave_factor);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x + 1, y + 1, z + 1, color, dist, normal, 1, 1, wave_factor);
						SetVertColNorm(ptVert, ptCols, ptNorm, ptUV, ptAttr, x, y + 1, z + 1, color, dist, normal, 0, 1, wave_factor);
						_NbVertices += 4;
					}

				}
			}

	//On attache le VBO pour pouvoir le modifier
	glBindBuffer(GL_ARRAY_BUFFER, _WorldBuffers[cubeType]);

	//On reserve la quantite totale de datas (creation de la zone memoire, mais sans passer les données)
	//Les tailles g_size* sont en octets, à vous de les calculer
	glBufferData(GL_ARRAY_BUFFER,
		_NbVertices * SIZE_VERTEX +
		_NbVertices * SIZE_COLOR +
		_NbVertices * SIZE_NORMAL +
		_NbVertices * SIZE_UV +
		_NbVertices * SIZE_ATTR,
		nullptr,
		GL_STREAM_DRAW);

	//Check error (la tester ensuite...)
	error = glGetError();

	//On copie les vertices
	glBufferSubData(GL_ARRAY_BUFFER,
		0, //Offset 0, on part du debut                        
		_NbVertices * SIZE_VERTEX, //Taille en octets des datas copiés
		_WorldVert[cubeType]);  //Datas          

						//Check error (la tester ensuite...)
	error = glGetError();

	//On copie les couleurs
	glBufferSubData(GL_ARRAY_BUFFER,
		_NbVertices * SIZE_VERTEX, //Offset : on se place après les vertices
		_NbVertices * SIZE_COLOR, //On copie tout le buffer couleur : on donne donc sa taille
		_WorldCols[cubeType]);  //Pt sur le buffer couleur       

	//Check error (la tester ensuite...)
	error = glGetError();

	//On copie les normales (a vous de déduire les params)
	glBufferSubData(GL_ARRAY_BUFFER,
		_NbVertices * SIZE_VERTEX + _NbVertices * SIZE_COLOR,
		_NbVertices * SIZE_NORMAL,
		_WorldNorm[cubeType]);

	//Check error (la tester ensuite...)
	error = glGetError();

	//On copie les normales (a vous de déduire les params)
	glBufferSubData(GL_ARRAY_BUFFER,
		_NbVertices * SIZE_VERTEX + _NbVertices * SIZE_COLOR + _NbVertices * SIZE_NORMAL,
		_NbVertices * SIZE_UV,
		_WorldUV[cubeType]);

	error = glGetError();

	//On copie les attributs de vertex
	glBufferSubData(GL_ARRAY_BUFFER,
		_NbVertices * SIZE_VERTEX + _NbVertices * SIZE_COLOR + _NbVertices * SIZE_NORMAL + _NbVertices * SIZE_UV,
		_NbVertices * SIZE_ATTR,
		_WorldAttr[cubeType]);

	error = glGetError();

	//On debind le buffer pour eviter une modif accidentelle par le reste du code
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
