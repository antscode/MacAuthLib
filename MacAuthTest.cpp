#include "MacAuth.h"
#include "MacAuthTest.h"
#include "Util.h"
#include "Keys.h"
#include <json/json.h>

MacWifiLib _wifiLib;
MacAuth _macAuth(&_wifiLib);
DialogPtr _window;

int main()
{
	InitToolBox();
	ShowMainWindow();
	EventLoop();
}

void InitToolBox()
{
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
	InitCursor();

	AEInstallEventHandler(
		kCoreEventClass,
		kAEAnswer,
		(AEEventHandlerUPP)ProcessResponseEvent, 
		0L,
		false);
}

void ShowMainWindow()
{
	_window = GetNewDialog(128, 0, (WindowPtr)-1);
	MacSetPort(_window);

	UpdateDialog(_window, _window->visRgn);
}


void EventLoop()
{
	EventRecord event;

	while (_run)
	{
		if (WaitNextEvent(everyEvent, &event, 0, NULL))
		{
			DoEvent(&event);
		}
	}
}

void DoEvent(EventRecord *eventPtr)
{
	_macAuth.HandleEvents(eventPtr);

	WindowPtr windowPtr;
	FindWindow(eventPtr->where, &windowPtr); 

	switch (eventPtr->what)
	{
		case kHighLevelEvent:
			AEProcessAppleEvent(eventPtr);
			break;

		case mouseDown:
			if (windowPtr == _window)
			{
				HandleMouseDown(eventPtr); 
			}
			break;

		case updateEvt:
			if (windowPtr == _window)
			{
				HandleUpdate(eventPtr); 
			}
			break;
	}
}

void HandleMouseDown(EventRecord *eventPtr)
{
	WindowPtr window;
	short int part;

	part = FindWindow(eventPtr->where, &window); 

	switch (part)
	{
		case inContent:
			if (window == FrontWindow())
				HandleInContent(eventPtr);
			break;

		case inDrag:
			if (window == FrontWindow())
				DragWindow(window, eventPtr->where, &qd.screenBits.bounds);
			break;

		case inGoAway: 
			if (TrackGoAway(window, eventPtr->where)) 
			{
				_run = false;
			}
			break;
	}
}

void HandleInContent(EventRecord *eventPtr)
{
	short int item;
	DialogRef dialogRef;

	if (DialogSelect(eventPtr, &dialogRef, &item))
	{
		switch (item)
		{
			case 1:
			{
				AuthRequest authRequest;
				 
				authRequest.ClientId = Keys::MacAuthClientId;
				authRequest.Provider = "spotify"; 

				authRequest.Params.insert(pair<string, string>("client_id", Keys::SpotifyClientId));
				authRequest.Params.insert(pair<string, string>("response_type", "code"));

				_macAuth.Authenticate(authRequest, HandleResponse);
			}
		}
	}
}

void HandleResponse(AuthResponse response)
{
	if (response.Success)
	{
		_wifiLib.Post(
			"https://accounts.spotify.com/api/token", 
			"client_id=" + Keys::SpotifyClientId + 
			"&client_secret=" + Keys::SpotifyClientSecret + 
			"&grant_type=authorization_code&code=" + response.Code + 
			"&redirect_uri=https://68k.io/login/callback", 
			TokenResponse);
	}
}

void TokenResponse(MacWifiResponse response)
{
	if (response.Success)
	{
		Json::Value root;
		Json::Reader reader;
		bool parseSuccess = reader.parse(response.Content.c_str(), root);

		if (parseSuccess)
		{
			string accessToken = root["access_token"].asString();

			_wifiLib.SetAuthorization("Bearer " + accessToken);
			_wifiLib.Get("https://api.spotify.com/v1/browse/new-releases?country=AU&limit=1", BrowseResponse);
		}
	}
}

void BrowseResponse(MacWifiResponse response)
{
	Util::DebugStr(response.Content);
}

void HandleUpdate(EventRecord *eventPtr)
{
	WindowPtr windowPtr = (WindowPtr)eventPtr->message;

	if (windowPtr == FrontWindow())
	{
		BeginUpdate(windowPtr); 
		UpdateDialog(windowPtr, windowPtr->visRgn);
		EndUpdate(windowPtr);
	}
}

pascal OSErr ProcessResponseEvent(AppleEvent* appleEvent, AppleEvent* reply, long refCon)
{
	_wifiLib.ProcessReply(appleEvent);
}