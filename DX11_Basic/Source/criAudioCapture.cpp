#include "criAudioCapture.h"
#include <iostream>
#include <algorithm>

SynchronizedCRIAudioCapture::SynchronizedCRIAudioCapture(std::shared_ptr<AVSynchronizer> sync)
	: m_audioTransport(nullptr)
	, m_recording(false)
	, m_playing(false)
	, m_synchronizer(sync)
	, m_processing(false)
	, m_lastCallbackTimeUs(0)
	, m_player(nullptr)
	, m_voicePool(nullptr)
	, m_acb(nullptr) {
	//1マイクロ秒当たりのサンプル数を計算
	m_samplesPerMicrosecond = static_cast<double>(kSampleRate) / 1000000.0;
}

SynchronizedCRIAudioCapture::~SynchronizedCRIAudioCapture() {
	Terminate();
}

int32_t SynchronizedCRIAudioCapture::Init() {
	//CRI ADX2LEの初期化
	if (!InitializeCRI()) {
		std::cerr << "CRI ADX2LEの初期化に失敗しました。" << std::endl;
		return -1;
	}

	//音声処理スレッドを開始
	m_processing = true;
	m_audioProcessingThread = std::thread(&SynchronizedCRIAudioCapture::AudioProcessingLoop, this);

	std::cout << "SynchronizedCRIAudioCaptureの初期化が完了しました。" << std::endl;

	return 0;
}

int32_t SynchronizedCRIAudioCapture::Terminate() {
	//音声処理スレッドを停止
	m_processing = false;
	m_audioQueueCondition.notify_all();

	if (m_audioProcessingThread.joinable()) {
		m_audioProcessingThread.join();
	}

	//CRI ADX2LEの終了処理
	if (m_player) {
		criAtomExPlayer_Destroy(m_player);
		m_player = nullptr;
	}

	if (m_acb) {
		criAtomExAcb_Release(m_acb);
		m_acb = nullptr;
	}

	if (m_voicePool) {
		criAtomExVoicePool_Free(m_voicePool);
		m_voicePool = nullptr;
	}

	criAtomEx_Finalize();

	std::cout << "SynchronizedCRIAudioCaptureの終了処理が完了しました。" << std::endl;

	return 0;
}

int32_t SynchronizedCRIAudioCapture::RegisterAudioCallback(webrtc::AudioTransport* audioCallback) {
	m_audioTransport = audioCallback;
	return 0;
}

int32_t SynchronizedCRIAudioCapture::StartRecording() {
	m_recording = true;
	std::cout << "Audio recording started." << std::endl;
	return 0;
}

int32_t SynchronizedCRIAudioCapture::StopRecording() {
	m_recording = false;
	std::cout << "Audio recording stopped." << std::endl;
	return 0;
}

int32_t SynchronizedCRIAudioCapture::StartPlayout() {
	m_playing = true;
	std::cout << "Audio playback started." << std::endl;
	return 0;
}

int32_t SynchronizedCRIAudioCapture::StopPlayout() {
	m_playing = false;
	std::cout << "Audio playback stopped." << std::endl;
	return 0;
}

bool SynchronizedCRIAudioCapture::LoadACB(const std::string& acbPath, const std::string& awbPath) {
	//既存のACBを解放
	if (m_acb) {
		criAtomExAcb_Release(m_acb);
		m_acb = nullptr;
	}

	//ACBファイルを読み込み
	m_acb = criAtomExAcb_LoadAcbFile(nullptr, acbPath.c_str(), nullptr, awbPath.c_str(), nullptr, 0);
	if (!m_acb) {
		std::cerr << "Failed to load ACB file: " << acbPath << std::endl;
		return false;
	}

	std::cout << "ACB file loaded successfully: " << acbPath << std::endl;
	return true;
}

void SynchronizedCRIAudioCapture::PlaySound(CriAtomExCueId cueId) {
	if (!m_player || !m_acb) {
		std::cerr << "Player or ACB not initialized." << std::endl;
		return;
	}

	//キューIDを設定して再生
	criAtomExPlayer_SetCueId(m_player, m_acb, cueId);
	criAtomExPlayer_Start(m_player);

}

void SynchronizedCRIAudioCapture::PlaySound(const std::string& cueName) {
	if (!m_player || !m_acb) {
		std::cerr << "Player or ACB not initialized." << std::endl;
		return;
	}
	//キュー名を設定して再生
	criAtomExPlayer_SetCueName(m_player, m_acb, cueName.c_str());
	criAtomExPlayer_Start(m_player);
}

bool SynchronizedCRIAudioCapture::InitializeCRI() {
	//CRI ADX2LEの基本設定
	CriAtomExConfig config;
	criAtomEx_SetDefaultConfig(&config);
	config.max_virtual_voices = 32; //仮想ボイス数
	config.max_voice_limit_groups = 8; //ボイス制限グループ数
	config.max_categories = 16; //カテゴリ数

	//初期化
	CriBool error = criAtomEx_Initialize(&config, nullptr, 0);
	if (error != CRI_TRUE) {
		std::cerr << "CRI ADX2LE initialization failed." << std::endl;
		return false;
	}

	//スタンダードボイスプールの作成
	CriAtomExStandardVoicePoolConfig voicePoolConfig;
	criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(&voicePoolConfig);
	voicePoolConfig.num_voices = 16; //ボイス数
	voicePoolConfig.player_config.streaming_flag = CRI_TRUE; //ストリーミング再生を有効化

	m_voicePool = criAtomExVoicePool_AllocateStandardVoicePool(&voicePoolConfig, nullptr, 0);
	if (!m_voicePool) {
		std::cerr << "Failed to allocate voice pool." << std::endl;
		return false;
	}

	//プレイヤーの作成
	CriAtomExPlayerConfig playerConfig;
	criAtomExPlayer_SetDefaultConfig(&playerConfig);

	return false;
}
