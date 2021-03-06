///////////////////////////////////////////////////////////////////
/*
 * Mentor System Application
 * System for Telemementoring with Augmented Reality (STAR) Project
 * Intelligent Systems and Assistive Technology (ISAT) Laboratory
 * Purdue University School of Industrial Engineering
 * 
 * Code programmed by: Edgar Javier Rojas Mu�oz
 * advised by the professor: Juan Pablo Wachs, Ph.D
 */
//---------------------------------------------------------------//
/*                        CODE OVERVIEW
 * Name: TouchOverlayController.cpp
 *
 * Overview: .cpp of TouchOverlayController.h
 */
///////////////////////////////////////////////////////////////////

//Include its header file
#include "TouchOverlayController.h"

//--------------------------Definitions--------------------------//
#define BIG_VALUE 10000
#define SMALL_VALUE -10000

//--------------------------Variables----------------------------//
CommandCenter* TouchOverlayController::myCommander;
GUIManager* TouchOverlayController::myGUI;
CameraManager* TouchOverlayController::myCamera;
int TouchOverlayController::annotationCounter;
int TouchOverlayController::selected_annotation_code;
double TouchOverlayController::last_param[2];
vector<long double> TouchOverlayController::roi_extremes;
vector<double> TouchOverlayController::real_element_features;
unsigned short TouchOverlayController::last_type;
int TouchOverlayController::button_clicked;
bool TouchOverlayController::debugMessagesEnabled = false;

/*
 * Method Overview: Constructor of the class
 * Parameters: None 
 * Return: Instance of the class
 */
TouchOverlayController::TouchOverlayController()
{
	TouchOverlayController::debugMessagesEnabled = false;

	annotationCounter = 0;
	roi_extremes.push_back(0.0);
	memset(m_pf_on_tges,0, sizeof(m_pf_on_tges));
}

/*
 * Method Overview: Destructor of the class
 * Parameters: None
 * Return: None
 */
TouchOverlayController::~TouchOverlayController()
{
	DisconnectServer();
}

bool TouchOverlayController::isInMockMode() {
	return _mockMode;
}

/*
 * Method Overview: Starts the Touch Overlay Controller
 * Parameters (1): Instance of the Communication Manager server
 * Parameters (2): Instance of the Command Center
 * Return: flag of successfulness
 */
int TouchOverlayController::Init(CommandCenter* pCommander, GUIManager* pGUI, CameraManager* pCamera)
{
	// initially set mock mode to false -- assume that multitouch system will be used
	_mockMode = false;

	//Sets the CommandCenter instance as own
	myCommander = pCommander;

	//Establishes the GUIManager
	myGUI = pGUI;

	myCamera = pCamera;

	last_param[0] = 0.0;
	last_param[1] = 0.0;

	//Successful Init
	int err_code = PQMTE_SUCCESS;

	//Initialize the handle functions of gestures;
	InitFuncOnTG();

	//Set the functions on server callback
	SetFuncsOnReceiveProc();

	//Connect server
	cout << " connect to server..." << endl;
	if((err_code = ConnectServer()) != PQMTE_SUCCESS){
		cout << " connect server fail, socket error code:" << err_code << endl;

		cout << "mock mode enabled... going to only support mouse clicks" << endl;
		_mockMode = true;
		return PQMTE_SUCCESS;

		//return err_code;
	}
	// send request to server
	cout << " connect success, send request." << endl;
	TouchClientRequest tcq = {0};
	tcq.type = RQST_RAWDATA_ALL | RQST_GESTURE_ALL;
	if((err_code = SendRequest(tcq)) != PQMTE_SUCCESS){
		cout << " send request fail, error code:" << err_code << endl;
		return err_code;
	}

	//get server resolution
	if((err_code = GetServerResolution(OnGetServerResolution, NULL)) != PQMTE_SUCCESS){
		cout << " get server resolution fail,error code:" << err_code << endl;
		return err_code;
	}
	
	// start receiving
	cout << " send request success, start recv." << endl;
	return err_code;
}

/*
 * Method Overview: Init the Touch Gestures methods handlers
 * Parameters: None
 * Return: None
 */
void TouchOverlayController:: InitFuncOnTG()
{
	/*
	 * Some handlers are not used. However, they could be useful
	 * later on. Therefore, they were commented instead of deleted
	 */

	m_pf_on_tges[TG_TOUCH_START] = &TouchOverlayController::OnTG_TouchStart;
	m_pf_on_tges[TG_DOWN] = &TouchOverlayController::OnTG_Down;
	m_pf_on_tges[TG_MOVE] = &TouchOverlayController::OnTG_Move;
	//m_pf_on_tges[TG_UP] = &TouchOverlayController::OnTG_Up;
	m_pf_on_tges[TG_CLICK] = &TouchOverlayController::OnTG_Click;

	//m_pf_on_tges[TG_SECOND_DOWN] = &TouchOverlayController::OnTG_SecondDown;
	//m_pf_on_tges[TG_SECOND_UP] = &TouchOverlayController::OnTG_SecondUp;

	m_pf_on_tges[TG_MOVE_RIGHT] = &TouchOverlayController::OnTG_MoveRight;
	m_pf_on_tges[TG_MOVE_LEFT] = &TouchOverlayController::OnTG_MoveLeft;
	m_pf_on_tges[TG_MOVE_DOWN] = &TouchOverlayController::OnTG_MoveDown;
	m_pf_on_tges[TG_MOVE_UP] = &TouchOverlayController::OnTG_MoveUp;

	m_pf_on_tges[TG_TOUCH_END] = &TouchOverlayController::OnTG_TouchEnd;
	
	m_pf_on_tges[TG_ROTATE_CLOCKWISE] = &TouchOverlayController::OnTG_RotateClock;
	m_pf_on_tges[TG_ROTATE_ANTICLOCKWISE] = &TouchOverlayController::OnTG_RotateAntiClock;
	//m_pf_on_tges[TG_SPLIT_START] = &TouchOverlayController::OnTG_SplitStart;
	m_pf_on_tges[TG_SPLIT_APART] = &TouchOverlayController::OnTG_SplitApart;
	m_pf_on_tges[TG_SPLIT_CLOSE] = &TouchOverlayController::OnTG_SplitClose;
	//m_pf_on_tges[TG_SPLIT_END] = &TouchOverlayController::OnTG_SplitEnd;
	m_pf_on_tges[TG_NEAR_PARALLEL_MOVE_UP] = &TouchOverlayController::onTG_NearParrellMoveUp;
	m_pf_on_tges[TG_NEAR_PARALLEL_MOVE_DOWN] = &TouchOverlayController::onTG_NearParrellMoveDown;
	m_pf_on_tges[TG_NEAR_PARALLEL_MOVE_RIGHT] = &TouchOverlayController::onTG_NearParrellMoveRight;
	m_pf_on_tges[TG_NEAR_PARALLEL_MOVE_LEFT] = &TouchOverlayController::onTG_NearParrellMoveLeft;

	m_pf_on_tges[TG_MULTI_DOWN] = &TouchOverlayController::onTG_MultiDown;
	m_pf_on_tges[TG_MULTI_MOVE] = &TouchOverlayController::onTG_MultiMove;
}

/*
 * Method Overview: Init the call back methods handlers
 * Parameters: None
 * Return: None
 */
void TouchOverlayController::SetFuncsOnReceiveProc()
{
	PFuncOnReceivePointFrame old_rf_func = SetOnReceivePointFrame(&TouchOverlayController::OnReceivePointFrame,this);
	PFuncOnReceiveGesture old_rg_func = SetOnReceiveGesture(&TouchOverlayController::OnReceiveGesture,this);
	PFuncOnServerBreak old_svr_break = SetOnServerBreak(&TouchOverlayController::OnServerBreak,NULL);
	PFuncOnReceiveError old_rcv_err_func = SetOnReceiveError(&TouchOverlayController::OnReceiveError,NULL);
	PFuncOnGetDeviceInfo old_gdi_func = SetOnGetDeviceInfo(&TouchOverlayController::OnGetDeviceInfo,NULL);
}

/*
 * Method Overview: Handles the reception of touch point frames
 * Parameters (1): Point frame, time stamp of the event
 * Parameters (2): Count of how many points are moving
 * Parameters (3): Array to store the moving points
 * Parameters (4): Object that created the call back event
 * Return: None
 */
void TouchOverlayController:: OnReceivePointFrame(int frame_id, int time_stamp, int moving_point_count, const TouchPoint * moving_point_array, void * call_back_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnReceivePointFrame" << std::endl;
	}
	TouchOverlayController * controller = static_cast<TouchOverlayController*>(call_back_object);
	assert(controller != NULL);
	const char * tp_event[] = 
	{
		"down",
		"move",
		"up",
	};
	
	//cout << " frame_id:" << frame_id << " time:"  << time_stamp << " ms" << " moving point count:" << moving_point_count << endl;
	for(int i = 0; i < moving_point_count; ++ i){
		TouchPoint tp = moving_point_array[i];
		//controller->OnTouchPoint(tp); //Enable if this handler is needed
	}
}

/*
 * Method Overview: Handles the reception of touch gestures
 * Parameters (1): Received gesture
 * Parameters (2): Object that created the call back event
 * Return: None
 */
void TouchOverlayController:: OnReceiveGesture(const TouchGesture & ges, void * call_back_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnReceiveGesture" << std::endl;
	}

	TouchOverlayController * controller = static_cast<TouchOverlayController*>(call_back_object);
	assert(controller != NULL);
	controller->OnTouchGesture(ges);
	//throw exception("test exception here");
}

/*
 * Method Overview: Handles server break event
 * Parameters (1): Server break parameter
 * Parameters (2): Object that created the call back event
 * Return: None
 */
void TouchOverlayController:: OnServerBreak(void * param, void * call_back_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnServerBreak" << std::endl;
	}

	// when the server break, disconenct server;
	cout << "server break, disconnect here" << endl;
	DisconnectServer();
}

/*
 * Method Overview: Handles error event
 * Parameters (1): Server break parameter
 * Parameters (2): Object that created the call back event
 * Return: None
 */
void TouchOverlayController::OnReceiveError(int err_code, void * call_back_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnReceiveError" << std::endl;
	}

	switch(err_code)
	{
	case PQMTE_RCV_INVALIDATE_DATA:
		cout << " error: receive invalidate data." << endl;
		break;
	case PQMTE_SERVER_VERSION_OLD:
		cout << " error: the multi-touch server is old for this client, please update the multi-touch server." << endl;
		break;
	case PQMTE_EXCEPTION_FROM_CALLBACKFUNCTION:
		cout << "**** some exceptions thrown from the call back functions." << endl;
		assert(0); //need to add try/catch in the callback functions to fix the bug;
		break;
	default:
		cout << " socket error, socket error code:" << err_code << endl;
	}
}

/*
 * Method Overview: Gets and prints the server resolution
 * Parameters (1): Server resolution
 * Parameters (2): Object that created the call back event
 * Return: None
 */
void TouchOverlayController:: OnGetServerResolution(int x, int y, void * call_back_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnGetServerResolution" << std::endl;
	}

	cout << " server resolution:" << x << "," << y << endl;

	SERVER_RESOLUTION_X = x;
	SERVER_RESOLUTION_Y = y;
}

/*
 * Method Overview: Gets and prints the device info
 * Parameters (1): Device info
 * Parameters (2): Object that created the call back event
 * Return: None
 */
void TouchOverlayController::OnGetDeviceInfo(const TouchDeviceInfo & deviceinfo,void *call_back_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnGetDeviceInfo" << std::endl;
	}

	cout << " touch screen, SerialNumber: " << deviceinfo.serial_number <<",(" << deviceinfo.screen_width << "," << deviceinfo.screen_height << ")."<<  endl;
}

//Touch Gestures Handlers not being used at the moment
/*
//NOT USED
void TouchOverlayController:: OnTouchPoint(const TouchPoint & tp)
{
	switch(tp.point_event)
	{
	case TP_DOWN:
		cout << "  point " << tp.id << " come at (" << tp.x << "," << tp.y 
			<< ") width:" << tp.dx << " height:" << tp.dy << endl;
		break;
	case TP_MOVE:
		cout << "  point " << tp.id << " move at (" << tp.x << "," << tp.y 
			<< ") width:" << tp.dx << " height:" << tp.dy << endl;
		break;
	case TP_UP:
		cout << "  point " << tp.id << " leave at (" << tp.x << "," << tp.y 
			<< ") width:" << tp.dx << " height:" << tp.dy << endl;
		break;
	}
}

//NOT USED
void TouchOverlayController:: DefaultOnTG(const TouchGesture & tg,void * call_object)
{

}

//NOT USED
void TouchOverlayController:: OnTG_Up(const TouchGesture & tg,void * call_object)
{
	assert(tg.type == TG_UP && tg.param_size >= 2);
}

//NOT USED
void TouchOverlayController:: OnTG_SecondDown(const TouchGesture & tg,void * call_object)
{
	assert(tg.type == TG_SECOND_DOWN && tg.param_size >= 4);
}

//NOT USED
void TouchOverlayController:: OnTG_SecondUp(const TouchGesture & tg,void * call_object)
{
	assert(tg.type == TG_SECOND_UP && tg.param_size >= 4);
}

//NOT USED
void TouchOverlayController::OnTG_SplitStart(const TouchGesture & tg,void * call_object)
{
	assert(tg.type == TG_SPLIT_START && tg.param_size >= 4);
	cout << "  the two fingers are splitting with one finger at: ( " 
		<< tg.params[0] << "," << tg.params[1] << " ),"
		<< " , the other at :( "
		<< tg.params[2] << "," << tg.params[3] << " )" << endl;
}

//NOT USED
void TouchOverlayController::OnTG_SplitEnd(const TouchGesture & tg,void * call_object)
{
	assert(tg.type == TG_SPLIT_END);
}
*/

/*
 * Method Overview: Handles the start touch gesture event
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController:: OnTG_TouchStart(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnTG_TouchStart" << std::endl;
	}

	assert(tg.type == TG_TOUCH_START);

	//if not in draw mode, clears the selected touch region
	if(!(myCommander->getLinesDrawableFlag()))
	{
		roi_extremes.clear();
	}

	last_type = tg.type;
}

/*
 * Method Overview: Handles the received touch gesture events
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController:: OnTouchGesture(const TouchGesture & tg)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnTouchGesture" << std::endl;
	}

	if(TG_NO_ACTION == tg.type)
		return ;
	
	assert(tg.type <= TG_TOUCH_END);
	//DefaultOnTG(tg,this); //Enable if this handler is needed
	PFuncOnTouchGesture pf = m_pf_on_tges[tg.type];
	if(NULL != pf){
		pf(tg,this);
	}
}

/*
 * Method Overview: Handles the received touch_down events
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController:: OnTG_Down(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnTG_Down" << std::endl;
	}

	assert(tg.type == TG_DOWN && tg.param_size >= 2);

	cv::Point2d worldSpacePoint = spaceToWorld(tg.params);
	
	button_clicked = myGUI->clickAnalysis(worldSpacePoint.x,worldSpacePoint.y);

	if(myCommander->getLinesDrawableFlag())
	{
		OpenGLtouchControls(ADD_POINT,annotationCounter, worldSpacePoint.x, worldSpacePoint.y);
	}

	last_type = tg.type;
} 

/*
 * Method Overview: Handles the received click events
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController:: OnTG_Click(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnTG_Click" << std::endl;
	}

	assert(tg.type == TG_CLICK);

	cv::Point2d worldSpacePoint = spaceToWorld(tg.params);

	if(!button_clicked)
	{
		if(!(myCommander->getVirtualAnnotationCreationFlag()))
		{
			//Checks if it is on editing mode
			if(!(myCommander->getLinesDrawableFlag()) && !(myCommander->getPointsDrawableFlag()))
			{
				int annotation_selected_id = myGUI->checkAnnotationSelected(worldSpacePoint.x, worldSpacePoint.y);

				if(annotation_selected_id==-1)
				{
					myCommander->setVirtualAnnotationSelectedFlag(0);

					myCommander->setRoiDrawnFlag(1);

					/*
					 * Initializes the extreme values as:
					 * [0] = MinX. Initizalized as a big value
					 * [1] = MinY. Initizalized as a big value
					 * [2] = MaxX. Initizalized as a small value
					 * [3] = MaxY. Initizalized as a small value
					 */
					roi_extremes.push_back(BIG_VALUE);
					roi_extremes.push_back(BIG_VALUE);
					roi_extremes.push_back(SMALL_VALUE);
					roi_extremes.push_back(SMALL_VALUE);
				}
			}
			//Checks if the annotations panel was clicked
			if (myCommander->getAnnotationPanelShownFlag() && (int)worldSpacePoint.x < OPEN_PANEL_TAB_MIN_X 
				&& (int)worldSpacePoint.y > CLOSED_PANEL_TAB_MAX_X)
			{
				int touchedAnnotationId = myGUI->touchedAnnotationIdentification(worldSpacePoint.x,worldSpacePoint.y);

				if (touchedAnnotationId > 0) {
					selected_annotation_code = touchedAnnotationId;

					myCommander->setVirtualAnnotationCreationFlag(1);

					myCommander->setLinesDrawableFlag(0);
					myCommander->setPointsDrawableFlag(0);
					//myCommander->setRoiDrawnFlag(0);
				}
			}
			//Checks if it is on draw points mode
			else if(myCommander->getPointsDrawableFlag())
			{
				OpenGLtouchControls(CLEAR_LINE,NULL,NULL,NULL);

				OpenGLtouchControls(ADD_POINT_ANNOTATION,annotationCounter, worldSpacePoint.x, worldSpacePoint.y);

				annotationCounter++;
			}
		}
		else
		{
			myGUI->createVirtualAnnotation(annotationCounter, worldSpacePoint.x, worldSpacePoint.y, selected_annotation_code);

			myCommander->setVirtualAnnotationCreationFlag(0);

			annotationCounter++;
		}
	}

	last_type = tg.type;
}

/*
 * Method Overview: Handles the general move gesture events
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController:: OnTG_Move(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnMove" << std::endl;
	}

	assert(tg.type == TG_MOVE && tg.param_size >= 2);

	if (!myCommander->getRealToolPlacedFlag())
	{
		cv::Point2d worldSpacePoint = spaceToWorld(tg.params);

		if (myCommander->getLinesDrawableFlag())
		{
			myCommander->setLineDrawnFlag(1);
			OpenGLtouchControls(ADD_POINT, annotationCounter, worldSpacePoint.x, worldSpacePoint.y);
		}
	}
}

/*
 * Method Overview: Handles the received move_right events
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController:: OnTG_MoveRight(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnTG_MoveRight" << std::endl;
	}

	assert(tg.type == TG_MOVE_RIGHT);

	if (!myCommander->getRealToolPlacedFlag())
	{
		cv::Point2d worldSpacePoint = spaceToWorld(tg.params);

		if (!(myCommander->getLinesDrawableFlag()))
		{
			myCommander->setRoiDrawnFlag(1);
			roi_extremes.push_back(worldSpacePoint.x);
			roi_extremes.push_back(worldSpacePoint.y);
		}
	}

	last_type = tg.type;
}


/*
 * Method Overview: Handles the received move_left events
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController:: OnTG_MoveLeft(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnTG_MoveLeft" << std::endl;
	}

	assert(tg.type == TG_MOVE_LEFT);
	if (!myCommander->getRealToolPlacedFlag())
	{
		cv::Point2d worldSpacePoint = spaceToWorld(tg.params);

		if (!(myCommander->getLinesDrawableFlag()))
		{
			myCommander->setRoiDrawnFlag(1);
			roi_extremes.push_back(worldSpacePoint.x);
			roi_extremes.push_back(worldSpacePoint.y);
		}
	}
	last_type = tg.type;
}

/*
 * Method Overview: Handles the received move_down events
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController::OnTG_MoveDown(const TouchGesture & tg, void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnTG_MoveDown" << std::endl;
	}

	assert(tg.type == TG_MOVE_DOWN);

	if (!myCommander->getRealToolPlacedFlag())
	{
		cv::Point2d worldSpacePoint = spaceToWorld(tg.params);

		if (!(myCommander->getLinesDrawableFlag()))
		{
			myCommander->setRoiDrawnFlag(1);
			roi_extremes.push_back(worldSpacePoint.x);
			roi_extremes.push_back(worldSpacePoint.y);
		}
	}
	last_type = tg.type;
}

/*
 * Method Overview: Handles the received move_up events
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController:: OnTG_MoveUp(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnTG_MoveUp" << std::endl;
	}

	assert(tg.type == TG_MOVE_UP);
	
	if (!myCommander->getRealToolPlacedFlag())
	{
		cv::Point2d worldSpacePoint = spaceToWorld(tg.params);

		if (!(myCommander->getLinesDrawableFlag()))
		{
			myCommander->setRoiDrawnFlag(1);
			roi_extremes.push_back(worldSpacePoint.x);
			roi_extremes.push_back(worldSpacePoint.y);
		}
	}

	last_type = tg.type;
}

/*
 * Method Overview: Handles the received touch end events
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController:: OnTG_TouchEnd(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnTG_TouchEnd" << std::endl;
	}

	assert(tg.type == TG_TOUCH_END);
	
	if(!myCommander->getRealToolPlacedFlag())
	{
		if(last_type == TG_ROTATE_CLOCKWISE || last_type == TG_ROTATE_ANTICLOCKWISE || last_type == TG_SPLIT_APART || 
			last_type == TG_SPLIT_CLOSE || last_type == TG_NEAR_PARALLEL_MOVE_RIGHT || last_type == TG_NEAR_PARALLEL_MOVE_LEFT || 
			last_type == TG_NEAR_PARALLEL_MOVE_UP || last_type == TG_NEAR_PARALLEL_MOVE_DOWN)
		{
			if(myCommander->getVirtualAnnotationSelectedFlag())
			{
				myGUI->startJSONAnnotationUpdate();
			}
			else if(myCommander->getLineSelectedFlag())
			{
				startJSONLineUpdate();
			}
		}

		//is draw mode is on, check if there is any line to draw
		if(myCommander->getLinesDrawableFlag())
		{
			if(myCommander->getLineDrawnFlag())
			{
				OpenGLtouchControls(ADD_LINE,annotationCounter,NULL,NULL);
		
				myCommander->setLineDrawnFlag(0);

				annotationCounter++;
			}
			OpenGLtouchControls(CLEAR_LINE,NULL,NULL,NULL);
		}
		//is draw mode is off, check if there is a line selection roi
		else
		{
			if(myCommander->getRoiDrawnFlag())
			{
				roi_extremes.push_back(roi_extremes.at(0));
				roi_extremes.push_back(roi_extremes.at(1));

				myCommander->setLineSelectedFlag(pointInPolygon(roi_extremes));

				myCommander->setRoiDrawnFlag(0);
			}
		}
	}
	else
	{
		double totalhipDis = getRealToolMean();
		myGUI->interpretRealTool(annotationCounter, totalhipDis, last_param);
		annotationCounter++;
		myCommander->setRealToolPlacedFlag(0);
	}

	last_type = tg.type;
}

/*
 * Method Overview: Handles the received rotate_clockwise events
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController::OnTG_RotateClock(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnTG_RotateClock" << std::endl;
	}

	assert(tg.type == TG_ROTATE_CLOCKWISE);
	
	if (!myCommander->getRealToolPlacedFlag())
	{
		if (myCommander->getVirtualAnnotationSelectedFlag())
		{
			myGUI->GUItouchControls(ROTATE_CLK);
		}
		else if (myCommander->getLineSelectedFlag())
		{
			OpenGLtouchControls(ROTATE_CLK, NULL, NULL, NULL);
		}
	}

	last_type = tg.type;
}

/*
 * Method Overview: Handles the received rotate_anticlockwise event
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController::OnTG_RotateAntiClock(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnTG_RotateAntiClock" << std::endl;
	}

	assert(tg.type == TG_ROTATE_ANTICLOCKWISE);
	
	if (!myCommander->getRealToolPlacedFlag())
	{
		if (myCommander->getVirtualAnnotationSelectedFlag())
		{
			myGUI->GUItouchControls(ROTATE_CNTR_CLK);
		}
		else if (myCommander->getLineSelectedFlag())
		{
			OpenGLtouchControls(ROTATE_CNTR_CLK, NULL, NULL, NULL);
		}
	}

	last_type = tg.type;
}

/*
 * Method Overview: Handles the received split_apart events
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController::OnTG_SplitApart(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnTG_SplitApart" << std::endl;
	}


	assert(tg.type == TG_SPLIT_APART && tg.param_size >= 1);
	
	if (!myCommander->getRealToolPlacedFlag())
	{
		if (myCommander->getVirtualAnnotationSelectedFlag())
		{
			myGUI->GUItouchControls(ZOOM_IN);
		}
		else if (myCommander->getLineSelectedFlag())
		{
			OpenGLtouchControls(ZOOM_IN, NULL, NULL, NULL);
		}
	}

	last_type = tg.type;
}

/*
 * Method Overview: Handles the received split_close events
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController::OnTG_SplitClose(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "OnTG_SplitClose" << std::endl;
	}

	assert(tg.type == TG_SPLIT_CLOSE && tg.param_size >= 1);
	
	if (!myCommander->getRealToolPlacedFlag())
	{
		if (myCommander->getVirtualAnnotationSelectedFlag())
		{
			myGUI->GUItouchControls(ZOOM_OUT);
		}
		else if (myCommander->getLineSelectedFlag())
		{
			OpenGLtouchControls(ZOOM_OUT, NULL, NULL, NULL);
		}
	}

	last_type = tg.type;
}

/*
 * Method Overview: Handles the received parrell_move_right events
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController::onTG_NearParrellMoveRight(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "onTG_NearParrellMoveRight" << std::endl;
	}

	assert(tg.type == TG_NEAR_PARALLEL_MOVE_RIGHT);
	
	if (!myCommander->getRealToolPlacedFlag())
	{
		if (myCommander->getVirtualAnnotationSelectedFlag())
		{
			myGUI->GUItouchControls(TRANSLATE_RIGHT);
		}
		else if (myCommander->getLineSelectedFlag())
		{
			OpenGLtouchControls(TRANSLATE_RIGHT, NULL, NULL, NULL);
		}
	}

	last_type = tg.type;
}

/*
 * Method Overview: Handles the received parrell_move_left events
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController::onTG_NearParrellMoveLeft(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "onTG_NearParrellMoveLeft" << std::endl;
	}

	assert(tg.type == TG_NEAR_PARALLEL_MOVE_LEFT);
	
	if (!myCommander->getRealToolPlacedFlag())
	{
		if (myCommander->getVirtualAnnotationSelectedFlag())
		{
			myGUI->GUItouchControls(TRANSLATE_LEFT);
		}
		else if (myCommander->getLineSelectedFlag())
		{
			OpenGLtouchControls(TRANSLATE_LEFT, NULL, NULL, NULL);
		}
	}

	last_type = tg.type;

}

/*
 * Method Overview: Handles the received parrell_move_up events
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController::onTG_NearParrellMoveUp(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "onTG_NearParrellMoveUp" << std::endl;
	}

	assert(tg.type == TG_NEAR_PARALLEL_MOVE_UP);

	if (!myCommander->getRealToolPlacedFlag())
	{
		if (myCommander->getVirtualAnnotationSelectedFlag())
		{
			myGUI->GUItouchControls(TRANSLATE_UP);
		}
		else if (myCommander->getLineSelectedFlag())
		{
			OpenGLtouchControls(TRANSLATE_UP, NULL, NULL, NULL);
		}
	}

	last_type = tg.type;
}

/*
 * Method Overview: Handles the received parrell_move_down events
 * Parameters: Received touch gesture
 * Return: None
 */
void TouchOverlayController::onTG_NearParrellMoveDown(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "onTG_NearParrellMoveDown" << std::endl;
	}

	assert(tg.type == TG_NEAR_PARALLEL_MOVE_DOWN);
	
	if (!myCommander->getRealToolPlacedFlag())
	{
		if (myCommander->getVirtualAnnotationSelectedFlag())
		{
			myGUI->GUItouchControls(TRANSLATE_DOWN);
		}
		else if (myCommander->getLineSelectedFlag())
		{
			OpenGLtouchControls(TRANSLATE_DOWN, NULL, NULL, NULL);
		}
	}

	last_type = tg.type;
}

/*
 * Method Overview: Handles the received multi down events
 * Parameters: Received multi touch gesture
 * Return: None
 */
void TouchOverlayController::onTG_MultiDown(const TouchGesture & tg,void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "onTG_MultiDown" << std::endl;
	}

	const double* data = tg.params;
	
	const double params1[2] = { tg.params[2],tg.params[3] };
	const double params2[2] = { tg.params[4],tg.params[5] };

	double mininumDif = 10.0;
	double dx = abs(params1[0] - params2[0]);
	double dy = abs(params1[1] - params2[1]);

	if(dx>mininumDif && dy>mininumDif)
	{
		cv::Point2d worldSpacePoint1 = spaceToWorld(params1);
		cv::Point2d worldSpacePoint2 = spaceToWorld(params2);

		double hipDis = sqrt((dx * dx) + (dy * dy));
		//cout << hipDis << endl;
		real_element_features.push_back(hipDis);
		last_param[0] = params2[0];
		last_param[1] = params2[1];
		myCommander->setRealToolPlacedFlag(1);
	}
	last_type = tg.type;
}

/*
* Method Overview: Handles the received multi move events
* Parameters: Received multi touch gesture
* Return: None
*/
void TouchOverlayController::onTG_MultiMove(const TouchGesture & tg, void * call_object)
{
	if (TouchOverlayController::debugMessagesEnabled) {
		std::cout << "onTG_MultiMove" << std::endl;
	}

	const double* data = tg.params;

	const double params1[2] = { tg.params[2],tg.params[3] };
	const double params2[2] = { tg.params[4],tg.params[5] };
	
	double mininumDif = 10.0;
	double dx = abs(params1[0] - params2[0]);
	double dy = abs(params1[1] - params2[1]);

	if (dx>mininumDif && dy>mininumDif)
	{
		cv::Point2d worldSpacePoint1 = spaceToWorld(params1);
		cv::Point2d worldSpacePoint2 = spaceToWorld(params2);

		double hipDis = sqrt((dx * dx) + (dy * dy));
		real_element_features.push_back(hipDis);
		last_param[0] = params2[0];
		last_param[1] = params2[1];
		myCommander->setRealToolPlacedFlag(1);
	}
	last_type = tg.type;
}

double TouchOverlayController::getRealToolMean()
{
	int i;
	double biggest = 0;
	double mean = 0;

	for (i = 0; i < real_element_features.size(); i++)
	{
		if (real_element_features[i] > biggest)
		{
			biggest = real_element_features[i];
		}
	}

	double difRate = biggest / 20.0;
	int counter = 0;

	for (i = 0; i < real_element_features.size(); i++)
	{
		if (abs(biggest - real_element_features[i]) <= difRate)
		{
			mean += real_element_features[i];
			counter++;
		}
	}

	mean /= counter;

	double this_hyp = sqrt((SERVER_RESOLUTION_X * SERVER_RESOLUTION_X) + (SERVER_RESOLUTION_Y * SERVER_RESOLUTION_Y));
	double this_cm_dif = (this_hyp * REF_CM_IN_PIX) / REF_HYP;

	cout << mean/this_cm_dif << endl;

	real_element_features.clear();

	return mean;
}

cv::Point2d TouchOverlayController::spaceToWorld(const double* spaceCoord)
{
	double posX = spaceCoord[0];
	double posY = spaceCoord[1];

	// the click analysis assumes that the incoming click was on a (GUI_MEASURED_RESOLUTION_X, GUI_MEASURED_RESOLUTION_Y) screen
	// but it may not be. So convert it.
	posX = posX * (double)GUI_MEASURED_RESOLUTION_X / (double)SERVER_RESOLUTION_X;
	posY = posY * (double)GUI_MEASURED_RESOLUTION_Y / (double)SERVER_RESOLUTION_Y;
	
	// input X/Y are in screen-space
	// we need to convert to world-space to be able to manipulate annotations the way we want to
	cv::Point2d worldSpacePoint = myCamera->convertScreenSpaceToWorldSpace(posX, posY);

	return worldSpacePoint;
}