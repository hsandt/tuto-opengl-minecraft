#ifndef __SIMPLE_CAM_H__
#define __SIMPLE_CAM_H__

#include "external/gl/glew.h"
#include "external/gl/freeglut.h" 
#include "engine/utils/types_3d.h"
#include "engine/utils/ny_utils.h"

// added
#include "engine/log/log_console.h"

class NYCamera
{
	public:
		NYVert3Df _Position; ///< Position de la camera
		NYVert3Df _LookAt; ///< Point regarde par la camera
		NYVert3Df _Direction; ///< Direction de la camera
		NYVert3Df _UpVec; ///< Vecteur up de la camera
		NYVert3Df _NormVec; ///< Si on se place dans la camera, indique la droite	
		NYVert3Df _UpRef; ///< Ce qu'on considère comme le "haut" dans notre monde (et pas le up de la cam)
		NYVert3Df _ProjectedForward; ///< ADDED to ease camera motion: _Direction projected on XY plane

		NYCamera()
		{
			_Position = NYVert3Df(0,-1,0);
			_LookAt = NYVert3Df(0,0,0);
			_UpRef = NYVert3Df(0,0,1);
			_UpVec = _UpRef;
			updateVecs();
		}

		/**
		  * Mise a jour de la camera                          
		  */
		virtual void update(float elapsed)
		{
		
		}

		/**
		  * Definition du point regarde
		  */
		void setLookAt(NYVert3Df lookat)
		{
			_LookAt = lookat;
			updateVecs();
		}

		/**
		  * Definition de la position de la camera
		  */
		void setPosition(NYVert3Df pos)
		{
			_Position = pos;
			updateVecs();
		}

		/**
		  * Definition du haut de notre monde
		  */
		void setUpRef(NYVert3Df upRef)
		{
			_UpRef = upRef;
			updateVecs();
		}

		/**
		  * Deplacement de la camera d'un delta donné, lookAt suit
		  */
		void move(NYVert3Df delta)
		{
			_Position += delta;
			_LookAt += delta;
			updateVecs();
		}

		/**
		  * Deplacement de la camera vers position absolue, lookAt suit
		  */
		void moveTo(NYVert3Df & target)
		{
			this->move(target-_Position);
		}

		/**
		  * On recalcule les vecteurs utiles au déplacement de la camera (_Direction, _NormVec, _UpVec)
		  * on part du principe que sont connus :
		  * - la position de la camera
		  * - le point regarde par la camera
		  * - la vecteur up de notre monde
		  */
		void updateVecs(void)
		{
			// if camera not looking at itself, should be okay
			_Direction = (_LookAt - _Position).normalize();
			// if camera not looking upward or downward, no problem
			_NormVec = _Direction.vecProd(_UpRef).normalize();
			_UpVec = _NormVec.vecProd(_Direction);
			// compute projected forward either by projecting _Direction on XY or getting crossprod between world Up and camera Right
			_ProjectedForward = _UpRef.vecProd(_NormVec);
		}

		/**
		  * Rotation droite gauche en subjectif (angle relatif en deg)
		  */
		void rotate(float angle)
		{
			NYVert3Df toLookAt = _LookAt - _Position;  // basically _Direction, but not normalized
			toLookAt.rotate(NYVert3Df(0, 0, 1), M_PI / 180 * angle);
			_LookAt = _Position + toLookAt;
			updateVecs();
		}

		/**
		  * Rotation haut bas en subjectif (angle relatif en deg)
		  */
		void rotateUp(float angle)
		{	
			//Log::log(Log::USER_INFO, ("_Direction.Z: " + toString(_Direction.Z)).c_str());
			float pitch = asin(_Direction.Z);  // Z is VERTICAL!!
			//Log::log(Log::USER_INFO, ("current pitch in deg: " + toString(180 / M_PI * pitch)).c_str());
			float newPitch = pitch;
			newPitch += M_PI / 180 * angle;
			// clamp
			float lbound = - M_PI_2 + M_PI / 180 * 30.f;
			float ubound = M_PI_2 - M_PI / 180 * 20.f;
			if (newPitch > ubound) newPitch = ubound;
			else if (newPitch < lbound) newPitch = lbound;
			//Log::log(Log::USER_INFO, ("newPitch in deg: " + toString(180 / M_PI * newPitch)).c_str());

			NYVert3Df toLookAt = _Direction;
			toLookAt.rotate(_NormVec, newPitch - pitch);
			//Log::log(Log::USER_INFO, ("toLookAt: " + toString(toLookAt.Y)).c_str());
			// optional denormalization
			//toLookAt *= (_LookAt - _Position).getSize();
			_LookAt = _Position + toLookAt;
			updateVecs();
			//Log::log(Log::USER_INFO, ("new _NormVec.x in deg: " + toString(_NormVec.X)).c_str());
		}

		/**
		  * Rotation droite gauche en troisième personne
		  */
		void rotateAround(float angle)
		{

		}

		/**
		  * Rotation haut bas en troisième personne
		  */
		void rotateUpAround(float angle)
		{		

		}
	
		/**
		  * Calcul du bon repère de départ pour la matrice monde 
		  */
		void look(void)
		{
			gluLookAt(_Position.X, _Position.Y,_Position.Z,_LookAt.X,_LookAt.Y,_LookAt.Z,_UpVec.X,_UpVec.Y,_UpVec.Z);
		}
};




#endif