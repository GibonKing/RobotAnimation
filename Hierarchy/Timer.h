#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer {
	public:
		Timer() {}
		~Timer(){}

		void Update() {
			if (isSetup) {
				prevTime = GetCurrentTime();
				isSetup = true;
			}
			currentTime = GetCurrentTime();
			deltaTime = (currentTime - prevTime) * 0.001;
			prevTime = currentTime;
		}

		float GetDeltaTime() {
			return deltaTime;
		}
	private:
		uint64_t GetCurrentTime() {
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		uint64_t currentTime, prevTime;
		float deltaTime;
		bool isSetup = false;
};

#endif