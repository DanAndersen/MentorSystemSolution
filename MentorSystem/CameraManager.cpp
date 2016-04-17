#include "CameraManager.h"

const float CameraManager::CAMERA_SCALE_DELTA = 0.05;
const float CameraManager::CAMERA_TRANSLATION_DELTA = 20;
const float CameraManager::CAMERA_ROTATION_DELTA = 5;

CameraManager::CameraManager()
	: _homographyDataType(CV_64F)
	, _homography(3, 3, _homographyDataType)
	, _inverseHomography(3, 3, _homographyDataType)
{
	std::cout << "initing CameraManager" << std::endl;

	reset();
}


CameraManager::~CameraManager()
{
}

cv::Point2d CameraManager::convertScreenSpaceToWorldSpace(long double x, long double y) {
	cv::Mat_<double> srcPoint(3, 1, _inverseHomography.type());
	srcPoint(0, 0) = x;
	srcPoint(1, 0) = SERVER_RESOLUTION_Y - y;
	srcPoint(2, 0) = 1.0;

	cv::Mat_<double> dstPoint = _inverseHomography * srcPoint;

	return cv::Point2d(dstPoint(0, 0), SERVER_RESOLUTION_Y - dstPoint(1, 0));
}

cv::Point2d CameraManager::convertWorldSpaceToScreenSpace(long double x, long double y) {
	cv::Mat_<double> srcPoint(3, 1, _homography.type());
	srcPoint(0, 0) = x;
	srcPoint(1, 0) = y;
	srcPoint(2, 0) = 1.0;

	cv::Mat_<double> dstPoint = _homography * srcPoint;

	return cv::Point2d(dstPoint(0, 0), dstPoint(1, 0));
}

cv::Mat CameraManager::getHomography() {
	return _homography;
}

cv::Mat CameraManager::getInverseHomography() {
	return _inverseHomography;
}

void CameraManager::updateHomography() {

	cv::Mat matrixRotationAboutImageCenter = cv::getRotationMatrix2D(cv::Point2f(SERVER_RESOLUTION_X / 2.0, SERVER_RESOLUTION_Y / 2.0), _thetaDegrees, _scale);

	cv::Mat row1 = cv::Mat::zeros(1, 3, matrixRotationAboutImageCenter.type());
	row1.at<double>(0, 2) = 1;
	
	matrixRotationAboutImageCenter.push_back(row1);






	// matrix to move screen so image center is at origin
	cv::Mat matrixTranslation = cv::Mat::eye(3, 3, matrixRotationAboutImageCenter.type());
	matrixTranslation.at<double>(0, 2) = _translation.x;
	matrixTranslation.at<double>(1, 2) = _translation.y;

	_homography = matrixTranslation * matrixRotationAboutImageCenter;
	_inverseHomography = _homography.inv();
}

void CameraManager::reset() {
	_scale = 1.0;

	_thetaDegrees = 0.0;

	_translation.x = 0.0;
	_translation.y = 0.0;

	updateHomography();
}

bool CameraManager::handleKey(unsigned char key) {

	bool consumed = true;

	switch (key)
	{
	case CAMERA_KEY_MOVE_LEFT:

		_translation.x -= CAMERA_TRANSLATION_DELTA;

		break;
	case CAMERA_KEY_MOVE_RIGHT:

		_translation.x += CAMERA_TRANSLATION_DELTA;

		break;
	case CAMERA_KEY_MOVE_UP:

		_translation.y += CAMERA_TRANSLATION_DELTA;

		break;
	case CAMERA_KEY_MOVE_DOWN:

		_translation.y -= CAMERA_TRANSLATION_DELTA;

		break;
	case CAMERA_KEY_ZOOM_IN:

		_scale += CAMERA_SCALE_DELTA;

		break;
	case CAMERA_KEY_ZOOM_OUT:

		_scale -= CAMERA_SCALE_DELTA;

		break;
	case CAMERA_KEY_ROTATE_CCW:

		_thetaDegrees -= CAMERA_ROTATION_DELTA;

		break;
	case CAMERA_KEY_ROTATE_CW:

		_thetaDegrees += CAMERA_ROTATION_DELTA;

		break;
	case CAMERA_KEY_RESET:

		reset();

		break;
	default:
		consumed = false;
		break;
	}

	if (consumed) {
		updateHomography();
	}

	return consumed;
}