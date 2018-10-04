#include "stdafx.h"
#include "GomokuRoom.h"

#include "SceneManager.h"
#include "GomokuLobby.h"
#include "GomokuTitle.h"


constexpr POINT c_playerPos = { 800, 0 };
constexpr POINT c_playerSize = { 400, 100 };
constexpr POINT c_buttonPos = { 800, 400 };
constexpr POINT c_buttonSize = { 400, 200 };
constexpr POINT c_buttonOffset = { 20, 20 };
constexpr POINT c_buttonRigidSize = { 360, 160 };


constexpr POINT c_hostPos = c_playerPos;
constexpr POINT c_guestPos = { c_hostPos.x, c_hostPos.y + c_playerSize.y };

constexpr POINT c_readyPos = c_buttonPos;
constexpr POINT c_exitPos = { c_readyPos.x, c_readyPos.y + c_buttonSize.y };
constexpr POINT c_readyRigidPos = { c_readyPos.x + c_buttonOffset.x, c_readyPos.y + c_buttonOffset.y };
constexpr POINT c_exitRigidPos = { c_exitPos.x + c_buttonOffset.x, c_exitPos.y + c_buttonOffset.y };


GomokuRoom::GomokuRoom(AsyncConnector* serverConnector, int id, const std::string& name)
	: m_serverConnector(serverConnector)
	, m_id(id)
	, m_name(name)

	, m_playerReady{ false }
{
}

GomokuRoom::~GomokuRoom()
{
}



void GomokuRoom::Init()
{
	auto CreateTex = [](const std::wstring& path, LPDIRECT3DTEXTURE9& target){ D3DXCreateTextureFromFileExW(DEVICE, path.data(), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, NULL, NULL, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, NULL, nullptr, nullptr, &target); };
	
	CreateTex(L"./Resource/room/background.png",	m_resource.background);
	CreateTex(L"./Resource/room/board.png",			m_resource.board);
								
	CreateTex(L"./Resource/room/guest.png",			m_resource.guest);
	CreateTex(L"./Resource/room/host.png",			m_resource.host);
	
	CreateTex(L"./Resource/room/ready.png",			m_resource.ready);
	CreateTex(L"./Resource/room/readySign.png",		m_resource.readyMark);
	CreateTex(L"./Resource/room/exit.png",			m_resource.quit);
	
	CreateTex(L"./Resource/room/whiteStone.png",	m_resource.stoneW);
	CreateTex(L"./Resource/room/blackStone.png",	m_resource.stoneB);
	CreateTex(L"./Resource/room/lastSign.png",		m_resource.stoneMarker);

	AttachConnectorReturner();
	locked_cout << "Room >> Enter" << endl;
}

void GomokuRoom::Update()
{
	auto GomokuAttack = [this](int x, int y)
	{
		arJSON oJSON;
		oJSON["Message"] = "Attack";
		arJSON attackJSON;
		{
			attackJSON["x"] = x;
			attackJSON["y"] = y;
		}
		oJSON["Attack"] = attackJSON;
		__ar_send(*m_serverConnector, oJSON);
	};
	if (g_inputDevice.IsKeyDown(VK_LBUTTON))
	{
		const POINT mousePos = g_inputDevice.MousePos();
		auto IsMouseIn = [&mousePos](int left, int top, int right, int bottom)->bool
		{
			return
				left <= mousePos.x &&
				top <= mousePos.y &&
				mousePos.x < right &&
				mousePos.y < bottom;
		};

		//Gomoku Board
		if (IsMouseIn(25, 25, 25 + 50 * 15, 25 + 50 * 15))
		{
			POINT gomokuPos;
			gomokuPos.x = (mousePos.x - 25) / 50;
			gomokuPos.y = (mousePos.y - 25) / 50;

			if (0 <= gomokuPos.x && gomokuPos.x < GomokuBoard::boardSizeX &&
				0 <= gomokuPos.y && gomokuPos.y < GomokuBoard::boardSizeY)
				GomokuAttack(gomokuPos.x, gomokuPos.y);
		}
		//Ready
		else
		if (IsMouseIn(c_readyRigidPos.x, c_readyRigidPos.y, c_readyRigidPos.x + c_buttonRigidSize.x, c_readyRigidPos.y + c_buttonRigidSize.y))
		{
			if (true)
			{
				arJSON oJSON;
				oJSON["Message"] = "Ready";
				oJSON["Ready"] = 1;
				__ar_send(*m_serverConnector, oJSON);
			}
			else
			{
				arJSON oJSON;
				oJSON["Message"] = "Ready";
				oJSON["Ready"] = 1;
				__ar_send(*m_serverConnector, oJSON);
			}
		}
		//Quit
		else
		if (IsMouseIn(c_exitRigidPos.x, c_exitRigidPos.y, c_exitRigidPos.x + c_buttonRigidSize.x, c_exitRigidPos.y + c_buttonRigidSize.y))
		{
			arJSON oJSON;
			oJSON["Message"] = "LeaveRoom";
			__ar_send(*m_serverConnector, oJSON);
		}
	}
	if (g_inputDevice.IsKeyDown('0'))
		std::terminate();
}

void GomokuRoom::Render()
{
	g_sprite->Begin(D3DXSPRITE_ALPHABLEND);
	auto Draw = [](LPDIRECT3DTEXTURE9 tex, int x, int y) { g_sprite->Draw(tex, nullptr, nullptr, &D3DXVECTOR3(x, y, 0), D3DXCOLOR(1, 1, 1, 1)); };
	
	Draw(m_resource.background, 0, 0);
	Draw(m_resource.board, 0, 0);
	for (int y = 0; y < m_gomokuBoard.boardSizeY; y++)
	{
		for (int x = 0; x < m_gomokuBoard.boardSizeX; x++)
		{
			int xPos = x * 50 + 25;
			int yPos = y * 50 + 25;
				 if (m_gomokuBoard.IsBlack(x, y))	Draw(m_resource.stoneB, xPos, yPos);
			else if (m_gomokuBoard.IsWhite(x, y))	Draw(m_resource.stoneW, xPos, yPos);
		}
	}

	Draw(m_resource.host, c_hostPos.x, c_hostPos.y);
	Draw(m_resource.guest, c_guestPos.x, c_guestPos.y);
	auto DrawReadySign = [&, this](int playerX, int playerY)
	{
		Draw(m_resource.readyMark, playerX + c_playerSize.x - 75, playerY + 25);
	};
	if (m_playerReady[0])	DrawReadySign(c_hostPos.x, c_hostPos.y);
	if (m_playerReady[1])	DrawReadySign(c_guestPos.x, c_guestPos.y);

	Draw(m_resource.ready, c_readyPos.x, c_readyPos.y);
	Draw(m_resource.quit, c_exitPos.x, c_exitPos.y);

	g_sprite->End();
}

void GomokuRoom::Release()
{
	DetachConnectorReturner();
}



bool GomokuRoom::MessageProcessing(AsyncConnector & user, int recvResult, SocketBuffer & recvData)
{
	if (recvResult > 0)
	{
		recvData[recvResult] = NULL;
		arJSON iJSON;
		if (JSON_To_arJSON(recvData.Buffer(), iJSON))
		{
			locked_cout << "Server returned JSON errored" << endl;
			return true;
		}

		const std::string& iMessage = iJSON["Message"].Str();
		///	 if (iMessage == "CreateRoom")	{ if (CreateRoom(user, iJSON))	return true; }
		///else if (iMessage == "JoinRoom")	{ if (EnterRoom(user, iJSON))	return true; }
		///else 

			 if (iMessage == "RoomLeaved")			{ if (RoomLeaved(iJSON))	return true; }
		else if (iMessage == "UserDisconnected")	{}

		else if (iMessage == "Ready")				{ if (Ready(iJSON))			return true; }
		else if (iMessage == "GomokuStart")			{ if (GomokuStart(iJSON))	return true; }
		else if (iMessage == "Attacked")			{ if (Attacked(iJSON))		return true; }
		else if (iMessage == "GomokuEnd")			{ if (GomokuEnd(iJSON))		return true; }
		else
		{ locked_cout << "Room >> Unknown message recived : " << iMessage << endl; }
	}
	else
	{
		locked_cout << "Lobby >> Server Disconnected" << endl;
		DetachConnectorReturner();
		SntInst(SceneManager).ChangeScene(new GomokuTitle());
		return true;
	}
	return false;
}

bool GomokuRoom::RoomLeaved(const arJSON & iJSON)
{
	if (iJSON.IsIn("Result") && iJSON["Result"].Int())
	{
		locked_cout << "Room >> Reaved" << endl;

		DetachConnectorReturner();
		SntInst(SceneManager).ChangeScene(new GomokuLobby(m_serverConnector));
	}
	return false;
}

bool GomokuRoom::Ready(const arJSON & iJSON)
{
	if (!iJSON.IsIn("Ready") || !iJSON.IsIn("By"))
		return false;

	bool ready = iJSON["Ready"].Int();
	bool isBlack = iJSON["By"].Str() == "Black";

	m_playerReady[isBlack ? 0 : 1] = ready;
	locked_cout << "Room >> Player(" << (isBlack ? "Black" : "White") << ") ready " << (ready ? "On" : "Off") << endl;
	return false;
}

bool GomokuRoom::GomokuStart(const arJSON & iJSON)
{
	locked_cout << "Room >> Game started" << endl;
	m_gomokuBoard.Clear();
	m_playerReady[0] = m_playerReady[1] = false;
	return false;
}


bool GomokuRoom::Attacked(const arJSON & iJSON)
{
	if (!iJSON.IsIn("Attack"))
		return false;
	const arJSON& attackJSON = iJSON["Attack"].Sub();
	if (!attackJSON.IsIn("Attacker") ||
		!attackJSON.IsIn("x") ||
		!attackJSON.IsIn("y"))
		return false;

	bool isBlack = attackJSON["Attacker"].Str() == "Black";
	int x = attackJSON["x"].Int();
	int y = attackJSON["y"].Int();

	m_gomokuBoard.At(x, y) = isBlack ? GomokuBoard::blackValue : GomokuBoard::whiteValue;
	{
		locked_cout << "Room >> Gomoku Board State" << endl;
		for (int y = 0; y < GomokuBoard::boardSizeY; y++)
		{
			for (int x = 0; x < GomokuBoard::boardSizeX; x++)
			{
				if		(m_gomokuBoard.IsBlack(x, y))	cout << "¡Ü";
				else if (m_gomokuBoard.IsWhite(x, y))	cout << "¡Û";
				else									cout << "£«";
			}
			cout << endl;
		}
	}
	return false;
}

bool GomokuRoom::GomokuEnd(const arJSON & iJSON)
{
	if (!iJSON.IsIn("Winner"))
		return false;

	const std::string& winner = iJSON["Winner"].Str();
	locked_cout << "Room >> Gomoku winner is " << winner << endl
				<< "Room >> Gomoku winner is " << winner << endl
				<< "Room >> Gomoku winner is " << winner << endl;
	return false;
}





GomokuRoom::Resource::Resource()
	: background(nullptr)
	, board(nullptr)

	, guest(nullptr)
	, host(nullptr)
	
	, ready(nullptr)
	, readyMark(nullptr)
	, quit(nullptr)
	
	, stoneW(nullptr)
	, stoneB(nullptr)
	, stoneMarker(nullptr)
{
}

GomokuRoom::Resource::~Resource()
{
	if (background)		background->Release();
	if (board)			board->Release();
	
	if (guest)			guest->Release();
	if (host)			host->Release();
	
	if (ready)			ready->Release();
	if (readyMark)		readyMark->Release();
	if (quit)			quit->Release();
	
	if (stoneW)			stoneW->Release();
	if (stoneB)			stoneB->Release();
	if (stoneMarker)	stoneMarker->Release();
}
