#pragma once
#include "Scene.h"

#include "Button.h"


class StaticButton;
class Button;

class GomokuRoomData
{
public:
	enum class State : int
	{
		Waiting = 0,
		Ready = 1,
		Playing = 2,
	};

public:
	GomokuRoomData() : id(-1), name(), isLocked(false), state(State::Waiting) {}

	int id;
	std::string name;
	bool isLocked;
	State state;
};

#include <map>
class GomokuLobby final :
	public Scene
{
private:
	enum class Message
	{
		RoomListRefresh,
		RoomCreated,
		RoomDestroyed,
		RoomEntered,
		RoomUpdate,
		LobbyLeaved,

		Invalid = -1
	};

public:
	GomokuLobby(AsyncConnector* serverConnector);
	~GomokuLobby();

public:
	void Init() override;
	void Update() override;
	void Render() override;
	void Release() override;
	LRESULT MsgProc(HWND, UINT, WPARAM, LPARAM) override;

private:
	inline void AttachConnectorReturner()
	{
		m_serverConnector->Returner([this](AsyncConnector & user, int recvResult, SocketBuffer & recvData)->bool { return MessageProcessing(user, recvResult, recvData); });
	}
	inline void DetachConnectorReturner() { m_serverConnector->Returner(nullptr); }
	bool MessageProcessing(AsyncConnector&, int, SocketBuffer&);
	Message CheckMessage(const std::string& msg) const;

	//JSON analysis
	bool RoomListRefresh	(const arJSON& iJSON);
	bool RoomCreated		(const arJSON& iJSON);
	bool RoomUpdate			(const arJSON& iJSON);
	bool RoomEntered		(const arJSON& iJSON);
	bool RoomDestroyed		(const arJSON& iJSON);
	bool LobbyLeaved		(const arJSON& iJSON);

	//Action
	bool RoomCreated(int id, const std::string& name, bool isLocked, GomokuRoomData::State state);
	bool RoomUpdate(int id, GomokuRoomData::State state);
	bool RoomEntered(int id, const std::string& name, bool isLocked);
	bool RoomDestroyed(int id);
	bool LobbyLeaved();

private:
	AsyncConnector * m_serverConnector;
	mutable std::mutex m_mtxProcessing;
	
	class UIRoomList
	{
	public:
		UIRoomList(AsyncConnector* serverConnector);

		void Init();
		void Update();
		void Render();
		void Release();

		inline void ListClear() { m_roomList.clear(); }
		void RegistRoom(const GomokuRoomData& data);
		void UpdateRoom(int roomID, GomokuRoomData::State state);
		void UnregistRoom(int roomID);

	private:
		AsyncConnector * m_serverConnector;

		using UIRoom = std::pair<GomokuRoomData, std::shared_ptr<Button>>;
		std::map<int, UIRoom> m_roomList;
		int m_scroll;

		LPD3DXFONT r_font;
		LPDIRECT3DTEXTURE9 r_listBar;
		LPDIRECT3DTEXTURE9 r_isLocked;
		LPDIRECT3DTEXTURE9 r_isPlaying;
	} m_uiRoomList;
	Button m_btnExit;
	class UIRoomCreator
	{
	public:
		UIRoomCreator(AsyncConnector* serverConnector);
		~UIRoomCreator();

		void Init();
		void Update();
		void Render();
		void Release();
		LRESULT MsgProc(HWND, UINT, WPARAM, LPARAM);

	private:
		AsyncConnector * m_serverConnector;
		IMEDevice* m_ime;
		enum class State
		{
			Nothing,
			Name,
			Password,
		} m_state;
		
		Button m_btnCreate;
		Button m_btnName;
		Button m_btnPassword;
		std::string m_name;
		std::string m_password;

		LPD3DXFONT r_font;
		LPDIRECT3DTEXTURE9 r_create;
		LPDIRECT3DTEXTURE9 r_input;
	} m_uiRoomCreator;

	struct Resource
	{
		LPDIRECT3DTEXTURE9 background;
		
		LPDIRECT3DTEXTURE9 exit;

		Resource();
		~Resource();
	} m_resource;
};
