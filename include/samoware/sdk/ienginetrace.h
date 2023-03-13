
#pragma once

#include "vector.h"
#include "angle.h"

// https://tenor.com/view/going-insane-cat-gif-22428080
class matrix3x4_t;
struct Ray_t {
public:
	VectorAligned m_start;
	VectorAligned m_delta;
	VectorAligned m_start_offset;
	VectorAligned m_extents;
	const matrix3x4_t* m_world_axis_transform;
	bool m_is_ray;
	bool m_is_swept;

	Ray_t() : m_world_axis_transform(nullptr), m_is_ray {false}, m_is_swept {false} { }

	Ray_t(Vector const& start, Vector const& end) {
		Init(start, end);
	}

	Ray_t(Vector const& start, Vector const& end, Vector const& mins, Vector const& maxs) {
		m_delta = end - start;

		m_world_axis_transform = nullptr;
		m_is_swept = m_delta.Length() != 0;

		m_extents = maxs - mins;
		m_extents *= 0.5f;
		m_is_ray = m_extents.LengthSqr() < 1e-6;

		m_start_offset = maxs + mins;
		m_start_offset *= 0.5f;
		m_start = start + m_start_offset;
		m_start_offset *= -1.0f;
	}

	void Init(Vector const& start, Vector const& end) {
		m_delta = end - start;

		m_is_swept = m_delta.LengthSqr() != 0;

		m_extents = Vector(0.f, 0.f, 0.f);

		m_world_axis_transform = nullptr;
		m_is_ray = true;

		m_start_offset = Vector(0.f, 0.f, 0.f);
		m_start = start;
	}

	void Init(Vector const& start, Vector const& end, Vector const& mins, Vector const& maxs) {
		m_delta = end - start;

		m_world_axis_transform = nullptr;
		m_is_swept = m_delta.Length() != 0;

		m_extents = maxs - mins;
		m_extents *= 0.5f;
		m_is_ray = m_extents.LengthSqr() < 1e-6;

		m_start_offset = maxs + mins;
		m_start_offset *= 0.5f;
		m_start = start + m_start_offset;
		m_start_offset *= -1.0f;
	}
};

struct csurface_t {
	const char* name;
	short surfaceProps;
	unsigned short flags;
};

struct cplane_t {
	Vector	normal;
	float	dist;
	uint8_t	type;			// for fast side tests
	uint8_t	signbits;		// signx + (signy<<1) + (signz<<1)
	uint8_t	pad[2];

	cplane_t() {}

private:
	// No copy constructors allowed if we're in optimal mode
	cplane_t(const cplane_t& vOther);
};

class CBaseEntity;
struct trace_t {
	Vector startpos;
	Vector endpos;
	cplane_t	plane;
	float fraction;
	int contents;
	unsigned short dispFlags;
	bool allsolid;
	bool startsolid;
	float fractionleftsolid;
	csurface_t	surface;
	int hitgroup;
	short physicsbone;
	CBaseEntity* m_pEnt;
	int hitbox;
};

class IHandleEntity;
class ICollideable;
class CPhysCollide;

template <typename T>
class CUtlVector;

class Vector4D;
class IEntityEnumerator;

class CTraceListData;

enum TraceType_t {
	TRACE_EVERYTHING = 0,
	TRACE_WORLD_ONLY,				// NOTE: This does *not* test static props!!!
	TRACE_ENTITIES_ONLY,			// NOTE: This version will *not* test static props
	TRACE_EVERYTHING_FILTER_PROPS,	// NOTE: This version will pass the IHandleEntity for props through the filter, unlike all other filters
};

class ITraceFilter {
public:
	virtual bool			ShouldHitEntity(void* pEntity, int mask) = 0;
	virtual TraceType_t            GetTraceType() const = 0;
};

// This is the one most normal traces will inherit from
class CTraceFilter : public ITraceFilter {
public:
	bool ShouldHitEntity(void* pEntity, int contentsMask) {
		return pEntity != pSkip;
	}

	virtual TraceType_t	GetTraceType() const {
		return TRACE_EVERYTHING;
	}

	void* pSkip;
};

class CTraceFilterWorldOnly : public ITraceFilter {
public:
	virtual bool ShouldHitEntity(void* pEntity, int contentsMask) {
		return false;
	}

	virtual TraceType_t GetTraceType() const {
		return TRACE_WORLD_ONLY;
	}
};

class IEngineTrace {
public:
	// Returns the contents mask + entity at a particular world-space position
	virtual int		GetPointContents(const Vector& vecAbsPosition, IHandleEntity** ppEntity = NULL) = 0;

	// Get the point contents, but only test the specific entity. This works
	// on static props and brush models.
	//
	// If the entity isn't a static prop or a brush model, it returns CONTENTS_EMPTY and sets
	// bFailed to true if bFailed is non-null.
	virtual int		GetPointContents_Collideable(ICollideable* pCollide, const Vector& vecAbsPosition) = 0;

	// Traces a ray against a particular entity
	virtual void	ClipRayToEntity(const Ray_t& ray, unsigned int fMask, IHandleEntity* pEnt, trace_t* pTrace) = 0;

	// Traces a ray against a particular entity
	virtual void	ClipRayToCollideable(const Ray_t& ray, unsigned int fMask, ICollideable* pCollide, trace_t* pTrace) = 0;

	// A version that simply accepts a ray (can work as a traceline or tracehull)
	virtual void	TraceRay(const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace) = 0;

	// A version that sets up the leaf and entity lists and allows you to pass those in for collision.
	virtual void	SetupLeafAndEntityListRay(const Ray_t& ray, CTraceListData& traceData) = 0;
	virtual void    SetupLeafAndEntityListBox(const Vector& vecBoxMin, const Vector& vecBoxMax, CTraceListData& traceData) = 0;
	virtual void	TraceRayAgainstLeafAndEntityList(const Ray_t& ray, CTraceListData& traceData, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace) = 0;

	// A version that sweeps a collideable through the world
	// abs start + abs end represents the collision origins you want to sweep the collideable through
	// vecAngles represents the collision angles of the collideable during the sweep
	virtual void	SweepCollideable(ICollideable* pCollide, const Vector& vecAbsStart, const Vector& vecAbsEnd,
		const Angle& vecAngles, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace) = 0;

	// Enumerates over all entities along a ray
	// If triggers == true, it enumerates all triggers along a ray
	virtual void	EnumerateEntities(const Ray_t& ray, bool triggers, IEntityEnumerator* pEnumerator) = 0;

	// Same thing, but enumerate entitys within a box
	virtual void	EnumerateEntities(const Vector& vecAbsMins, const Vector& vecAbsMaxs, IEntityEnumerator* pEnumerator) = 0;

	// Convert a handle entity to a collideable.  Useful inside enumer
	virtual ICollideable* GetCollideable(IHandleEntity* pEntity) = 0;

	// HACKHACK: Temp for performance measurments
	virtual int GetStatByIndex(int index, bool bClear) = 0;


	//finds brushes in an AABB, prone to some false positives
	virtual void GetBrushesInAABB(const Vector& vMins, const Vector& vMaxs, CUtlVector<int>* pOutput, int iContentsMask = 0xFFFFFFFF) = 0;

	//Creates a CPhysCollide out of all displacements wholly or partially contained in the specified AABB
	virtual CPhysCollide* GetCollidableFromDisplacementsInAABB(const Vector& vMins, const Vector& vMaxs) = 0;

	//retrieve brush planes and contents, returns true if data is being returned in the output pointers, false if the brush doesn't exist
	virtual bool GetBrushInfo(int iBrush, CUtlVector<Vector4D>* pPlanesOut, int* pContentsOut) = 0;

	virtual bool PointOutsideWorld(const Vector& ptTest) = 0; //Tests a point to see if it's outside any playable area

	// Walks bsp to find the leaf containing the specified point
	virtual int GetLeafContainingPoint(const Vector& ptTest) = 0;
};
