#include "stdafx.h"
#include "GomokuLobby.h"

#include "SceneManager.h"
#include "GomokuTitle.h"
#include "GomokuRoom.h"


constexpr POINT c_listBarStartPos = { 0, 0 };
constexpr POINT c_listBarSize = { 780, 80 };
constexpr size_t c_listBarInterval = 120;

constexpr POINT c_createPos = { 800, 400 };
constexpr POINT c_createRealPos = { 820, 420 };
constexpr POINT c_createSize = { 360, 160 };
constexpr POINT c_exitPos = { 800, 600 };
constexpr POINT c_exitRealPos = { 820, 620 };
constexpr POINT c_exitSize = { 360, 160 };



GomokuLobby::GomokuLobby(AsyncConnector* serverConnector)
	: m_serverConnector(serverConnector)
{
}

GomokuLobby::~GomokuLobby()
{
}



void GomokuLobby::Init()
{
	auto _CreateFont = [](size_t size, const std::wstring& font, LPD3DXFONT& target) { D3DXCreateFontW(DEVICE, size, NULL, FW_DONTCARE, NULL, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, font.data(), &target); };
	auto CreateTex = [](const std::wstring& path, LPDIRECT3DTEXTURE9& target){ D3DXCreateTextureFromFileExW(DEVICE, path.data(), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, NULL, NULL, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, NULL, nullptr, nullptr, &target); };
	
	_CreateFont(40, L"", m_resource.font);

	CreateTex(L"./Resource/lobby/background.png", m_resource.background);

	CreateTex(L"./Resource/lobby/listBar.png", m_resource.listBar);
	CreateTex(L"./Resource/lobby/create.png", m_resource.create);
	CreateTex(L"./Resource/lobby/exit.png", m_resource.exit);



	AttachConnectorReturner();
	arJSON readyToInit;
	readyToInit["Message"] = "ReadyToInit";
	__ar_send(*m_serverConnector, readyToInit);

	locked_cout << "Lobby >> Enter" << endl;
}

void GomokuLobby::Update()
{
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

		//Create room
		if (IsMouseIn(c_createRealPos.x, c_createRealPos.y, c_createRealPos.x + c_createSize.x, c_createRealPos.y + c_createSize.y))
		{
			arJSON oJSON;
			oJSON["Message"] = "CreateRoom";
			arJSON roomJSON;
			{
				roomJSON["Name"] = []()->std::string
				{
					switch (rand() % 4)
					{
					case 0:	return "Hello Gomoku";
					case 1:	return "i am god";
					case 2:	return "´ýº­";
					case 3:	return "Ang";
					}
					return {};
				}();
			}
			oJSON["Room"] = roomJSON;
			__ar_send(*m_serverConnector, oJSON);


			locked_cout << "Lobby >> Create Room..." << endl;
		}
		//Exit
		else if (IsMouseIn(c_exitRealPos.x, c_exitRealPos.y, c_exitRealPos.x + c_exitSize.x, c_exitRealPos.y + c_exitSize.y))
		{
			arJSON oJSON;
			oJSON["Message"] = "LeaveLobby";
			__ar_send(*m_serverConnector, oJSON);

			locked_cout << "Lobby >> Leave Lobby..." << endl;
		}
		//Enter room(room list)
		else
		{
			mutex_lock_guard lockerR(m_mtxProcessing);
			for (int index = 0; index < m_roomList.size(); index++)
			{
				if (IsMouseIn(c_listBarStartPos.x + 10, c_listBarStartPos.y + 10 + index * c_listBarInterval,
					c_listBarStartPos.x + 10 + c_listBarSize.x, c_listBarStartPos.y + 10 + c_listBarSize.y + index * c_listBarInterval))
				{
					auto EnterRoomPlease = [this](int id)
					{
						arJSON oJSON;
						oJSON["Message"] = "EnterRoom";
						arJSON roomJSON;
						{
							roomJSON["ID"] = id;
						}
						oJSON["Room"] = roomJSON;
						__ar_send(*m_serverConnector, oJSON);

						locked_cout << "Lobby >> Room Enter..." << endl;
					};
					auto GetIndex = [this](int i)->int
					{
						auto iter = m_roomList.begin();
						while (i--)
							iter++;
						return iter->first;
					};
					EnterRoomPlease(GetIndex(index));
					break;
				}
			}
		}
	}

	if (g_inputDevice.IsKeyDown(VK_NUMPAD1))
	{
		cout_region_lock;
		cout << "[ Room List ]" << endl;
		for (auto& iter : m_roomList)
			cout << iter.first << " : " << iter.second.name << endl;
	}

	if (g_inputDevice.IsKeyDown(VK_ESCAPE))
		std::terminate();
}

void GomokuLobby::Render()
{
	g_sprite->Begin(D3DXSPRITE_ALPHABLEND);

	auto TextA = [](LPD3DXFONT font, int x, int y, const std::string& text, DWORD flags = NULL, const D3DXCOLOR& color = D3DXCOLOR(0, 0, 0, 1))
	{
		RECT rc;
		SetRect(&rc, x, y, x, y);
		font->DrawTextA(g_sprite, text.data(), -1, &rc, flags, color);
	};
	auto TextW = [](LPD3DXFONT font, int x, int y, const std::wstring& text, DWORD flags = NULL, const D3DXCOLOR& color = D3DXCOLOR(0, 0, 0, 1))
	{
		RECT rc;
		SetRect(&rc, x, y, x, y);
		font->DrawTextW(g_sprite, text.data(), -1, &rc, flags, color);
	};
	auto Draw = [](LPDIRECT3DTEXTURE9 tex, int x, int y) { g_sprite->Draw(tex, nullptr, nullptr, &D3DXVECTOR3(x, y, 0), D3DXCOLOR(1, 1, 1, 1)); };


	Draw(m_resource.background, 0, 0);
	
	Draw(m_resource.create, c_createPos.x, c_createPos.y);
	Draw(m_resource.exit, c_exitPos.x, c_exitPos.y);
	{
		std::lock_guard<std::mutex> locker(m_mtxProcessing);
		auto DrawRoomBar = [&, this](int x, int y, int id, const std::string& name, bool isWaiting)
		{
			Draw(m_resource.listBar, x, y);
			TextA(m_resource.font, x + 40					, y + 50, "ID : " + std::to_string(id)	, DT_NOCLIP | DT_LEFT | DT_VCENTER);
			TextA(m_resource.font, x + 180					, y + 50, name, DT_NOCLIP | DT_LEFT | DT_VCENTER);
			TextA(m_resource.font, x + c_listBarSize.x - 40	, y + 50, isWaiting ? "Waiting" : "Playing", DT_NOCLIP | DT_RIGHT | DT_VCENTER);
		};
		
		int count = 0;
		for (auto& iter : m_roomList)
		{
			int id = iter.first;
			std::string name = iter.second.name;

			DrawRoomBar(c_listBarStartPos.x, c_listBarStartPos.y + c_listBarInterval * count, id, name, iter.second.state == GomokuRoomData::State::Waiting);
			count++;
		}
	}

	g_sprite->End();
}

void GomokuLobby::Release()
{
	DetachConnectorReturner();
}





bool GomokuLobby::MessageProcessing(AsyncConnector & server, int recvResult, SocketBuffer & recvData)
{
	if (recvResult > 0)
	{
		arJSON iJSON;
		Message msg = Message::Invalid;
		{
			recvData[recvResult] = NULL;
			if (JSON_To_arJSON(recvData.Buffer(), iJSON))
			{
				locked_cout << "JSON Errored by received message." << endl;
				return true;
			}

			if ((msg = CheckMessage(iJSON["Message"].Str())) == Message::Invalid)
			{
				locked_cout << "An invalid message(" << static_cast<int>(msg) << ") was received by Lobby" << endl;
				return false;
			}
		}
		mutex_lock_guard actionLocker(m_mtxProcessing);

		switch (msg)
		{
		case GomokuLobby::Message::RoomListRefresh:		RoomListRefresh	(iJSON);	break;
		case GomokuLobby::Message::RoomCreated:			RoomCreated		(iJSON);	break;
		case GomokuLobby::Message::RoomDestroyed:		RoomDestroyed	(iJSON);	break;
		case GomokuLobby::Message::RoomEntered:			RoomEntered		(iJSON);	break;
		case GomokuLobby::Message::RoomUpdate:			RoomUpdate		(iJSON);	break;
		case GomokuLobby::Message::LobbyLeaved:			LobbyLeaved		(iJSON);	break;
		default:
			locked_cout << "An invalid message(" << static_cast<int>(msg) << ") was received by Lobby" << endl;
			break;
		}
	}
	else
	{
		mutex_lock_guard actionLocker(m_mtxProcessing);

		locked_cout << "The connection to the server has been lost." << endl;
		DetachConnectorReturner();
		SntInst(SceneManager).ChangeScene(new GomokuTitle());
		return true;
	}
	return false;
}

GomokuLobby::Message GomokuLobby::CheckMessage(const std::string & msg) const
{
	if (msg.empty())					return Message::Invalid;
	else if (msg == "RoomListRefresh")	return Message::RoomListRefresh;
	else if (msg == "RoomCreated")		return Message::RoomCreated;
	else if (msg == "RoomDestroyed")	return Message::RoomDestroyed;
	else if (msg == "RoomEntered")		return Message::RoomEntered;
	else if (msg == "RoomUpdate")		return Message::RoomUpdate;
	else if (msg == "LobbyLeaved")		return Message::LobbyLeaved;
	else								return Message::Invalid;
}



bool GomokuLobby::RoomListRefresh(const arJSON & iJSON)
{
	if (!iJSON.IsIn("RoomList"))
		return false;

	{ locked_cout << "Room list refreshing..." << endl; }
	m_roomList.clear();
	for (auto& iter : iJSON["RoomList"])
	{
		if (!iter.IsIn("ID") || !iter.IsIn("State"))
			continue;

		int id						= iter["ID"].Int();
		std::string name;
		bool locked;
		GomokuRoomData::State state	= static_cast<GomokuRoomData::State>(iter["State"].Int());

		if (iter.IsIn("Name"))
			name = iter["Name"].Str();
		if (iter.IsIn("Locked"))
			locked = iter["Locked"].Int();

		if (RoomCreated(id, name, locked, state))
			return true;
	}
	{ locked_cout << "The room list has been refreshed." << endl; }
	return false;
}

bool GomokuLobby::RoomCreated(const arJSON & iJSON)
{
	if (!iJSON.IsIn("Room"))
		return false;

	const arJSON& roomJSON = iJSON["Room"].Sub();
	if (!roomJSON.IsIn("ID"))
		return false;

	int id						= roomJSON["ID"].Int();
	std::string name;
	bool locked;
	GomokuRoomData::State state	= GomokuRoomData::State::Waiting;
	///GomokuRoomData::State state	= static_cast<GomokuRoomData::State>(roomJSON["State"].Int());

	if (roomJSON.IsIn("Name"))
		name = roomJSON["Name"].Str();
	if (roomJSON.IsIn("Locked"))
		locked = roomJSON["Locked"].Int();

	return RoomCreated(id, name, locked, state);
}

bool GomokuLobby::RoomUpdate(const arJSON & iJSON)
{
	if (!iJSON.IsIn("Room"))
		return false;
	const arJSON& roomJSON = iJSON["Room"].Sub();
	if (!roomJSON.IsIn("ID") || !roomJSON.IsIn("State"))
		return false;

	int id = roomJSON["ID"].Int();
	GomokuRoomData::State state = static_cast<GomokuRoomData::State>(roomJSON["State"].Int());

	return RoomUpdate(id, state);
}

bool GomokuLobby::RoomEntered(const arJSON & iJSON)
{
	if (!iJSON.IsIn("Room"))
		return false;

	const arJSON& roomJSON = iJSON["Room"].Sub();
	if (!roomJSON.IsIn("ID") || !roomJSON.IsIn("Name") || !roomJSON.IsIn("Locked"))
		return false;

	int id = roomJSON["ID"].Int();
	std::string name = roomJSON["Name"].Str();
	bool isLocked = roomJSON["Locked"].Int();
	return RoomEntered(id, name, isLocked);
}

bool GomokuLobby::RoomDestroyed(const arJSON & iJSON)
{
	if (iJSON.IsIn("RoomDestroyed"))
		return RoomDestroyed(iJSON["RoomDestroyed"].Int());
	return false;
}

bool GomokuLobby::LobbyLeaved(const arJSON & iJSON)
{
	if (iJSON.IsIn("Result") && iJSON["Result"].Int())
		return LobbyLeaved();
	return false;
}



bool GomokuLobby::RoomCreated(int id, const std::string& name, bool isLocked, GomokuRoomData::State state)
{
	if (m_roomList.find(id) != m_roomList.end())
		return false;

	GomokuRoomData room;
	room.id = id;
	room.name = name;
	room.isLocked = isLocked;
	room.state = state;

	m_roomList.insert(std::make_pair(id, room));
	return false;
}

bool GomokuLobby::RoomUpdate(int id, GomokuRoomData::State state)
{
	auto iter = m_roomList.find(id);
	if (iter == m_roomList.end())
		return false;

	GomokuRoomData& room = iter->second;
	room.state = state;
	return false;
}

bool GomokuLobby::RoomEntered(int id, const std::string& name, bool isLocked)
{
	DetachConnectorReturner();
	SntInst(SceneManager).ChangeScene(new GomokuRoom(m_serverConnector, id, name));
	///SntInst(SceneManager).ChangeScene(new GomokuRoom(m_serverConnector, id, name, isLocked));
	return false;
}

bool GomokuLobby::RoomDestroyed(int id)
{
	auto iter = m_roomList.find(id);
	if (iter != m_roomList.end())
		m_roomList.erase(iter);
	return false;
}

bool GomokuLobby::LobbyLeaved()
{
	DetachConnectorReturner();
	SntInst(SceneManager).ChangeScene(new GomokuTitle());
	locked_cout << "Lobby >> Leaved lobby" << endl;
	return true;
}





GomokuLobby::Resource::Resource()
	: background(nullptr)
	
	, font(nullptr)
	, listBar(nullptr)
	, create(nullptr)
	, exit(nullptr)
{
}

GomokuLobby::Resource::~Resource()
{
	if (background)	background->Release();

	if (font)		font->Release();
	if (listBar)	listBar->Release();
	if (create)		create->Release();
	if (exit)		exit->Release();
}
