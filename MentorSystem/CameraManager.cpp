#include "CameraManager.h"

const float CameraManager::CAMERA_SCALE_DELTA = 0.1;
const float CameraManager::CAMERA_TRANSLATION_DELTA = 10;
const float CameraManager::CAMERA_ROTATION_DELTA = 0.1;

CameraManager::CameraManager()
	: _homographyDataType(CV_64F)
	, _homography(3, 3, _homographyDataType)
{
	std::cout << "initing CameraManager" << std::endl;

	reset();
}


CameraManager::~CameraManager()
{
}

void CameraManager::updateHomography() {
	std::cout << "updateHomography" << std::endl;

	std::cout << "_translation.x: " << _translation.x << std::endl;
	std::cout << "_translation.y: " << _translation.y << std::endl;


	cv::Mat matrixRotationAboutImageCenter = cv::getRotationMatrix2D(cv::Point2f(SERVER_RESOLUTION_X / 2.0, SERVER_RESOLUTION_Y / 2.0), _thetaRadians, _scale);

	cv::Mat row1 = cv::Mat::zeros(1, 3, matrixRotationAboutImageCenter.type());
	row1.at<double>(0, 2) = 1;
	
	matrixRotationAboutImageCenter.push_back(row1);






	// matrix to move screen so image center is at origin
	cv::Mat matrixTranslation = cv::Mat::eye(3, 3, matrixRotationAboutImageCenter.type());
	matrixTranslation.at<double>(0, 2) = _translation.x;
	matrixTranslation.at<double>(1, 2) = _translation.y;

	std::cout << "matrixRotationAboutImageCenter: " << matrixRotationAboutImageCenter << std::endl;

	

	std::cout << "matrixTranslation: " << matrixTranslation << std::endl;


	_homography = matrixTranslation * matrixRotationAboutImageCenter;

	std::cout << "_homography: " << _homography << std::endl;
}

void CameraManager::reset() {
	_scale = 1.0;

	_thetaRadians = 0.0;

	_translation.x = 0.0;
	_translation.y = 0.0;

	updateHomography();
}

bool CameraManager::handleKey(unsigned char key) {

	bool consumed = true;

	switch (key)
	{
	case CAMERA_KEY_MOVE_LEFT:
		std::cout << "todo: move left" << std::endl;

		_translation.x -= CAMERA_TRANSLATION_DELTA;

		break;
	case CAMERA_KEY_MOVE_RIGHT:
		std::cout << "todo: move right" << std::endl;

		_translation.x += CAMERA_TRANSLATION_DELTA;

		break;
	case CAMERA_KEY_MOVE_UP:
		std::cout << "todo: move up" << std::endl;

		_translation.y -= CAMERA_TRANSLATION_DELTA;

		break;
	case CAMERA_KEY_MOVE_DOWN:
		std::cout << "todo: move down" << std::endl;

		_translation.y += CAMERA_TRANSLATION_DELTA;

		break;
	case CAMERA_KEY_ZOOM_IN:
		std::cout << "todo: zoom in" << std::endl;

		_scale += CAMERA_SCALE_DELTA;

		break;
	case CAMERA_KEY_ZOOM_OUT:
		std::cout << "todo: zoom out" << std::endl;

		_scale -= CAMERA_SCALE_DELTA;

		break;
	case CAMERA_KEY_ROTATE_CCW:
		std::cout << "todo: rotate ccw" << std::endl;

		_thetaRadians += CAMERA_ROTATION_DELTA;

		break;
	case CAMERA_KEY_ROTATE_CW:
		std::cout << "todo: rotate cw" << std::endl;

		_thetaRadians -= CAMERA_ROTATION_DELTA;

		break;
	case CAMERA_KEY_RESET:
		std::cout << "todo: reset" << std::endl;

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