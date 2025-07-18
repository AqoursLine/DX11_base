#include "main.h"
#include "camera.h"
#include "renderer.h"
#include "input.h"
#include "system.h"
#include "manager.h"
#include "player.h"
#include "scene.h"

//カメラクラス初期化
bool Camera::Initialize() {
	//回転量をラジアン角で初期化
	m_rotateSpeed = XMConvertToRadians(45.0f);

	m_offset = { 0.0f, 3.0f, -10.0f }; // カメラのオフセット位置
	m_position = m_offset; // 初期位置をオフセット位置に設定

	m_moveSpeed = 10.0f; // カメラの移動速度
	m_rotateSpeed = 10.0f; // カメラの回転速度

	return true;
}

//カメラクラス終了処理
void Camera::Finalize() {
}

//カメラクラス更新処理
void Camera::Update(double deltaTime) {
	auto player = SYSTEM.GetManager()->GetScene()->GetGameObject<Player>();

	if (player) {
		auto oldposition = m_targetPosition;

		//プレイヤーの位置をカメラのターゲット位置に設定
		m_targetPosition = player->GetPosition();
		m_targetPosition.y += 1.0f; // プレイヤーの高さを少し上げる

		//カメラのオフセット位置を計算
		Vector3 forward = player->GetForward();
		m_position = m_targetPosition + forward * m_offset.z + player->GetUp() * m_offset.y;

	} else {
		//プレイヤーが見つからない場合はデフォルトのターゲット位置
		m_targetPosition = {0.0f, 0.0f, 0.0f};
	}
}

//カメラクラス描画処理
void Camera::Draw() const {
	//プロジェクション行列を設定
	XMMATRIX projection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.0f), //視野角
		static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT), //アスペクト比
		0.05f, //近くのクリッピング面
		1000.0f //遠くのクリッピング面
	);
	//プロジェクション行列を設定
	RENDERER.SetProjectionMatrix(projection);

	//カメラ位置を設定
	XMMATRIX view = XMMatrixLookAtLH(
		XMVectorSet(m_position.x, m_position.y, m_position.z, 0.0f), //カメラ位置
		XMVectorSet(m_targetPosition.x, m_targetPosition.y, m_targetPosition.z, 0.0f), //カメラの前方ベクトル
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) //カメラの上方向ベクトル
	);
	//ビュー行列を設定
	RENDERER.SetViewMatrix(view);

	//カメラ位置をレンダラーに設定
	RENDERER.SetCameraPosition(m_position);
}

void Camera::CleanUp() {
}

void Camera::MoveSide(bool isRight, double deltaTime) {
	Vector3 right = GetRight();

	//地面に対して平行なベクトル
	Vector3 direction = Vector3(right.x, 0.0f, right.z);
	direction.normalize();

	//移動
	m_position += direction * (isRight ? 1.0f : -1.0f) * m_moveSpeed * static_cast<float>(deltaTime);
	//ターゲット位置も同様に移動
	m_targetPosition += direction * (isRight ? 1.0f : -1.0f) * m_moveSpeed * static_cast<float>(deltaTime);
}

void Camera::MoveForward(bool isForward, double deltaTime) {
	Vector3 forward = GetForward();

	//地面に対して平行なベクトル
	Vector3 direction = Vector3(forward.x, 0.0f, forward.z);
	direction.normalize();

	//移動
	m_position += direction * (isForward ? 1.0f : -1.0f) * m_moveSpeed * static_cast<float>(deltaTime);
	//ターゲット位置も同様に移動
	m_targetPosition += direction * (isForward ? 1.0f : -1.0f) * m_moveSpeed * static_cast<float>(deltaTime);
}

void Camera::RotateAroundTarget(double deltaTime) {
	// カメラのオフセット位置を計算
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	XMVECTOR offsetVector = XMVectorSet(m_offset.x, m_offset.y, m_offset.z, 0.0f);
	offsetVector = XMVector3Transform(offsetVector, rotationMatrix);
	// カメラの位置をターゲット位置からオフセット位置を加えた位置に設定
	m_position = Vector3(
		m_targetPosition.x + XMVectorGetX(offsetVector),
		m_targetPosition.y + XMVectorGetY(offsetVector),
		m_targetPosition.z + XMVectorGetZ(offsetVector)
	);
}

