#include "LvGameClock.h"

#include "NvLib.h"
#include "LvTypes.h"
#include "LvGame.h"

LvGameClock::LvGameClock()
{
	LogAssert(frequency.QuadPart == 0); // we don't expect multiple instantiations of the clock

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&this->creationTick);
}

void LvGameClock::StartNewFrame()
{
	LogAssert(!this->isFrameRunning);

	this->isFrameRunning = true;
	if (!this->isInGameplayState && lvGame->IsGameplayRunning())
	{
		QueryPerformanceCounter(&this->gameStartTick);
		this->isInGameplayState = true;
	}

	this->lastFrameBeginTick.QuadPart = this->currFrameBeginTick.QuadPart;
	QueryPerformanceCounter(&this->currFrameBeginTick);

	this->currentFrameDelta = (float)((this->currFrameBeginTick.QuadPart - this->lastFrameBeginTick.QuadPart) / ((double)frequency.QuadPart));
}

void LvGameClock::EndFrame()
{
	LogAssert(this->isFrameRunning);

	this->isFrameRunning = false;
	this->totalFramesElapsed++;
}

double LvGameClock::GetCurrentFrameTime() const
{
	LogAssert(this->isFrameRunning);

	LARGE_INTEGER currTick;
	QueryPerformanceCounter(&currTick);

	return (currTick.QuadPart - this->currFrameBeginTick.QuadPart) / (frequency.QuadPart / 1000.0);
}

LvGameTimer::LvGameTimer(bool noInit)
{
	if (noInit)
		this->beginFrameTime.QuadPart = 0;
	else
		this->Reset();
}

void LvGameTimer::Reset()
{
	this->beginFrameTime = lvClock->currFrameBeginTick;
}

void LvGameTimer::Reset(double offsetSeconds)
{
	this->Reset();
	this->beginFrameTime.QuadPart += (long long)(LvGameClock::frequency.QuadPart * offsetSeconds);
}

void LvGameTimer::ResetToServerStart()
{
	this->beginFrameTime = lvClock->creationTick;
}

double LvGameTimer::Seconds() const
{
	return (lvClock->currFrameBeginTick.QuadPart - this->beginFrameTime.QuadPart) / (double)LvGameClock::frequency.QuadPart;
}

double LvGameTimer::SecondsSinceServerInit()
{
	return lvClock->GetTotalTimeMillis() / 1000.0;
}

double LvGameTimer::SecondsSinceGameStart()
{
	return lvClock->GetInGameMillis() / 1000.0;
}
