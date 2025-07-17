#pragma once
#include <chrono>
#include <atomic>
#include <mutex>

#include "rtc_base/time_utils.h"

/*
* @brief 音声と映像の同期を管理するクラス
* 
* WebRTCストリーミングにおいて、音声と映像のタイムスタンプを統一し、
* 同期を保つためのタイムスタンプ管理を行う。
*/

class AVSynchronizer {
public:
	/*
	* @brief コンストラクタ
	* 初期状態では遅延補正なし、ドリフト補正なしで初期化
	*/
	AVSynchronizer();

	/*
	* @brief 同期タイマーをリセット
	* 新しい配信開始時や同期がずれた際に呼び出す
	*/
	void Reset();

	/*
	* @brief 現在の統一タイムスタンプを取得
	* @return 現在のタイムスタンプ(マイクロ秒)
	* 
	* 開始時刻から経過時間とクロックドリフト補正を考慮した
	* 統一タイムスタンプを返す。
	*/
	int64_t GetCurrentTimestampUs();

	/*
	* @brief 音声用の補正されたタイムスタンプを取得
	* @return 音声用のタイムスタンプ(マイクロ秒)
	* 
	* 音声遅延補正を適用したタイムスタンプを返す。
	*/
	int64_t GetAudioTimestampUs();

	/*
	* @brief 映像用の補正されたタイムスタンプを取得
	* @return 映像用のタイムスタンプ(マイクロ秒)
	* 
	* 映像遅延補正を適用したタイムスタンプを返す。
	*/
	int64_t GetVideoTimestampUs();

	/*
	* @brief 音声遅延補正値を設定
	* @param delayUs 音声遅延補正値(マイクロ秒)
	* 
	* 正の値で音声を遅らせ、負の値で音声を早める。
	* リップシンク調整に使用。
	*/
	void SetAudioDelay(int64_t delayUs);

	/*
	* @brief 映像遅延補正値を設定
	* @param delayUs 映像遅延補正値(マイクロ秒)
	* 
	* 正の値で映像を遅らせ、負の値で映像を早める。
	*/
	void SetVideoDelay(int64_t delayUs);

	/*
	* @brief クロックドリフト補正係数を更新
	* @param drift ドリフト補正係数
	* 
	* 長時間の配信でのクロックずれを補正。
	* 1.0が基準で、音声フレーム間隔から自動計算。
	*/
	void UpdateClockDrift(double drift);

private:
	//同期開始時の基準時刻
	std::chrono::high_resolution_clock::time_point m_startTime;

	//WebRTCで使用する基準タイムスタンプ(マイクロ秒)
	std::atomic<int64_t> m_baseTimestampUs;

	//タイムスタンプアクセス用のミューテックス
	std::mutex m_timestampMutex;

	//音声の遅延補正値(マイクロ秒)
	//正の値で音声を遅らせ、負の値で音声を早める
	int64_t m_audioDelayUs = 0;

	//映像の遅延補正値(マイクロ秒)
	//正の値で映像を遅らせ、負の値で映像を早める
	int64_t m_videoDelayUs = 0;

	//クロックドリフト補正係数
	//1.0が基準で、1.0より大きいと時間を早め、小さいと遅める
	double m_clockDriftCorrection = 1.0;
};

inline AVSynchronizer::AVSynchronizer() : m_audioDelayUs(0), m_videoDelayUs(0), m_clockDriftCorrection(1.0) {
}

inline void AVSynchronizer::Reset() {
	m_startTime = std::chrono::high_resolution_clock::now();
	m_baseTimestampUs = webrtc::TimeMicros();
}

inline int64_t AVSynchronizer::GetCurrentTimestampUs() {
	auto now = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - m_startTime).count();
	return m_baseTimestampUs + static_cast<int64_t>(elapsed * m_clockDriftCorrection);
}

inline int64_t AVSynchronizer::GetAudioTimestampUs() {
	return GetCurrentTimestampUs() + m_audioDelayUs;
}

inline int64_t AVSynchronizer::GetVideoTimestampUs() {
	return GetCurrentTimestampUs() + m_videoDelayUs;
}

inline void AVSynchronizer::SetAudioDelay(int64_t delayUs) {
	m_audioDelayUs = delayUs;
}

inline void AVSynchronizer::SetVideoDelay(int64_t delayUs) {
	m_videoDelayUs = delayUs;
}

inline void AVSynchronizer::UpdateClockDrift(double drift) {
	std::lock_guard<std::mutex> lock(m_timestampMutex);
	m_clockDriftCorrection = drift;
}
