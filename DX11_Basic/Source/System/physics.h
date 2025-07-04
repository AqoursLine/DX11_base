#pragma once

#include <bullet/btBulletDynamicsCommon.h>

class Physics {
public:
	Physics() = default;
	~Physics() = default;

	bool Initialize();
	void Finalize();
	void Update(double deltaTime);

	btRigidBody* CreateStaticBox(
		const Vector3& position,
		const Vector3& rotation,
		const Vector3& size
	);

	btRigidBody* CreateDynamicBox(
		const Vector3& position,
		const Vector3& rotation,
		const Vector3& size,
		float mass
	);

	btRigidBody* CreateDynamicSphere(
		const Vector3& position,
		const Vector3& rotation,
		float radius,
		float mass
	);

	void AddRigidBody(btRigidBody* body) {
		if (m_dynamicsWorld) {
			m_dynamicsWorld->addRigidBody(body);
			m_rigidBodies.push_back(body);
		}
	}

	void RemoveRigidBody(btRigidBody* body) {
		if (m_dynamicsWorld) {
			m_dynamicsWorld->removeRigidBody(body);

			auto it = std::find(m_rigidBodies.begin(), m_rigidBodies.end(), body);
			if (it != m_rigidBodies.end()) {
				m_rigidBodies.erase(it);
			}

			//関連するメモリを解放
			delete body->getMotionState();
			delete body->getCollisionShape();
			delete body;
		}
	}

	void GetObjectTransform(btRigidBody* body, Vector3& position, Vector3& rotation) const {
		if (body) {
			btTransform transform;
			body->getMotionState()->getWorldTransform(transform);
			position = Vector3(transform.getOrigin().x(), transform.getOrigin().y(), transform.getOrigin().z());
			rotation = Vector3(transform.getRotation().x(), transform.getRotation().y(), transform.getRotation().z());
		}
	}

private:
	btDefaultCollisionConfiguration* m_collisionConfig = nullptr;
	btCollisionDispatcher* m_dispatcher = nullptr;
	btBroadphaseInterface* m_broadphase = nullptr;
	btSequentialImpulseConstraintSolver* m_solver = nullptr;
	btDiscreteDynamicsWorld* m_dynamicsWorld = nullptr;

	std::vector<btRigidBody*> m_rigidBodies; // 物理オブジェクトのリスト
};
