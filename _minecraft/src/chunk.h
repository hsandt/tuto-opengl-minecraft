#pragma once

#include "engine/render/renderer.h"
#include "Game.h"
#include "cube.h"

#define NB_FLOAT_VECTOR3 3
#define NB_FLOAT_COLOR4 4
#define NB_FLOAT_UV 2
#define NB_FLOAT_ATTR1 1

#define NB_VBO 3

/**
  * On utilise des chunks pour que si on modifie juste un cube, on ait pas
  * besoin de recharger toute la carte dans le buffer, mais juste le chunk en question
  */
class NYChunk
{
	public :

		static const int CHUNK_SIZE = 16; ///< Taille d'un chunk en nombre de cubes (n*n*n)
		NYCube _Cubes[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; ///< Cubes contenus dans le chunk

		GLuint _WorldBuffers[NB_VBO]; ///< Array d'identifiants des VBOs pour le monde
		
		static float _WorldVert[NB_VBO][CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * NB_FLOAT_VECTOR3 * 4 * 6]; ///< Buffer pour les sommets
//		static float _WorldCols[NB_VBO][CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * NB_FLOAT_COLOR4 * 4 * 6]; ///< Buffer pour les couleurs
		static float _WorldNorm[NB_VBO][CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * NB_FLOAT_VECTOR3 * 4 * 6]; ///< Buffer pour les normales
		static float _WorldUV[NB_VBO][CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * NB_FLOAT_UV * 4 * 6]; ///< Buffer pour les UV
		static float _WorldAttr[NB_VBO][CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * NB_FLOAT_ATTR1 * 4 * 6]; /// Buffer pour les attributs de vertex

		static const int SIZE_VERTEX = NB_FLOAT_VECTOR3 * sizeof(float); ///< Taille en octets d'un vertex dans le VBO
//		static const int SIZE_COLOR = NB_FLOAT_COLOR4 * sizeof(float);  ///< Taille d'une couleur dans le VBO
		static const int SIZE_NORMAL = NB_FLOAT_VECTOR3 * sizeof(float);  ///< Taille d'une normale dans le VBO
		static const int SIZE_UV = NB_FLOAT_UV * sizeof(float);  ///< Taille d'une coord UV dans le VBO
		static const int SIZE_ATTR = NB_FLOAT_ATTR1 * sizeof(float);  /// size of a float attribute

		int _TotalNbVertices; ///< Nombre de vertices dans l'ensemble des VBO
		int _NbVertices[NB_VBO]; ///< Nombre de vertices dans le VBO courant (on ne met que les faces visibles)

		NYChunk * Voisins[6];
		
		// added
		GLenum error;

		NYChunk()
		{
			memset(Voisins,0x00,sizeof(void*) * 6);
		}

		void setVoisins(NYChunk * xprev, NYChunk * xnext,NYChunk * yprev,NYChunk * ynext,NYChunk * zprev,NYChunk * znext)
		{
			Voisins[0] = xprev;
			Voisins[1] = xnext;
			Voisins[2] = yprev;
			Voisins[3] = ynext;
			Voisins[4] = zprev;
			Voisins[5] = znext;
		}

		/**
		  * Raz de l'état des cubes (a draw = false)
		  */
		void reset(void)
		{
			for(int x=0;x<CHUNK_SIZE;x++)
				for(int y=0;y<CHUNK_SIZE;y++)
					for(int z=0;z<CHUNK_SIZE;z++)
					{
						_Cubes[x][y][z]._Draw = true;
						_Cubes[x][y][z]._Type = CUBE_AIR;
					}
		}

		// Genere tous les VBOs
		void toVbos();

		//On met le chunk ddans son VBO
		void toVbo(NYCubeType cubeType);

		// DEBUG! should render opaque layers from ALL VBOs first
		void render(bool opaque)
		{
			// render opaque, then transparent layers
			if (opaque)
			{
				for (NYCubeType cubeType : { CUBE_EARTH, CUBE_GRASS })
					// setup uniform shader params for this type
					renderVbo(cubeType);
			}
			else
			{
				// note: translucent rendering is complex because farther objects must be rendered first,
				// so if there are several types of translucent cubes, need a lot of work
				// even water cubes themselves should be drawn in this order, which we don't do, so depending on the camera angle
				// alpha blending looks different
				// The best is probably to work on Fragment shader directly to calculate correct alpha blendingrenderVbo(CUBE_WATER);
			
				// setup uniform shader params for this type
				renderVbo(CUBE_WATER);
			}
		}

		void renderVbo(NYCubeType cubeType)
		{
			// for debug, remove when using textures
//			glEnable(GL_COLOR_MATERIAL);
			glEnable(GL_LIGHTING);

			if (cubeType == CUBE_GRASS)
			{
				glUseProgram(NYRenderer::getInstance()->_ProgramCubeGrass);
//				glEnable(GL_TEXTURE_2D);
//				glActiveTexture(GL_TEXTURE0);
//				glBindTexture(GL_TEXTURE_2D, NYRenderer::getInstance()->_TexGrass->Texture);
//
//				GLuint texLoc = glGetUniformLocation(NYRenderer::getInstance()->_ProgramCubeGrass, "Texture0");
//				glUniform1i(texLoc, 0);

				NYColor ambientColor = NYCube::cubeAmbientColors[cubeType];
				NYColor diffuseColor = NYCube::cubeDiffuseColors[cubeType];
				NYColor specularColor = NYCube::cubeSpecularColors[cubeType];
				float shininess = NYCube::cubeShininess[cubeType];

				GLuint amb = glGetUniformLocation(Game::Instance().g_renderer->_ProgramCubeGrass, "ambientColor");
				GLuint diff = glGetUniformLocation(Game::Instance().g_renderer->_ProgramCubeGrass, "diffuseColor");
				GLuint spec = glGetUniformLocation(Game::Instance().g_renderer->_ProgramCubeGrass, "specularColor");
				GLuint shini = glGetUniformLocation(Game::Instance().g_renderer->_ProgramCubeGrass, "shininess");
				glUniform4f(amb, ambientColor.R, ambientColor.G, ambientColor.B, ambientColor.A);
				glUniform3f(diff, diffuseColor.R, diffuseColor.G, diffuseColor.B);
				glUniform3f(spec, specularColor.R, specularColor.G, specularColor.B);
				glUniform1f(shini, shininess);
			}
			else
			{
				glUseProgram(NYRenderer::getInstance()->_ProgramCube);

				// parameters that are common to all cubes of this type (no need for buffer array)
				NYColor ambientColor = NYCube::cubeAmbientColors[cubeType];
				NYColor diffuseColor = NYCube::cubeDiffuseColors[cubeType];
				NYColor specularColor = NYCube::cubeSpecularColors[cubeType];
				float shininess = NYCube::cubeShininess[cubeType];
			
				GLuint amb = glGetUniformLocation(Game::Instance().g_renderer->_ProgramCube, "ambientColor");
				GLuint diff = glGetUniformLocation(Game::Instance().g_renderer->_ProgramCube, "diffuseColor");
				GLuint spec = glGetUniformLocation(Game::Instance().g_renderer->_ProgramCube, "specularColor");
				GLuint shini = glGetUniformLocation(Game::Instance().g_renderer->_ProgramCube, "shininess");
				glUniform4f(amb, ambientColor.R, ambientColor.G, ambientColor.B, ambientColor.A);
				glUniform3f(diff, diffuseColor.R, diffuseColor.G, diffuseColor.B);
				glUniform3f(spec, specularColor.R, specularColor.G, specularColor.B);
				glUniform1f(shini, shininess);
			}

			//On bind le buffer
			glBindBuffer(GL_ARRAY_BUFFER, _WorldBuffers[cubeType]);
			NYRenderer::checkGlError("glBindBuffer");

			//On active les datas que contiennent le VBO
			glEnableClientState(GL_VERTEX_ARRAY);
//			glEnableClientState(GL_COLOR_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			GLuint wave_factor_loc = glGetAttribLocation(NYRenderer::getInstance()->_ProgramCube, "wave_factor");
			glEnableVertexAttribArray(wave_factor_loc);

			//On place les pointeurs sur les datas, aux bons offsets
			int nbVertices = _NbVertices[cubeType];
			glVertexPointer(NB_FLOAT_VECTOR3, GL_FLOAT, 0, (void*)(0));  // c'est bien une adresse relative
//			glColorPointer(NB_FLOAT_COLOR4, GL_FLOAT, 0, (void*)(nbVertices*SIZE_VERTEX));
			glNormalPointer(GL_FLOAT, 0, (void*)(nbVertices*SIZE_VERTEX));
			glTexCoordPointer(NB_FLOAT_UV, GL_FLOAT, 0, (void*)(nbVertices*SIZE_VERTEX + nbVertices*SIZE_NORMAL));
			// pointer on wave amplitude attribute data, so that earth and water blocks are drawn with the correct wave effect
			glVertexAttribPointer(wave_factor_loc, NB_FLOAT_ATTR1, GL_FLOAT, GL_FALSE, 0, (void*)(nbVertices*SIZE_VERTEX + nbVertices*SIZE_NORMAL + nbVertices*SIZE_UV));

//			glUseProgram(0);  // DEBUG: textures alone work!

			//On demande le dessin
			glDrawArrays(GL_QUADS, 0, nbVertices);

			//On cleane
//			glDisableClientState(GL_COLOR_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableVertexAttribArray(wave_factor_loc);

			glDisable(GL_LIGHTING);
//			glDisable(GL_COLOR_MATERIAL);

//			glBindBuffer(GL_ARRAY_BUFFER, 0);

			if (cubeType == CUBE_GRASS)
			{
//				glBindTexture(GL_TEXTURE_2D, 0);
//				glDisable(GL_TEXTURE_2D);
			}

			glUseProgram(0);

		}
		
		void get_surrounding_cubes(int x, int y, int z,
			NYCube* &cubeXPrev, NYCube* &cubeXNext,
			NYCube* &cubeYPrev, NYCube* &cubeYNext,
			NYCube* &cubeZPrev, NYCube* &cubeZNext)
		{
			// if cube not on X- edge (x > 0), the X- neighbor is in the same chunk
			// else the neighbor must be taken from the X- neighboring chunk (if any)
			// ALTERNATIVE: work in world cube coordinates and use getCube(x, y, z) which already contains a modulo op
			if (x == 0 && Voisins[0] != NULL)
				cubeXPrev = &(Voisins[0]->_Cubes[CHUNK_SIZE - 1][y][z]);
			else if (x > 0)
				cubeXPrev = &(_Cubes[x - 1][y][z]);

			if (x == CHUNK_SIZE - 1 && Voisins[1] != NULL)
				cubeXNext = &(Voisins[1]->_Cubes[0][y][z]);
			else if (x < CHUNK_SIZE - 1)
				cubeXNext = &(_Cubes[x + 1][y][z]);

			if (y == 0 && Voisins[2] != NULL)
				cubeYPrev = &(Voisins[2]->_Cubes[x][CHUNK_SIZE - 1][z]);
			else if (y > 0)
				cubeYPrev = &(_Cubes[x][y - 1][z]);

			if (y == CHUNK_SIZE - 1 && Voisins[3] != NULL)
				cubeYNext = &(Voisins[3]->_Cubes[x][0][z]);
			else if (y < CHUNK_SIZE - 1)
				cubeYNext = &(_Cubes[x][y + 1][z]);

			if (z == 0 && Voisins[4] != NULL)
				cubeZPrev = &(Voisins[4]->_Cubes[x][y][CHUNK_SIZE - 1]);
			else if (z > 0)
				cubeZPrev = &(_Cubes[x][y][z - 1]);

			if (z == CHUNK_SIZE - 1 && Voisins[5] != NULL)
				cubeZNext = &(Voisins[5]->_Cubes[x][y][0]);
			else if (z < CHUNK_SIZE - 1)
				cubeZNext = &(_Cubes[x][y][z + 1]);
		}

		/**
		  * On verifie si le cube peut être vu
		  */
		bool test_hidden(int x, int y, int z)
		{
			NYCube * cubeXPrev = NULL; 
			NYCube * cubeXNext = NULL; 
			NYCube * cubeYPrev = NULL; 
			NYCube * cubeYNext = NULL; 
			NYCube * cubeZPrev = NULL; 
			NYCube * cubeZNext = NULL;

			NYCube cube = _Cubes[x][y][z];

			get_surrounding_cubes(x, y, z, cubeXPrev, cubeXNext, cubeYPrev, cubeYNext, cubeZPrev, cubeZNext);

			if (cubeXPrev == NULL || cubeXNext == NULL ||
				cubeYPrev == NULL || cubeYNext == NULL ||
				cubeZPrev == NULL || cubeZNext == NULL)
				return false;

			// pour un cube d'eau, cacher si entoure d'eau ou de solide (pas de Beer-Lambert ici, une seule couche de semi-transparence)
			// sinon, cacher si entoure de solide only
			if (cube._Type == CUBE_WATER)
			{
				if ((cubeXPrev->isSolid() == true || cubeZNext->_Type == CUBE_WATER) && //droite
					(cubeXNext->isSolid() == true || cubeXNext->_Type == CUBE_WATER) && //gauche
					(cubeYPrev->isSolid() == true || cubeYPrev->_Type == CUBE_WATER) && //devant
					(cubeYNext->isSolid() == true || cubeYNext->_Type == CUBE_WATER) && //derriere
					(cubeZPrev->isSolid() == true || cubeZPrev->_Type == CUBE_WATER) && //dessous
					(cubeZNext->isSolid() == true || cubeZNext->_Type == CUBE_WATER))
					return true;
			}

			else
				if (cubeXPrev->isSolid() == true && //droite
					cubeXNext->isSolid() == true && //gauche
					cubeYPrev->isSolid() == true && //devant
					cubeYNext->isSolid() == true && //derriere
					cubeZPrev->isSolid() == true && //dessous
					cubeZNext->isSolid() == true)  //dessus
					return true;

			return false;
		}

		void disableHiddenCubes(void)
		{
			for(int x=0;x<CHUNK_SIZE;x++)
				for(int y=0;y<CHUNK_SIZE;y++)
					for(int z=0;z<CHUNK_SIZE;z++)
					{
						_Cubes[x][y][z]._Draw = true;
						if(test_hidden(x,y,z))
							_Cubes[x][y][z]._Draw = false;
					}
		}


};