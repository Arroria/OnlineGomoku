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
	
	_CreateFont(20, L"", m_resource.font);

	CreateTex(L"./Resource/lobby/background.png", m_resource.background);

	CreateTex(L"./Resource/lobby/listBar.png", m_resource.listBar);
	CreateTex(L"./Resource/lobby/create.png", m_resource.create);
	CreateTex(L"./Resource/lobby/exit.png", m_resource.exit);



	AttachConnectorReturner();
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
			mutex_lock_guard lockerR(m_mtxRoomList);
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
					EnterRoomPlease(index);
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
			cout << iter.first << " : " << iter.second << endl;
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
		std::lock_guard<std::mutex> locker(m_mtxRoomList);
		int count = 0;
		for (auto& iter : m_roomList)
		{
			Draw(m_resource.listBar, c_listBarStartPos.x, c_listBarStartPos.y + c_listBarInterval * count);

			std::wstring id = std::to_wstring(iter.first);
			std::string name = iter.second;
			//TextW(m_resource.font, c_roomListPos.x + 50, c_roomListPos.y + c_roomListSize.y * count + 25, id, DT_NOCLIP | DT_LEFT | DT_CENTER);
			//TextA(m_resource.font, c_roomListPos.x + 250, c_roomListPos.y + c_roomListSize.y * count + 25, name, DT_NOCLIP | DT_LEFT | DT_CENTER);

			count++;
		}
	}

	g_sprite->End();
}

void GomokuLobby::Release()
{
	DetachConnectorReturner();
}



bool GomokuLobby::MessageProcessing(AsyncConnector & user, int recvResult, SocketBuffer & recvData)
{
	if (recvResult > 0)
	{
		recvData[recvResult] = NULL;
		arJSON iJSON;
		if (JSON_To_arJSON(recvData.Buffer(), iJSON))
		{
			locked_cout << "Lobby >> Server returned JSON errored" << endl;
			return true;
		}

		const std::string& iMessage = iJSON["Message"].Str();
		///	 if (iMessage == "CreateRoom")	{ if (CreateRoom(user, iJSON))	return true; }
		///else if (iMessage == "JoinRoom")	{ if (EnterRoom(user, iJSON))	return true; }
		///else 

			 if (iMessage == "RoomCreated")		{ if (RoomCreated(iJSON))	return true; }
		else if (iMessage == "RoomEntered")		{ if (RoomEntered(iJSON))	return true; }
		else if (iMessage == "LobbyLeaved")		{ if (LobbyLeaved(iJSON))	return true; }
		else if (iMessage == "RoomList")		{ if (RoomList(iJSON))		return true; }
		else if (iMessage == "RoomDestroyed")	{ if (RoomDestroyed(iJSON))	return true; }
		else
		{ locked_cout << "Lobby >> UnknownMessage recived : " << iMessage << endl; }
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



bool GomokuLobby::RoomCreated(const arJSON & iJSON)
{
	if (!iJSON.IsIn("Room"))
		return false;
	const arJSON& roomJSON = iJSON["Room"].Sub();
	if (!roomJSON.IsIn("ID"))
		return false;

	int id = roomJSON["ID"].Int();
	std::string name;
	if (roomJSON.IsIn("Name"))
		name = roomJSON["Name"].Str();

	mutex_lock_guard lockerR(m_mtxRoomList);
	m_roomList.insert(std::make_pair(id, name));
	locked_cout << "Lobby >> Room list updated : created" << endl;
	return false;
}

bool GomokuLobby::RoomEntered(const arJSON & iJSON)
{
	if (!iJSON.IsIn("Room"))
		return false;

	const arJSON& roomJSON = iJSON["Room"].Sub();
	if (!roomJSON.IsIn("ID") || !roomJSON.IsIn("Name"))
		return false;

	int id = roomJSON["ID"].Int();
	std::string name = roomJSON["Name"].Str();

	DetachConnectorReturner();
	SntInst(SceneManager).ChangeScene(new GomokuRoom(m_serverConnector, id, name));

	cout_region_lock;
	cout << "Lobby >> RoomEntered : " << id << ":" << name << endl;
	return false;
}

bool GomokuLobby::LobbyLeaved(const arJSON & iJSON)
{
	if (iJSON.IsIn("Result") && iJSON["Result"].Int())
	{
		DetachConnectorReturner();
		SntInst(SceneManager).ChangeScene(new GomokuTitle());
		locked_cout << "Lobby >> Leaved lobby" << endl;
		return true;
	}
	return false;
}

bool GomokuLobby::RoomList(const arJSON & iJSON)
{
	if (iJSON.IsIn("RoomList"))
	{
		mutex_lock_guard locker2(m_mtxRoomList);
		for (auto& iter : iJSON["RoomList"])
		{
			if (!iter.IsIn("ID"))
				continue;

			int id = iter["ID"].Int();
			std::string name;

			if (iter.IsIn("Name"))
				name = iter["Name"].Str();

			m_roomList.insert(std::make_pair(id, name));
		}
		locked_cout << "Lobby >> Room list updated : resetted" << endl;
	}
	return false;
}

bool GomokuLobby::RoomDestroyed(const arJSON & iJSON)
{
	if (iJSON.IsIn("RoomDestroyed"))
	{
		mutex_lock_guard locker2(m_mtxRoomList);
		auto iter = m_roomList.find(iJSON["RoomDestroyed"].Int());
		if (iter != m_roomList.end())
			m_roomList.erase(iter);

		locked_cout << "Lobby >> Room list updated : destroyed" << endl;
	}
	return false;
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
