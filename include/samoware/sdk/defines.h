
#pragma once

enum class EntityFlags {
	ONGROUND = (1 << 0),
	DUCKING = (1 << 1),
	WATERJUMP = (1 << 2),
	ONTRAIN = (1 << 3),
	INRAIN = (1 << 4),
	FROZEN = (1 << 5),
	ATCONTROLS = (1 << 6),
	CLIENT = (1 << 7),
	FAKECLIENT = (1 << 8),

	INWATER = (1 << 9),

	FLY = (1 << 10),
	SWIM = (1 << 11),
	CONVEYOR = (1 << 12),
	NPC = (1 << 13),
	GODMODE = (1 << 14),
	NOTARGET = (1 << 15),
	AIMTARGET = (1 << 16),
	PARTIALGROUND = (1 << 17),
	STATICPROP = (1 << 18),
	GRAPHED = (1 << 19),
	GRENADE = (1 << 20),
	STEPMOVEMENT = (1 << 21),
	DONTTOUCH = (1 << 22),
	BASEVELOCITY = (1 << 23),
	WORLDBRUSH = (1 << 24),
	OBJECT = (1 << 25),
	KILLME = (1 << 26),
	ONFIRE = (1 << 27),
	DISSOLVING = (1 << 28),
	TRANSRAGDOLL = (1 << 29),
	UNBLOCKABLE_BY_PLAYER = (1 << 30)
};

enum class MoveType {
	NONE = 0,			// never moves
	ISOMETRIC,			// For players -- in TF2 commander view, etc.
	WALK,				// Player only - moving on the ground
	STEP,				// gravity, special edge handling -- monsters use this
	FLY,				// No gravity, but still collides with stuff
	FLYGRAVITY,		// flies through the air + is affected by gravity
	VPHYSICS,			// uses VPHYSICS for simulation
	PUSH,				// no clip to world, push and crush
	NOCLIP,			// No gravity, no collisions, still do velocity/avelocity
	LADDER,			// Used by players only when going onto a ladder
	OBSERVER,			// Observer movement, depends on player's observer mode
	CUSTOM,			// Allows the entity to describe its own physics

	// should always be defined as the last item in the list
	LAST = CUSTOM,

	MAX_BITS = 4
};

enum class WaterLevel {
	NotInWater,
	Feet,
	Waist,
	Eyes
};

enum class EFlags {
	KILLME = (1 << 0),	// This entity is marked for death -- This allows the game to actually delete ents at a safe time
	DORMANT = (1 << 1),	// Entity is dormant, no updates to client
	NOCLIP_ACTIVE = (1 << 2),	// Lets us know when the noclip command is active.
	SETTING_UP_BONES = (1 << 3),	// Set while a model is setting up its bones.
	KEEP_ON_RECREATE_ENTITIES = (1 << 4), // This is a special entity that should not be deleted when we restart entities only

	HAS_PLAYER_CHILD = (1 << 4),	// One of the child entities is a player.

	DIRTY_SHADOWUPDATE = (1 << 5),	// Client only- need shadow manager to update the shadow...
	NOTIFY = (1 << 6),	// Another entity is watching events on this entity (used by teleport)

	// The default behavior in ShouldTransmit is to not send an entity if it doesn't
	// have a model. Certain entities want to be sent anyway because all the drawing logic
	// is in the client DLL. They can set this flag and the engine will transmit them even
	// if they don't have a model.
	FORCE_CHECK_TRANSMIT = (1 << 7),

	BOT_FROZEN = (1 << 8),	// This is set on bots that are frozen.
	SERVER_ONLY = (1 << 9),	// Non-networked entity.
	NO_AUTO_EDICT_ATTACH = (1 << 10), // Don't attach the edict; we're doing it explicitly

	// Some dirty bits with respect to abs computations
	DIRTY_ABSTRANSFORM = (1 << 11),
	DIRTY_ABSVELOCITY = (1 << 12),
	DIRTY_ABSANGVELOCITY = (1 << 13),
	DIRTY_SURROUNDING_COLLISION_BOUNDS = (1 << 14),
	DIRTY_SPATIAL_PARTITION = (1 << 15),
	//	UNUSED						= (1<<16),

	IN_SKYBOX = (1 << 17),	// This is set if the entity detects that it's in the skybox.
	// This forces it to pass the "in PVS" for transmission.
	USE_PARTITION_WHEN_NOT_SOLID = (1 << 18),	// Entities with this flag set show up in the partition even when not solid
	TOUCHING_FLUID = (1 << 19),	// Used to determine if an entity is floating

	// FIXME: Not really sure where I should add this...
	IS_BEING_LIFTED_BY_BARNACLE = (1 << 20),
	NO_ROTORWASH_PUSH = (1 << 21),		// I shouldn't be pushed by the rotorwash
	NO_THINK_FUNCTION = (1 << 22),
	NO_GAME_PHYSICS_SIMULATION = (1 << 23),

	CHECK_UNTOUCH = (1 << 24),
	DONTBLOCKLOS = (1 << 25),		// I shouldn't block NPC line-of-sight
	DONTWALKON = (1 << 26),		// NPC;s should not walk on this entity
	NO_DISSOLVE = (1 << 27),		// These guys shouldn't dissolve
	NO_MEGAPHYSCANNON_RAGDOLL = (1 << 28),	// Mega physcannon can't ragdoll these guys.
	NO_WATER_VELOCITY_CHANGE = (1 << 29),	// Don't adjust this entity's velocity when transitioning into water
	NO_PHYSCANNON_INTERACTION = (1 << 30),	// Physcannon can't pick these up or punt them
	NO_DAMAGE_FORCES = (1 << 31),	// Doesn't accept forces from physics damage
};

enum class EEffects {
	BONEMERGE = 0x001,	// Performs bone merge on client side
	BRIGHTLIGHT = 0x002,	// DLIGHT centered at entity origin
	DIMLIGHT = 0x004,	// player flashlight
	NOINTERP = 0x008,	// don't interpolate the next frame
	NOSHADOW = 0x010,	// Don't cast no shadow
	NODRAW = 0x020,	// don't draw entity
	NORECEIVESHADOW = 0x040,	// Don't receive no shadow
	BONEMERGE_FASTCULL = 0x080,	// For use with EF_BONEMERGE. If this is set, then it places this ent's origin at its
									// parent and uses the parent's bbox + the max extents of the aiment.
									// Otherwise, it sets up the parent's bones every frame to figure out where to place
									// the aiment, which is inefficient because it'll setup the parent's bones even if
									// the parent is not in the PVS.

	ITEM_BLINK = 0x100,	// blink an item so that the user notices it.
	PARENT_ANIMATES = 0x200,	// always assume that the parent entity is animating
	MAX_BITS = 10
};
