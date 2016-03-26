#pragma once

//Includes application
#include <conio.h>
#include <vector>
#include <string>
#include <windows.h>

#include "external/gl/glew.h"
#include "external/gl/freeglut.h"

//Moteur
#include "engine/utils/types_3d.h"
#include "engine/timer.h"
#include "engine/log/log_console.h"
#include "engine/render/renderer.h"
#include "engine/gui/screen.h"
#include "engine/gui/screen_manager.h"

// RCC++
#include "RuntimeObjectSystem/IObjectFactorySystem.h"
#include "RuntimeObjectSystem/ObjectInterface.h"
#include "RuntimeCompiler/AUArray.h"

// Game
class NYWorld;

// Singleton Game
class Game : public IObjectFactoryListener
{
public:
	virtual ~Game();

	static Game& Instance()
	{
		static Game instance;
		return instance;
	}

	bool Init(int argc, char* argv[]);
	void Clean();

	// IObjectFactoryListener

	virtual void OnConstructorsAdded() override;
	// ~IObjectFactoryListener

	// Game global variables
	NYWorld * g_world = nullptr;
	NYRenderer * g_renderer = NULL;
	NYTimer * g_timer = NULL;

private:
	Game() {};

	void update();
	void internalUpdate();
	void render2d();
	void renderObjects();
	void setLights();
	void resizeFunction(int width, int height);
	void specialDownFunction(int key, int p1, int p2);
	void specialUpFunction(int key, int p1, int p2);
	void keyboardDownFunction(unsigned char key, int p1, int p2);
	void keyboardUpFunction(unsigned char key, int p1, int p2);
	void mouseWheelFunction(int wheel, int dir, int x, int y);
	void mouseFunction(int button, int state, int x, int y);
	void mouseMoveFunction(int x, int y, bool pressed);
	void mouseMoveActiveFunction(int x, int y);
	void mouseMovePassiveFunction(int x, int y);
	void clickBtnParams(GUIBouton* bouton);
	void clickBtnCloseParam(GUIBouton* bouton);

	/* RCC++ */
	// Runtime Systems
	ICompilerLogger*		m_pCompilerLogger = nullptr;
	IRuntimeObjectSystem*	m_pRuntimeObjectSystem = nullptr;

	// Runtime object
//	IUpdateable* 			m_pUpdateable = nullptr;
	ObjectId	   			m_ObjectId;
	/*END*/

	// params
	float cameraXRotateMouseRatio = -0.5f; // negative to rotate CW on the right
	float cameraYRotateMouseRatio = -0.4f; // inverted axis
	float cameraZoomWheelRatio = 10.f; // wheel up to zoom
	float cameraXYMotionMouseRatio = 0.5f; // "speed" of XY motion with ctrl + mouse move
	float cameraXZMotionMouseRatio = 0.5f; // "speed" of XZ motion with middle button mouse pan (probably = XY ratio)
	float cameraKeyboardMotionSpeed = 100.f;

	//GUI
	GUIScreenManager * g_screen_manager = nullptr;
	GUIBouton * BtnParams = nullptr;
	GUIBouton * BtnClose = nullptr;
	GUILabel * LabelFps = nullptr;
	GUILabel * LabelCam = nullptr;
	GUIScreen * g_screen_params = nullptr;
	GUIScreen * g_screen_jeu = nullptr;

	GUISlider * g_slider_cameraXRotateMouseRatio = nullptr;
	GUISlider * g_slider_cameraYRotateMouseRatio = nullptr;
	GUISlider * g_slider_cameraZoomWheelRatio = nullptr;
	GUISlider * g_slider_cameraXYMotionMouseRatio = nullptr;
	GUISlider * g_slider_cameraXZMotionMouseRatio = nullptr;
	GUISlider * g_slider_cameraKeyboardMotionSpeed = nullptr;

	GUISlider * g_slider_ambient;
	GUISlider * g_slider_diffuse;
	GUISlider * g_slider_specular;

	GUISlider * g_slider_wave_amplitude;
	GUISlider * g_slider_normalized_wavelength;
	GUISlider * g_slider_wave_period;
	
		// state vars
	
	int g_nb_frames = 0;
	float g_elapsed_fps = 0;
	int g_main_window_id;
	int g_mouse_btn_gui_state = 0;
	bool g_fullscreen = false;

	// added last mouse coordinates (simulate start at center, but will detect mouse arriving from outside)
	int lastMouseX = 400;
	int lastMouseY = 300;
	// added ctrl state (true if left ctrl is pressed)
	bool leftCtrlPressed = false;
	bool mouseLeftButtonPressed = false;
	bool mouseMiddleButtonPressed = false;
	bool mouseRightButtonPressed = false;
	// keyboard motion input
	bool moveForwardKeyPressed = false;
	bool moveBackwardKeyPressed = false;
	bool moveLeftKeyPressed = false;
	bool moveRightKeyPressed = false;
	bool moveUpwardKeyPressed = false;
	bool moveDownwardKeyPressed = false;

};

