#include <math.h>
#include <stdio.h>

#include "LinearAlgebra.hpp"
#include "mycamera.hpp"


/**
 * Constructs a camera with standard 500 far and 60 fov.
 */
MyCamera::MyCamera() {
	pos = Vector4(0, 0, 2, 1);
	refPoint = Vector4(0, 0, 0, 1);
	upVec = Vector4(0, 1, 0, 0);

	far = 500;
	fov = 60;

	
}

/**
 * Construct a camera with given far and fov.
 */
MyCamera::MyCamera(float farDistance, float fovAngle) {
	pos = Vector4(0, 0, 2, 1);
	refPoint = Vector4(0, 0, 0, 1);
	upVec = Vector4(0, 1, 0, 0);

	far = farDistance;
	fov = fovAngle;
}

MyCamera::~MyCamera() {
}

/**
 * Gets a forward vector, upvector and right vector then constructs the view matrix from them.
 * @return a new view matrix
 */
Matrix MyCamera::getViewMatrix() {

	// Algorithm copied from glm::lookAt

	Vector3 worldUp = Vector3(upVec.vec[0], upVec.vec[1], upVec.vec[2]);
	Vector3 reference3d = Vector3(refPoint.vec[0], refPoint.vec[1], refPoint.vec[2]);
	Vector3 position3d = Vector3(pos.vec[0], pos.vec[1], pos.vec[2]);

	Vector3 forward = (position3d - reference3d).normalize();
	Vector3 right = worldUp.cross(forward).normalize();
	Vector3 up = forward.cross(right).normalize();

	Matrix result = Matrix();
	result.mat[0] = right.vec[0];
	result.mat[1] = right.vec[1];
	result.mat[2] = right.vec[2];
	
	result.mat[4] = up.vec[0];
	result.mat[5] = up.vec[1];
	result.mat[6] = up.vec[2];
	
	result.mat[8] = forward.vec[0];
	result.mat[9] = forward.vec[1];
	result.mat[10] = forward.vec[2];

	result.mat[3] = -(right * position3d);
	result.mat[7] = -(up * position3d);
	result.mat[11] = -(forward * position3d);

	return result;
}

/**
 * Constructs the projection/perspectiv matrix from the fov and the near and far planes.
 * @return a new projection/perspectiv matrix
 */
Matrix MyCamera::getProjectionMatrix() {
	if (parallellPerspective) {
		Matrix temp = Matrix(
			1/(top*aspectRatio),0,obliqueScale*cos(obliqueAngleRad),0,
			0,1/top,obliqueScale*sin(obliqueAngleRad),0,
			0,0,-1/(far-near),0,
			0,0,0,1
		);
		return temp;

	} else {
		float halfTanFOV = tan((fov/2)*(M_PI/180));
		Matrix temp = Matrix(
			(1/halfTanFOV)/aspectRatio, 0, 0, 0,
			0, 1/halfTanFOV, 0, 0,
			0, 0, -(far+near)/(far-near), (-2*far*near)/(far-near),
			0, 0, -1, 0
		);
		return temp;		
	}
}

/**
 * Moves the camera along its x,y,z axels relative to where it is looking.
 */
void MyCamera::move(float x, float y, float z) {
	// Forward vector
	Vector4 lookVec = refPoint - pos;

	Vector3 look3 = Vector3(lookVec.vec[0], lookVec.vec[1], lookVec.vec[2]).normalize();
	Vector3 up3 = Vector3(upVec.vec[0], upVec.vec[1], upVec.vec[2]).normalize();
	Vector3 right3 = look3.cross(up3).normalize();
	 
	Vector4 movement = Vector4(right3,0)*x + upVec*y + Vector4(look3,0)*z;
	pos = pos + movement;
	refPoint = refPoint + movement;
}

void MyCamera::updateView(float fovAngle, float farDistance, float planeTop, float oScale, float oAngleRad, int proj_current_idx, float ratio) {
	far = farDistance;
	fov = fovAngle;
	top = planeTop;
	obliqueScale = oScale;
	obliqueAngleRad = oAngleRad;
	parallellPerspective = proj_current_idx;
	aspectRatio = ratio;
}

void MyCamera::rotate(float deltaX, float deltaY) {
	
	Matrix rotationMatrix = Matrix();

	rotationMatrix.translate(pos.vec[0], pos.vec[1], pos.vec[2]);
	rotationMatrix.rotatey(-deltaX);

	// Forward vector
	Vector4 lookVec = refPoint - pos;
	Vector3 look3 = Vector3(lookVec.vec[0], lookVec.vec[1], lookVec.vec[2]).normalize();
	Vector3 up3 = Vector3(upVec.vec[0], upVec.vec[1], upVec.vec[2]).normalize();
	Vector3 right3 = look3.cross(up3).normalize();

	rotationMatrix.rotateAroundAxis(right3, deltaY);
	rotationMatrix.translate(-pos.vec[0], -pos.vec[1], -pos.vec[2]);

	refPoint = rotationMatrix * refPoint;
}

Vector3 MyCamera::getPosition() {
	return Vector3(pos.vec[0], pos.vec[1], pos.vec[2]);
}