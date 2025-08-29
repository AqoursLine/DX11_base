#include "vehicle.h"
#include "system.h"
#include "physicsWorld.h"
#include <algorithm>

Vehicle::Vehicle()
	: m_vehicleBody(nullptr)
	, m_vehicle(nullptr)
	, m_vehicleRayCaster(nullptr)
	, m_chassisShape(nullptr)
	, m_currentEngineForce(0.0f)
	, m_currentBrakingForce(0.0f)
	, m_currentSteering(0.0f) {
	m_params = VehicleParams();
}

bool Vehicle::Initialize() {
	if (!m_dynamicsWorld) {
		m_dynamicsWorld = SYSTEM.GetPhysicsWorld()->GetDynamicsWorld();
	}

	//車体を作成
	CreateChassis();
	if (!m_vehicleBody) {
		return false;
	}

	//レイキャスターを作成
	m_vehicleRayCaster = new btDefaultVehicleRaycaster(m_dynamicsWorld);

	//ビークルを作成
	m_vehicle = new btRaycastVehicle(m_tuning, m_vehicleBody, m_vehicleRayCaster);

	//ビークルを物理世界に追加
	m_dynamicsWorld->addVehicle(m_vehicle);

	//座標系設定
	m_vehicle->setCoordinateSystem(0, 1, 2);

	//ホイールを追加
	AddWheels();

	return true;
}

void Vehicle::Update(double deltaTime) {
	if (!m_vehicle) {
		return;
	}

	//ビークルの物理演算を更新
	UpdateTransform();
}

void Vehicle::Finalize() {
	if (m_dynamicsWorld && m_vehicle) {
		m_dynamicsWorld->removeVehicle(m_vehicle);
		delete m_vehicle;
		m_vehicle = nullptr;
	}

	if (m_vehicleRayCaster) {
		delete m_vehicleRayCaster;
		m_vehicleRayCaster = nullptr;
	}

	if (m_dynamicsWorld && m_vehicleBody) {
		m_dynamicsWorld->removeRigidBody(m_vehicleBody);

		if (m_vehicleBody->getMotionState()) {
			delete m_vehicleBody->getMotionState();
		}

		delete m_vehicleBody;
		m_vehicleBody = nullptr;
	}

	if (m_chassisShape) {
		delete m_chassisShape;
		m_chassisShape = nullptr;
	}
}

void Vehicle::SetEngineForce(float force) {
	m_currentEngineForce = std::clamp(force, -m_params.maxEngineForce, m_params.maxEngineForce);
	if (m_vehicle) {
		//後輪にエンジン出力を適用
		m_vehicle->applyEngineForce(m_currentEngineForce, WheelIndex::REAR_LEFT);
		m_vehicle->applyEngineForce(m_currentEngineForce, WheelIndex::REAR_RIGHT);
	}
}

void Vehicle::SetSteeringValue(float steering) {
	m_currentSteering = std::clamp(steering, -m_params.maxSteeringAngle, m_params.maxSteeringAngle);
	if (m_vehicle) {
		//前輪にハンドル操作を適用
		m_vehicle->setSteeringValue(m_currentSteering, WheelIndex::FRONT_LEFT);
		m_vehicle->setSteeringValue(m_currentSteering, WheelIndex::FRONT_RIGHT);
	}
}

void Vehicle::SetBrakingForce(float brake) {
	m_currentBrakingForce = std::clamp(brake, 0.0f, m_params.maxBrakingForce);
	if (m_vehicle) {
		//後輪にブレーキ力を適用
		m_vehicle->setBrake(m_currentBrakingForce, WheelIndex::REAR_LEFT);
		m_vehicle->setBrake(m_currentBrakingForce, WheelIndex::REAR_RIGHT);
		//前輪にもブレーキ力を適用(前輪ブレーキ)
		m_vehicle->setBrake(m_currentBrakingForce * 0.5f, WheelIndex::FRONT_LEFT);
		m_vehicle->setBrake(m_currentBrakingForce * 0.5f, WheelIndex::FRONT_RIGHT);
	}
}

float Vehicle::GetCurrentSpeed() const {
	if (!m_vehicleBody) return 0.0f;

	btVector3 velocity = m_vehicleBody->getLinearVelocity();
	return velocity.length() * 3.6f; // m/s to km/h
}

btTransform Vehicle::GetWheelTransform(int wheelIndex) const {
	if (!m_vehicle || wheelIndex < 0 || wheelIndex >= m_vehicle->getNumWheels()) {
		return btTransform::getIdentity();
	}

	return m_vehicle->getWheelTransformWS(wheelIndex);
}

Vector3 Vehicle::QuaternionToEuler(const btQuaternion& q) {
	//角変数
	float x = q.getX();
	float y = q.getY();
	float z = q.getZ();
	float w = q.getW();

	//Yaw
	float siny_cosp = 2.0f * (w * z + x * y);
	float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
	float yaw = std::atan2(siny_cosp, cosy_cosp);

	//Pitch
	float sinp = 2.0f * (w * y - z * x);
	float pitch;
	if (std::abs(sinp) >= 1.0f) {
		pitch = std::copysign(XM_PI / 2.0f, sinp);
	} else {
		pitch = std::asin(sinp);
	}

	//Roll
	float sinr_cosp = 2.0f * (w * x + y * z);
	float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
	float roll = std::atan2(sinr_cosp, cosr_cosp);

	//正規化して返す
	return Vector3(WrapAngle(roll), WrapAngle(pitch), WrapAngle(yaw));

}

void Vehicle::CreateChassis() {
	//車体の形状を作成(ボックス)
	btVector3 chassisHalfExtents = ToBtVector3(m_params.chassisSize * 0.5f);
	m_chassisShape = new btBoxShape(chassisHalfExtents);

	//慣性モーメントを計算
	btVector3 localInertia(0.0f, 0.0f, 0.0f);
	m_chassisShape->calculateLocalInertia(m_params.chassisMass, localInertia);

	//初期トランスフォーム
	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(ToBtVector3(m_position));
	btQuaternion rotation;
	rotation.setEulerZYX(
		btScalar(m_rotation.y), //yaw
		btScalar(m_rotation.x), //pitch
		btScalar(m_rotation.z)  //roll
	);
	startTransform.setRotation(rotation);

	//モーションステートを作成
	btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);

	//リジッドボディを作成
	btRigidBody::btRigidBodyConstructionInfo rbInfo(
		m_params.chassisMass,
		motionState,
		m_chassisShape,
		localInertia
	);

	m_vehicleBody = new btRigidBody(rbInfo);
	m_vehicleBody->setActivationState(DISABLE_DEACTIVATION); //非アクティブ化を無効にする

	//物理世界に車体を追加
	m_dynamicsWorld->addRigidBody(m_vehicleBody);
}

void Vehicle::AddWheels() {
	if (!m_vehicle) return;

	//ホイール方向ベクトル
	btVector3 wheelDirectionCS0(0.0f, -1.0f, 0.0f); //下方向
	btVector3 wheelAxleCS(-1.0f, 0.0f, 0.0f); //左方向

	//前輪左
	btVector3 connectionPointCS0 = ToBtVector3(m_params.frontLeftWheelPos);
	m_vehicle->addWheel(
		connectionPointCS0,
		wheelDirectionCS0,
		wheelAxleCS,
		m_params.suspensionRestLength,
		m_params.wheelRadius,
		m_tuning,
		true //前輪
	);

	//前輪右
	connectionPointCS0 = ToBtVector3(m_params.frontRightWheelPos);
	m_vehicle->addWheel(
		connectionPointCS0,
		wheelDirectionCS0,
		wheelAxleCS,
		m_params.suspensionRestLength,
		m_params.wheelRadius,
		m_tuning,
		true //前輪
	);

	//後輪左
	connectionPointCS0 = ToBtVector3(m_params.rearLeftWheelPos);
	m_vehicle->addWheel(
		connectionPointCS0,
		wheelDirectionCS0,
		wheelAxleCS,
		m_params.suspensionRestLength,
		m_params.wheelRadius,
		m_tuning,
		false //後輪
	);

	//後輪右
	connectionPointCS0 = ToBtVector3(m_params.rearRightWheelPos);
	m_vehicle->addWheel(
		connectionPointCS0,
		wheelDirectionCS0,
		wheelAxleCS,
		m_params.suspensionRestLength,
		m_params.wheelRadius,
		m_tuning,
		false //後輪
	);

	//各ホイールの物理特性を設定
	for (int i = 0; i < m_vehicle->getNumWheels(); i++) {
		btWheelInfo& wheel = m_vehicle->getWheelInfo(i);
		wheel.m_suspensionStiffness = m_params.suspensionStiffness;
		wheel.m_wheelsDampingRelaxation = m_params.wheelDamping;
		wheel.m_wheelsDampingCompression = m_params.wheelCompression;
		wheel.m_frictionSlip = m_params.wheelFriction;
		wheel.m_rollInfluence = m_params.rollInfluence;
	}
}

void Vehicle::UpdateTransform() {
	if (!m_vehicleBody) return;

	//車体のトランスフォームを取得
	btTransform trans;
	m_vehicleBody->getMotionState()->getWorldTransform(trans);

	//GameObjectの位置と回転を更新
	btVector3 origin = trans.getOrigin();
	m_position = ToVector3(origin);

	//回転をオイラー角に変換
	btQuaternion rotation = trans.getRotation();
	btScalar roll, pitch, yaw;
	rotation.getEulerZYX(yaw, pitch, roll); // ZYX順にオイラー角を取得
	m_rotation = { roll, pitch, yaw };

}
