#pragma once
#include "Session.h"

class GameSession : public PacketSession
{
public:
	GameSession():nPlayerID(0)
	{


	}
	~GameSession()
	{
		//cout << "~GameSession" << endl;
	}

	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;

	void	SetPlayerID(int nPlayerid)
	{
		nPlayerID = nPlayerid;
	}

	//Vector<PlayerRef> _players;

private:
	int		nPlayerID;
	//PlayerRef _currentPlayer;
	//weak_ptr<class Room> _room;
};