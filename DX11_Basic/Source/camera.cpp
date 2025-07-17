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

	m_position = {0.0f, 2.0f, -5.0f}; //カメラ位置

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

		//カメラの位置をプレイヤーの向いてる方向の後ろに設定
		Vector3 forward = player->GetVelocity();
		m_position = m_targetPosition + (-forward + m_offset);

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
	//地面に対して平行なベクトル
	Vector3 direction = Vector3(m_right.x, 0.0f, m_right.z);
	direction.normalize();

	//移動
	m_position += direction * (isRight ? 1.0f : -1.0f) * m_moveSpeed * static_cast<float>(deltaTime);
}

void Camera::MoveForward(bool isForward, double deltaTime) {
	//地面に対して平行なベクトル
	Vector3 direction = Vector3(m_forward.x, 0.0f, m_forward.z);
	direction.normalize();

	//移動
	m_position += direction * (isForward ? 1.0f : -1.0f) * m_moveSpeed * static_cast<float>(deltaTime);
}
