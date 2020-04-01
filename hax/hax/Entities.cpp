#pragma once
#include <iostream>
#include "Entities.hxx"

Entity::Entity(HANDLE hProcess, DWORD Address)
{
	this->hProcess = hProcess;
	this->sEntity.Flag = 0;
	this->sEntity.Health = 0;
	this->sEntity.Team = 0;
	this->sEntity.Pos = { 0, 0, 0 };
	this->pEntity = Reader::RPM<DWORD>(hProcess, Address);
}

Entity::~Entity()
{
}

bool Entity::EntityExist()
{
	if (this->pEntity != NULL)
		return true;
	return false;
}

bool Entity::IsAlive()
{
	if (this->sEntity.Health > 0 && this->sEntity.Health < 101)
		return true;
	return false;
}

int Entity::ReadTeamNum()
{
	this->sEntity.Team = Reader::RPM<int>(hProcess, this->pEntity + m_iTeamNum);
	return this->sEntity.Team;
}

void Entity::ReadEntityDatas()
{
	this->sEntity.Flag = Reader::RPM<BYTE>(this->hProcess, this->pEntity + m_fFlags);
	this->sEntity.Health = Reader::RPM<int>(this->hProcess, this->pEntity + m_iHealth);
	this->sEntity.Pos = Reader::RPM<vector>(this->hProcess, this->pEntity + m_vecOrigin);
	this->sEntity.Head = this->sEntity.Pos;
	if (this->sEntity.Flag == 7) // sitting
		this->sEntity.Head.z += 50.f; // 50.f is the physical size of the body in the world when is sitting, i think ?
	else
		this->sEntity.Head.z += 72.f; // 75.f is the physical size of the body in the world, i think ?

}