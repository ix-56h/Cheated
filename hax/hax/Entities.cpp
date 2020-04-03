#pragma once
#include <iostream>
#include "Entities.hxx"

Entity::Entity(HANDLE hProcess, DWORD Address)
{
	this->hProcess = hProcess;
	this->sEntity.Flag = 0;
	this->sEntity.Health = 0;
	this->sEntity.Team = 0;
	this->sEntity.RectWidth = 0;
	this->sEntity.RectHeight = 0;
	this->sEntity.Foot = { 0, 0, 0 };
	this->sEntity.Head = { 0, 0, 0 };
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

void Entity::WToS(view_matrix_t view_matrix)
{
	this->sEntity.Foot = WorldToScreen(this->sEntity.Foot, view_matrix);
	this->sEntity.Head = WorldToScreen(this->sEntity.Head, view_matrix);
	this->sEntity.RectHeight = (this->sEntity.Foot.y - this->sEntity.Head.y);
	this->sEntity.RectWidth = (this->sEntity.RectHeight / 2.4f);
	this->sEntity.Head.x -= this->sEntity.RectWidth / 2;
}

void Entity::ReadEntityDatas()
{
	this->sEntity.Flag = Reader::RPM<BYTE>(this->hProcess, this->pEntity + m_fFlags);
	this->sEntity.Health = Reader::RPM<int>(this->hProcess, this->pEntity + m_iHealth);
	this->sEntity.Armor = Reader::RPM<int>(this->hProcess, this->pEntity + m_ArmorValue);
	this->sEntity.Foot = Reader::RPM<vector>(this->hProcess, this->pEntity + m_vecOrigin);
	this->sEntity.Head = this->sEntity.Foot;
	if (this->sEntity.Flag == 7) // sitting
	{
		this->sEntity.Head.z += 60.f;
		this->sEntity.Head.x -= 4.f;
	}
	else
		this->sEntity.Head.z += 75.f;
}