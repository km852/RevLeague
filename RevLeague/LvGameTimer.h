#pragma once

#include <Windows.h>

class LvGameClock final {
	friend class LvGameTimer;

private:
	LARGE_INTEGER creationTick;
	LARGE_INTEGER gameStartTick = { 0 };

	LARGE_INTEGER lastFrameBeginTick = { 0 };
	LARGE_INTEGER currFrameBeginTick = { 0 };

	unsigned long long totalFramesElapsed = 0;
	double currentFrameDelta = 0.0;

	bool isFrameRunning = false;
	bool isInGameplayState = false;

public:
	LvGameClock();
	LvGameClock(LvGameClock const&) = delete;
	LvGameClock& operator=(LvGameClock const&) = delete;

	static inline LARGE_INTEGER frequency{};

	void StartNewFrame();
	void EndFrame();

	// Get the wall clock time since the current frame started.
	double GetCurrentFrameTime() const;

	unsigned long long GetCurrentFrameId() const { return totalFramesElapsed; }
	void IncrementFrameId() { ++totalFramesElapsed; }

	double GetCurrentFrameDelta() const { return currentFrameDelta; }

	unsigned long long GetTotalTimeMillis() const { return (this->currFrameBeginTick.QuadPart - creationTick.QuadPart) / (frequency.QuadPart / 1000); }
	unsigned long long GetInGameMillis() const { return isInGameplayState ? (this->currFrameBeginTick.QuadPart - gameStartTick.QuadPart) / (frequency.QuadPart / 1000) : 0; }
};

class LvGameTimer final
{
private:
	LARGE_INTEGER beginFrameTime; // time when the measurement was taken

public:
	LvGameTimer() : LvGameTimer(false) {}
	LvGameTimer(bool noInit); // "noInit" will not initialize the timer, leaves the beginTime at zero
	LvGameTimer& operator=(LvGameTimer const& other) { this->beginFrameTime.QuadPart = other.beginFrameTime.QuadPart; return *this; }

	void Reset();
	void Reset(double offsetSeconds);
	void ResetToServerStart();
	double Seconds() const;

	static double SecondsSinceServerInit();
	static double SecondsSinceGameStart();
};

inline LvGameClock* lvClock;
