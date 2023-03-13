
#include "samoware/cheats/threaded_simulation.h"

#include "samoware/sdk/ienginetrace.h"
#include "samoware/sdk/iengineclient.h"
#include "samoware/sdk/icliententitylist.h"
#include "samoware/interfaces.h"

static void TraceBBoxRay(MovementSimulationDataInput* moveData, const Vector& startPos, const Vector& endPos, trace_t* pTrace) {
	Ray_t ray;
	ray.Init(startPos, endPos, moveData->obbMins, moveData->obbMaxs);

	const int MASK_PLAYERSOLID = 33636363;
	CTraceFilterWorldOnly filter;
	//filter.pSkip = interfaces::entityList->GetClientEntity(interfaces::engineClient->GetLocalPlayer());
	interfaces::engineTrace->TraceRay(ray, MASK_PLAYERSOLID, &filter, pTrace);
}

static void StartGravity(MovementSimulationDataInput* moveData) {
	float entGravity = 1.f;
	if (moveData->gravity != 0)
		entGravity = moveData->gravity;

	moveData->m_vecVelocity.z -= entGravity * moveData->currentGravity * 0.5f * moveData->deltaTime;
}

static void FinishGravity(MovementSimulationDataInput* moveData) {
	float entGravity = 1.f;
	if (moveData->gravity != 0)
		entGravity = moveData->gravity;

	// Get the correct velocity for the end of the dt
	moveData->m_vecVelocity.z -= entGravity * moveData->currentGravity * 0.5f * moveData->deltaTime;
}

static void CheckJumpButton(MovementSimulationDataInput* moveData) {
	// No more effect
	if (!moveData->groundEntity) {
		moveData->m_nOldButtons |= CUserCmd::IN_JUMP;
		return; 
	}

	if (moveData->m_nOldButtons & CUserCmd::IN_JUMP) {
		return; // Don't pogo stick
	}

	// In the air now
	moveData->groundEntity = nullptr;

	float flGroundFactor = 1.f;
	float flMul = moveData->jumpPower;

	moveData->m_vecVelocity.z += flGroundFactor * flMul; // 2 * gravity * height

	FinishGravity(moveData);

	moveData->m_nOldButtons |= CUserCmd::IN_JUMP; // Don't jump again until released
}

static void AirAccelerate(MovementSimulationDataInput* moveData, const Vector& wishDir, float wishSpeed, float accel) {
	float wishSpd = wishSpeed;

	// Cap speed
	const float AIR_SPEED_CAP = 30.f;
	if (wishSpd > AIR_SPEED_CAP)
		wishSpd = AIR_SPEED_CAP;

	// Determine veer amount
	float currentSpeed = moveData->m_vecVelocity.Dot(wishDir);

	// See how much to add
	float addSpeed = wishSpd - currentSpeed;

	// If not adding any, done
	if (addSpeed <= 0)
		return;

	// Determine acceleration speed after acceleration
	float accelSpeed = accel * wishSpeed * moveData->deltaTime * moveData->m_surfaceFriction;

	// Cap it
	if (accelSpeed > addSpeed)
		accelSpeed = addSpeed;

	// Adjust pmove vel.
	moveData->m_vecVelocity += wishDir * accelSpeed;
}

/*
static void TryPlayerMove(MovementSimulationDataInput* moveData) {
	Vector startPos = moveData->m_vecAbsOrigin;
	Vector endPos = moveData->m_vecAbsOrigin;
	endPos += moveData->m_vecVelocity * Vector(1.f, 1.f, 0.f) * moveData->deltaTime;

	trace_t trace;
	TraceBBoxRay(moveData, startPos, endPos, &trace);

	if (trace.fraction < 1.f || trace.allsolid || trace.startsolid) {
 		moveData->m_vecAbsOrigin = trace.endpos;

		// Don't drop velocity on flat planes
		const auto& normal = trace.plane.normal;
		if (normal.x != 0 && normal.y != 0 && normal.z != 1)
			moveData->m_vecVelocity = Vector(0.f, 0.f, 0.f);

		return;
	}

	moveData->m_vecAbsOrigin += moveData->m_vecVelocity * moveData->deltaTime;
}
*/

static void ClipVelocity(Vector& in, Vector& normal, Vector& out, float overbounce) {
	float	backoff;
	float	change;
	float angle;
	int		i;

	angle = normal.z;

	// Determine how far along plane to slide based on incoming direction.
	backoff = in.Dot(normal) * overbounce;
	out = in - normal * backoff;

	// iterate once to make sure we aren't still moving through the plane
	float adjust = out.Dot(normal);
	if (adjust < 0.0f) 
		out -= (normal * adjust);
}

static void TryPlayerMove(MovementSimulationDataInput* moveData) {
	const int MAX_CLIP_PLANES = 5;

	int			bumpcount, numbumps;
	Vector		dir;
	float		d;
	int			numplanes;
	Vector		planes[MAX_CLIP_PLANES];
	Vector		primal_velocity, original_velocity;
	Vector      new_velocity;
	int			i, j;
	trace_t	pm;
	Vector		end;
	float		time_left, allFraction;
	int			blocked;

	numbumps = 4;           // Bump up to four times

	blocked = 0;           // Assume not blocked
	numplanes = 0;           //  and not sliding along any planes

	original_velocity = moveData->m_vecVelocity;  // Store original velocity
	primal_velocity = moveData->m_vecVelocity;

	allFraction = 0;
	time_left = moveData->deltaTime;   // Total time for this movement operation.

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++) {
		if (moveData->m_vecVelocity.Length() == 0.0)
			break;

		// Assume we can move all the way from the current origin to the
		//  end point.
		end = moveData->m_vecAbsOrigin + moveData->m_vecVelocity * time_left;

		// See if we can make it from origin to end point.
		TraceBBoxRay(moveData, moveData->m_vecAbsOrigin, end, &pm);

		allFraction += pm.fraction;

		// If we started in a solid object, or we were in solid space
		//  the whole way, zero out our velocity and return that we
		//  are blocked by floor and wall.
		if (pm.allsolid) {
			// entity is trapped in another solid
			moveData->m_vecVelocity = Vector(0.f, 0.f, 0.f);
			return;
		}

		// If we moved some portion of the total distance, then
		//  copy the end position into the pmove.origin and 
		//  zero the plane counter.
		if (pm.fraction > 0) {
			if (numbumps > 0 && pm.fraction == 1) {
				// There's a precision issue with terrain tracing that can cause a swept box to successfully trace
				// when the end position is stuck in the triangle.  Re-run the test with an uswept box to catch that
				// case until the bug is fixed.
				// If we detect getting stuck, don't allow the movement
				trace_t stuck;
				TraceBBoxRay(moveData, pm.endpos, pm.endpos, &stuck);
				if (stuck.startsolid || stuck.fraction != 1.0f) {
					//Msg( "Player will become stuck!!!\n" );
					moveData->m_vecVelocity = Vector(0.f, 0.f, 0.f);
					break;
				}
			}

			// actually covered some distance
			moveData->m_vecAbsOrigin = pm.endpos;
			original_velocity = moveData->m_vecVelocity;
			numplanes = 0;
		}

		// If we covered the entire distance, we are done
		//  and can return.
		if (pm.fraction == 1) {
			break;		// moved the entire distance
		}

		// If the plane we hit has a high z component in the normal, then
		//  it's probably a floor
		if (pm.plane.normal.z > 0.7f) {
			blocked |= 1;		// floor
		}
		// If the plane has a zero z component in the normal, then it's a 
		//  step or wall
		if (!pm.plane.normal.z) {
			blocked |= 2;		// step / wall
		}

		// Reduce amount of m_flFrameTime left by total time left * fraction
		//  that we covered.
		time_left -= time_left * pm.fraction;

		// Did we run out of planes to clip against?
		if (numplanes >= MAX_CLIP_PLANES) {
			// this shouldn't really happen
			//  Stop our movement if so.
			moveData->m_vecVelocity = Vector(0.f, 0.f, 0.f);
			//Con_DPrintf("Too many planes 4\n");

			break;
		}

		// Set up next clipping plane
		planes[numplanes] = pm.plane.normal;
		numplanes++;

		// modify original_velocity so it parallels all of the clip planes
		//

		// reflect player velocity 
		// Only give this a try for first impact plane because you can get yourself stuck in an acute corner by jumping in place
		//  and pressing forward and nobody was really using this bounce/reflection feature anyway...
		if (numplanes == 1 && moveData->groundEntity == nullptr) {
			for (i = 0; i < numplanes; i++) {
				if (planes[i].z > 0.7f) {
					// floor or slope
					ClipVelocity(original_velocity, planes[i], new_velocity, 1);
					original_velocity = new_velocity;
				} else {
					ClipVelocity(original_velocity, planes[i], new_velocity, 1.0 + /*sv_bounce.GetFloat()*/ 0.f * (1 - moveData->m_surfaceFriction));
				}
			}

			moveData->m_vecVelocity = new_velocity;
			original_velocity = new_velocity;
		} else {
			for (i = 0; i < numplanes; i++) {
				ClipVelocity(
					original_velocity,
					planes[i],
					moveData->m_vecVelocity,
					1);

				for (j = 0; j < numplanes; j++) {
					// Are we now moving against this plane?
					if (j != i && moveData->m_vecVelocity.Dot(planes[j]) < 0)
						break;	// not ok
				}

				if (j == numplanes)  // Didn't have to clip, so we're ok
					break;
			}

			// Did we go all the way through plane set
			if (i != numplanes) {	// go along this plane
				// pmove.velocity is set in clipping call, no need to set again.
				;
			} else {	// go along the crease
				if (numplanes != 2) {
					moveData->m_vecVelocity = Vector(0.f, 0.f, 0.f);
					break;
				}

				dir = planes[0].Cross(planes[1]);
				dir.Normalize();
				d = dir.Dot(moveData->m_vecVelocity);
				moveData->m_vecVelocity = dir * d;
			}

			//
			// if original velocity is against the original velocity, stop dead
			// to avoid tiny occilations in sloping corners
			//
			d = moveData->m_vecVelocity.Dot(primal_velocity);
			if (d <= 0) {
				moveData->m_vecVelocity = Vector(0.f, 0.f, 0.f);
				break;
			}
		}
	}

	if (allFraction == 0) {
		moveData->m_vecVelocity = Vector(0.f, 0.f, 0.f);
	}
}

static void AirMove(MovementSimulationDataInput* moveData) {
	// Determine movement angles
	Vector forward, right, up;
	moveData->m_vecViewAngles.Vectors(forward, right, up);

	// Copy movement amounts
	float fMove = moveData->m_flForwardMove;
	float sMove = moveData->m_flSideMove;

	// Zero out z components of movement vectors
	forward.z = right.z = 0.f;

	// Normalize remainder of vectors
	forward.Normalize();
	right.Normalize();

	// Determine x and y parts of velocity
	Vector wishVel = forward * fMove + right * sMove;

	// Zero out z part of velocity
	wishVel.z = 0.f;

	Vector wishDir = wishVel; // Determine maginitude of speed of move
	float wishSpeed = wishDir.Normalize();

	// Clamp to server defined max speed
	if (wishSpeed != 0 && wishSpeed > moveData->m_flMaxSpeed) {
		wishVel *= moveData->m_flMaxSpeed / wishSpeed;
		wishSpeed = moveData->m_flMaxSpeed;
	}

	AirAccelerate(moveData, wishDir, wishSpeed, moveData->sv_airaccelerate);

	TryPlayerMove(moveData);
}

static void TryTouchGround(MovementSimulationDataInput* moveData) {
	float flOffset = 2.f;

	Vector startPos = moveData->m_vecAbsOrigin;
	Vector endPos = moveData->m_vecAbsOrigin;
	endPos.z -= flOffset;

	// Try and move down
	trace_t trace;
	TraceBBoxRay(moveData, startPos, endPos, &trace);

	// If we are on something... (SetGroundEntity)
	if (trace.fraction < 1.f || trace.allsolid || trace.startsolid) {
		moveData->groundEntity = trace.m_pEnt;
		moveData->m_vecVelocity.z = 0.f;
		// moveData->m_vecAbsOrigin = trace.endpos;
		return;
	}

	moveData->groundEntity = nullptr;
}

static void StartMove(MovementSimulationDataInput* moveData) {
	// Only apply the jump boost in FinishMove if the player has jumped during this frame
	moveData->wasJumping = (moveData->m_nButtons & CUserCmd::IN_JUMP) && !(moveData->m_nOldButtons & CUserCmd::IN_JUMP) && (moveData->groundEntity != nullptr);
}

static void FinishMove(MovementSimulationDataInput* moveData) {
	// If the player has jumped this frame
	if (moveData->wasJumping) {
		// Get their orientation
		Angle viewAngles = moveData->m_vecViewAngles;
		viewAngles.p = 0.f;
		Vector forward = viewAngles.Forward();

		// Compute the speed boost
		float speedBoostPerc = 0.5f;

		float speedAddition = std::abs(moveData->m_flForwardMove * speedBoostPerc);
		float maxSpeed = moveData->m_flMaxSpeed * (1 + speedBoostPerc);
		float newSpeed = speedAddition + moveData->m_vecVelocity.Length2D();

		// Clamp it to make sure they can't bunnyhop to ludicrous speed
		/*
		if (newSpeed > maxSpeed)
			speedAddition = speedAddition - (newSpeed - maxSpeed)
		*/

		// Reverse it if the player is running backwards
		if (moveData->m_vecVelocity.Dot(forward) < 0.f)
			speedAddition *= -1.f;

		// Apply the speed boost
		moveData->m_vecVelocity += forward * speedAddition;
	}
}

static void CheckParameters(MovementSimulationDataInput* moveData) {
	float spd = (moveData->m_flForwardMove * moveData->m_flForwardMove) +
				(moveData->m_flSideMove * moveData->m_flSideMove);
	
	float maxSpeed = moveData->m_flMaxSpeed;

	if ((spd != 0.f) && (spd > moveData->m_flMaxSpeed)) {
		float fRatio = moveData->m_flMaxSpeed / std::sqrt(spd);
		moveData->m_flForwardMove *= fRatio;
		moveData->m_flSideMove *= fRatio;
	}
} 

static void CategorizePosition(MovementSimulationDataInput* moveData) {
	// Reset this each time we-recategorize, otherwise we have bogus friction when we jump into water and plunge downward really quickly
	moveData->m_surfaceFriction = 1.f;

	TryTouchGround(moveData);
}

static void PartialProcessMovement(MovementSimulationDataInput* moveData) {
	if (moveData->sandboxAccel)
		StartMove(moveData);

	CategorizePosition(moveData);

	CheckParameters(moveData);

	StartGravity(moveData);

	// Was jump button pressed?
	if (moveData->m_nButtons & CUserCmd::IN_JUMP) {
		CheckJumpButton(moveData);
	} else {
		moveData->m_nOldButtons &= ~CUserCmd::IN_JUMP;
	}

	// Fricion is handled before we add in any base velocity. That way, if we are on a conveyor, 
	//  we don't slow when standing still, relative to the conveyor.
	if (moveData->groundEntity)
		moveData->m_vecVelocity.z = 0.f;

	AirMove(moveData); // Take into account movement when in air.

	// Set final flags
	CategorizePosition(moveData);

	// Add any remaining gravitational component.
	FinishGravity(moveData);

	// If we are on ground, no downward velocity.
	if (moveData->groundEntity) {
		moveData->m_vecVelocity.z = 0.f;
	}

	if (moveData->sandboxAccel)
		FinishMove(moveData);
}

/*
static unsigned int RunSimulation(ThreadedMovementSimulation::ThreadInput* input) {
	auto* moveData = input->simData;

	float initialSpeed = moveData->m_vecVelocity.Length2D();

	while (moveData->timeUntilTimeout > 0.f) {
		// Setup circle strafe data
		{
	
			float yaw = Angle::Normalize180(moveData->m_vecViewAngles.y + moveData->idealYawDelta);
			float sideMove;
			if (std::abs(moveData->idealYawDelta) < 1e-3f)
				sideMove = 0.f;
			else
				sideMove = moveData->idealYawDelta > 0.f ? -5250.f : 5250.f;

			// Force bhop
			int m_nButtons = moveData->m_nButtons;
			if (moveData->groundEntity == nullptr)
				m_nButtons &= ~CUserCmd::IN_JUMP;
			else
				m_nButtons |= CUserCmd::IN_JUMP;

			int m_nOldButtons = m_nButtons & ~CUserCmd::IN_JUMP;

			moveData->m_vecViewAngles = Angle(0.f, yaw, 0.f);
			moveData->m_flForwardMove = 0.f;
			moveData->m_flSideMove = sideMove;
			moveData->m_nButtons = m_nButtons;
			moveData->m_nOldButtons = m_nOldButtons;
		}

		PartialProcessMovement(moveData);
		input->threadInfo->simulatedTicks.emplace(MovementSimulationDataOutput {moveData->timeUntilTimeout, moveData->m_vecVelocity, moveData->m_vecAbsOrigin});

		bool speedLost = (moveData->m_vecVelocity.Length2D() - initialSpeed) >= moveData->maxSpeedLoss;
		if (speedLost)
			break;

		moveData->timeUntilTimeout -= moveData->deltaTime;
	}
}

ThreadedMovementSimulation::ThreadedMovementSimulation(token) {
	
}


void ThreadedMovementSimulation::CreateJob(MovementSimulationDataInput* data) {
	
}

bool ThreadedMovementSimulation::WaitForNext(std::stack<MovementSimulationDataOutput>& outData) {
	if (_threads.empty())
		return false;

	auto& thread = _threads.top();
	bool result = tier0::ThreadJoin(thread.threadHandle, 0xFFFFFFFF);
	if (!result) {
		tier0::ReleaseThreadHandle(thread.threadHandle);
		return false;
	}

	thread.simulatedTicks.swap(outData);
	_threads.pop();

	return tier0::ReleaseThreadHandle(thread.threadHandle);
}
*/

static std::vector<MovementSimulationDataOutput> ProcessJob(MovementSimulationDataInput& job) {
	float initialSpeed = job.m_vecVelocity.Length2D();

	std::vector<MovementSimulationDataOutput> outputs;

	while (job.timeUntilTimeout > 0.f) {
		// Setup circle strafe data
		{

			float yaw = Angle::Normalize180(job.m_vecViewAngles.y + job.idealYawDelta);
			float sideMove;
			if (std::abs(job.idealYawDelta) < 1e-3f)
				sideMove = 0.f;
			else
				sideMove = job.idealYawDelta > 0.f ? -5250.f : 5250.f;

			// Force bhop
			int m_nButtons = job.m_nButtons;
			if (job.groundEntity == nullptr)
				m_nButtons &= ~CUserCmd::IN_JUMP;
			else
				m_nButtons |= CUserCmd::IN_JUMP;

			int m_nOldButtons = m_nButtons & ~CUserCmd::IN_JUMP;

			job.m_vecViewAngles = Angle(0.f, yaw, 0.f);
			job.m_flForwardMove = 0.f;
			job.m_flSideMove = sideMove;
			job.m_nButtons = m_nButtons;
			job.m_nOldButtons = m_nOldButtons;
		}

		PartialProcessMovement(&job);

		outputs.push_back(MovementSimulationDataOutput {
			job.timeUntilTimeout,
			job.m_vecVelocity,
			job.m_vecAbsOrigin
		});

		bool speedLost = (job.m_vecVelocity.Length2D() - initialSpeed) >= job.maxSpeedLoss;
		if (speedLost)
			break;

		job.timeUntilTimeout -= job.deltaTime;
	}

	return outputs;
}

static unsigned int ThreadLoop() {
	auto& simulation = ThreadedMovementSimulation::Get();

	while (true) {
		// Get a new job
		JobInput input;
		{
			std::unique_lock<std::mutex> lock(simulation.job_mutex);
			simulation.input_cond.wait(lock, [&simulation] {
				return !simulation.job_inputs.empty() || simulation.should_stop;
			});

			if (simulation.should_stop)
				break;

			input = std::move(simulation.job_inputs.top());
			simulation.job_inputs.pop();
		}

		auto output = ProcessJob(input.input);

		{
			std::unique_lock<std::mutex> lock(simulation.job_mutex);
			simulation.job_outputs.emplace_back(input.index, std::move(output));
			simulation.output_cond.notify_one();
		}
	}

	return 0;
}

ThreadedMovementSimulation::ThreadedMovementSimulation(token) {

}

void ThreadedMovementSimulation::CreateThreads(size_t numThreads) {
	// Remove old threads
	ReleaseAllThreads();

	should_stop = false;

	// Create new threads
	for (auto i = 0; i < numThreads; i++) {
		auto thread = tier0::CreateSimpleThread(reinterpret_cast<tier0::ThreadFunc_t>(ThreadLoop), nullptr, 0);
		_threads.push_back(thread);
	}
}

void ThreadedMovementSimulation::ReleaseAllThreads() {
	{
		std::unique_lock<std::mutex> lock(job_mutex);
		should_stop = true;
	}

	input_cond.notify_all();

	for (auto& thread : _threads) {
		tier0::ThreadJoin(thread, 0xFFFFFFFF);
		tier0::ReleaseThreadHandle(thread);
	}

	_threads.clear();
}

void ThreadedMovementSimulation::CreateJob(MovementSimulationDataInput& input, size_t index) {
	{
		std::unique_lock<std::mutex> lock(job_mutex);
		job_inputs.push({index, input});
	}

	input_cond.notify_one();
}

void ThreadedMovementSimulation::ClearJobs() {
	while (!job_inputs.empty())
		job_inputs.pop();

	job_outputs.clear();
}

void ThreadedMovementSimulation::WaitForAllJobs(size_t numJobs) {
	{
		std::unique_lock<std::mutex> lock(job_mutex);
		output_cond.wait(lock, [this, numJobs] {
			return job_outputs.size() == numJobs;
		});
	}
}
