#include "Game.h"

//Includes application
#include <conio.h>
#include <vector>
#include <functional>
#include <string>
#include <windows.h>

#include "external/gl/glew.h"
#include "external/gl/freeglut.h"

//Moteur
#include "engine/utils/types_3d.h"
#include "engine/timer.h"
#include "engine/log/log_console.h"
#include "engine/render/renderer.h"
//#include "engine/gui/bouton.h"  // added
#include "engine/gui/screen.h"
#include "engine/gui/screen_manager.h"

// RCC++
#include "RuntimeCompiler/AUArray.h"
#include "RuntimeCompiler/BuildTool.h"
#include "RuntimeCompiler/ICompilerLogger.h"
#include "RuntimeCompiler/FileChangeNotifier.h"
#include "RuntimeObjectSystem/IObjectFactorySystem.h"
#include "RuntimeObjectSystem/ObjectFactorySystem/ObjectFactorySystem.h"
#include "RuntimeObjectSystem/RuntimeObjectSystem.h"

#include "StdioLogSystem.h"

#include "RuntimeObjectSystem/IObject.h"
//#include "IUpdateable.h"
//#include "InterfaceIds.h"

// Game
#include "world.h"

Game::~Game()
{
	delete g_screen_jeu;
	delete g_screen_params;
	delete g_screen_manager;
	delete g_world;
	delete g_timer;
}

bool Game::Init(int argc, char* argv[])
{
	/* RCC++ INIT */

	//Initialise the RuntimeObjectSystem
	m_pRuntimeObjectSystem = new RuntimeObjectSystem;
	m_pCompilerLogger = new StdioLogSystem();
	if (!m_pRuntimeObjectSystem->Initialise(m_pCompilerLogger, 0))
	{
		m_pRuntimeObjectSystem = 0;
		return false;
	}
	m_pRuntimeObjectSystem->GetObjectFactorySystem()->AddListener(this);


	// construct first object
	IObjectConstructor* pCtor = m_pRuntimeObjectSystem->GetObjectFactorySystem()->GetConstructor("RuntimeObject01");
	if (pCtor)
	{
		IObject* pObj = pCtor->Construct();
		//		pObj->GetInterface(&m_pUpdateable);
		//		if (0 == m_pUpdateable)
		//		{
		//			delete pObj;
		//			m_pCompilerLogger->LogError("Error - no updateable interface found\n");
		//			return false;
		//		}
		m_ObjectId = pObj->GetObjectId();

	}

	/* END */

	int screen_width = 800;
	int screen_height = 600;

	glutInit(&argc, argv);
	glutInitContextVersion(3, 0);
	glutSetOption(
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
		);

	glutInitWindowSize(screen_width, screen_height);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);

	glEnable(GL_MULTISAMPLE);

	Log::log(Log::ENGINE_INFO, (toString(argc) + " arguments en ligne de commande.").c_str());
	bool gameMode = true;
	for (int i = 0; i<argc; i++)
	{
		if (argv[i][0] == 'w')
		{
			Log::log(Log::ENGINE_INFO, "Arg w mode fenetre.\n");
			gameMode = false;
		}
	}

	if (gameMode)
	{
		int width = glutGet(GLUT_SCREEN_WIDTH);
		int height = glutGet(GLUT_SCREEN_HEIGHT);

		char gameModeStr[200];
		sprintf(gameModeStr, "%dx%d:32@60", width, height);
		glutGameModeString(gameModeStr);
		g_main_window_id = glutEnterGameMode();
	}
	else
	{
		g_main_window_id = glutCreateWindow("MyNecraft");
		glutReshapeWindow(screen_width, screen_height);
	}

	if (g_main_window_id < 1)
	{
		Log::log(Log::ENGINE_ERROR, "Erreur creation de la fenetre.");
		exit(EXIT_FAILURE);
	}

	GLenum glewInitResult = glewInit();

	if (glewInitResult != GLEW_OK)
	{
		Log::log(Log::ENGINE_ERROR, ("Erreur init glew " + std::string((char*)glewGetErrorString(glewInitResult))).c_str());
		_cprintf("ERROR : %s", glewGetErrorString(glewInitResult));
		exit(EXIT_FAILURE);
	}

	//Affichage des capacités du système
	Log::log(Log::ENGINE_INFO, ("OpenGL Version : " + std::string((char*)glGetString(GL_VERSION))).c_str());

//	glutDisplayFunc([]() {game->update(); });
//	auto f = game->update;
//	glutDisplayFunc(game->update);
//	glutDisplayFunc(std::bind(update, game));
//	glutDisplayFunc(Instance().update);
//	glutDisplayFunc(std::bind(&Game::update, Instance()));
//	static auto updateF = [this]() {update(); };
//	glutDisplayFunc([]() {updateF();});
	glutDisplayFunc([](){Instance().update();});
	glutReshapeFunc([](int width, int height) {Instance().resizeFunction(width, height); });
	glutKeyboardFunc([](unsigned char key, int p1, int p2) {Instance().keyboardDownFunction(key, p1, p2); });
	glutKeyboardUpFunc([](unsigned char key, int p1, int p2) {Instance().keyboardUpFunction(key, p1, p2); });
	glutSpecialFunc([](int key, int p1, int p2) {Instance().specialDownFunction(key, p1, p2); });
	glutSpecialUpFunc([](int key, int p1, int p2) {Instance().specialUpFunction(key, p1, p2); });
	glutMouseFunc([](int button, int state, int x, int y) {Instance().mouseFunction(button, state, x, y); });
	glutMotionFunc([](int x, int y) {Instance().mouseMoveActiveFunction(x, y); });
	glutPassiveMotionFunc([](int x, int y) {Instance().mouseMovePassiveFunction(x, y); });
	glutIgnoreKeyRepeat(1);

	//Initialisation du renderer
	g_renderer = NYRenderer::getInstance();
	g_renderer->setRenderObjectFun([]() {Instance().renderObjects(); });
	g_renderer->setRender2DFun([]() {Instance().render2d(); });
	g_renderer->setLightsFun([]() {Instance().setLights(); });
	g_renderer->setBackgroundColor(NYColor());
	g_renderer->initialise(true);  // modified to activate post-process

	//On applique la config du renderer
	glViewport(0, 0, g_renderer->_ScreenWidth, g_renderer->_ScreenHeight);
	g_renderer->resize(g_renderer->_ScreenWidth, g_renderer->_ScreenHeight);

	//Ecran de jeu
	uint16 x = 10;
	uint16 y = 10;
	g_screen_jeu = new GUIScreen();

	g_screen_manager = new GUIScreenManager();

	//Bouton pour afficher les params
	BtnParams = new GUIBouton();
	BtnParams->Titre = std::string("Params");
	BtnParams->X = x;
	BtnParams->setOnClick([](GUIBouton * bouton) {Instance().clickBtnParams(bouton); });
	g_screen_jeu->addElement(BtnParams);

	y += BtnParams->Height + 1;

	LabelFps = new GUILabel();
	LabelFps->Text = "FPS";
	LabelFps->X = x;
	LabelFps->Y = y;
	LabelFps->Visible = true;
	g_screen_jeu->addElement(LabelFps);

	//Ecran de parametrage
	x = 10;
	y = 10;
	g_screen_params = new GUIScreen();

	GUIBouton * btnClose = new GUIBouton();
	btnClose->Titre = std::string("Close");
	btnClose->X = x;
	btnClose->setOnClick([](GUIBouton * bouton) {Instance().clickBtnCloseParam(bouton); });
	g_screen_params->addElement(btnClose);

	y += btnClose->Height + 1;
	y += 10;
	x += 10;

	// slider for camera X rotate
	g_slider_cameraXRotateMouseRatio = GUISlider::CreateGUISlider(0, 1, -cameraXRotateMouseRatio);
	g_screen_params->addLabeledElement(x, y, "Camera X rotate: ", g_slider_cameraXRotateMouseRatio);

	// slider for camera Y rotate
	g_slider_cameraYRotateMouseRatio = GUISlider::CreateGUISlider(0, 1, -cameraYRotateMouseRatio);
	g_screen_params->addLabeledElement(x, y, "Camera Y rotate: ", g_slider_cameraYRotateMouseRatio);

	// slider for camera Z by wheel
	g_slider_cameraZoomWheelRatio = GUISlider::CreateGUISlider(0, 20, cameraZoomWheelRatio);
	g_screen_params->addLabeledElement(x, y, "Camera zoom speed: ", g_slider_cameraZoomWheelRatio);

	// slider for camera XY plane motion
	g_slider_cameraXYMotionMouseRatio = GUISlider::CreateGUISlider(0, 1, cameraXYMotionMouseRatio);
	g_screen_params->addLabeledElement(x, y, "Camera XY motion: ", g_slider_cameraXYMotionMouseRatio);

	// slider for camera XY plane motion
	g_slider_cameraXZMotionMouseRatio = GUISlider::CreateGUISlider(0, 1, cameraXZMotionMouseRatio);
	g_screen_params->addLabeledElement(x, y, "Camera XZ motion: ", g_slider_cameraXZMotionMouseRatio);

	// slider for camera WASD local
	g_slider_cameraKeyboardMotionSpeed = GUISlider::CreateGUISlider(50, 200, cameraKeyboardMotionSpeed);
	g_screen_params->addLabeledElement(x, y, "Camera WASD motion: ", g_slider_cameraKeyboardMotionSpeed);
	
	// slider for ambient light intensity
	g_slider_ambient = GUISlider::CreateGUISlider(0, 1, g_renderer->_Ambient);
	g_screen_params->addLabeledElement(x, y, "Ambient: ", g_slider_ambient);
	
	// slider for water wave amplitude
	g_slider_wave_amplitude = GUISlider::CreateGUISlider(0, 10, g_renderer->_WaveAmplitude);
	g_screen_params->addLabeledElement(x, y, "Water wave amplitude: ", g_slider_wave_amplitude);
	
	// slider for Normalized water wave length:
	g_slider_normalized_wavelength = GUISlider::CreateGUISlider(1, 20, g_renderer->_NormalizedWaveLength);
	g_screen_params->addLabeledElement(x, y, "Normalized water wave length: ", g_slider_normalized_wavelength);
	

	//Ecran a rendre
	g_screen_manager->setActiveScreen(g_screen_jeu);

	//Init Camera
	g_renderer->_Camera->setPosition(NYVert3Df(0, 0, 240));
	g_renderer->_Camera->setLookAt(NYVert3Df(1, 1, 320));

	//Fin init moteur

	//Init application

	// generate world
	g_world = new NYWorld();
	g_world->init_world(600);

	//Init Timer
	g_timer = new NYTimer();

	//On start
	g_timer->start();

	glutMainLoop();

	return true;
}

void Game::Clean()
{
	if (m_pRuntimeObjectSystem)
	{
		// clean temp object files
		m_pRuntimeObjectSystem->CleanObjectFiles();
	}

	if (m_pRuntimeObjectSystem && m_pRuntimeObjectSystem->GetObjectFactorySystem())
	{
		m_pRuntimeObjectSystem->GetObjectFactorySystem()->RemoveListener(this);

		// delete object via correct interface
//		IObject* pObj = m_pRuntimeObjectSystem->GetObjectFactorySystem()->GetObject(m_ObjectId);
//		delete pObj;
	}

	delete m_pRuntimeObjectSystem;
	delete m_pCompilerLogger;
}

void Game::OnConstructorsAdded()
{
	// This could have resulted in a change of object pointer, so release old and get new one.
//	if (m_pUpdateable)
//	{
//		IObject* pObj = m_pRuntimeObjectSystem->GetObjectFactorySystem()->GetObject(m_ObjectId);
//		pObj->GetInterface(&m_pUpdateable);
//		if (0 == m_pUpdateable)
//		{
//			delete pObj;
//			m_pCompilerLogger->LogError("Error - no updateable interface found\n");
//		}
//	}
}

//////////////////////////////////////////////////////////////////////////
// GESTION APPLICATION
//////////////////////////////////////////////////////////////////////////
void Game::update(void)
{
	// RCC++
	// Time in userspace, ignoring frametime and whether we are paused, compiling, etc.
	// That seems most appropriate to the filechangenotifier
//	float m_GameSpeed = 1.f;
	// don't call getElapsed twice, it will mess up fps!
//	float fSessionTimeDelta = g_timer->getElapsedSeconds(true);
//	float fClampedDelta = min(fSessionTimeDelta * m_GameSpeed, 0.1f); // used for IObject updates

//	m_pRuntimeObjectSystem->GetFileChangeNotifier()->Update(fSessionTimeDelta);

//	if (m_pEnv->sys->pRuntimeObjectSystem->GetIsCompiling() && m_CompileStartedTime == 0.0)
//	{
//		m_CompileStartedTime = pTimeSystem->GetSessionTimeNow();
//	}

	//check status of any compile
	bool bLoadModule = false;
	if (m_pRuntimeObjectSystem->GetIsCompiledComplete())
	{
		bLoadModule = true; //we load module after update/display, to get notification on screen correct
	}

//	pTimeSystem->StartFrame();

//	AUDynArray<AUEntityId> entities;
//	m_EntityUpdateProtector.pEntitySystem->GetAll(m_EntityUpdateProtector.entities);
//	m_EntityUpdateProtector.fDeltaTime = fClampedDelta;

	// EXCEPTIONS
//	if (!m_pRuntimeObjectSystem->TryProtectedFunction(&m_EntityUpdateProtector))
//	{
//		m_pCompilerLogger->LogError("Have caught an exception in main entity Update loop, code will not be run until new compile - please fix.\n");
//	}

	internalUpdate();

	if (bLoadModule)
	{
		// load module when compile complete, and notify console - TODO replace with event system 
		bool bSuccess = m_pRuntimeObjectSystem->LoadCompiledModule();
//		m_pConsole->OnCompileDone(bSuccess);
		if (bSuccess)
		{
//			float compileAndLoadTime = (float)(pTimeSystem->GetSessionTimeNow() - m_CompileStartedTime);
//			m_pCompilerLogger->LogInfo("Compile and Module Reload Time: %.1f s\n", compileAndLoadTime);
			m_pCompilerLogger->LogInfo("Compile and Module Reload\n");

		}
//		m_CompileStartedTime = 0.0;
	}
}

void Game::internalUpdate(void)
{
	// RCC++
	//check status of any compile
	if (m_pRuntimeObjectSystem->GetIsCompiledComplete())
	{
		// load module when compile complete
		m_pRuntimeObjectSystem->LoadCompiledModule();
	}

	if (!m_pRuntimeObjectSystem->GetIsCompiling())
	{

		float elapsed = g_timer->getElapsedSeconds(true);

		static float g_eval_elapsed = 0;

		//Calcul des fps
		g_elapsed_fps += elapsed;
		g_nb_frames++;
		if (g_elapsed_fps > 1.0)
		{
			LabelFps->Text = std::string("FPS : ") + toString(g_nb_frames);
			g_elapsed_fps -= 1.0f;
			g_nb_frames = 0;
		}

		if (mouseLeftButtonPressed || mouseMiddleButtonPressed || mouseRightButtonPressed)
		{
			float forwardMotionInput = 0;
			float horizontalMotionInput = 0;
			float verticalMotionInput = 0;
			if (moveForwardKeyPressed) forwardMotionInput = 1.f;
			else if (moveBackwardKeyPressed) forwardMotionInput = -1.f;
			if (moveLeftKeyPressed) horizontalMotionInput = -1.f;
			else if (moveRightKeyPressed) horizontalMotionInput = 1.f;
			if (moveUpwardKeyPressed) verticalMotionInput = 1.f;
			else if (moveDownwardKeyPressed) verticalMotionInput = -1.f;
			// I tested and prefer forward/backward motion projected on XY rather than camera _Direction
			NYVert3Df motionInput = g_renderer->_Camera->_ProjectedForward * forwardMotionInput + g_renderer->_Camera->_NormVec * horizontalMotionInput + g_renderer->_Camera->_UpRef * verticalMotionInput;
			g_renderer->_Camera->move(motionInput * elapsed * cameraKeyboardMotionSpeed);
		}

		//Rendu
		g_renderer->render(elapsed);

		//		const float deltaTime = 1.0f;
		//		m_pRuntimeObjectSystem->GetFileChangeNotifier()->Update(deltaTime);
		//		m_pUpdateable->Update(deltaTime);
		//		Sleep(1000);

	}
}


void Game::render2d(void)
{
	g_screen_manager->render();
}

void Game::renderObjects(void)
{
	//Rendu des axes (temporarily activate color material for unrealistic colors)
	glEnable(GL_COLOR_MATERIAL);
	glBegin(GL_LINES);
	glColor3d(1, 0, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(10000, 0, 0);
	glColor3d(0, 1, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 10000, 0);
	glColor3d(0, 0, 1);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, 10000);
	glEnd();
	glDisable(GL_COLOR_MATERIAL);

	// use loaded shader program
	glUseProgram(g_renderer->_ProgramCube);

	GLuint elap = glGetUniformLocation(g_renderer->_ProgramCube, "elapsed");
	glUniform1f(elap, NYRenderer::_DeltaTimeCumul);

	GLuint amb = glGetUniformLocation(g_renderer->_ProgramCube, "ambientLevel");
	glUniform1f(amb, g_renderer->_Ambient);

	GLuint invView = glGetUniformLocation(g_renderer->_ProgramCube, "invertView");
	glUniformMatrix4fv(invView, 1, true, g_renderer->_Camera->_InvertViewMatrix.Mat.t);

	GLuint wave_amplitude_loc = glGetUniformLocation(g_renderer->_ProgramCube, "wave_amplitude");
	GLuint normalized_wavelength_loc = glGetUniformLocation(g_renderer->_ProgramCube, "normalized_wavelength");
	GLuint wave_period_loc = glGetUniformLocation(g_renderer->_ProgramCube, "wave_period");
	glUniform1f(wave_amplitude_loc, g_renderer->_WaveAmplitude);
	glUniform1f(normalized_wavelength_loc, g_renderer->_NormalizedWaveLength);
	glUniform1f(wave_period_loc, g_renderer->_WavePeriod);

	glPushMatrix();
	//　g_world->render_world_old_school();
	g_world->render_world_vbo();
	glPopMatrix();

	glUseProgram(0);

}

void Game::setLights(void)
{
	//glClearDepth(1);
	//glEnable(GL_DEPTH_TEST);

	glEnable(GL_LIGHTING);
	//Log::log(Log::USER_INFO, ("_DeltaTimeCumul: " + toString(g_renderer->_DeltaTimeCumul)).c_str());

	// Skybox (blending between orange and blue based on time, period of 24s)
	NYColor orange = NYColor{ 255.f / 255.f, 193.f / 255.f, 93.f / 255.f, 1 };
	NYColor blue = NYColor{ 62.f / 255.f, 199.f / 255.f, 255.f / 255.f, 1 };
	float dayTime = fmod(g_renderer->_DeltaTimeCumul, 24.f);  // from 0. to 24.
															  // blue at noon, then back to orange
	float alpha;
	if (dayTime <= 12.f)
	{
		alpha = dayTime / 12.f;
	}
	else
	{
		alpha = (24.f - dayTime) / 12.f;
	}
	NYColor skyColor = orange.interpolate(blue, alpha);
	g_renderer->setBackgroundColor(skyColor);

	//On active la light 0
	glEnable(GL_LIGHT0);

	//On définit une lumière directionelle (un soleil)
	float direction[4] = { 0,0,1,0 }; ///w = 0 donc elle est a l'infini
	glLightfv(GL_LIGHT0, GL_POSITION, direction);  // la direction est en fait l'oppose de ce qu'on a passe
												   // La lumière dépend du daytime, similaire à skybox
	GLfloat whiteColor[4] = { 0.3f,0.3f,0.3f };
	GLfloat sunColor[4] = { skyColor.R, skyColor.V, skyColor.B };
	glLightfv(GL_LIGHT0, GL_AMBIENT, whiteColor);
	GLfloat color2[4] = { 0.3f,0.3f,0.3f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, sunColor);
	GLfloat color3[4] = { 0.3f,0.3f,0.3f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, whiteColor);

	//On active la light 1
	glEnable(GL_LIGHT1);

	//On définit une point light
	float position[4] = { 2,2,2,1 };
	glLightfv(GL_LIGHT1, GL_POSITION, position);
	float ambient[4] = { 0.3f,0.3f,0.3f };
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	float diffuse[4] = { 0.5f,0.5f,0.5f };
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	float specular[4] = { 0.5f,0.5f,0.5f };
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
}

void Game::resizeFunction(int width, int height)
{
	glViewport(0, 0, width, height);
	g_renderer->resize(width, height);
}

//////////////////////////////////////////////////////////////////////////
// GESTION CLAVIER SOURIS
//////////////////////////////////////////////////////////////////////////

void Game::specialDownFunction(int key, int p1, int p2)
{
	Log::log(Log::USER_INFO, ("keyboardDownFunction key: " + toString(key) + ", p1: " + toString(p1) + ", p2: " + toString(p2)).c_str());

	//On change de mode de camera
	if (key == GLUT_KEY_LEFT)
	{
	}

	if (key == GLUT_KEY_CTRL_L)
	{
		// note that this won't work if the application starts while left ctrl is pressed
		// so you'll need to release ctrl and press it again
		leftCtrlPressed = true;
	}

}

void Game::specialUpFunction(int key, int p1, int p2)
{
	Log::log(Log::USER_INFO, ("specialUpFunction key: " + toString(key) + ", p1: " + toString(p1) + ", p2: " + toString(p2)).c_str());

	if (key == GLUT_KEY_CTRL_L)
	{
		leftCtrlPressed = false;
	}
}

void Game::keyboardDownFunction(unsigned char key, int p1, int p2)
{
	if (key == VK_ESCAPE)
	{
		glutDestroyWindow(g_main_window_id);
		exit(0);
	}

	if (key == 'f')
	{
		if (!g_fullscreen) {
			glutFullScreen();
			g_fullscreen = true;
		}
		else if (g_fullscreen) {
			glutLeaveGameMode();
			glutLeaveFullScreen();
			glutReshapeWindow(g_renderer->_ScreenWidth, g_renderer->_ScreenWidth);
			glutPositionWindow(0, 0);
			g_fullscreen = false;
		}
	}

	if (key == 'w') moveForwardKeyPressed = true;
	if (key == 's')	moveBackwardKeyPressed = true;
	if (key == 'a')	moveLeftKeyPressed = true;
	if (key == 'd')	moveRightKeyPressed = true;
	if (key == 'q')	moveDownwardKeyPressed = true;
	if (key == 'e')	moveUpwardKeyPressed = true;
}

void Game::keyboardUpFunction(unsigned char key, int p1, int p2)
{
	if (key == 'w') moveForwardKeyPressed = false;
	if (key == 's')	moveBackwardKeyPressed = false;
	if (key == 'a')	moveLeftKeyPressed = false;
	if (key == 'd')	moveRightKeyPressed = false;
	if (key == 'q')	moveDownwardKeyPressed = false;
	if (key == 'e')	moveUpwardKeyPressed = false;
}

void Game::mouseWheelFunction(int wheel, int dir, int x, int y)
{
	//Log::log(Log::USER_INFO, ("mouseWheelFunction wheel: " + toString(wheel) + ", dir: " + toString(dir) + ", x: " + toString(x) + ", y: " + toString(y)).c_str());

	// move forward / backward
	g_renderer->_Camera->move(g_renderer->_Camera->_Direction * dir * cameraZoomWheelRatio);
}

void Game::mouseFunction(int button, int state, int x, int y)
{
	//Log::log(Log::USER_INFO, ("mouseFunction button: " + toString(button) + ", state: " + toString(state) + ", x: " + toString(x) + ", y: " + toString(y)).c_str());

	//Gestion de la roulette de la souris
	if ((button & 0x07) == 3 && state)
		mouseWheelFunction(button, 1, x, y);
	if ((button & 0x07) == 4 && state)
		mouseWheelFunction(button, -1, x, y);

	// on mouse button press, record press and initialize mouse position to get relative motion later
	if (state == GLUT_DOWN) {
		switch (button)
		{
		case GLUT_LEFT_BUTTON:
			Log::log(Log::USER_INFO, "Pressed left mouse button");
			mouseLeftButtonPressed = true;
			lastMouseX = x;
			lastMouseY = y;
			break;
		case GLUT_MIDDLE_BUTTON:
			Log::log(Log::USER_INFO, "Pressed middle mouse button");
			mouseMiddleButtonPressed = true;
			lastMouseX = x;
			lastMouseY = y;
			break;
		case GLUT_RIGHT_BUTTON:
			Log::log(Log::USER_INFO, "Pressed right mouse button");
			mouseRightButtonPressed = true;
			lastMouseX = x;
			lastMouseY = y;
			break;
		}
	}
	else if (state == GLUT_UP)  // or else
	{
		switch (button)
		{
		case GLUT_LEFT_BUTTON:
			Log::log(Log::USER_INFO, "Released left mouse button");
			mouseLeftButtonPressed = false;
			break;
		case GLUT_MIDDLE_BUTTON:
			Log::log(Log::USER_INFO, "Released middle mouse button");
			mouseMiddleButtonPressed = false;
			break;
		case GLUT_RIGHT_BUTTON:
			Log::log(Log::USER_INFO, "Released right mouse button");
			mouseRightButtonPressed = false;
			break;
		}
	}

	//GUI
	g_mouse_btn_gui_state = 0;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		g_mouse_btn_gui_state |= GUI_MLBUTTON;

	bool mouseTraite = false;
	mouseTraite = g_screen_manager->mouseCallback(x, y, g_mouse_btn_gui_state, 0, 0);
}

void Game::mouseMoveFunction(int x, int y, bool pressed)
{
	//Log::log(Log::USER_INFO, ("mouseMoveFunction to: " + toString(x) + ", " + toString(y) + ", pressed: " + toString(pressed)).c_str());

	bool mouseTraite = false;

	mouseTraite = g_screen_manager->mouseCallback(x, y, g_mouse_btn_gui_state, 0, 0);

	if (!mouseTraite && (mouseLeftButtonPressed || mouseMiddleButtonPressed || mouseRightButtonPressed))
	{
		int deltaMouseX = x - lastMouseX;
		int deltaMouseY = y - lastMouseY;
		//Log::log(Log::USER_INFO, ("deltaMouse: " + toString(deltaMouseX) + ", " + toString(deltaMouseY)).c_str());

		lastMouseX = x;
		lastMouseY = y;

		// if left mouse button is pressed, translate forward and rotate around Z (yaw)
		// a) translate forward following camera target
		// b) translate on forward projected on world XY plane (crossProd(_UpRef, _NormVec))
		// here: a)
		if (mouseLeftButtonPressed)
		{
			// coords are top-left so for negative delta Y, move UP!
			g_renderer->_Camera->rotate(deltaMouseX * cameraXRotateMouseRatio);
			g_renderer->_Camera->move(g_renderer->_Camera->_Direction * (-deltaMouseY) * cameraXYMotionMouseRatio);
		}

		// translate in camera XZ with middle button hold
		if (mouseMiddleButtonPressed)
		{
			// update camera view on passive (no click) mouse motion
			g_renderer->_Camera->move((g_renderer->_Camera->_NormVec * deltaMouseX + g_renderer->_Camera->_UpRef * (-deltaMouseY)) * cameraXZMotionMouseRatio);
		}

		// rotate head with right-hold mouse move
		if (mouseRightButtonPressed)
		{
			// update camera view on passive (no click) mouse motion
			g_renderer->_Camera->rotate(deltaMouseX * cameraXRotateMouseRatio);
			g_renderer->_Camera->rotateUp(deltaMouseY * cameraYRotateMouseRatio);
		}
	}

	if (pressed && mouseTraite)
	{
		Log::log(Log::USER_INFO, ("slider value update to: " + toString(g_slider_cameraXRotateMouseRatio->Value) + ", " + toString(g_slider_cameraYRotateMouseRatio->Value) + ", Z: " + toString(g_slider_cameraZoomWheelRatio->Value)).c_str());
		//Mise a jour des variables liées aux sliders
		// oppose for negative values
		cameraXRotateMouseRatio = -g_slider_cameraXRotateMouseRatio->Value;
		cameraYRotateMouseRatio = -g_slider_cameraYRotateMouseRatio->Value;
		cameraZoomWheelRatio = g_slider_cameraZoomWheelRatio->Value;
		cameraXYMotionMouseRatio = g_slider_cameraXYMotionMouseRatio->Value;
		g_renderer->_Ambient = g_slider_ambient->Value;
		g_renderer->_WaveAmplitude = g_slider_wave_amplitude->Value;
		g_renderer->_NormalizedWaveLength = g_slider_normalized_wavelength->Value;
	}

}

void Game::mouseMoveActiveFunction(int x, int y)
{
	mouseMoveFunction(x, y, true);
}
void Game::mouseMovePassiveFunction(int x, int y)
{
	mouseMoveFunction(x, y, false);
}


void Game::clickBtnParams(GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_params);
}

void Game::clickBtnCloseParam(GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_jeu);
}
