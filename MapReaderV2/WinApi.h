#pragma once
#include <iostream>
#include <chrono>
#include <vector>
#include <string>

#include <Windows.h>
#include <tchar.h>

template<typename ...Args>
FORCEINLINE void LogFatalErrorEx(const char* Error, Args... args) {
	time_t Time = time(NULL);
	const tm* LocalTime = localtime(&Time);

	std::string format = "::FATAL[%.2d:%.2d:%.2d]:: ";

	format += Error;

	format += "\n";

	printf(format.c_str(), LocalTime->tm_hour, LocalTime->tm_min, LocalTime->tm_sec, std::forward<Args>(args)...);
}

template<typename ...Args>
FORCEINLINE void LogWarningEx(const char* Error, Args... args) {
	time_t Time = time(NULL);
	const tm* LocalTime = localtime(&Time);

	std::string format = "::WARNING[%.2d:%.2d:%.2d]:: ";

	format += Error;

	format += "\n";

	printf(format.c_str(), LocalTime->tm_hour, LocalTime->tm_min, LocalTime->tm_sec, std::forward<Args>(args)...);
}

template<typename ...Args>
FORCEINLINE void LogInfoEx(const char* Error, Args... args) {
	time_t Time = time(NULL);
	const tm* LocalTime = localtime(&Time);

	std::string format = "::INFO[%.2d:%.2d:%.2d]:: ";

	format += Error;

	format += "\n";

	printf(format.c_str(), LocalTime->tm_hour, LocalTime->tm_min, LocalTime->tm_sec, std::forward<Args>(args)...);
}

static char messageBuffer[2048];

template<typename ...Args>
FORCEINLINE void Message(const char* Error, Args... args) {
	std::string format;
	format += Error;
	format += "\n";

	sprintf_s(messageBuffer, format.c_str(), std::forward<Args>(args)...);

	MessageBoxA(NULL, messageBuffer, "Info", MB_OK);
}

template<class T>
struct TRef {
	using LPT = T*;

	TRef() : Ptr(nullptr) {}
	TRef(LPT Ptr) : Ptr(Ptr) {}
	TRef(const TRef& Other) : Ptr(Other.Ptr) {}
	TRef(TRef&& Other) : Ptr(Other.Ptr) {
		Other.Ptr = nullptr;
	}

	FORCEINLINE LPT Release() noexcept {
		LPT Temp = Ptr;
		Ptr = nullptr;
		return Temp;
	}
	FORCEINLINE void Reset(LPT NewPtr = nullptr) noexcept {
		Ptr = NewPtr;
	}
	FORCEINLINE LPT Get() const noexcept {
		return Ptr;
	}
	FORCEINLINE LPT* GetSelfPtr() noexcept {
		return &Ptr;
	}
	FORCEINLINE LPT operator->() const noexcept {
		return Ptr;
	}
	FORCEINLINE void operator=(const TRef& Other) noexcept {
		Ptr = Other.Ptr;
	}
	FORCEINLINE void operator=(LPT Ptr) noexcept {
		Reset(Ptr);
	}

	~TRef() {
		Ptr = nullptr;
	}

	LPT	Ptr = nullptr;
};

struct Timer {
	float			Elapsed = 0.0f;
	float			TotalTime = 0.0f;
	float			FrequencySeconds = 0.0f;
	LARGE_INTEGER	I;
	LONGLONG		Start = 0;

	Timer() {
		Elapsed = TotalTime = FrequencySeconds = 0.0;
		Start = 0;
		I.QuadPart = 0;
	}

	INT32 Init();

	inline double Tick() noexcept {
		QueryPerformanceCounter(&I);
		Elapsed = (float)(I.QuadPart - Start) / FrequencySeconds;

		Start = I.QuadPart;
		TotalTime += Elapsed;

		return TotalTime;
	}
};

FORCEINLINE UINT64 TimerGetCurrentUTC() {
	return time(NULL);
}
FORCEINLINE UINT32 TimerGetDaySeconds() {
	return 0;
}

template<typename T>
struct RawDataWrapper {
	T Value;

	RawDataWrapper() { memset(&Value, 0, sizeof(T)); }
	RawDataWrapper(const T& Other) { memcpy_s(&Value, sizeof(T), &Other, sizeof(T)); }
	RawDataWrapper(const RawDataWrapper& Other) { memcpy_s(&Value, sizeof(T), &Other, sizeof(T)); }
	void operator=(const RawDataWrapper& Other) { memcpy_s(&Value, sizeof(T), &Other, sizeof(T)); }
};

#define NOVTABLE __declspec(novtable)

void GetDesktopResolution(int& horizontal, int& vertical);