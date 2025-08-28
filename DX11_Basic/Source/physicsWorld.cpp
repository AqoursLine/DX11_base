#include "physicsWorld.h"
#include <stdexcept>
#include <iostream>

bool PhysicsWorld::Initialize() {
	try {
		//コリジョン設定を作成
		m_collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();

		//コリジョンディスパッチャーを作成
		m_dispatcher = std::make_unique<btCollisionDispatcher>(m_collisionConfig.get());

		//ブロードフェーズを作成
		m_oveerlappingPairCache = std::make_unique<btDbvtBroadphase>();

		//静的ソルバーを作成
		m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();

		//物理ワールドを作成
		m_dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
			m_dispatcher.get(),
			m_oveerlappingPairCache.get(),
			m_solver.get(),
			m_collisionConfig.get()
		);

		//重量設定
		m_dynamicsWorld->setGravity(btVector3(0.0f, -9.81f, 0.0f));

		//基本的な地面を作成
		CreateGround();

		return true;
	}
	catch (const std::exception& e) {
		Finalize();
		std::cerr << "PhysicsWorld Initialization Error: " << e.what() << std::endl;
		return false;
	}
}

void PhysicsWorld::Finalize() {
	//既にクリーンアップ済み
	if (!m_dynamicsWorld) {
		return;
	}

	//管理しているオブジェクトを物理世界から削除
	
	//地面を物理世界から削除
	if (m_groundBody) {
		m_dynamicsWorld->removeRigidBody(m_groundBody.get());
	}

	//静的オブジェクトを物理世界から削除
	for (auto& body : m_staticBodies) {
		if (body) {
			m_dynamicsWorld->removeRigidBody(body.get());
		}
	}

	//外部オブジェクトの確認とクリーンアップ
	std::vector<btCollisionObject*> collisionObjects;
	//残っているオブジェクトを収集
	for (int i = 0; i < m_dynamicsWorld->getNumCollisionObjects(); i++) {
		btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		collisionObjects.push_back(obj);
	}

	//収集したオブジェクトをクリーンアップ
	if (!collisionObjects.empty()) {
		for (btCollisionObject* obj : collisionObjects) {
			btRigidBody* body = btRigidBody::upcast(obj);
			
			if (body && body->getMotionState()) {
				delete body->getMotionState();
			}

			m_dynamicsWorld->removeCollisionObject(obj);
			delete obj;
		}
	}

	//静的オブジェクトのクリーンアップ
	m_staticBodies.clear();
	m_staticMotionStates.clear();
	m_staticShapes.clear();

	//地面のクリーンアップ
	m_groundBody.reset();
	m_groundMotionState.reset();
	m_groundShape.reset();

	//Bullet Physicsコンポーネントのクリーンアップ
	m_dynamicsWorld.reset();
	m_solver.reset();
	m_oveerlappingPairCache.reset();
	m_dispatcher.reset();
	m_collisionConfig.reset();
}

void PhysicsWorld::StepSimulation(double deltaTime, int maxSubteps, float fixedTimeStep) {
	if (m_dynamicsWorld) {
		m_dynamicsWorld->stepSimulation(static_cast<btScalar>(deltaTime), maxSubteps, fixedTimeStep);
	}
}

void PhysicsWorld::CreateGround(float width, float depth, float y) {
	//地面の形状を作成
	btVector3 groundHalfExtents(width * 0.5f, 1.0f, depth * 0.5f);
	m_groundShape = std::make_unique<btBoxShape>(groundHalfExtents);

	//地面の位置
	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0.0f, y, 0.0f));

	//地面のモーションステートを作成
	m_groundMotionState = std::make_unique<btDefaultMotionState>(groundTransform);

	//地面の剛体情報
	btRigidBody::btRigidBodyConstructionInfo groundRBInfo(
		0.0f, //質量0は静的オブジェクト
		m_groundMotionState.get(),
		m_groundShape.get(),
		btVector3(0.0f, 0.0f, 0.0f) //慣性モーメント
	);
	
	//地面の剛体を作成
	m_groundBody = std::make_unique<btRigidBody>(groundRBInfo);

	//地面の物理特性設定
	m_groundBody->setFriction(0.8f);
	m_groundBody->setRestitution(0.2f);
	m_groundBody->setRollingFriction(0.1f);

	//物理世界に地面を追加
	m_dynamicsWorld->addRigidBody(m_groundBody.get());
}

void PhysicsWorld::AddStaticMesh(btCollisionShape* shape, const btTransform& transform) {
	//モーションステート作成
	auto motionState = std::make_unique<btDefaultMotionState>(transform);

	//剛体情報作成
	btRigidBody::btRigidBodyConstructionInfo rbInfo(
		0.0f, //質量0は静的オブジェクト
		motionState.get(),
		shape,
		btVector3(0.0f, 0.0f, 0.0f) //慣性モーメント
	);

	//剛体作成
	auto body = std::make_unique<btRigidBody>(rbInfo);

	//物理特性設定
	body->setFriction(0.8f);
	body->setRestitution(0.3f);

	//物理世界に追加
	m_dynamicsWorld->addRigidBody(body.get());

	//メモリ管理用に保存
	m_staticMotionStates.push_back(std::move(motionState));
	m_staticBodies.push_back(std::move(body));
}

void PhysicsWorld::SetDebugDrawer(btIDebugDraw* debugDrawer) {
	if (m_dynamicsWorld) {
		m_dynamicsWorld->setDebugDrawer(debugDrawer);
	}
}

void PhysicsWorld::DebugDrawWorld() {
	if (m_dynamicsWorld && m_dynamicsWorld->getDebugDrawer()) {
		m_dynamicsWorld->debugDrawWorld();
	}
}
