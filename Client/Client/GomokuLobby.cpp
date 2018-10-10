#include "stdafx.h"
#include "GomokuLobby.h"

#include "SceneManager.h"
#include "GomokuTitle.h"
#include "GomokuRoom.h"

#include "Button.h"


constexpr int c_listBarInterval = 100;
constexpr POINT c_createPos = { 400, 600 };
constexpr POINT c_exitPos = { 800, 600 };


#include <filesystem>
inline std::string temp_wtoa(const std::wstring& w)
{
	return std::filesystem::path(w).string();
}


GomokuLobby::GomokuLobby(AsyncConnector* serverConnector)
	: m_serverConnector(serverConnector)
	, m_uiRoomList(serverConnector)
	, m_uiRoomCreator(serverConnector)

	, m_btnExit()
{
}

GomokuLobby::~GomokuLobby()
{
}



void GomokuLobby::Init()
{
	auto CreateTex = [](const std::wstring& path, LPDIRECT3DTEXTURE9& target){ D3DXCreateTextureFromFileExW(DEVICE, path.data(), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, NULL, NULL, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, NULL, nullptr, nullptr, &target); };

	CreateTex(L"./Resource/lobby/background.png", m_resource.background);
	CreateTex(L"./Resource/lobby/exit.png", m_resource.exit);


	m_uiRoomList.Init();
	m_uiRoomCreator.Init();
	
	m_btnExit.ObjLocalPos() = D3DXVECTOR3(c_exitPos.x, c_exitPos.y, 0);
	m_btnExit.SetArea(20, 20, 380, 180);
	m_btnExit.SetTex(m_resource.exit);
	m_btnExit.SetEvent([this]()
	{
		arJSON oJSON;
		oJSON["Message"] = "LeaveLobby";
		__ar_send(*m_serverConnector, oJSON);
	});

	AttachConnectorReturner();
	arJSON readyToInit;
	readyToInit["Message"] = "ReadyToInit";
	__ar_send(*m_serverConnector, readyToInit);

	locked_cout << "Lobby >> Enter" << endl;
}

void GomokuLobby::Update()
{
	m_uiRoomList.Update();
	m_uiRoomCreator.Update();
	m_btnExit.UpdateObj();

	if (g_inputDevice.IsKeyDown(VK_ESCAPE))
		std::terminate();
}

void GomokuLobby::Render()
{
	g_sprite->Begin(D3DXSPRITE_ALPHABLEND);

	auto Draw = [](LPDIRECT3DTEXTURE9 tex, int x, int y) { g_sprite->Draw(tex, nullptr, nullptr, &D3DXVECTOR3(x, y, 0), D3DXCOLOR(1, 1, 1, 1)); };

	Draw(m_resource.background, 0, 0);
	
	m_uiRoomList.Render();
	m_uiRoomCreator.Render();
	m_btnExit.RenderObj();

	g_sprite->End();
}

void GomokuLobby::Release()
{
	m_uiRoomList.Release();
	m_uiRoomCreator.Release();
	DetachConnectorReturner();
}

LRESULT GomokuLobby::MsgProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	return m_uiRoomCreator.MsgProc(hWnd, iMsg, wParam, lParam);
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

	m_uiRoomList.ListClear();
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

	int id = roomJSON["ID"].Int();
	std::string name;
	bool locked;
	GomokuRoomData::State state	= GomokuRoomData::State::Waiting;

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
	GomokuRoomData room;
	room.id = id;
	room.name = name;
	room.isLocked = isLocked;
	room.state = state;

	m_uiRoomList.RegistRoom(room);
	return false;
}

bool GomokuLobby::RoomUpdate(int id, GomokuRoomData::State state)
{
	m_uiRoomList.UpdateRoom(id, state);
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
	m_uiRoomList.UnregistRoom(id);
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
	
	, exit(nullptr)
{
}

GomokuLobby::Resource::~Resource()
{
	if (background)	background->Release();

	if (exit)		exit->Release();
}







GomokuLobby::UIRoomList::UIRoomList(AsyncConnector* serverConnector)
	: m_serverConnector(serverConnector)

	, m_scroll(0)

	, r_font(nullptr)
	, r_listBar(nullptr)
	, r_isLocked(nullptr)
	, r_isPlaying(nullptr)
{
}



void GomokuLobby::UIRoomList::Init()
{
	auto CreateTex = [](const std::wstring& path, LPDIRECT3DTEXTURE9& target) { D3DXCreateTextureFromFileExW(DEVICE, path.data(), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, NULL, NULL, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, NULL, nullptr, nullptr, &target); };
	auto _CreateFont = [](size_t size, const std::wstring& font, LPD3DXFONT& target) { D3DXCreateFontW(DEVICE, size, NULL, FW_DONTCARE, NULL, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, font.data(), &target); };
	
	_CreateFont(40, L"", r_font);
	CreateTex(L"./Resource/lobby/listBar.png",		r_listBar);
	CreateTex(L"./Resource/lobby/isLocked.png",		r_isLocked);
	CreateTex(L"./Resource/lobby/isPlaying.png",	r_isPlaying);
}

void GomokuLobby::UIRoomList::Update()
{
	constexpr int c_scrollSize = 10;

	m_scroll += g_inputDevice.MouseWheel() * c_scrollSize / 120;
	int maxScroll = (static_cast<int>(m_roomList.size()) - 6) * c_listBarInterval;

	if (m_scroll > maxScroll) m_scroll = maxScroll;
	if (m_scroll < 0) m_scroll = 0;


	int listIndex = 0;
	for (auto& iter : m_roomList)
	{
		auto button = iter.second.second;
		button->ObjLocalPos().y = listIndex * c_listBarInterval - m_scroll;

		listIndex++;

		if		(button->ObjLocalPos().y < 0)	continue;
		else if (button->ObjLocalPos().y > 500)	break;
		else									button->UpdateObj();
	}
}

void GomokuLobby::UIRoomList::Render()
{
	for (auto& iter : m_roomList)
	{
		auto button = iter.second.second;

		auto Text = [](LPD3DXFONT font, int x, int y, const std::string& text, DWORD flags = NULL, const D3DXCOLOR& color = D3DXCOLOR(0, 0, 0, 1))
		{
			RECT rc;
			SetRect(&rc, x, y, x, y);
			font->DrawTextA(g_sprite, text.data(), -1, &rc, flags, color);
		};
		auto Draw = [](LPDIRECT3DTEXTURE9 tex, int x, int y) { g_sprite->Draw(tex, nullptr, nullptr, &D3DXVECTOR3(x, y, 0), D3DXCOLOR(1, 1, 1, 1)); };

		auto DrawUI = [&, this](int x, int y)
		{
			const auto& data = iter.second.first;

			int id = data.id;
			const std::string& name = data.name;
			auto state = data.state;
			bool isLocked = data.isLocked;

			button->RenderObj();
			
			if (isLocked)									Draw(r_isLocked, x + 25, y + 25);
			if (state == GomokuRoomData::State::Playing)	Draw(r_isPlaying, x + 1200 - 75, y + 25);

			Text(r_font, x + 100, y + 50, "ID : " + std::to_string(id), DT_NOCLIP | DT_LEFT | DT_VCENTER);
			Text(r_font, x + 250, y + 50, name, DT_NOCLIP | DT_LEFT | DT_VCENTER);
			Text(r_font, x + 1200 - 100, y + 50, (state == GomokuRoomData::State::Waiting ? "1/2" : "2/2"), DT_NOCLIP | DT_RIGHT | DT_VCENTER);
		};

			 if (button->ObjLocalPos().y < 0)	continue;
		else if (button->ObjLocalPos().y > 500)	break;
		else									DrawUI(button->ObjWorldPos().x, button->ObjWorldPos().y);
	}
}

void GomokuLobby::UIRoomList::Release()
{
	if (r_font)			r_font->Release();
	if (r_listBar)		r_listBar->Release();
	if (r_isLocked)		r_isLocked->Release();
	if (r_isPlaying)	r_isPlaying->Release();
}



void GomokuLobby::UIRoomList::RegistRoom(const GomokuRoomData & data)
{
	auto pairib = m_roomList.insert(std::make_pair(data.id, std::make_pair(data, nullptr)));
	if (!pairib.second)
		return;

	auto RequestEnterRoom = [](int id, AsyncConnector* connector)
		{
			arJSON oJSON;
			oJSON["Message"] = "EnterRoom";
			arJSON roomJSON;
			{
				roomJSON["ID"] = id;
			}
			oJSON["Room"] = roomJSON;
			__ar_send(*connector, oJSON);
		};

	std::shared_ptr<Button> btn(new Button());
	btn->SetArea(10, 10, 1190, 90);
	btn->SetTex(r_listBar);
	int& _id = pairib.first->second.first.id;
	btn->SetEvent([&, this]() { RequestEnterRoom(_id, m_serverConnector); });

	pairib.first->second.second = btn;
}

void GomokuLobby::UIRoomList::UpdateRoom(int roomID, GomokuRoomData::State state)
{
	auto iter = m_roomList.find(roomID);
	if (iter != m_roomList.end())
		iter->second.first.state = state;
}

void GomokuLobby::UIRoomList::UnregistRoom(int roomID)
{
	auto iter = m_roomList.find(roomID);
	if (iter != m_roomList.end())
		m_roomList.erase(iter);
}







GomokuLobby::UIRoomCreator::UIRoomCreator(AsyncConnector* serverConnector)
	: m_serverConnector(serverConnector)
	, m_ime(nullptr)
	, m_state(State::Nothing)

	, r_font(nullptr)
	, r_create(nullptr)
	, r_input(nullptr)
{
}

GomokuLobby::UIRoomCreator::~UIRoomCreator()
{
}



void GomokuLobby::UIRoomCreator::Init()
{
	auto CreateTex = [](const std::wstring& path, LPDIRECT3DTEXTURE9& target) { D3DXCreateTextureFromFileExW(DEVICE, path.data(), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, NULL, NULL, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, NULL, nullptr, nullptr, &target); };
	auto _CreateFont = [](size_t size, const std::wstring& font, LPD3DXFONT& target) { D3DXCreateFontW(DEVICE, size, NULL, FW_DONTCARE, NULL, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, font.data(), &target); };

	_CreateFont(30, L"", r_font);
	CreateTex(L"./Resource/lobby/roomInfo.png", r_input);
	CreateTex(L"./Resource/lobby/create.png", r_create);



	m_btnCreate.ObjLocalPos() = D3DXVECTOR3(c_createPos.x, c_createPos.y, 0);
	m_btnCreate.SetArea(20, 20, 380, 180);
	m_btnCreate.SetTex(r_create);
	m_btnCreate.SetEvent([this]()
	{
		arJSON oJSON;
		oJSON["Message"] = "CreateRoom";
		arJSON roomJSON;
		{
			roomJSON["Name"] = m_name.size() ?
				 m_name : []()->std::string
			{
				srand(time(NULL));
				switch (rand() % 4)
				{
				case 0:	return "Hello Gomoku";
				case 1:	return "You never win me";
				case 2:	return "No gomoku no life";
				case 3:	return "JUST COME IN";
				}
				return {};
			}();
			roomJSON["Password"] = m_password;
		}
		oJSON["Room"] = roomJSON;
		__ar_send(*m_serverConnector, oJSON);
	});

	m_btnName.ObjLocalPos() = D3DXVECTOR3(0, 600, 0);
	m_btnName.SetArea(30, 30, 370, 95);
	m_btnName.SetEvent([this]()
	{
		if (m_ime)
		{
			delete m_ime;
			m_ime = nullptr;
		}

		m_state = State::Name;
		m_ime = new IMEDevice;
	});

	m_btnPassword.ObjLocalPos() = D3DXVECTOR3(0, 600, 0);
	m_btnPassword.SetArea(30, 105, 370, 170);
	m_btnPassword.SetEvent([this]()
	{
		if (m_ime)
		{
			delete m_ime;
			m_ime = nullptr;
		}

		m_state = State::Password;
		m_ime = new IMEDevice;
	});
}

void GomokuLobby::UIRoomCreator::Update()
{
	m_btnCreate		.UpdateObj();
	m_btnName		.UpdateObj();
	m_btnPassword	.UpdateObj();
}

void GomokuLobby::UIRoomCreator::Render()
{
	auto Text = [](LPD3DXFONT font, int x, int y, const std::string& text, DWORD flags = NULL, const D3DXCOLOR& color = D3DXCOLOR(0, 0, 0, 1))
	{
		RECT rc;
		SetRect(&rc, x, y, x, y);
		font->DrawTextA(g_sprite, text.data(), -1, &rc, flags, color);
	};
	auto Draw = [](LPDIRECT3DTEXTURE9 tex, int x, int y) { g_sprite->Draw(tex, nullptr, nullptr, &D3DXVECTOR3(x, y, 0), D3DXCOLOR(1, 1, 1, 1)); };
	
	m_btnName.RenderObj();
	m_btnPassword.RenderObj();
	Draw(r_input, m_btnName.ObjLocalPos().x, m_btnName.ObjLocalPos().y);
	Text(r_font, m_btnName.ObjLocalPos().x + 30 + 20, m_btnName.ObjLocalPos().y + 72.5f	, m_name, DT_NOCLIP | DT_LEFT | DT_VCENTER);
	Text(r_font, m_btnName.ObjLocalPos().x + 30 + 20, m_btnName.ObjLocalPos().y + 147.5f	, m_password, DT_NOCLIP | DT_LEFT | DT_VCENTER);

	m_btnCreate.RenderObj();
}

void GomokuLobby::UIRoomCreator::Release()
{
	if (m_ime)
	{
		delete m_ime;
		m_ime = nullptr;
	}

	if (r_font)		r_font->Release();
	if (r_create)	r_create->Release();
	if (r_input)	r_input	->Release();
}

LRESULT GomokuLobby::UIRoomCreator::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (m_ime)
	{
		m_ime->MsgProc(hWnd, msg, wParam, lParam);

		auto __Set = [this](std::string& pos)
		{
			std::wstring data = m_ime->GetString();
			if (data.size() > 10)
				data.resize(10);
			
			pos = temp_wtoa(data);
		};

		switch (m_state)
		{
		case GomokuLobby::UIRoomCreator::State::Name:		__Set(m_name);		break;
		case GomokuLobby::UIRoomCreator::State::Password:	__Set(m_password);	break; 
		}
	}
	return NULL;
}
