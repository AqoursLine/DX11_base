#include "main.h"
#include "camera.h"
#include "DX11/renderer.h"
#include "System/input.h"

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
	//左右移動
	if (Input::GetKeyPress(KK_D)) {
		//右方向に移動
		MoveSide(true, deltaTime);
	} else if (Input::GetKeyPress(KK_A)) {
		//左方向に移動
		MoveSide(false, deltaTime);
	}

	//前後移動
	if (Input::GetKeyPress(KK_W)) {
		//前方向に移動
		MoveForward(true, deltaTime);
	} else if (Input::GetKeyPress(KK_S)) {
		//後方向に移動
		MoveForward(false, deltaTime);
	}

	//上下移動
	if (Input::GetKeyPress(KK_SPACE)) {
		//上方向に移動
		m_position.y += m_moveSpeed * static_cast<float>(deltaTime);
	} else if (Input::GetKeyPress(KK_LEFTSHIFT)) {
		//下方向に移動
		m_position.y -= m_moveSpeed * static_cast<float>(deltaTime);
	}

	//左右回転
	if (Input::GetKeyPress(KK_RIGHT)) {
		//右方向に回転
		m_rotation.y += m_rotateSpeed * static_cast<float>(deltaTime);
		//ベクトル計算
		CalculateVector();
	} else if (Input::GetKeyPress(KK_LEFT)) {
		//左方向に回転
		m_rotation.y -= m_rotateSpeed * static_cast<float>(deltaTime);
		//ベクトル計算
		CalculateVector();
	}

	//上下回転
	if (Input::GetKeyPress(KK_UP)) {
		//上方向に回転
		m_rotation.x -= m_rotateSpeed * static_cast<float>(deltaTime);
		//ベクトル計算
		CalculateVector();
	} else if (Input::GetKeyPress(KK_DOWN)) {
		//下方向に回転
		m_rotation.x += m_rotateSpeed * static_cast<float>(deltaTime);
		//ベクトル計算
		CalculateVector();
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
	XMMATRIX view = XMMatrixLookToLH(
		XMVectorSet(m_position.x, m_position.y, m_position.z, 0.0f), //カメラ位置
		XMVectorSet(m_forward.x, m_forward.y, m_forward.z, 0.0f), //カメラの前方ベクトル
		XMVectorSet(m_up.x, m_up.y, m_up.z, 0.0f) //カメラの上方向ベクトル
	);
	//ビュー行列を設定
	RENDERER.SetViewMatrix(view);
}

void Camera::CleanUp() {
}

//カメラクラスのベクトル計算
void Camera::CalculateVector() {
	//回転行列
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);

	//前方ベクトル
	XMVECTOR forward = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotationMatrix);
	//上方向ベクトル
	XMVECTOR up = XMVector3TransformNormal(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotationMatrix);
	//右方向ベクトル
	XMVECTOR right = XMVector3TransformNormal(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), rotationMatrix);

	//正規化
	forward = XMVector3Normalize(forward);
	up = XMVector3Normalize(up);
	right = XMVector3Normalize(right);

	//ベクトルを格納
	m_forward = { XMVectorGetX(forward), XMVectorGetY(forward), XMVectorGetZ(forward) };
	m_up = { XMVectorGetX(up), XMVectorGetY(up), XMVectorGetZ(up) };
	m_right = { XMVectorGetX(right), XMVectorGetY(right), XMVectorGetZ(right) };
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
