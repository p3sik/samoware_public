
#pragma once

#include <cstddef>

class ILuaCallback;
class ILuaObject;
class ILuaThreadedCall;
class Angle;
class Vector;
class Color;

class ILuaBase;

class CLuaGamemode;

struct lua_State {
	unsigned char _ignore_this_common_lua_header_[92 + 22];
	ILuaBase* luabase;
};

typedef int(*CFunc)(lua_State* L);

#define LUA_FUNCTION(FUNC)								\
		int FUNC##__Imp(ILuaBase* LUA);					\
		int FUNC(lua_State* L) {						\
			ILuaBase* LUA = L->luabase;					\
			LUA->SetState(L);							\
			return FUNC##__Imp(LUA);					\
		}												\
		int FUNC##__Imp(ILuaBase* LUA)

#define LUA_FUNCTION_EXTERN(FUNC)						\
		int FUNC##__Imp(ILuaBase* LUA);					\
		int FUNC(lua_State* L);

#define LUA_FUNCTION_GETTER(FUNC, type, field)	\
		LUA_FUNCTION(FUNC) { LUA->Push##type(field); return 1; }

#define LUA_FUNCTION_SETTER(FUNC, type, field)	\
		LUA_FUNCTION(FUNC) { LUA->Check##type(1); field = LUA->GetNumber(1); return 1; }

#define LUA_FUNCTION_EXTERN_GETSET(FUNC)	\
		LUA_FUNCTION_EXTERN(Get ## FUNC);	\
		LUA_FUNCTION_EXTERN(Set ## FUNC);


#define LUA_FUNCTION_GETSET(FUNC, type, field)			\
		LUA_FUNCTION_GETTER(Get ## FUNC, type, field);	\
		LUA_FUNCTION_SETTER(Set ## FUNC, type, field);

namespace Lua {
	enum {
		CLIENT,
		SERVER,
		MENU
	};

	enum {
		// Lua Types
		NONE = -1,
		NIL,
		BOOL,
		LIGHTUSERDATA,
		NUMBER,
		STRING,
		TABLE,
		FUNCTION,
		USERDATA,
		THREAD,

		// GMod Types
		ENTITY,
		VECTOR, // GMOD: GO TODO - This was renamed... I'll probably forget to fix it before this ends up public
		ANGLE,
		PHYSOBJ,
		SAVE,
		RESTORE,
		DAMAGEINFO,
		EFFECTDATA,
		MOVEDATA,
		RECIPIENTFILTER,
		USERCMD,
		SCRIPTEDVEHICLE,
		MATERIAL,
		PANEL,
		PARTICLE,
		PARTICLEEMITTER,
		TEXTURE,
		USERMSG,
		CONVAR,
		IMESH,
		MATRIX,
		SOUND,
		PIXELVISHANDLE,
		DLIGHT,
		VIDEO,
		FILE,
		LOCOMOTION,
		PATH,
		NAVAREA,
		SOUNDHANDLE,
		NAVLADDER,
		PARTICLESYSTEM,
		PROJECTEDTEXTURE,
		PHYSCOLLIDE,

		COUNT
	};

	enum {
		SPECIAL_GLOB,	// Global table
		SPECIAL_ENV,	// Environment table
		SPECIAL_REG		// Registry table
	};
}

class ILuaBase {
public:
	lua_State* GetLuaState() {
		return *reinterpret_cast<lua_State**>(this + 1);
	}

	virtual int				Top(void) = 0;
	virtual void			Push(int iStackPos) = 0;
	virtual void			Pop(int iAmt = 1) = 0;
	virtual void			GetTable(int iStackPos) = 0;
	virtual void			GetField(int iStackPos, const char* strName) = 0;
	virtual void			SetField(int iStackPos, const char* strName) = 0;
	virtual void			CreateTable() = 0;
	virtual void			SetTable(int i) = 0;
	virtual void			SetMetaTable(int i) = 0;
	virtual bool			GetMetaTable(int i) = 0;
	virtual void			Call(int iArgs, int iResults) = 0;
	virtual int				PCall(int iArgs, int iResults, int iErrorFunc) = 0;
	virtual int				Equal(int iA, int iB) = 0;
	virtual int				RawEqual(int iA, int iB) = 0;
	virtual void			Insert(int iStackPos) = 0;
	virtual void			Remove(int iStackPos) = 0;
	virtual int				Next(int iStackPos) = 0;
	virtual void*			NewUserdata(unsigned int iSize) = 0;
	virtual void			ThrowError(const char* strError) = 0;
	virtual void			CheckType(int iStackPos, int iType) = 0;
	virtual void			ArgError(int iArgNum, const char* strMessage) = 0;
	virtual void			RawGet(int iStackPos) = 0;
	virtual void			RawSet(int iStackPos) = 0;
	virtual const char*		GetString(int iStackPos = -1, unsigned int* iOutLen = NULL) = 0;
	virtual double			GetNumber(int iStackPos = -1) = 0;
	virtual bool			GetBool(int iStackPos = -1) = 0;
	virtual CFunc			GetCFunction(int iStackPos = -1) = 0;
	virtual void*			GetUserdata(int iStackPos = -1) = 0;
	virtual void			PushNil() = 0;
	virtual void			PushString(const char* val, unsigned int iLen = 0) = 0;
	virtual void			PushNumber(double val) = 0;
	virtual void			PushBool(bool val) = 0;
	virtual void			PushCFunction(CFunc val) = 0;
	virtual void			PushCClosure(CFunc val, int iVars) = 0;
	virtual void			PushUserdata(void*) = 0;
	virtual int				ReferenceCreate() = 0;
	virtual void			ReferenceFree(int i) = 0;
	virtual void			ReferencePush(int i) = 0;
	virtual void			PushSpecial(int iType) = 0;
	virtual bool			IsType(int iStackPos, int iType) = 0;
	virtual int				GetType(int iStackPos) = 0;
	virtual const char*		GetTypeName(int iType) = 0;
	virtual void			CreateMetaTableType(const char* strName, int iType) = 0;
	virtual const char*		CheckString(int iStackPos = -1) = 0;
	virtual double			CheckNumber(int iStackPos = -1) = 0;
	virtual int				ObjLen(int) = 0;
	virtual const Angle&	GetAngle(int) = 0;
	virtual const Vector&   GetVector(int) = 0;
	virtual int				PushAngle(Angle const&) = 0;
	virtual int				PushVector(Vector const&) = 0;
	virtual int				SetState(lua_State*) = 0;
	virtual int				CreateMetaTable(const char*) = 0;
	virtual char			PushMetaTable(int) = 0;
	virtual int				PushUserType(void*, int) = 0;
	virtual void**			SetUserType(int, void*) = 0;
	virtual char			Init(ILuaCallback*, bool) = 0;
	virtual int				Shutdown(void) = 0;
	virtual int				Cycle(void) = 0;
	virtual int				Global(void) = 0;
	virtual int				Lua_GetObject(int) = 0;
	virtual int				PushLuaObject(ILuaObject*) = 0;
	virtual int				PushLuaFunction(int*, lua_State*) = 0;
	virtual int				LuaError(const char*, int) = 0;
	virtual int				TypeError(const char*, int) = 0;
	virtual int				CallInternal(int, int) = 0;
	virtual int				CallInternalNoReturns(int) = 0;
	virtual bool			CallInternalGetBool(int) = 0;
	virtual const char*		CallInternalGetString(int) = 0;
	virtual int				CallInternalGet(int, ILuaObject*) = 0;
	virtual char*			NewGlobalTable(const char*) = 0;
	virtual int				NewTemporaryObject(int) = 0;
	virtual bool			isUserData(int) = 0;
	virtual int				GetMetaTableObject(const char*, int) = 0;
	virtual int				GetMetaTableObject(int) = 0;
	virtual int				GetReturn(int) = 0;
	virtual bool			IsServer(void) = 0;
	virtual bool			IsClient(void) = 0;
	virtual bool			IsMenu(void) = 0;
	virtual int				DestroyObject(ILuaObject*) = 0;
	virtual int				CreateObject(void) = 0;
	virtual int				SetMember(ILuaObject*, ILuaObject*, ILuaObject*) = 0;
	virtual int				GetNewTable(void) = 0;
	virtual int				SetMember(ILuaObject*, float) = 0;
	virtual int				SetMember(ILuaObject*, float, ILuaObject*) = 0;
	virtual int				SetMember(ILuaObject*, const char*) = 0;
	virtual int				SetMember(ILuaObject*, const char*, ILuaObject*) = 0;
	virtual int				SetType(unsigned char) = 0;
	virtual int				PushLong(long) = 0;
	virtual int				GetFlags(int) = 0;
	virtual int				FindOnObjectsMetaTable(int, int) = 0;
	virtual int				FindObjectsOnMetaTable(int, int) = 0;
	virtual int				SetMemberFast(ILuaObject*, int, int) = 0;
	virtual int				RunString(const char* filename, const char* path, const char* stringToRun, bool run, bool showErrors) = 0;
	virtual bool			IsEqual(ILuaObject*, ILuaObject*) = 0;
	virtual int				Error(const char*) = 0;
	virtual int				GetStringOrError(int) = 0;
	virtual int				RunLuaModule(const char*) = 0;
	virtual int				FindAndRunScript(const char*, bool, bool, const char*, bool) = 0;
	virtual int				SetPathID(const char*) = 0;
	virtual char*			GetPathID(void) = 0;
	virtual int				ErrorNoHalt(const char*, ...) = 0;
	virtual int				Msg(const char*, ...) = 0;
	virtual int				PushPath(const char*) = 0;
	virtual int				PopPath(void) = 0;
	virtual int				GetPath(void) = 0;
	virtual Color			GetColor(int) = 0;
	virtual int				PushColor(Color) = 0;
	virtual int				GetStack(int, void*) = 0;
	virtual int				GetInfo(const char*, void*) = 0;
	virtual int				GetLocal(void*, int) = 0;
	virtual int				GetUpvalue(int, int) = 0;
	virtual int				RunStringEx(const char* filename, const char* path, const char* stringToRun, bool run, bool showErrors, bool) = 0;
	virtual int				GetDataString(int, void**) = 0;
	virtual int				ErrorFromLua(const char*, ...) = 0;
	virtual void*			GetCurrentLocation(void) = 0;
	virtual int				MsgColour(const Color&, const char*, ...) = 0;
	virtual int				GetCurrentFile(void) = 0;
	virtual int				CompileString(void) = 0;
	virtual int				CallFunctionProtected(int, int, bool) = 0;
	virtual int				Require(const char*) = 0;
	virtual const char*		GetActualTypeName(int) = 0;
	virtual int				PreCreateTable(int, int) = 0;
	virtual int				PushPooledString(int) = 0;
	virtual int				GetBooledString(int) = 0;
	virtual int				AddThreadedCall(ILuaThreadedCall*) = 0;
	virtual int				AppendStackTrace(char*, unsigned long) = 0;
	virtual int				CreateConVar(const char*, const char*, const char*, int) = 0;
	virtual int				CreateConCommand(void) = 0;

	struct UserData {
		void* data;
		unsigned char type;
	};

	template <class T>
	T* GetUserType(int iStackPos, int iType) {
		UserData* ud = reinterpret_cast<UserData*>(GetUserdata(iStackPos));

		if (ud == NULL || ud->data == NULL || ud->type != iType)
			return NULL;

		return reinterpret_cast<T*>(ud->data);
	}
};
