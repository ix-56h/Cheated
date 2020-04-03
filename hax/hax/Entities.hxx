#pragma once
#include "hax.hxx"
#include "Reader.hxx"

struct pEntityVars {
	int		Health;
	int		Armor;
	int		Team;
	BYTE	Flag;
	//string	Name;
	vector	Foot;
	vector	Head;
	float	RectWidth;
	float	RectHeight;
};

class Entity
{
	public:
		pEntityVars sEntity;
		Entity(HANDLE hProcess, DWORD Address);
		~Entity();

		bool EntityExist();

		bool IsAlive();
		
		void WToS(view_matrix_t view_matrix);

		int ReadTeamNum();

		void ReadEntityDatas();
	private:
		DWORD		pEntity;
		HANDLE		hProcess;
};