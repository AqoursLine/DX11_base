#pragma once
#include <cri_adx2le.h>
#include <cri_le_atom.h>
#include <api/audio/audio_frame.h>
#include <api/audio/audio_device.h>
#include "avSynchronizer.h"
#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

/*
* @brief CRI ADX2LEと統合された音声キャプチャクラス
* 
* CRI ADXLEで再生される音声をWebRTCに送信するための
* AudioDeviceModuleの実装。音声同期とバッファリングを管理
*/
class SynchronizedCRIAudioCapture : public webrtc::AudioDeviceModule {
public:
	/*
	* @brief コンストラクタ
	* @param sync AVSynchronizerインスタンス
	*/
	explicit SynchronizedCRIAudioCapture(std::shared_ptr<AVSynchronizer> sync);

	/*
	* @brief デストラクタ
	* リソースの解放。
	*/
	~SynchronizedCRIAudioCapture();

	//インターフェース実装
	/*
	* @brief AudioDeviceModuleの初期化
	* @return 成功時は0、失敗時は-1
	* 
	* CRI ADX2LEの初期化と音声処理スレッドの開始。
	*/
	int32_t Init() override;

	/*
	* @brief AudioDeviceModuleの終了
	* @return 常に0
	* 
	* CRI ADX2LEの終了処理と音声処理スレッドの停止。
	*/
	int32_t Terminate() override;

	/*
	* @brief 音声コールバックの登録
	* @param audioCallback WebRTCへの音声データ送信用コールバック
	* @return 常に0
	*/
	int32_t RegisterAudioCallback(webrtc::AudioTransport* audioCallback) override;

	/*
	* @brief 録音開始
	* @return 常に0
	* 
	* WebRTCへの音声データ送信を開始。
	*/
	int32_t StartRecording() override;

	/*
	* @brief 録音停止
	* @return 常に0
	* 
	* WebRTCへの音声データ送信を停止。
	*/
	int32_t StopRecording() override;

	/*
	* @brief 再生開始
	* @return 常に0
	*/
	int32_t StartPlayout() override;

	/*
	* @brief 再生停止
	* @return 常に0
	*/
	int32_t StopPlayout() override;

	//CRI ADX2LE制御メソッド

	/*
	* @brief ACBファイルの読み込み
	* @param acbPath ACBファイルのパス
	* @param awbPath AWBファイルのパス(オプション)
	* @return 成功時はtrue、失敗時はfalse
	* 
	* CRI ADX2LEで使用する音声データベースを読み込み。
	*/
	bool LoadACB(const std::string& acbPath, const std::string& awbPath = "");

	/*
	* @brief 音声の再生
	* @param cueId 再生するキューID
	* 
	* 指定されたキューIDの音声を再生。
	* この音声はWebRTCストリームに含まれる。
	*/
	void PlaySound(CriAtomExCueId cueId);

	/*
	* @brief 音声の再生(キュー名指定)
	* @param cueName 再生するキュー名
	*/
	void PlaySound(const std::string& cueName);

private:
	/*
	* @brief 音声フレームデータ構造体
	* キュー管理で使用する音声フレームの構造体の情報を格納
	*/
	struct AudioFrame {
		//音声データ(インターリーブ形式)
		std::vector<int16_t> data;
		//このフレームのタイムスタンプ(マイクロ秒)
		int64_t timestampUs;
		//サンプリングレート
		int sampleRate;
		//チャンネル数
		int channels;
	};

	//CRI ADXLE関連
	//CRI ADX2LEのプレイヤーハンドル
	CriAtomExPlayerHn m_player;
	//CRI ADX2のボイスプールハンドル
	CriAtomExVoicePoolHn m_voicePool;
	//現在読み込まれているACBバンドル
	CriAtomExAcbHn m_acb;

	//WebRTC関連
	//WebRTCオーディオトランスポート(音声データの送信先)
	webrtc::AudioTransport* m_audioTransport;
	//録音状態フラグ
	bool m_recording;
	//再生状態フラグ
	bool m_playing;

	//同期管理
	//音声・映像同期管理オブジェクト
	std::shared_ptr<AVSynchronizer> m_synchronizer;

	//音声バッファリング
	//音声フレームのキュー
	std::queue<AudioFrame> m_audioFrameQueue;
	//キューアクセス用ミューテックス
	std::mutex m_audioQueueMutex;
	//キューの状態変化通知用
	std::condition_variable m_audioQueueCondition;

	//音声処理スレッド
	//音声処理を行うワーカースレッド
	std::thread m_audioProcessingThread;
	//音声処理の継続フラグ
	std::atomic<bool> m_processing;

	//タイミング管理
	//前回のCRIコールバック実行時刻(マイクロ秒)
	int64_t m_lastCallbackTimeUs;
	//1マイクロ秒あたりのサンプル数(ドリフト計算用)
	double m_samplesPerMicrosecond;

	//音声設定定義
	//サンプリングレート(48kHz)
	static const int kSampleRate = 48000;
	//チャンネル数(ステレオ)
	static const int kChannels = 2;
	//1フレームあたりのサンプル数(10ms分)
	static const int kFramesPerBuffer = 480;

	/*
	* @brief CRI ADX2LEの初期化
	* @return 成功時はtrue
	* 
	* CRI ADX2LEライブラリの初期化と各種設定
	*/
	bool InitializeCRI();

	/*
	* @brief CRI音声出力コールバック(静的メソッド)
	* @param obj thisポインタ
	* @param format PCMフォーマット
	* @param numChannels チャンネル数
	* @param numSamples サンプル数
	* @param data 音声データ配列
	* 
	* CRI ADX2LEから呼び出される音声出力コールバック。
	*/
	static void AudioOutputCallback(
		void* obj,
		CriAtomPcmFormat format,
		CriSint32 numChannels,
		CriSint32 numSamples,
		void* data[]
	);

	/*
	* @brief CRI音声出力の処理
	* @param format PCMフォーマット
	* @param numChannels チャンネル数
	* @param numSamples サンプル数
	* @param data 音声データ配列
	* 
	* CRI ADX2LEからの音声データを受け取り、WebRTC用にフォーマット
	*/
	void HandleAudioOutput(
		CriAtomPcmFormat format,
		CriSint32 numChannels,
		CriSint32 numSamples,
		void* data[]
	);

	/*
	* @brief 音声処理ループ
	* 
	* 別スレッドで実行される音声処理のメインループ
	* キューからフレームを取り出して、WebRTCに送信
	*/
	void AudioProcessingLoop();

	//WebRTC AudioDeviceModuleの必須実装メソッド群
	bool Recording() const override { return m_recording; }
	bool Playing() const override { return m_playing; }
	int16_t RecordingDevices() override { return 1; }
	int16_t PlayoutDevices() override { return 1; }

	int32_t RecordingDeviceName(
		uint16_t index,
		char name[webrtc::kAdmMaxDeviceNameSize],
		char guid[webrtc::kAdmMaxGuidSize]) override;

	int32_t PlayoutDeviceName(
		uint16_t index,
		char name[webrtc::kAdmMaxDeviceNameSize],
		char guid[webrtc::kAdmMaxGuidSize]) override;

	//その他の必須実装(簡略実装)
	int32_t SetRecordingDevice(uint16_t index) override { return 0; }
	int32_t SetPlayoutDevice(uint16_t index) override { return 0; }
	int32_t RecordingIsAvailable(bool* available) override { *available = true; return 0; }
	int32_t PlayoutIsAvailable(bool* available) override { *available = true; return 0; }
	int32_t MicrophoneVolumeIsAvailable(bool* available) override { *available = false; return 0; }
	int32_t SpeakerVolumeIsAvailable(bool* available) override { *available = false; return 0; }
	int32_t MicrophoneMuteIsAvailable(bool* available) override { *available = false; return 0; }
	int32_t SpeakerMuteIsAvailable(bool* available) override { *available = false; return 0; }
	int32_t SetMicrophoneVolume(uint32_t volume) override { return -1; }
	int32_t SetMicrophoneMute(bool enable) override { return -1; }
	int32_t SetSpeakerVolume(uint32_t volume) override { return -1; }
	int32_t SetSpeakerMute(bool enable) override { return -1; }
	bool BuiltInAECIsAvailable() const override { return false; }
	int32_t EnableBuiltInAEC(bool enable) override { return -1; }
	bool BuiltInAGCIsAvailable() const override { return false; }
	int32_t EnableBuiltInAGC(bool enable) override { return -1; }
	bool BuiltInNSIsAvailable() const override { return false; }
	int32_t EnableBuiltInNS(bool enable) override { return -1; }
	bool Initialized() const override { return true; }
};

