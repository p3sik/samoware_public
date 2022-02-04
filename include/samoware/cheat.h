
#pragma once

#include "samoware/menu.h"

#include "cfw/basecheat.h"

class Samoware : public cfw::BaseCheat, public cfw::Singleton<Samoware> {
	friend class cfw::Singleton<Samoware>;

private:
	HMODULE _module;
	
public:
	SamowareMenu* menu;

	void Initialize(HMODULE module);
	virtual void Unload();

	Samoware(token) : _module(NULL), menu(nullptr) {
		shouldUnload = false;
		unloaded = false;
	}
};
