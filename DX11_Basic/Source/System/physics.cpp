#include "main.h"
#include "system.h"
#include "physics.h"

bool Physics::Initialize() {
	// Bullet Physicsの初期化
	m_collisionConfig = new btDefaultCollisionConfiguration();
	m_dispatcher = new btCollisionDispatcher(m_collisionConfig);
	m_broadphase = new btDbvtBroadphase();
	m_solver = new btSequentialImpulseConstraintSolver();

	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfig);
	if (!m_dynamicsWorld) {
		ErrorMessage(L"Bullet Physicsの初期化に失敗しました", E_FAIL);
		return false;
	}
	// 重力を設定
	btVector3 gravity(0, -9.81f, 0);
	m_dynamicsWorld->setGravity(gravity);

	return true;
}

void Physics::Finalize() {
	// 物理オブジェクトの削除
	for (btRigidBody* body : m_rigidBodies) {
		if (body) {
			m_dynamicsWorld->removeRigidBody(body);
			delete body->getMotionState();
			delete body->getCollisionShape();
			delete body;
		}
	}

	// Bullet Physicsの終了
	if (m_dynamicsWorld) {
		delete m_dynamicsWorld;
		m_dynamicsWorld = nullptr;
	}
	if (m_solver) {
		delete m_solver;
		m_solver = nullptr;
	}
	if (m_broadphase) {
		delete m_broadphase;
		m_broadphase = nullptr;
	}
	if (m_dispatcher) {
		delete m_dispatcher;
		m_dispatcher = nullptr;
	}
	if (m_collisionConfig) {
		delete m_collisionConfig;
		m_collisionConfig = nullptr;
	}
}

void Physics::Update(double deltaTime) {
	if (m_dynamicsWorld) {
		// 物理シミュレーションの更新
		m_dynamicsWorld->stepSimulation(static_cast<btScalar>(deltaTime), 4, 1.0f / 60.0f);
	}
}

btRigidBody* Physics::CreateStaticBox(const Vector3& position, const Vector3& rotation, const Vector3& size) {
	//コリジョンシェイプの作成
	btBoxShape* boxShape = new btBoxShape(btVector3(size.x / 2, size.y / 2, size.z / 2));
	// 変換行列の作成
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(position.x, position.y, position.z));
	transform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z));

	// 物理ボディのプロパティ設定
	btScalar mass = 0.0f; // 静的オブジェクトなので質量は0
	btVector3 localInertia(0, 0, 0); // 静的オブジェクトなので慣性はゼロ

	// 物理ボディの作成
	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, boxShape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);

	// 物理ボディをワールドに追加
	AddRigidBody(body);

	return body;
}

btRigidBody* Physics::CreateDynamicBox(const Vector3& position, const Vector3& rotation, const Vector3& size, float mass) {
	//コリジョンシェイプの作成
	btBoxShape* boxShape = new btBoxShape(btVector3(size.x / 2, size.y / 2, size.z / 2));

	// 変換行列の作成
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(position.x, position.y, position.z));
	transform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z));

	// 物理ボディのプロパティ設定
	btVector3 localInertia(0, 0, 0);
	boxShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, boxShape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);

	// 物理ボディをワールドに追加
	AddRigidBody(body);

	return body;
}

btRigidBody* Physics::CreateDynamicSphere(const Vector3& position, const Vector3& rotation, float radius, float mass) {
	// コリジョンシェイプの作成
	btSphereShape* sphereShape = new btSphereShape(radius);
	// 変換行列の作成
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(position.x, position.y, position.z));
	transform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z));
	// 物理ボディのプロパティ設定
	btVector3 localInertia(0, 0, 0);
	sphereShape->calculateLocalInertia(mass, localInertia);
	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, sphereShape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);
	// 物理ボディをワールドに追加
	AddRigidBody(body);
	return body;
}

