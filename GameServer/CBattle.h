#pragma once
using namespace Sector;
class CBattle
{
public:

	bool Attack(int nAttack, OUT int& nKillcount,ObjectInfo& stTarget);

};
extern shared_ptr<CBattle> GBattle;

