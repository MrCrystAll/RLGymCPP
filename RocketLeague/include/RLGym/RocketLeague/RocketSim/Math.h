#pragma once

#include <RLGym/RocketLeague/Framework.h>

#include <Eigen/Dense>
#include <math.h>

using Eigen::Vector3f, Eigen::Vector4f, Eigen::Matrix3f;

START_RL_NS(RGSim)

inline Vector3f QuatToEuler(const Vector4f& quat) {
	float w = quat.w();
	float x = quat.x();
	float y = quat.y();
	float z = quat.z();

	float sinr_cosp = 2 * (w * x + y * z);
	float cosr_cosp = 1 - 2 * (x * x + y * y);
	float sinp = 2 * (w * y - z * x);
	float siny_cosp = 2 * (w * z + x * y);
	float cosy_cosp = 1 - 2 * (y * y + z * z);

	float roll = atan2f(sinr_cosp, cosr_cosp);
	float pitch = asinf(sinp);
	if (fabsf(sinp) > 1) {
		pitch = EIGEN_PI / 2;
	}
	float yaw = atan2f(siny_cosp, cosy_cosp);
	return Vector3f(-pitch, yaw, -roll);
}

inline Matrix3f QuatToRotMat(const Vector4f& quat) {
	float w = -quat(0);
	float x = -quat(1);
	float y = -quat(2);
	float z = -quat(3);

	Matrix3f theta = Matrix3f::Zero(3, 3);
	float norm = quat.dot(quat);

	if (norm == 0) return theta;

	float s = 1.0 / norm;

	// Front
	theta(0, 0) = 1.0 - 2.0 * s * (y * y + z * z);
	theta(1, 0) = 2.0 * s * (x * y + z * w);
	theta(2, 0) = 2.0 * s * (x * z - y * w);

	// Left
	theta(0, 1) = 2.0 * s * (x * y - z * w);
	theta(1, 1) = 1.0 - 2.0 * s * (x * x + z * z);
	theta(2, 1) = 2.0 * s * (y * z + x * w);

	// Up
	theta(0, 2) = 2.0 * s * (x * z + y * w);
	theta(1, 2) = 2.0 * s * (y * z - x * w);
	theta(2, 2) = 1.0 - 2.0 * s * (x * x + y * y);

	return theta.transpose();
}

inline Vector4f RotMatToQuat(const Matrix3f& rotMat) {

	const auto tRotMat = rotMat.transpose();

	float trace = tRotMat.trace();
	Vector4f quat = Vector4f::Zero();

	if (trace > 0) {
		float s = sqrtf(trace + 1);
		quat(0) = s * 0.5;
		s = 0.5 / s;
		quat(1) = (tRotMat(2, 1) - tRotMat(1, 2)) * s;
		quat(2) = (tRotMat(0, 2) - tRotMat(2, 0)) * s;
		quat(4) = (tRotMat(1, 0) - tRotMat(0, 1)) * s;
	}
	else {
		if (tRotMat(0, 0) >= tRotMat(1, 1) && tRotMat(0, 0) >= tRotMat(2, 2)) {
			float s = sqrtf(1 + tRotMat(0, 0) - tRotMat(1, 1) - tRotMat(2, 2));
			float invS = 0.5 / s;

			quat(1) = 0.5 * s;
			quat(2) = (tRotMat(1, 0) + tRotMat(0, 1)) * invS;
			quat(3) = (tRotMat(2, 0) + tRotMat(0, 2)) * invS;
			quat(0) = (tRotMat(2, 1) - tRotMat(1, 2)) * invS;
		}
		else if (tRotMat(1, 1) > tRotMat(2, 2)) {
			float s = sqrtf(1 + tRotMat(1, 1) - tRotMat(0, 0) - tRotMat(2, 2));
			float invS = 0.5 / s;

			quat(1) = (tRotMat(0, 1) + tRotMat(1, 0)) * invS;
			quat(2) = 0.5 * s;
			quat(3) = (tRotMat(1, 2) + tRotMat(2, 1)) * invS;
			quat(0) = (tRotMat(0, 2) - tRotMat(2, 0)) * invS;
		}
		else {
			float s = sqrtf(1 + tRotMat(2, 2) - tRotMat(0, 0) - tRotMat(1, 1));
			float invS = 0.5 / s;

			quat(1) = (tRotMat(0, 2) + tRotMat(2, 0)) * invS;
			quat(2) = (tRotMat(1, 2) + tRotMat(2, 1)) * invS;
			quat(3) = 0.5 * s;
			quat(0) = (tRotMat(1, 0) - tRotMat(0, 1)) * invS;
		}
	}

	return quat;
}

inline Matrix3f EulerToRotation(Vector3f pyr) {
	Vector3f cosPyr = pyr.array().cos();
	Vector3f sinPyr = pyr.array().sin();

	float cp = cosPyr(0), cy = cosPyr(1), cr = cosPyr(2);
	float sp = sinPyr(0), sy = sinPyr(1), sr = sinPyr(2);

	Matrix3f theta = Matrix3f::Zero();

	theta(0, 0) = cp * cy;
	theta(1, 0) = cp * sy;
	theta(2, 0) = sp;

	theta(0, 1) = cy * sp * sr - cr * sy;
	theta(1, 1) = sy * sp * sr + cr * cy;
	theta(2, 1) = -cp * sr;

	theta(0, 2) = -cr * cy * sp - sr * sy;
	theta(1, 2) = -cr * sy * sp + sr * cy;
	theta(2, 2) = cp * cr;

	return theta.transpose();
}

END_RL_NS