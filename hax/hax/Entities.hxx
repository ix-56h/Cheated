#pragma once
#include "hax.hxx"
#include "Reader.hxx"

class Entity
{
	public:
		pEntityVars sEntity;
		Entity(HANDLE hProcess, DWORD Address);
		~Entity();

		bool EntityExist();

		bool IsAlive();

		int ReadTeamNum();

		void ReadEntityDatas();
	private:
		DWORD		pEntity;
		HANDLE		hProcess;
};