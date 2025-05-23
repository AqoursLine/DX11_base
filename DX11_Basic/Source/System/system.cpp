#include "main.h"
#include "system.h"
#include "manager.h"
#include "timer.h"
#include "Component/model.h"

//システム初期化
bool System::Initialize() {
	//マネージャークラス
	m_manager = new Manager();
	bool isInitialized = m_manager->Initialize();

	if (!isInitialized) {
		ErrorMessage(L"マネージャークラスの初期化に失敗しました", E_FAIL);
		return false;
	}

	//タイマー初期化
	m_timer = new Timer();
	m_timer->Reset();
	m_timer->Start();

	return true;
}

//システム終了
void System::Finalize() {
	//マネージャークラスの終了
	if (m_manager) {
		m_manager->Finalize();
		delete m_manager;
		m_manager = nullptr;
	}

	//モデルのキャッシュをクリア
	Model::ClearCache();

	//タイマー終了
	if (m_timer) {
		m_timer->Stop();
		m_timer->Reset();
		delete m_timer;
		m_timer = nullptr;
	}
}

bool System::Excute() {
	m_timer->Tick();

	m_manager->Update(m_timer->GetDeltaTime());
	m_manager->Draw();
	if (m_manager->CleanUp()) {
		return true;
	}

	return false;
}
