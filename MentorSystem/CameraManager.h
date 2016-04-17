#pragma once

/*

The CameraManager maintains the state of the virtual camera; i.e. the 
transformations that happen to world-space elements like the video frame and 
annotations before they are drawn to the screen. It also uses this
transformation to interpret input UI interactions in screen-space as 
world-space queries.

By default, the input frame from the trainee fills the camera view on the mentor side, with this coordinate system:

(0,0) ----------------- (w,0)
|                           |
|                           |
|                           |
|                           |
|                           |
(0,h) ----------------- (w,h)

The internal representation of annotations is in a world-space coordinate system that matches this. 

When displaying the frame and annotations to the screen, the world-space coordinates are converted to screen-space coordinates,
using a homography that is updated by use of the camera controls (keyboard and gesture controls).

The GUI is not affected by this, as it is always in screen-space.

When interpreting input from the user (via mouse or touch controls), if the input is not consumed by the GUI then it is converted
from screen-space back to world-space coordinates and then allowed to update annotations.

The camera can be controlled using the keyboard's keypad. NOTE: Num Lock must be turned on for the keys to function:

+--------+--------+--------+
|    7   |    8   |    9   |
|        |        |        |
+--------+--------+--------+
|    4   |    5   |   6    |
|        |        |        |
+--------+--------+--------+
|    1   |   2    |    3   |
|        |        |        |
+--------+--------+--------+


+--------+--------+--------+
|  zoom  |  move  |  zoom  |
|  out   |  up    |   in   |
+--------+--------+--------+
|  move  |  reset |  move  |
|  left  |        |  right |
+--------+--------+--------+
| rotate |  move  | rotate |
|  ccw   |  down  |   cw   |
+--------+--------+--------+

*/

#include <iostream>
#include <opencv2/opencv.hpp>
#include "Config.h"



class CameraManager
{
public:
	CameraManager();
	~CameraManager();

	// Handles keyboard input for camera controls.
	// Updates homography if camera control was provided.
	// Returns true if a camera control key was pressed, false otherwise;
	bool handleKey(unsigned char key);

	cv::Mat getHomography();	// convert from world space to screen space
	cv::Mat getInverseHomography();	// convert from screen space to world space

	cv::Point2d convertScreenSpaceToWorldSpace(long double x, long double y);
	cv::Point2d convertWorldSpaceToScreenSpace(long double x, long double y);

private:
	static const unsigned char CAMERA_KEY_ZOOM_OUT = '7';
	static const unsigned char CAMERA_KEY_MOVE_UP = '8';
	static const unsigned char CAMERA_KEY_ZOOM_IN = '9';
	static const unsigned char CAMERA_KEY_MOVE_LEFT = '4';
	static const unsigned char CAMERA_KEY_RESET = '5';
	static const unsigned char CAMERA_KEY_MOVE_RIGHT = '6';
	static const unsigned char CAMERA_KEY_ROTATE_CCW = '1';
	static const unsigned char CAMERA_KEY_MOVE_DOWN = '2';
	static const unsigned char CAMERA_KEY_ROTATE_CW = '3';

	float _scale;	// zoom in/out factor
	float _thetaDegrees; // rotation ccw
	cv::Point2f _translation; // translation from default point

	cv::Mat _homography;
	cv::Mat _inverseHomography;

	void reset();

	void updateHomography();

	const int _homographyDataType;

	static const float CAMERA_SCALE_DELTA;
	static const float CAMERA_TRANSLATION_DELTA;
	static const float CAMERA_ROTATION_DELTA;
};

