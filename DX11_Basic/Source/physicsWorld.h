#pragma once

#include <bullet/btBulletDynamicsCommon.h>
#include <memory>
#include <vector>

class PhysicsWorld {
public:
	PhysicsWorld() = default;
	~PhysicsWorld() = default;

	//初期化
	bool Initialize();
	//終了処理
	void Finalize();

	//物理演算更新
	void StepSimulation(double deltaTime, int maxSubteps = 10, float fixedTimeStep = 1.0f / 60.0f);

	//ワールド取得
	btDiscreteDynamicsWorld* GetDynamicsWorld() const { return m_dynamicsWorld.get(); }

	//地面や静的オブジェクトの生成
	void CreateGround(float width = 1000.0f, float depth = 1000.0, float y = -3.0f);
	void AddStaticMesh(btCollisionShape* shape, const btTransform& transform);

	//デバッグ描画
	void SetDebugDrawer(btIDebugDraw* debugDrawer);
	void DebugDrawWorld();
private:
	//Bullet Physicsの基本コンポーネント
	std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfig;
	std::unique_ptr<btCollisionDispatcher> m_dispatcher;
	std::unique_ptr<btDbvtBroadphase> m_oveerlappingPairCache;
	std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
	std::unique_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;

	//地面用の剛体
	std::unique_ptr<btCollisionShape> m_groundShape;
	std::unique_ptr<btRigidBody> m_groundBody;
	std::unique_ptr<btDefaultMotionState> m_groundMotionState;

	//静的オブジェクト管理
	std::vector<std::unique_ptr<btCollisionShape>> m_staticShapes;
	std::vector<std::unique_ptr<btRigidBody>> m_staticBodies;
	std::vector<std::unique_ptr<btDefaultMotionState>> m_staticMotionStates;
};
