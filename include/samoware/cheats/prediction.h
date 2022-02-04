
#pragma once

#include "cfw/singleton.h"

#include "../sdk/igamemovement.h"

class CUserCmd;

class EnginePrediction : public cfw::Singleton<EnginePrediction> {
public:
	int unpredictedFlags;
	int predictedFlags;

	CMoveData moveData;

public:
	EnginePrediction(token);

	float GetServerTime(CUserCmd* cmd = nullptr, int tickOffset = 0) const;

	void Start(CUserCmd* cmd);
	void Finish();

	void StartSimulation(CBasePlayer* ply, CUserCmd* cmd);
	void SimulateTick(CBasePlayer* ply, CUserCmd* cmd, int numTick);
	void FinishSimulation(CBasePlayer* ply);

private:
	float _unpredictedCurtime;
	float _unpredictedFrametime;

	Vector _unpredictedVelocity;

	float _unsimulatedCurtime;
	float _unsimulatedFrametime;

	int _unsimulatedFlags;

	int _ticksToSimulate;

	int* _predictionRandomSeed;
};
