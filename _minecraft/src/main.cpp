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

// Game
#include "world.h"

// GLOBAL
// start random engine with seed
std::mt19937 random_engine(100);

// params
float cameraXRotateMouseRatio = -0.5f; // negative to rotate CW on the right
float cameraYRotateMouseRatio = -0.4f; // inverted axis
float cameraZMotionWheelRatio = 0.4f; // wheel up to rise
float cameraXYMotionMouseRatio = 0.5f; // "speed" of XY motion with ctrl + mouse move

NYRenderer * g_renderer = NULL;
NYTimer * g_timer = NULL;
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

//GUI 
GUIScreenManager * g_screen_manager = NULL;
GUIBouton * BtnParams = NULL;
GUIBouton * BtnClose = NULL;
GUILabel * LabelFps = NULL;
GUILabel * LabelCam = NULL;
GUIScreen * g_screen_params = NULL;
GUIScreen * g_screen_jeu = NULL;
GUISlider * g_slider_cameraXRotateMouseRatio;
GUISlider * g_slider_cameraYRotateMouseRatio;
GUISlider * g_slider_cameraZMotionWheelRatio;
GUISlider * g_slider_cameraXYMotionMouseRatio;

// Game global variables
NYWorld * g_world;

//////////////////////////////////////////////////////////////////////////
// GESTION APPLICATION
//////////////////////////////////////////////////////////////////////////
void update(void)
{
	float elapsed = g_timer->getElapsedSeconds(true);

	static float g_eval_elapsed = 0;

	//Calcul des fps
	g_elapsed_fps += elapsed;
	g_nb_frames++;
	if(g_elapsed_fps > 1.0)
	{
		LabelFps->Text = std::string("FPS : ") + toString(g_nb_frames);
		g_elapsed_fps -= 1.0f;
		g_nb_frames = 0;
	}

	//Rendu
	g_renderer->render(elapsed);
}


void render2d(void)
{
	g_screen_manager->render();
}

void renderObjects(void)
{
	//Rendu des axes (temporarily activate color material for unrealistic colors)
	glEnable(GL_COLOR_MATERIAL);
	glBegin(GL_LINES);
	glColor3d(1,0,0);
	glVertex3d(0,0,0);
	glVertex3d(10000,0,0);
	glColor3d(0,1,0);
	glVertex3d(0,0,0);
	glVertex3d(0,10000,0);
	glColor3d(0,0,1);
	glVertex3d(0,0,0);
	glVertex3d(0,0,10000);
	glEnd();
	glDisable(GL_COLOR_MATERIAL);

	glPushMatrix();
	g_world->render_world_old_school();
	glPopMatrix();

	/*

	// DEBUG: rotation dans le temps pour verifier que toutes les faces sont OK
	// move GUI slider to change rotation speed
	//glRotatef(NYRenderer::_DeltaTimeCumul * 100, g_slider -> Value * 10.0f, 1, cos(NYRenderer::_DeltaTimeCumul));
	glRotatef(NYRenderer::_DeltaTimeCumul * 100, 0, 0, 1);

	// activate to debug back face
	//glDisable(GL_CULL_FACE);

	// a
	//glRotatef(45, 0, 0, 1);
	//glTranslatef(1.f, 0.f, 0.5f);

	// b
	//glTranslatef(1.f, 1.f, 0.5f);
	//glRotatef(45, 0, 0, 1);

	glBegin(GL_QUADS);

	// face 1
	
	// define materials (reddish)
	// ambient
	GLfloat redMaterialAmbient[] = { 0.8, 0, 0, 1.0 };
	GLfloat greenMaterialAmbient[] = { 0, 0.8, 0, 1.0 };
	GLfloat blueMaterialAmbient[] = { 0, 0, 0.8, 1.0 };

	// diffuse
	GLfloat redMaterialDiffuse[] = { 0.7, 0, 0, 1.0 };
	GLfloat greenMaterialDiffuse[] = { 0, 0.7, 0, 1.0 };
	GLfloat blueMaterialDiffuse[] = {0, 0, 0.7, 1.0};
	
	// specular
	GLfloat whiteMaterialSpecular[] = { 0.3, 0.3, 0.3, 1.0 };
	GLfloat materialShininess = 100;

	// color (overriden by lighting)
	//glColor3d(0.5f, 0, 0);
	glMaterialfv(GL_FRONT, GL_AMBIENT, redMaterialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, redMaterialDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterialSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

	// face and normal
	glNormal3d(1, 0, 0);
	glVertex3d(1, 1, 1);
	glVertex3d(1, -1, 1);
	glVertex3d(1, -1, -1);
	glVertex3d(1, 1, -1);

	// face 2
	glMaterialfv(GL_FRONT, GL_AMBIENT, greenMaterialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, greenMaterialDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterialSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

	//glColor3d(0, 0.5f, 0);
	glNormal3f(0, 1, 0);
	glVertex3d(-1, 1, 1);
	glVertex3d(1, 1, 1);
	glVertex3d(1, 1, -1);
	glVertex3d(-1, 1, -1);

	// face 3
	//glColor3d(0, 0, 0.5f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, blueMaterialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, blueMaterialDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterialSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

	glNormal3f(0, 0, 1);
	glVertex3d(-1, -1, 1);
	glVertex3d(1, -1, 1);
	glVertex3d(1, 1, 1);
	glVertex3d(-1, 1, 1);

	// face 4
	//glColor3d(0.5f, 0, 0);
	glMaterialfv(GL_FRONT, GL_AMBIENT, redMaterialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, redMaterialDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterialSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

	glNormal3f(-1, 0, 0);
	glVertex3d(-1, -1, -1);
	glVertex3d(-1, -1, 1);
	glVertex3d(-1, 1, 1);
	glVertex3d(-1, 1, -1);

	// face 5
	//glColor3d(0, 0.5f, 0);
	glMaterialfv(GL_FRONT, GL_AMBIENT, greenMaterialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, greenMaterialDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterialSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

	glNormal3f(0, -1, 0);
	glVertex3d(-1, -1, 1);
	glVertex3d(-1, -1, -1);
	glVertex3d(1, -1, -1);
	glVertex3d(1, -1, 1);

	// face 6
	//glColor3d(0, 0, 0.5f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, blueMaterialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, blueMaterialDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterialSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);
	
	glNormal3f(0, 0, -1);
	glVertex3d(-1, -1, -1);
	glVertex3d(-1, 1, -1);
	glVertex3d(1, 1, -1);
	glVertex3d(1, -1, -1);

	glEnd();

	*/

}

void setLights(void)
{
	//glClearDepth(1);
	//glEnable(GL_DEPTH_TEST);

	glEnable(GL_LIGHTING);
	//Log::log(Log::USER_INFO, ("_DeltaTimeCumul: " + toString(g_renderer->_DeltaTimeCumul)).c_str());

	// Skybox (blending between orange and blue based on time, period of 24s)
	NYColor orange = NYColor{ 255.f / 255.f, 193.f / 255.f, 93.f / 255.f, 1 };
	NYColor blue = NYColor { 62.f / 255.f, 199.f / 255.f, 255.f / 255.f, 1 };
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
	float direction[4] = {0,0,1,0}; ///w = 0 donc elle est a l'infini
	glLightfv(GL_LIGHT0, GL_POSITION, direction );  // la direction est en fait l'oppose de ce qu'on a passe
	// La lumière dépend du daytime, similaire à skybox
	GLfloat whiteColor[4] = { 0.3f,0.3f,0.3f };
	GLfloat sunColor[4] = {skyColor.R, skyColor.V, skyColor.B};
	glLightfv(GL_LIGHT0, GL_AMBIENT, whiteColor);
	GLfloat color2[4] = {0.3f,0.3f,0.3f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, sunColor);
	GLfloat color3[4] = {0.3f,0.3f,0.3f};
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

void resizeFunction(int width, int height)
{
	glViewport(0, 0, width, height);
	g_renderer->resize(width,height);
}

//////////////////////////////////////////////////////////////////////////
// GESTION CLAVIER SOURIS
//////////////////////////////////////////////////////////////////////////

void specialDownFunction(int key, int p1, int p2)
{
	Log::log(Log::USER_INFO, ("keyboardDownFunction key: " + toString(key) + ", p1: " + toString(p1) + ", p2: " + toString(p2)).c_str());
	
	//On change de mode de camera
	if(key == GLUT_KEY_LEFT)
	{
	}

	if (key == GLUT_KEY_CTRL_L)
	{
		// note that this won't work if the application starts while left ctrl is pressed
		// so you'll need to release ctrl and press it again
		leftCtrlPressed = true;
	}

}

void specialUpFunction(int key, int p1, int p2)
{
	Log::log(Log::USER_INFO, ("specialUpFunction key: " + toString(key) + ", p1: " + toString(p1) + ", p2: " + toString(p2)).c_str());
	
	if (key == GLUT_KEY_CTRL_L)
	{
		leftCtrlPressed = false;
	}
}

void keyboardDownFunction(unsigned char key, int p1, int p2)
{
	if(key == VK_ESCAPE)
	{
		glutDestroyWindow(g_main_window_id);	
		exit(0);
	}

	if(key == 'f')
	{
		if(!g_fullscreen){
			glutFullScreen();
			g_fullscreen = true;
		} else if(g_fullscreen){
			glutLeaveGameMode();
			glutLeaveFullScreen();
			glutReshapeWindow(g_renderer->_ScreenWidth, g_renderer->_ScreenWidth);
			glutPositionWindow(0,0);
			g_fullscreen = false;
		}
	}
}

void keyboardUpFunction(unsigned char key, int p1, int p2)
{
}

void mouseWheelFunction(int wheel, int dir, int x, int y)
{
	//Log::log(Log::USER_INFO, ("mouseWheelFunction wheel: " + toString(wheel) + ", dir: " + toString(dir) + ", x: " + toString(x) + ", y: " + toString(y)).c_str());
	
	g_renderer->_Camera->move(NYVert3Df(0, 0, 1) * dir * cameraZMotionWheelRatio);
}

void mouseFunction(int button, int state, int x, int y)
{
	//Log::log(Log::USER_INFO, ("mouseFunction button: " + toString(button) + ", state: " + toString(state) + ", x: " + toString(x) + ", y: " + toString(y)).c_str());
	
	//Gestion de la roulette de la souris
	if((button & 0x07) == 3 && state)
		mouseWheelFunction(button,1,x,y);
	if((button & 0x07) == 4 && state)
		mouseWheelFunction(button,-1,x,y);

	// on mouse button press, record press and initialize mouse position to get relative motion later
	if (state == GLUT_DOWN ) {
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
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		g_mouse_btn_gui_state |= GUI_MLBUTTON;
	
	bool mouseTraite = false;
	mouseTraite = g_screen_manager->mouseCallback(x,y,g_mouse_btn_gui_state,0,0);
}

void mouseMoveFunction(int x, int y, bool pressed)
{
	//Log::log(Log::USER_INFO, ("mouseMoveFunction to: " + toString(x) + ", " + toString(y) + ", pressed: " + toString(pressed)).c_str());

	bool mouseTraite = false;

	mouseTraite = g_screen_manager->mouseCallback(x,y,g_mouse_btn_gui_state,0,0);

	if (!mouseTraite && (mouseLeftButtonPressed || mouseMiddleButtonPressed || mouseRightButtonPressed))
	{
		int deltaMouseX = x - lastMouseX;
		int deltaMouseY = y - lastMouseY;
		//Log::log(Log::USER_INFO, ("deltaMouse: " + toString(deltaMouseX) + ", " + toString(deltaMouseY)).c_str());

		lastMouseX = x;
		lastMouseY = y;

		// if left ctrl or middle mouse button is pressed, translate in XY
		// a) translate forward following camera target
		// b) translate on forward projected on world XY plane (crossProd(_UpRef, _NormVec))
		// here: a)
		if (leftCtrlPressed || mouseMiddleButtonPressed)
		{
			// coords are top-left so for negative delta Y, move UP!
			g_renderer->_Camera->move((g_renderer->_Camera->_NormVec * deltaMouseX + g_renderer->_Camera->_Direction * (- deltaMouseY)) * cameraXYMotionMouseRatio);
		}

		// translate in camera XZ with right-hold mouse move
		if (mouseRightButtonPressed)
		{
			// update camera view on passive (no click) mouse motion
			g_renderer->_Camera->rotate(deltaMouseX * cameraXRotateMouseRatio);
			g_renderer->_Camera->rotateUp(deltaMouseY * cameraYRotateMouseRatio);
		}
	}
	
	if(pressed && mouseTraite)
	{
		Log::log(Log::USER_INFO, ("slider value update to: " + toString(g_slider_cameraXRotateMouseRatio->Value) + ", " + toString(g_slider_cameraYRotateMouseRatio->Value) + ", Z: " + toString(g_slider_cameraZMotionWheelRatio->Value)).c_str());
		//Mise a jour des variables liées aux sliders
		// oppose for negative values
		cameraXRotateMouseRatio = - g_slider_cameraXRotateMouseRatio->Value;
		cameraYRotateMouseRatio = - g_slider_cameraYRotateMouseRatio->Value;
		cameraZMotionWheelRatio = g_slider_cameraZMotionWheelRatio->Value;
		cameraXYMotionMouseRatio = g_slider_cameraXYMotionMouseRatio->Value;
	}

}

void mouseMoveActiveFunction(int x, int y)
{
	mouseMoveFunction(x,y,true);
}
void mouseMovePassiveFunction(int x, int y)
{
	mouseMoveFunction(x,y,false);
}


void clickBtnParams (GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_params);
}

void clickBtnCloseParam (GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_jeu);
}

/**
  * POINT D'ENTREE PRINCIPAL
  **/
int main(int argc, char* argv[])
{
	LogConsole::createInstance();

	int screen_width = 800;
	int screen_height = 600;

	glutInit(&argc, argv); 
	glutInitContextVersion(3,0);
	glutSetOption(
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
		);

	glutInitWindowSize(screen_width,screen_height);
	glutInitWindowPosition (0, 0);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE );

	glEnable(GL_MULTISAMPLE);

	Log::log(Log::ENGINE_INFO, (toString(argc) + " arguments en ligne de commande.").c_str());	
	bool gameMode = true;
	for(int i=0;i<argc;i++)
	{
		if(argv[i][0] == 'w')
		{
			Log::log(Log::ENGINE_INFO,"Arg w mode fenetre.\n");
			gameMode = false;
		}
	}

	if(gameMode)
	{
		int width = glutGet(GLUT_SCREEN_WIDTH);
		int height = glutGet(GLUT_SCREEN_HEIGHT);
		
		char gameModeStr[200];
		sprintf(gameModeStr,"%dx%d:32@60",width,height);
		glutGameModeString(gameModeStr);
		g_main_window_id = glutEnterGameMode();
	}
	else
	{
		g_main_window_id = glutCreateWindow("MyNecraft");
		glutReshapeWindow(screen_width,screen_height);
	}

	if(g_main_window_id < 1) 
	{
		Log::log(Log::ENGINE_ERROR,"Erreur creation de la fenetre.");
		exit(EXIT_FAILURE);
	}
	
	GLenum glewInitResult = glewInit();

	if (glewInitResult != GLEW_OK)
	{
		Log::log(Log::ENGINE_ERROR,("Erreur init glew " + std::string((char*)glewGetErrorString(glewInitResult))).c_str());
		_cprintf("ERROR : %s",glewGetErrorString(glewInitResult));
		exit(EXIT_FAILURE);
	}

	//Affichage des capacités du système
	Log::log(Log::ENGINE_INFO,("OpenGL Version : " + std::string((char*)glGetString(GL_VERSION))).c_str());

	glutDisplayFunc(update);
	glutReshapeFunc(resizeFunction);
	glutKeyboardFunc(keyboardDownFunction);
	glutKeyboardUpFunc(keyboardUpFunction);
	glutSpecialFunc(specialDownFunction);
	glutSpecialUpFunc(specialUpFunction);
	glutMouseFunc(mouseFunction);
	glutMotionFunc(mouseMoveActiveFunction);
	glutPassiveMotionFunc(mouseMovePassiveFunction);
	glutIgnoreKeyRepeat(1);

	//Initialisation du renderer
	g_renderer = NYRenderer::getInstance();
	g_renderer->setRenderObjectFun(renderObjects);
	g_renderer->setRender2DFun(render2d);
	g_renderer->setLightsFun(setLights);
	g_renderer->setBackgroundColor(NYColor());
	g_renderer->initialise();

	//On applique la config du renderer
	glViewport(0, 0, g_renderer->_ScreenWidth, g_renderer->_ScreenHeight);
	g_renderer->resize(g_renderer->_ScreenWidth,g_renderer->_ScreenHeight);
	
	//Ecran de jeu
	uint16 x = 10;
	uint16 y = 10;
	g_screen_jeu = new GUIScreen(); 

	g_screen_manager = new GUIScreenManager();
		
	//Bouton pour afficher les params
	BtnParams = new GUIBouton();
	BtnParams->Titre = std::string("Params");
	BtnParams->X = x;
	BtnParams->setOnClick(clickBtnParams);
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
	btnClose->setOnClick(clickBtnCloseParam);
	g_screen_params->addElement(btnClose);

	y += btnClose->Height + 1;
	y+=10;
	x+=10;

	// slider for camera X rotate
	g_slider_cameraXRotateMouseRatio = new GUISlider();
	g_slider_cameraXRotateMouseRatio->setMaxMin(1, 0);  // will be opposed
	g_slider_cameraXRotateMouseRatio->setValue(-cameraXRotateMouseRatio); // init with param value to ensure sync
	g_slider_cameraXRotateMouseRatio->Visible = true;
	g_screen_params->addLabeledElement(x, y, "Camera X rotate: ", g_slider_cameraXRotateMouseRatio);
	// add vertical space to prepare position of next element
	y += g_slider_cameraXRotateMouseRatio->Height + 1;
	y += 10;

	// slider for camera Y rotate
	g_slider_cameraYRotateMouseRatio = new GUISlider();
	g_slider_cameraYRotateMouseRatio->setMaxMin(1, 0);  // will be opposed
	g_slider_cameraYRotateMouseRatio->setValue(-cameraYRotateMouseRatio); // init with param value to ensure sync
	g_slider_cameraYRotateMouseRatio->Visible = true;
	g_screen_params->addLabeledElement(x, y, "Camera Y rotate: ", g_slider_cameraYRotateMouseRatio);
	// add vertical space to prepare position of next element
	y += g_slider_cameraYRotateMouseRatio->Height + 1;
	y += 10;

	// slider for camera Z by wheel
	g_slider_cameraZMotionWheelRatio = new GUISlider();
	g_slider_cameraZMotionWheelRatio->setMaxMin(1, 0);
	g_slider_cameraZMotionWheelRatio->setValue(cameraZMotionWheelRatio); // init with param value to ensure sync
	g_slider_cameraZMotionWheelRatio->Visible = true;
	g_screen_params->addLabeledElement(x, y, "Camera Z motion: ", g_slider_cameraZMotionWheelRatio);
	// add vertical space to prepare position of next element
	y += g_slider_cameraZMotionWheelRatio->Height + 1;
	y += 10;

	// slider for camera XY plane motion
	g_slider_cameraXYMotionMouseRatio = new GUISlider();
	g_slider_cameraXYMotionMouseRatio->setMaxMin(1, 0);
	g_slider_cameraXYMotionMouseRatio->setValue(cameraXYMotionMouseRatio); // init with param value to ensure sync
	g_slider_cameraXYMotionMouseRatio->Visible = true;
	g_screen_params->addLabeledElement(x, y, "Camera XY motion: ", g_slider_cameraXYMotionMouseRatio);
	// add vertical space to prepare position of next element
	y += g_slider_cameraXYMotionMouseRatio->Height + 1;
	y += 10;

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

	return 0;
}

