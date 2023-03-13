
#pragma once

#include <stack>
#include <vector>

#include "cfw/singleton.h"

#include "../sdk/igamemovement.h"
#include "../sdk/cusercmd.h"
#include "../sdk/vector.h"
#include "../sdk/tier0.h"

class CBaseEntity;
struct MovementSimulationDataInput {
	float timeUntilTimeout;
	float deltaTime;
	float idealYawDelta;
	float maxSpeedLoss;
	bool sandboxAccel;

	float gravity;
	float currentGravity;
	float jumpPower;
	CBaseEntity* groundEntity;
	Vector obbMins;
	Vector obbMaxs;
	float m_surfaceFriction;
	bool wasJumping;

	float sv_airaccelerate;

	Angle m_vecViewAngles;
	Vector m_vecVelocity;
	Vector m_vecAbsOrigin;

	float m_flForwardMove;
	float m_flSideMove;

	int m_nButtons;
	int m_nOldButtons;

	float m_flMaxSpeed;
};

struct MovementSimulationDataOutput {
	float timeUntilTimeout;
	Vector m_vecVelocity;
	Vector m_vecAbsOrigin;
};

/*
class ThreadedMovementSimulation : public cfw::Singleton<ThreadedMovementSimulation> {
public:
	ThreadedMovementSimulation(token);

	void CreateJob(MovementSimulationDataInput* data);
	bool WaitForNext(std::stack<MovementSimulationDataOutput>& outData);
	std::size_t ActiveThreads() const { return _threads.size(); }

	struct ThreadInfo {
		tier0::ThreadHandle_t threadHandle;
		std::stack<MovementSimulationDataOutput> simulatedTicks;
	};

	struct ThreadInput {
		ThreadInfo* threadInfo;
		MovementSimulationDataInput* simData;
	};

private:
	std::stack<ThreadInfo> _threads;
};
*/

#include <condition_variable>

struct JobInput {
	size_t index;
	MovementSimulationDataInput input;
};

struct JobOutput {
	size_t index;
	std::vector<MovementSimulationDataOutput> ticks;
};

// thread pool version
class ThreadedMovementSimulation : public cfw::Singleton<ThreadedMovementSimulation> {
public:
	ThreadedMovementSimulation(token);

	void CreateThreads(size_t numThreads);
	void ReleaseAllThreads();

	size_t NumThreads() {
		return _threads.size();
	}

	void CreateJob(MovementSimulationDataInput& input, size_t index);
	void ClearJobs();
	void WaitForAllJobs(size_t numJobs);

public:
	std::mutex job_mutex;
	std::condition_variable input_cond;
	std::condition_variable output_cond;

	bool should_stop;
	std::condition_variable stop_cond;

	std::stack<JobInput> job_inputs;
	std::vector<JobOutput> job_outputs;

private:
	std::vector<tier0::ThreadHandle_t> _threads;
};

