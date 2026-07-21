#pragma once

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/Math.h>

#include <Eigen/Dense>

#include <optional>

using Eigen::Vector3f;
using Eigen::Vector4f;
using Eigen::Matrix3f;

START_RL_NS(RGSim)

const Vector3f INV_VEC { -1, -1, 1 };
const Matrix3f INV_MTX { { -1, -1, -1}, { -1, -1, -1}, {1, 1, 1} };

class PhysicsObject {
public:
	Vector3f position = Vector3f::Zero(), linearVelocity = Vector3f::Zero(), angularVelocity = Vector3f::Zero();

	const PhysicsObject Inverted() const {
		PhysicsObject inv = PhysicsObject();
		inv.position = this->position.cwiseProduct(INV_VEC);
		inv.linearVelocity = this->linearVelocity.cwiseProduct(INV_VEC);
		inv.angularVelocity = this->angularVelocity.cwiseProduct(INV_VEC);

		if (this->m_rotationMtx.has_value() or this->m_eulerAngles.has_value() || this->m_quaternion.has_value()) {
			inv.SetRotMat(this->GetRotMat() * INV_MTX);
		}
		return inv;
	}

	const Vector4f GetQuaternion() const {
		if (this->m_quaternion.has_value()) return this->m_quaternion.value();
		if (this->m_rotationMtx.has_value()) return RotMatToQuat(this->m_rotationMtx.value());
		if (this->m_eulerAngles.has_value()) return RotMatToQuat(EulerToRotation(this->m_eulerAngles.value()));
	}

	const Vector4f GetQuaternion() {
		if (this->m_quaternion.has_value()) {
			return this->m_quaternion.value();
		}

		if (this->m_rotationMtx.has_value()) {
			this->m_quaternion = RotMatToQuat(this->m_rotationMtx.value());
		}
		else if (this->m_eulerAngles.has_value()) {
			this->m_quaternion = RotMatToQuat(EulerToRotation(this->m_eulerAngles.value()));
		}

		// The case where nothing has a value is an error, so i let it crash on the line below.

		return this->m_quaternion.value();
	}
	void SetQuaternion(const Vector4f& quat) {
		this->m_quaternion = quat;
		this->m_eulerAngles = std::nullopt;
		this->m_rotationMtx = std::nullopt;
	}

	const Matrix3f GetRotMat() const {
		if (this->m_rotationMtx.has_value()) return this->m_rotationMtx.value();
		if (this->m_quaternion.has_value()) return QuatToRotMat(this->m_quaternion.value());
		if (this->m_eulerAngles.has_value()) return EulerToRotation(this->m_eulerAngles.value());

		//Error if we arrive here
		return this->m_rotationMtx.value();
	}

	const Matrix3f GetRotMat() {
		if (this->m_rotationMtx.has_value()) {
			return this->m_rotationMtx.value();
		}

		if (this->m_quaternion.has_value()) {
			this->m_rotationMtx = QuatToRotMat(this->m_quaternion.value());
		}

		else if (this->m_eulerAngles.has_value()) {
			this->m_rotationMtx = EulerToRotation(this->m_eulerAngles.value());
		}

		// The case where nothing has a value is an error, so i let it crash on the line below.
		return this->m_rotationMtx.value();
	}
	void SetRotMat(const Matrix3f& rotMat) {
		this->m_rotationMtx = rotMat;
		this->m_eulerAngles = std::nullopt;
		this->m_quaternion = std::nullopt;
	}

	const Vector3f GetEulerAngles() const {
		if (this->m_eulerAngles.has_value()) return this->m_eulerAngles.value();
		if (this->m_quaternion.has_value()) return QuatToEuler(this->m_quaternion.value());
		if (this->m_rotationMtx.has_value()) return QuatToEuler(RotMatToQuat(this->m_rotationMtx.value()));

		return this->m_eulerAngles.value();
	}

	const Vector3f GetEulerAngles() {
		if (this->m_eulerAngles.has_value()) {
			return this->m_eulerAngles.value();
		}

		if (this->m_quaternion.has_value()) {
			this->m_eulerAngles = QuatToEuler(this->m_quaternion.value());
		}

		else if (this->m_rotationMtx.has_value()) {
			this->m_eulerAngles = QuatToEuler(RotMatToQuat(this->m_rotationMtx.value()));
		}

		// The case where nothing has a value is an error, so i let it crash on the line below.
		return this->m_eulerAngles.value();
	}
	void SetEulerAngles(const Vector3f& eulerAngles) {
		this->m_eulerAngles = eulerAngles;
		this->m_rotationMtx = std::nullopt;
		this->m_quaternion = std::nullopt;
	}

	const Vector3f Forward() const { return this->GetRotMat().row(0); };
	const Vector3f Right() const { return this->GetRotMat().row(1); };
	const Vector3f Left() const { return this->Right() * -1; };
	const Vector3f Up() const { return this->GetRotMat().row(2); };

	const float Pitch() const { return this->GetEulerAngles()(0); };
	const float Yaw() const { return this->GetEulerAngles()(1); };
	const float Roll() const { return this->GetEulerAngles()(2); };


private:
	std::optional<Vector4f> m_quaternion = std::nullopt;
	std::optional<Vector3f> m_eulerAngles = std::nullopt;
	std::optional<Matrix3f> m_rotationMtx = std::nullopt;

};

END_RL_NS