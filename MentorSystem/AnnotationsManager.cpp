///////////////////////////////////////////////////////////////////
/*
 * Mentor System Application
 * System for Telemementoring with Augmented Reality (STAR) Project
 * Intelligent Systems and Assistive Technology (ISAT) Laboratory
 * Purdue University School of Industrial Engineering
 * 
 * Code programmed by: Edgar Javier Rojas Muñoz
 * advised by the professor: Juan Pablo Wachs, Ph.D
 */
//---------------------------------------------------------------//
/*                        CODE OVERVIEW
 * Name: AnnotationsManager.cpp
 *
 * Overview: .cpp of AnnotationsManager.h
 */
///////////////////////////////////////////////////////////////////

//Include its header file
#include "AnnotationsManager.h"
#include <mutex>


//--------------------------Definitions--------------------------//
#define ZERO 0.000001
#define PI 3.14159265358979323846

float ANNOTATION_COLOR_UNSELECTED_RGB[3] = { 0.97f, 1.0f, 0.0f };
float ANNOTATION_COLOR_SELECTED_RGB[3] = { 0.97f, 0.0f, 0.0f };

//Define a RGB struct to have in position of the framebuffer
typedef struct {
	double r;
	double g;
	double b;
} COLOR;

//---------------------------Variables---------------------------//
//Screen resolution
int resolutionX, resolutionY;

//current Id of the line being drawn
int currentID = -1;

bool _usingMouseAndMotionCallbacks = false;

//Command Center instance
CommandCenter* myCommander;

//JSONManager instance
JSONManager* myJSON;

//CameraManager instance
CameraManager* myCamera;

//Stores all the lines that are going to be drawn by Bressenham
map<int, LineAnnotation*> lines; 
std::mutex linesMutex;  // protects lines

//Stores the Ids of the line that are inside of a specific roi
vector<int> selected_lines_id;

//Stores the points of a the line currently being created
LineAnnotation* temp_line=NULL;

//Framebuffer. It has the RGB values of the whole shown window
//COLOR **buffer;

//Clipping Manager instance
LiangBarsky* my_clip_window;

//Map Manager instance
Mapping* MapManager;

/*
 * Method Overview: Adds point to the line currently being created
 * Parameters: Line ID, X and Y coordinates of the point to add
 * Return: None
 */
void addPoint(int id,long double x, long double y)
{
	//if it receives a new line ID
	if(currentID!=id)
	{
		temp_line = new LineAnnotation(id);
		currentID = id;
	}
	//Adds the new point
	temp_line->addPoint(x,abs(y-resolutionY));

	/*
	 * Adds the temporal line to the line vector
	 * This is done in order to draw the line as soon as the
	 * touch event is received
	 */
    lines.insert(pair<int, LineAnnotation*>(id, temp_line));
}

/*
 * Method Overview: Method used to see if a line crosses the X axis
 * Parameters (1): Initial and final line-in-analysis coordiantes
 * Parameters (2): Generated exception code from the last run
 * Return: Whereas the point is inside of the region or not
 */
int crossingLine(long double Xin, long double Yin,long double Xfin, long double Yfin, int* wasException)
{
	int cross = 1;

	if(wasException[0]!=0)
	{
		if(wasException[0]==1)
		{
			if(Yfin < ZERO)
			{
				cross = 0;
			}
		}
		else
		{
			if(Yfin > ZERO)
			{
				cross = 0;
			}
		}
		wasException[0] = 0;
	}

	if(Xin < ZERO && Xfin < ZERO)
	{
		cross = 0;
	}
	else if(Yin < ZERO && Yfin < ZERO)
	{
		cross = 0;
	}
	else if(Yin > ZERO && Yfin > ZERO)
	{
		cross = 0;
	}
	else if((Yfin - ZERO) < ZERO)
	{
		if(Yin < ZERO)
		{
			wasException[0] = 1;
		}
		else
		{
			wasException[0] = 2;
		}
	}
	
	return cross;
}

/*
 * Method Overview: Translates the lines by an specific amount
 * Parameters: X and Y translation amounts
 * Return: None
 */
void translate(long double transX, long double transY)
{
	int i, j;

	std::lock_guard<std::mutex> linesLock(linesMutex);

	//Loop through all the lines
	for (i = 0; i < (int)selected_lines_id.size(); i++) 
	{
		LineAnnotation* to_transf = lines.find(selected_lines_id.at(i))->second;

		for (j = 0; j < (int)((to_transf)->getPoints())->size(); j = j+2) 
        {
			//Adds the translation amount
			((to_transf)->getPoints())->at(j) += transX;
			((to_transf)->getPoints())->at(j+1) += transY;
        }
		//changs the extreme values of the line
		(to_transf->getExtremePoints())[0] += transX;
		(to_transf->getExtremePoints())[1] += transY;
		(to_transf->getExtremePoints())[2] += transX;
		(to_transf->getExtremePoints())[3] += transY;

		//calls the function to recalculate the center
		to_transf->recalculateCenter();
    }
}

/*
 * Method Overview: Zooms the lines by an specific amount
 * Parameters: Zoom amount
 * Return: None
 */
void zoom(long double scale)
{
	int i, j;
	
	long double general_center_X = 0.0;
	long double general_center_Y = 0.0;

	std::lock_guard<std::mutex> linesLock(linesMutex);

	//calculates the general center of a group of lines
	for (i = 0; i < (int)selected_lines_id.size(); i++) 
	{
		general_center_X += (lines.find(selected_lines_id.at(i))->second)->getAnnotationCenter()[0];
		general_center_Y += (lines.find(selected_lines_id.at(i))->second)->getAnnotationCenter()[1];
	}

	general_center_X = general_center_X/(long double)selected_lines_id.size();
	general_center_Y = general_center_Y/(long double)selected_lines_id.size();

	//Loop through all the lines
	for (i = 0; i < (int)selected_lines_id.size(); i++) 
	{
		LineAnnotation* to_transf = lines.find(selected_lines_id.at(i))->second;

		for (j = 0; j < (int)((to_transf)->getPoints())->size(); j = j+2) 
        {
			/*
			 * Translates the point to the center of the scene
			 * Multiplies by zoom value
			 * Retranslates the point to its new zoomed value 
			 */
			((to_transf)->getPoints())->at(j) = ((((to_transf)->getPoints())->at(j) - 
				(general_center_X))*scale)+(general_center_X);
			((to_transf)->getPoints())->at(j+1) = ((((to_transf)->getPoints())->at(j+1) - 
				(general_center_Y))*scale)+(general_center_Y);
        }
		//changes the extreme values of the line
		(to_transf->getExtremePoints())[0] = (((to_transf->getExtremePoints())[0] - 
			(general_center_X))*scale)+(general_center_X);
		(to_transf->getExtremePoints())[1] = (((to_transf->getExtremePoints())[1] - 
			(general_center_Y))*scale)+(general_center_Y);
		(to_transf->getExtremePoints())[2] = (((to_transf->getExtremePoints())[2] - 
			(general_center_X))*scale)+(general_center_X);
		(to_transf->getExtremePoints())[3] = (((to_transf->getExtremePoints())[3] - 
			(general_center_Y))*scale)+(general_center_Y);

		//calls the function to recalculate the center
		to_transf->recalculateCenter();
    }
}

/*
 * Method Overview: Rotates the lines by an specific amount
 * Parameters: Rotation degree (in radians)
 * Return: None
 */
void rotate(long double degree)
{
	long double pi = PI/180.0;
	long double rad = degree*pi;

	int i, j;

	long double general_center_X = 0.0;
	long double general_center_Y = 0.0;

	long double rotated_values[2];

	std::lock_guard<std::mutex> linesLock(linesMutex);

	//calculates the general center of a group of lines
	for (i = 0; i < (int)selected_lines_id.size(); i++) 
	{
		general_center_X += (lines.find(selected_lines_id.at(i))->second)->getAnnotationCenter()[0];
		general_center_Y += (lines.find(selected_lines_id.at(i))->second)->getAnnotationCenter()[1];
	}

	//gets the average value
	general_center_X = general_center_X/(long double)selected_lines_id.size();
	general_center_Y = general_center_Y/(long double)selected_lines_id.size();

	//Loop through all the lines
	for (i = 0; i < (int)selected_lines_id.size(); i++) 
	{
		LineAnnotation* to_transf = lines.find(selected_lines_id.at(i))->second;

		for (j = 0; j < (int)((to_transf)->getPoints())->size(); j = j+2) 
        {
			//calls the rotete point function
			pointRotation(((to_transf)->getPoints())->at(j), ((to_transf)->getPoints())->at(j+1), 
				0, 0, rad, general_center_X, general_center_Y,rotated_values);

			//assigns the results
			((to_transf)->getPoints())->at(j) = rotated_values[0];
			((to_transf)->getPoints())->at(j+1) = rotated_values[1];
			
        }
		//calls the rotate point function
		pointRotation((to_transf->getExtremePoints())[0], (to_transf->getExtremePoints())[1], 
				0, 0, rad, general_center_X, general_center_Y, rotated_values);

		//assigns the results
		(to_transf->getExtremePoints())[0] = rotated_values[0];
		(to_transf->getExtremePoints())[1] = rotated_values[1];

		//calls the rotate point function
		pointRotation((to_transf->getExtremePoints())[2], (to_transf->getExtremePoints())[3], 
				0, 0, rad, general_center_X, general_center_Y, rotated_values);

		//assigns the results
		(to_transf->getExtremePoints())[2] = rotated_values[0];
		(to_transf->getExtremePoints())[3] = rotated_values[1];

		//calls the function to recalculate the center
		to_transf->recalculateCenter();
    }
}

/*
 * Method Overview: Rotates an specific point
 * Parameters (1): Point to rotate, distance to the center
 * Parameters (2): Angle to rotate, point to rotate around off
 * Parameters (3): Array with the rotated values
 * Return: None
 */
void pointRotation(long double X, long double Y, long double transX,long double transY,
				   long double angle, long double origX, long double origY, long double* rotated_values)
{
	//Variables used during the process
	long double xtransf0, ytransf0, cosx0, senx0, cosy0, seny0;

	//Translates the point to the center of the scene
	xtransf0 = (X + transX) - (origX);
	ytransf0 = (Y + transY) - (origY);

	//Calculates trigonometric values of the point
	cosx0 = cos(angle)*xtransf0;
	senx0 = sin(angle)*xtransf0;
	cosy0 = cos(angle)*ytransf0;
	seny0 = sin(angle)*ytransf0;

	//Calculates the new point
	xtransf0 = ((cosx0) - (seny0));
	ytransf0 = ((senx0) + (cosy0));

	rotated_values[0] = (xtransf0)+(origX);
	rotated_values[1] = (ytransf0)+(origY);	
}

/*
 * Method Overview: Has the respective processes to draw each line
 * Parameters: None
 * Return: None
 */
void openGLDrawLines()
{
	int i;

	//[xin,yin,xfin,yfin] of the line to draw
	int numbers[4];

	//Iterator to go through the map
    std::map<int, LineAnnotation*>::iterator iter;
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	std::lock_guard<std::mutex> linesLock(linesMutex);

	//loops to get each line
    for (iter = lines.begin(); iter != lines.end(); iter++)
    {
		//gets the line to be drawn
		LineAnnotation* to_draw;
		to_draw = iter->second;

		//Whereas this line is currently being selected
		int to_draw_state = to_draw->getSelectedState();

		
		if (to_draw_state) {
			glColor3f(ANNOTATION_COLOR_SELECTED_RGB[0], ANNOTATION_COLOR_SELECTED_RGB[1], ANNOTATION_COLOR_SELECTED_RGB[2]);
		}
		else {
			glColor3f(ANNOTATION_COLOR_UNSELECTED_RGB[0], ANNOTATION_COLOR_UNSELECTED_RGB[1], ANNOTATION_COLOR_UNSELECTED_RGB[2]);
		}
		

		glLineWidth(5.0f);
		glBegin(GL_LINE_STRIP);

		//loops through the line
		for (i = 0; i < to_draw->getPoints()->size() - 1; i += 2) {
			float x = to_draw->getPoints()->at(i);
			float y = to_draw->getPoints()->at(i+1);

			glVertex2f(x, y);
		}

		glEnd();
	}
	

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}







/*
 * Method Overview: Keyboard events handling
 * Parameters: Pressed Key, (x,y) positions of the mouse in screen
 * Return: None
 */
void keyboard(unsigned char key, int x, int y)
{
	
	bool keyConsumed = false;

	keyConsumed = myCamera->handleKey(key);

	if (!keyConsumed) {
		//analyzes which key was pressed
		switch (key)
		{
			//quit
		case 'q': case 'Q':
			glutLeaveMainLoop();
			exit(0);
			break;
		case 'd': case 'D':
			if (TouchOverlayController::debugMessagesEnabled) {
				std::cout << "disabling touch overlay debug messages" << std::endl;
				TouchOverlayController::debugMessagesEnabled = false;
			}
			else {
				std::cout << "enabling touch overlay debug messages" << std::endl;
				TouchOverlayController::debugMessagesEnabled = true;
			}
			break;
		case 'm': case 'M':
			if (_usingMouseAndMotionCallbacks) {
				std::cout << "disabling mouse/motion callbacks (using touchscreen instead)" << std::endl;
				_usingMouseAndMotionCallbacks = false;
			}
			else {
				std::cout << "enabling mouse/motion callbacks (don't use touchscreen while enabled)" << std::endl;
				_usingMouseAndMotionCallbacks = true;
			}
			break;
		}
	}
	
}

/*
 * Method Overview: Method used to remove the selected lines
 * Parameters: None
 * Return: None
 */
void eraseSelectedLines()
{			
	int counter;

	std::lock_guard<std::mutex> linesLock(linesMutex);

	//Loop through all the lines
	for (counter = 0; counter < (int)selected_lines_id.size(); counter++) 
	{
		lines.erase(selected_lines_id.at(counter));
	}
	

	//Gets the virtual annotation's selected IDs
	vector<int> selected = myCommander->getSelectedIDs();

	for(counter = 0; counter < (int)selected.size();counter++)
	{
		selected_lines_id.push_back(selected.at(counter));
	}

	vector<long double> null_long_vector;
	vector<double> null_double_vector;


	for (counter = 0; counter < (int)selected_lines_id.size(); counter++)
	{
		myJSON->createJSONable(NULL, DELETE_ANNOTATION_COMMAND, &null_long_vector, NULL, null_double_vector, selected_lines_id.at(counter));
	}

	

	selected_lines_id.clear();

}

/*
 * Method Overview: Method used to remove the selected lines
 * Parameters: None
 * Return: None
 */
void deselectAllLines()
{
	int counter;

	std::lock_guard<std::mutex> linesLock(linesMutex);

	for(counter = 0; counter< selected_lines_id.size();counter++)
	{
		auto selectedLineId = selected_lines_id.at(counter);
		
		auto foundLine = lines.find(selectedLineId);
		if (foundLine != lines.end()) {
			auto selectedLine = foundLine->second;
			if (selectedLine != NULL) {
				selectedLine->setSelectedState(0);
			}
		}
	}

	selected_lines_id.clear();

	myCommander->setLineSelectedFlag(0);

}

/*
 * Method Overview: Method used to remove all the lines
 * Parameters: None
 * Return: None
 */
void clearAllLines()
{

	selected_lines_id.clear();
			
	//Iterator to go through the map
    std::map<int, LineAnnotation*>::iterator iter;
	
	//Loops through all the lines
    for (iter = lines.begin(); iter != lines.end(); iter++)
    {
		selected_lines_id.push_back(iter->second->getID());
    }

	eraseSelectedLines();
}

/*
 * Method Overview: Kills the thread
 * Parameters: None
 * Return: None
 */
void endOpenGLContext()
{
	glutLeaveMainLoop();
	exit(0);
}

/*
 * Method Overview: Checks and interprets a command (if any)
 * Parameters: None
 * Return: None
 */
void checkAndInterpretCommand()
{
	if(myCommander->getAnnotationCommandFlag())
	{
		switch (myCommander->getAnnotationCommand())
		{	
			case DESELECT_ALL_LINES:
				deselectAllLines();
				break;

			case CLEAR_ALL_LINES:
				clearAllLines();
				break;

			case ERASE_LINES:
				eraseSelectedLines();
				break;

			case END_OPENGL_CONTEXT:
				endOpenGLContext();
				break;
		}
		myCommander->setAnnotationCommandFlag(0);
	}
}

/*
 * Method Overview: Uses the JSONManager to create a JSON Message
 * Parameters: Message command, annotation to make the massage from
 * Return: None
 */
void createJSONLineMessage(string command, LineAnnotation* annotation)
{
	int null_int = 0;
	vector<double> null_double_vector;
	myJSON->createJSONable(annotation->getID(), command, annotation->getPoints(), NULL, null_double_vector, null_int);
}

/*
 * Method Overview: Starts the JSON Update process
 * Parameters: None
 * Return: None
 */
void startJSONLineUpdate()
{
	int i;

	std::lock_guard<std::mutex> linesLock(linesMutex);

	//Loop through all the lines
	for (i = 0; i < (int)selected_lines_id.size(); i++) 
	{
		LineAnnotation* selected = lines.find(selected_lines_id.at(i))->second;

		createJSONLineMessage(UPDATE_ANNOTATION_COMMAND, selected);
	}
}

/*
 * Method Overview: Touch events handling
 * Parameters (1): Received action, line-to-work-on ID
 * Parameters (2): (x,y) positions of the event
 * Return: None
 */
void OpenGLtouchControls(int command, int id, long double x, long double y)
{
	//analyzes which event occured
    switch (command)
    {	
		//Rotations
		//rotate counterclockwise
		case ROTATE_CNTR_CLK:	
			rotate(2.0f);
			break;

		//rotate clockwise
		case ROTATE_CLK:	
			rotate(-2.0f);
			break;

		//Zooms
		//zoom in
		case ZOOM_IN:
			zoom(1.2f);
			break;

		//zoom out
		case ZOOM_OUT:
			zoom(0.8f);
			break;

		//Pans
		//right pan
		case TRANSLATE_RIGHT:
			translate(resolutionX/50.0f,0);
			break;

		//left pan
		case TRANSLATE_LEFT:
			translate(-resolutionX/50.0f,0);
			break;

		//up pan
		case TRANSLATE_UP:
			translate(0,resolutionY/50.0f);
			break;

		//down pan
		case TRANSLATE_DOWN:
			translate(0,-resolutionY/50.0f);
			break;

		//add a point to the line
		case ADD_POINT:
			addPoint(id,x,y);
			lines.erase(id);
			break;

		//add line to line map
		case ADD_LINE:
			lines.insert(pair<int, LineAnnotation*>(id, temp_line));
			createJSONLineMessage(CREATE_ANNOTATION_COMMAND, temp_line);
			break;

		//clear line to prevent wrong lines
		case CLEAR_LINE:
			if(temp_line!=NULL)
			{
				if((int)(temp_line->getPoints()->size())==2)
				{
					temp_line->getPoints()->clear();
					temp_line->setInitialExtremes();
				}
			}
			break;

		case ADD_POINT_ANNOTATION:
			{
				long double initial_point_distance = 5.0;
				long double angle;
				long double val = PI/180.0;
				long double rotated_values[2];

				int counter;

				//create enough points to make a round shape
				for(counter = 0; counter<=360;counter=counter+18)
				{
					//gets the new angle value
					angle = counter*val;

					//calls the rotete point function
					pointRotation(x, y, initial_point_distance, initial_point_distance, angle, x, y, rotated_values);

					//assigns the results
					addPoint(id,rotated_values[0],rotated_values[1]);
				}
				lines.insert(pair<int, LineAnnotation*>(id, temp_line));
				createJSONLineMessage(CREATE_ANNOTATION_COMMAND, temp_line);
			}
			break;
	}

	//Redraw scene
	glutPostRedisplay();
}



cv::Mat _currentBackgroundOpenCVImage;
bool _readyToUpdateBackgroundImage = true;
bool _hasReceivedBackgroundImage = false;

bool _testTextureInitialized = false;
GLuint _backgroundTextureId;

// updates current opencv image to be used for background
void updateBackgroundOpenCVImage(cv::Mat image) {
	if (_readyToUpdateBackgroundImage) {
		//std::cout << "ready to update background image" << std::endl;
		image.copyTo(_currentBackgroundOpenCVImage);
		_readyToUpdateBackgroundImage = false;
		_hasReceivedBackgroundImage = true;
	}
	else {
		//std::cout << "not ready to update background image" << std::endl;
	}
}

void init_test_texture() {
	std::cout << "initing background texture" << std::endl;

	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &_backgroundTextureId);
	glBindTexture(GL_TEXTURE_2D, _backgroundTextureId);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	int num_channels = _currentBackgroundOpenCVImage.channels();
	int w = _currentBackgroundOpenCVImage.size().width;
	int h = _currentBackgroundOpenCVImage.size().height;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, _currentBackgroundOpenCVImage.data);

	glBindTexture(GL_TEXTURE_2D, 0);

	_testTextureInitialized = true;
}










/*
 * Method Overview: Creates a char* of the most recently added line
 * Parameters: line-to-return ID
 * Return: Char* with all the points of the line
 */
char* OpenGLgetNewLine(int id)
{
	string message;

	unsigned int counter;

	//for each parameter of the touch gesture
	for(counter = 0; counter<(lines.find(id)->second)->getPoints()->size();)
	{
		//converts double long value to int, then to string
		ostringstream param;
		param << (int)((lines.find(id)->second)->getPoints()->at(counter));
		message+=param.str();

		if(counter!=((lines.find(id)->second)->getPoints()->size()-1))
		{
			//add a / to the string
			message+="/";
		}

		counter++;
	}

	//create a char* from the created string
	char* writable = new char[message.size()+1];
	copy(message.begin(),message.end(),writable);
	writable[message.size()] = '\0';

	//return char*
	return writable;
}

/*
 * Method Overview: Point-in-polygon Algorithm Variant (see theory)
 * Parameters: Region of interest polygon
 * Return: None
 */
int pointInPolygon(vector<long double> roi_extremes)
{
	int selected = 0;

	std::lock_guard<std::mutex> linesLock(linesMutex);

	int counter;

	for(counter = 0; counter< selected_lines_id.size();counter++)
	{
		auto selectedLineId = selected_lines_id.at(counter);
		
		auto foundLine = lines.find(selectedLineId);
		if (foundLine != lines.end()) {
			auto selectedLine = foundLine->second;
			if (selectedLine != NULL) {
				selectedLine->setSelectedState(0);
			}
		}
	}

	selected_lines_id.clear();

	int i; 

	//Iterator to go through the map
    std::map<int, LineAnnotation*>::iterator iter;
	
	//Loops through all the lines
    for (iter = lines.begin(); iter != lines.end(); iter++)
    {
		int crosses = 0;
		int wasException[1];
		wasException[0] = 0;

		long double* center = iter->second->getAnnotationCenter();

		for(i=0;i < (int)roi_extremes.size()-2; i=i+2)
		{
			//Yin-Yfin changes because of OpenGL Y axis orintation
			if(crossingLine((roi_extremes.at(i))-center[0],(abs(roi_extremes.at(i+3)-resolutionY))-center[1],
				(roi_extremes.at(i+2))-center[0],(abs(roi_extremes.at(i+1)-resolutionY))-center[1], wasException))
				crosses = !crosses;
		}
		if(crosses)
		{
			selected_lines_id.push_back(iter->second->getID());
			iter->second->setSelectedState(1);
		}
    }

	if((int)selected_lines_id.size()>0)
	{
		//Redraw the scene
		glutPostRedisplay();

		selected = 1;
	}

	return selected;
}



















/*
 * Method Overview: OpenGL pixel coloring for framebuffer positions
 * Parameters: None
 * Return: None
 */
void draw_scene()
{
	checkAndInterpretCommand();

	// clear framebuffer with white
	glClear(GL_COLOR_BUFFER_BIT);


	if (_hasReceivedBackgroundImage) {
		if (!_testTextureInitialized) {
			init_test_texture();
		}

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, _backgroundTextureId);

		if (!_readyToUpdateBackgroundImage) {

			int w = _currentBackgroundOpenCVImage.size().width;
			int h = _currentBackgroundOpenCVImage.size().height;

			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGR, GL_UNSIGNED_BYTE, _currentBackgroundOpenCVImage.data);

			_readyToUpdateBackgroundImage = true;
		}

		
		glColor3f(1.0, 1.0, 1.0);
		// some testing about drawing polygons
		glBegin(GL_QUADS);

		glTexCoord2f(0.0, 0.0);
		glVertex2f(-0.5, -0.5);

		glTexCoord2f(1.0, 0.0);
		glVertex2f(resolutionX + 0.5, -0.5);

		glTexCoord2f(1.0, 1.0);
		glVertex2f(resolutionX + 0.5, resolutionY + 0.5);

		glTexCoord2f(0.0, 1.0);
		glVertex2f(-0.5, resolutionY + 0.5);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	openGLDrawLines();



	




	glFlush();
}

/*
 * Method Overview: Refresh the scene when idle
 * Parameters: None
 * Return: None
 */
void refresh()
{
	//Redraw scene
	glutPostRedisplay();
}

void motion(int x, int y) {
	if (_usingMouseAndMotionCallbacks) {
		//std::cout << "motion: (" << x << ", " << y << ")" << std::endl;

		TouchGesture tg;
		tg.param_size = 2;
		tg.params[0] = x;
		tg.params[1] = y;

		tg.type = TG_MOVE_LEFT;	// TODO, don't need to say left, but any direction should do

		TouchOverlayController::OnTG_MoveLeft(tg, NULL);
	}
}

void mouse(int button, int state, int x, int y) {
	if (_usingMouseAndMotionCallbacks) {
		//std::cout << "mouse: (" << x << ", " << y << ")" << std::endl;

		TouchGesture tg;
		tg.param_size = 2;
		tg.params[0] = x;
		tg.params[1] = y;

		/* Show the button and the event on the mouse */
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{
			//std::cout << "Mouse: Left button down" << std::endl;


			tg.type = TG_TOUCH_START;
			TouchOverlayController::OnTG_TouchStart(tg, NULL);


			tg.type = TG_DOWN;
			TouchOverlayController::OnTG_Down(tg, NULL);

		}
		else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		{
			//std::cout << "Mouse: Left button up" << std::endl;

			tg.type = TG_CLICK;
			TouchOverlayController::OnTG_Click(tg, NULL);

			tg.type = TG_TOUCH_END;
			TouchOverlayController::OnTG_TouchEnd(tg, NULL);
		}
		else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
		{
			std::cout << "Mouse: Middle button down" << std::endl;
		}
		else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_UP)
		{
			std::cout << "Mouse: Middle button up" << std::endl;
		}
		else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
		{
			std::cout << "Mouse: Right button down" << std::endl;
		}
		else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
		{
			std::cout << "Mouse: Right button up" << std::endl;

			tg.type = TG_ROTATE_CLOCK;
			TouchOverlayController::OnTG_RotateClock(tg, NULL);

			tg.type = TG_TOUCH_END;
			TouchOverlayController::OnTG_TouchEnd(tg, NULL);
		}
	}
	
}

/*
 * Method Overview: Creates the OpenGL scene and context
 * Parameters (1): Main values, scene-to-create resolution
 * Parameters (2): Instance of the Command Center
 * Parameters (3): Instance of the JSON Manager
 * Return: None
 */
void initWindow(int argc, char* argv[], int resX, int resY, CommandCenter* pCommander, JSONManager* pJSON, CameraManager* pCamera)
{
	//Sets the CommandCenter instance as own
	myCommander = pCommander;

	//Sets the JSONManager instance as own
	myJSON = pJSON;

	// Sets the CameraManager instance as own
	myCamera = pCamera;

	//Assigns the scene resolution
	resolutionX = resX;
	resolutionY = resY;

	//Coordinates of the clipping window
	my_clip_window = new LiangBarsky(0, 0, resolutionX-1, resolutionY-1);

	//In charge of perform octant mappings
	MapManager = new Mapping();

	//OpenGL Context and Functions initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(resolutionX,resolutionY);
	glutCreateWindow("STAR Mentor System");
	glutFullScreen();
	glutIdleFunc(refresh);
	glutDisplayFunc(draw_scene);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	gluOrtho2D(-0.5, resolutionX +0.5, -0.5, resolutionY + 0.5);
	glutMainLoop();
}
