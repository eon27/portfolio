#include <math.h>
#include <stdio.h>

#include "LinearAlgebra.hpp"

/**
 * Constructs a 4x4 identity matrix.
 */
Matrix::Matrix() {
    float temp_mat[16] = {
        1.0,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
    for (int i = 0; i < 16; i++) {
        mat[i] = temp_mat[i];
    }
}

/**
 * Constructs a matrix with the given values in row major form.
 */
Matrix::Matrix(float a0, float a1, float a2, float a3, float a4, float a5, float a6, float a7, float a8, float a9, float a10, float a11, float a12, float a13, float a14, float a15) {
    mat[0] = a0;
    mat[1] = a1;
    mat[2] = a2;
    mat[3] = a3;

    mat[4] = a4;
    mat[5] = a5;
    mat[6] = a6;
    mat[7] = a7;

    mat[8] = a8;
    mat[9] = a9;
    mat[10] = a10;
    mat[11] = a11;

    mat[12] = a12;
    mat[13] = a13;
    mat[14] = a14;
    mat[15] = a15;
}

/**
 * Constructs a matrix in row major form from the given array.
 */
Matrix::Matrix(float arr[16]) {
    for (int i = 0; i < 16; i++) {
        mat[i] = arr[i];
    }
}

/**
 * Constructs a matrix in row major form from te given array.
 */
Matrix::Matrix(float temp_mat[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            mat[(i*4)+j] = temp_mat[i][j];
        }
    }
}

Matrix::Matrix(Vector4 v1, Vector4 v2, Vector4 v3, Vector4 v4) {
    mat[0] = v1.vec[0];
    mat[1] = v2.vec[0];
    mat[2] = v3.vec[0];
    mat[3] = v4.vec[0];
    
    mat[4] = v1.vec[1];
    mat[5] = v2.vec[1];
    mat[6] = v3.vec[1];
    mat[7] = v4.vec[1];
    
    mat[8] = v1.vec[2];
    mat[9] = v2.vec[2];
    mat[10] = v3.vec[2];
    mat[11] = v4.vec[2];

    mat[12] = v1.vec[3];
    mat[13] = v2.vec[3];
    mat[14] = v3.vec[3];
    mat[15] = v4.vec[3];
}

/**
 * Multiplies the left matrix with the right matrix, order matters.
 * @param rhs a reference to the right matrix
 * @return a new matrix as the result.
 */
Matrix Matrix::operator*(const Matrix& rhs) {
    float result_matrix[4][4] = {
		{0,0,0,0},
		{0,0,0,0},
        {0,0,0,0},
        {0,0,0,0}
    };
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            for (int i = 0; i < 4; i++) {
                result_matrix[r][c] += this->mat[(r*4)+i] * rhs.mat[(i*4)+c]; 
            }
        }
    }
    Matrix result(result_matrix);
    return result;
}

/**
 * Translates the matrix in the given x,y,z directions.
 * Moddifies the current matrix.
 */
void Matrix::translate(float dx, float dy, float dz) {
    float xScale = 1;
    float yScale = 1;
    float zScale = 1;
    // if (mat[0] != 0) xScale = mat[0];
    // if (mat[5] != 0) yScale = mat[5];
    // if (mat[10] != 0) zScale = mat[10];
    float translation_array[4][4] = {
        {1,0,0,dx/xScale},
        {0,1,0,dy/yScale},
        {0,0,1,dz/zScale},
        {0,0,0,1}};
    Matrix translation_matrix(translation_array);
    
    *this = (*this) * translation_matrix;
}
/**
 * Scales the matrix in x,y,z direction.
 * Moddifies the current matrix.
 * @param sx amount to scale in the x-direction with.
 * @param sy amount to scale in the y-direction with.
 * @param sz amount to scale in the z-direction with.
 */
void Matrix::scale(float sx, float sy, float sz) {
    float scale_array[4][4] = {{sx,0,0,0},{0,sy,0,0},{0,0,sz,0},{0,0,0,1}};
    Matrix scale_matrix(scale_array);
    
    *this = (*this) * scale_matrix;
}

Matrix Matrix::transpose() {
    Matrix temp = Matrix();
    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = 0; j < 4; j++)
        {
            temp.mat[(4*i)+j] = mat[(4*j)+i];
        }
        
    }
    return temp;
}

#define SMALLNUMBER 0.000001
/**
 * Rounds all the small numbers in the matrix to 0.
 * @param number numbers smaller than this are rounded to 0
 */
void Matrix::floatingpointError(float number = SMALLNUMBER) {
    for (size_t i = 0; i < 16; i++) {
        if (abs(mat[i]) < SMALLNUMBER) mat[i] = 0;
    }
}
        
/**
 * Rotates the matrix around the x-axis by a given amount of radians.
 * Moddifies the current matrix
 * @param a the radian amount to rotate.
 */
void Matrix::rotatex(float a) {
    float rotation_array[4][4] = {
        {1,0,0,0},
        {0,cos(a),-sin(a),0},
        {0,sin(a),cos(a),0},
        {0,0,0,1}};
    Matrix rotation_matrix(rotation_array);
    
    *this = (*this) * rotation_matrix;
    // Because cos and sin operations may result in small numbers we round the matrix.
    floatingpointError();
}
 
/**
 * Rotates the matrix around the y-axis by a given amount of radians.
 * Moddifies the current matrix
 * @param a the radian amount to rotate.
 */
void Matrix::rotatey(float a) {
    float rotation_array[4][4] = {
        {cos(a),0,-sin(a),0},
        {0,1,0,0},
        {sin(a),0,cos(a),0},
        {0,0,0,1}};
    Matrix rotation_matrix(rotation_array);

    *this = (*this) * rotation_matrix;
    // Because cos and sin operations may result in small numbers we round the matrix.
    floatingpointError();
}

/**
 * Rotates the matrix around the z-axis by a given amount of radians.
 * Moddifies the current matrix
 * @param a the radian amount to rotate.
 */
void Matrix::rotatez(float a) {
    float rotation_array[4][4] = {
        {cos(a),-sin(a),0,0},
        {sin(a),cos(a),0,0},
        {0,0,1,0},
        {0,0,0,1}};
    Matrix rotation_matrix(rotation_array);

    *this = (*this) * rotation_matrix;
    // Because cos and sin operations may result in small numbers we round the matrix.
    floatingpointError();
}

void Matrix::rotateAroundAxis(const Vector3& axis, float a) {

	float const c = cos(a);
    const float c1 = 1.0f - c;
	float const s = sin(a);
	
	Vector3 temp = Vector3(axis * c1);
	
	Matrix rotate = Matrix();
	
	rotate.mat[0] = c + temp.vec[0] * axis.vec[0];
	rotate.mat[1] = temp.vec[0] * axis.vec[1] + s * axis.vec[2];
	rotate.mat[2] = temp.vec[0] * axis.vec[2] - s * axis.vec[1];

	rotate.mat[4] = temp.vec[1] * axis.vec[0] - s * axis.vec[2];
	rotate.mat[5] = c + temp.vec[1] * axis.vec[1];
	rotate.mat[6] = temp.vec[1] * axis.vec[2] + s * axis.vec[0];
	
	rotate.mat[8] = temp.vec[2] * axis.vec[0] + s * axis.vec[1];
	rotate.mat[9] = temp.vec[1] * axis.vec[1] - s * axis.vec[0];
	rotate.mat[10] = c + temp.vec[2] * axis.vec[2];

    *this = (*this) * rotate;
}

/**
 * Multiplies the left matrix with the right matrix, order matters.
 * @param rhs referens to the right matrix
 * @return a new matrix as the result
 */
Vector4 Matrix::operator*(const Vector4& rhs) {
	float vector[4] = {0,0,0,0};
	for (int r = 0; r < 4; r++) {
		for (int c = 0; c < 4; c++) {
			vector[r] += this->mat[(r*4)+c] * rhs.vec[c];
		}
	}
	Vector4 result(vector);
	return result;
}

/**
 * Adds the left matrix and the right matrix.
 * @param rhs referens to the right matrix
 * @return a new matrix as the result
 */
Matrix Matrix::operator+(const Matrix& rhs) {
	float result_matrix[16];
    for (int i = 0; i < 16; i++) {
        result_matrix[i] = this->mat[i] + rhs.mat[i]; 
    }
    Matrix result(result_matrix);
    return result;
}

/**
 * Subtracts the right matrix from the left matrix.
 * @param rhs referens to the right matrix
 * @return a new matrix as the result
 */
Matrix Matrix::operator-(const Matrix& rhs) {
	float result_matrix[16];
    for (int i = 0; i < 16; i++) {
        result_matrix[i] = this->mat[i] - rhs.mat[i]; 
    }
    Matrix result(result_matrix);
    return result;
}

void Matrix::printMatrix() {
    for (int r = 0; r < 4; ++r) {
        printf("%f, %f, %f, %f\n",
            mat[(r*4)],
            mat[(r*4)+1],
            mat[(r*4)+2],
            mat[(r*4)+3]
        );
    }
}

/**
 * Constructs the vector (0,0,0).
 */
Vector3::Vector3() {
    for (int i = 0; i < 3; i++){
        vec[i] = 0;
    }
}

/**
 * Constructs a vector from the given values.
 * @param x constructed vectors x component
 * @param y constructed vectors y component
 * @param z constructed vectors z component
 */
Vector3::Vector3(float x, float y, float z) {
    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
}

/**
 * Constructs a vector from the given array.
 * @param vector an array with the x,y and z component of the vector
 */
Vector3::Vector3(double vector[3]) {
    vec[0] = (float)vector[0];
    vec[1] = (float)vector[1];
    vec[2] = (float)vector[2];
}

/**
 * Multiplies the left vector with the right vector, order doesn't matter.
 * @param rhs reference to the right vector
 * @return the single value result
 */
float Vector3::operator*(const Vector3& rhs) {
    return (vec[0] * rhs.vec[0] + vec[1] * rhs.vec[1] + vec[2] * rhs.vec[2]);
}

Vector3 Vector3::operator*(const float& rhs) const {
    Vector3 temp = Vector3(vec[0]*rhs, vec[1]*rhs, vec[2]*rhs);
    return temp;
}

/**
 * Cross multiplication between the left and the right vectors.
 * @param lhs reference to the left vector
 * @param rhs reference to the right vector
 * @returns a new vector as the result
 */
Vector3 Vector3::cross(const Vector3& lhs, const Vector3& rhs) {
	float row1 = lhs.vec[1] * rhs.vec[2] - lhs.vec[2] * rhs.vec[1];
	float row2 = lhs.vec[2] * rhs.vec[0] - lhs.vec[0] * rhs.vec[2];
	float row3 = lhs.vec[0] * rhs.vec[1] - lhs.vec[1] * rhs.vec[0];
	Vector3 result({row1, row2, row3});
	return result;
}

/**
 * Cross multiplication between this/the left vector and the the right vector.
 * @param rhs reference to the right vector
 * @returns a new vector the is the result of the cross multiplication
 */
Vector3 Vector3::cross(const Vector3& rhs) {
	float row1 = this->vec[1] * rhs.vec[2] - this->vec[2] * rhs.vec[1];
	float row2 = this->vec[2] * rhs.vec[0] - this->vec[0] * rhs.vec[2];
	float row3 = this->vec[0] * rhs.vec[1] - this->vec[1] * rhs.vec[0];
	Vector3 result({row1, row2, row3});
	return result;
}

/**
 * Adds the x,y,z components in the left with the respective components in the right vector.
 * @param rhs reference to the right vector.
 * @return a new vector as the result of the addition
 */
Vector3 Vector3::operator+(const Vector3& rhs) {
	Vector3 result({this->vec[0] + rhs.vec[0], this->vec[1] + rhs.vec[1], this->vec[2] + rhs.vec[2]});
	return result;
}

/**
 * Subtracts the x,y,z components in the left vector by the x,y,z components in the right vector.
 * @param rhs reference to the right vector
 * @return a vector as the result
 */
Vector3 Vector3::operator-(const Vector3& rhs) {
	Vector3 result({this->vec[0] - rhs.vec[0], this->vec[1] - rhs.vec[1], this->vec[2] - rhs.vec[2]});
	return result;
}

/**
 * Finds the length of the vector with pythagoran theorem and then divides the x,y,z components by the length.
 * @return a new vector that is the normalization result
 */
Vector3 Vector3::normalize() {
    Vector3 temp = Vector3(vec[0], vec[1], vec[2]);
    float length = sqrt((temp.vec[0] * temp.vec[0]) + (temp.vec[1] * temp.vec[1]) + (temp.vec[2] * temp.vec[2]));
    if (length) {
        temp.vec[0] = temp.vec[0] / length;
        temp.vec[1] = temp.vec[1] / length;
        temp.vec[2] = temp.vec[2] / length;
    }
    return temp;
}

float Vector3::length() {
    return sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

/**
 * Constructs a (0,0,0,0) vector.
 */
Vector4::Vector4() {
    for (int i = 0; i < 4; i++){
        vec[i] = 0;
    }
}

/**
 * Constructs a vector (x, y, z, 1).
 * @param x first value
 * @param y second value
 * @param z third value
 */
Vector4::Vector4(float x, float y, float z) {
    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
    vec[3] = 1;
}

/**
 * Constructs a new vector by taking a vector and copies the values into the first three of the new vector and takes an argument for the forth value.
 * @param vector 3d vector with x,y,z values for the new vector
 * @param w the forth value of the vector
 */
Vector4::Vector4(Vector3 vector, float w) {
    vec[0] = vector.vec[0];
    vec[1] = vector.vec[1];
    vec[2] = vector.vec[2];
    vec[3] = w;
}

/**
 * Constructs a new vector with the given values.
 */
Vector4::Vector4(float x, float y, float z, float w) {
    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
    vec[3] = w;
}

/**
 * Constructs a new vector from the values in the given array.
 * @param vector a vector with the 4 values to be inserted in the vector
 */
Vector4::Vector4(float vector[4]) {
    vec[0] = vector[0];
    vec[1] = vector[1];
    vec[2] = vector[2];
    vec[3] = vector[3];
}

/**
 * Adds the components of the left and right vectors together to construct a new vector.
 * @param rhs a reference to the right vector
 * @return a new vector with the summized components.
 */
Vector4 Vector4::operator+(const Vector4& rhs) {
	Vector4 result({this->vec[0] + rhs.vec[0], this->vec[1] + rhs.vec[1], this->vec[2] + rhs.vec[2], this->vec[3] + rhs.vec[3]});
	return result;
}

/**
 * Subtracts the components of the left vector by the components in the right vector.
 * @param rhs a reference to the right vector
 * @return a new vector with the differance of the componentets.
 */
Vector4 Vector4::operator-(const Vector4& rhs) {
	Vector4 result({this->vec[0] - rhs.vec[0], this->vec[1] - rhs.vec[1], this->vec[2] - rhs.vec[2], this->vec[3] - rhs.vec[3]});
	return result;
}

Vector4 Vector4::operator*(const float& rhs) {
    Vector4 temp = Vector4();
    for (int i = 0; i < 4; i++) {
        temp.vec[i] = vec[i] * rhs;
    }
    return temp;
}

Vector2::Vector2() {
    vec[0] = 0;
    vec[1] = 0;
}

Vector2::Vector2(float u, float v) {
    vec[0] = u;
    vec[1] = v;
}

Vector2 Vector2::normalize() {
    Vector2 temp = Vector2(vec[0], vec[1]);
    float length = sqrt((temp.vec[0] * temp.vec[0]) + (temp.vec[1] * temp.vec[1]));
    if (length) {
        temp.vec[0] = temp.vec[0] / length;
        temp.vec[1] = temp.vec[1] / length;
    }
    return temp;
}