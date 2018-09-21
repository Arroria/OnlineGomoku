#include "stdafx.h"
#include "GomokuLobby.h"

#include "SceneManager.h"
#include "GomokuTitle.h"
#include "GomokuRoom.h"



constexpr POINT c_createPos = { 0, 0 };
constexpr POINT c_createSize = { 400, 200 };
constexpr POINT c_quitPos = { c_createPos.x + c_createSize.x, c_createPos.y };
constexpr POINT c_quitSize = { 400, 200 };

constexpr POINT c_roomListPos = { 0, c_createPos.y + c_createSize.y };
constexpr POINT c_roomListSize = { 1000, 100 };



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

	CreateTex(L"./Resource/quit.png", m_resource.quit);
	CreateTex(L"./Resource/createRoom.png", m_resource.create);
	CreateTex(L"./Resource/roomList.png", m_resource.list);



	AttachConnectorReturner();
	locked_cout << "Lobby >> Enter" << endl;
}

void GomokuLobby::Update()
{
	if (g_inputDevice.IsKeyDown(VK_LBUTTON))
	{
		POINT mousePos = g_inputDevice.MousePos();
		
		//Create Room
		if (c_createPos.x <= mousePos.x && mousePos.x < c_createPos.x + c_createSize.x &&
			c_createPos.y <= mousePos.y && mousePos.y < c_createPos.y + c_createSize.y)
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
		//Quit
		else
		if (c_quitPos.x <= mousePos.x && mousePos.x < c_quitPos.x + c_quitSize.x &&
			c_quitPos.y <= mousePos.y && mousePos.y < c_quitPos.y + c_quitSize.y)
		{
			arJSON oJSON;
			oJSON["Message"] = "LeaveLobby";
			__ar_send(*m_serverConnector, oJSON);

			locked_cout << "Lobby >> Leave Lobby..." << endl;
		}
		else
		{
			//List Click (Enter the room)
			mutex_lock_guard lockerR(m_mtxRoomList);
			if (m_roomList.size())
			{
				if (c_roomListPos.x <= mousePos.x && mousePos.x < c_roomListPos.x + c_roomListSize.x &&
					c_roomListPos.y <= mousePos.y && mousePos.y < c_roomListPos.y + c_roomListSize.y * m_roomList.size())
				{
					int index = (mousePos.y - c_roomListPos.y) / c_roomListSize.y;
					if (0 <= index && index < m_roomList.size())
					{
						int id = -1;
						for (auto& iter : m_roomList)
						{
							if (!index)
							{
								id = iter.first;
								break;
							}
							index--;
						}

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
						if (id >= 0)
							EnterRoomPlease(id);
					}
				}
			}
		}
	}

	if (g_inputDevice.IsKeyDown('1'))
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


	Draw(m_resource.create, c_createPos.x, c_createPos.y);
	Draw(m_resource.quit, c_quitPos.x, c_quitPos.y);
	std::lock_guard<std::mutex> locker(m_mtxRoomList);
	int count = 0;
	for (auto& iter : m_roomList)
	{
		Draw(m_resource.list, c_roomListPos.x, c_roomListPos.y + c_roomListSize.y * count);
		
		std::wstring id = std::to_wstring(iter.first);
		std::string name = iter.second;
		TextW(m_resource.font, c_roomListPos.x + 50, c_roomListPos.y + c_roomListSize.y * count + 25, id, DT_NOCLIP | DT_LEFT | DT_CENTER);
		TextA(m_resource.font, c_roomListPos.x + 250, c_roomListPos.y + c_roomListSize.y * count + 25, name, DT_NOCLIP | DT_LEFT | DT_CENTER);

		count++;
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
	: font(nullptr)
	, list(nullptr)
	, quit(nullptr)
	, create(nullptr)
{
}

GomokuLobby::Resource::~Resource()
{
	if (font)	font->Release();

	if (list)	list->Release();
	if (quit)	quit->Release();
	if (create)	create->Release();
}
