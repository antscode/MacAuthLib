#include "MacAuth.h"
#include "MacAuthTest.h"
#include "Keys.h"
#include <gason/gason.hpp>
#include <stdio.h>
#include <vector>
#include "MacAuthUtils.h"

MacWifiLib _wifiLib;
MacAuth _macAuth(&_wifiLib);
DialogPtr _window;

using namespace gason;
using namespace std;

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
			WindowPtr windowPtr;
			FindWindow(event.where, &windowPtr);

			switch (event.what)
			{
				case kHighLevelEvent:
					AEProcessAppleEvent(&event);
					break;

				case mouseDown:
					if (windowPtr == _window)
					{
						HandleMouseDown(&event);
					}
					break;

				case updateEvt:
					HandleUpdate(&event);
					break;
			}
		}
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

				AuthResponse authResponse = _macAuth.Authenticate(authRequest);

				if (authResponse.Success)
				{
					_wifiLib.Post(
						"https://accounts.spotify.com/api/token",
						"client_id=" + Keys::SpotifyClientId +
						"&client_secret=" + Keys::SpotifyClientSecret +
						"&grant_type=authorization_code&code=" + authResponse.Code +
						"&redirect_uri=https://68k.io/login/callback",
						TokenResponse);
				}
			}
		}
	}
}

void TokenResponse(MacWifiResponse& response)
{
	if (response.Success)
	{
		JsonAllocator allocator;
		JsonValue root;
		JsonParseStatus status = jsonParse((char*)response.Content.c_str(), root, allocator);

		if (status == JSON_PARSE_OK)
		{
			string accessToken = root("access_token").toString();

			_wifiLib.SetAuthorization("Bearer " + accessToken);
			_wifiLib.Get("https://api.spotify.com/v1/browse/new-releases?country=AU&limit=1", BrowseResponse);
		}
	}
}

void BrowseResponse(MacWifiResponse& response)
{
	if (response.Success)
	{
		JsonAllocator allocator;
		JsonValue root;
		JsonParseStatus status = jsonParse((char*)response.Content.c_str(), root, allocator);

		if (status == JSON_PARSE_OK)
		{
			JsonValue albums = root("albums");
			JsonValue topAlbum = albums("items")[0];
			JsonValue artist = topAlbum("artists")[0];
			JsonValue image = topAlbum("images")[0];

			_albumName = topAlbum("name").toString();
			_artistName = artist("name").toString();

			string imageUrl = image("url").toString();

			_wifiLib.SetAuthorization("");
			_wifiLib.Utf8ToMacRoman(false);
			_wifiLib.Get(
				"https://68k.io/image?ma_client_id=" + Keys::MacAuthClientId + 
				"&source_url=" + imageUrl +
				"&dest_width=300&dest_height=300", 
				ImageResponse);
		}
	}
}

void ImageResponse(MacWifiResponse& response)
{
	if (response.Success)
	{
		vector<char> v(response.Content.begin(), response.Content.end());
		_image = &v[512]; // Skip 512-byte PICT1 header
		DoUpdate();
	}
}

void HandleUpdate(EventRecord *eventPtr)
{
	WindowPtr windowPtr = (WindowPtr)eventPtr->message;

	BeginUpdate(windowPtr); 
	DoUpdate();
	EndUpdate(windowPtr);
}

void DoUpdate()
{
	MacSetPort(_window);

	if (_image > 0)
	{
		PicHandle imageHandle = (PicHandle)&_image;

		Rect pictRect;
		MacSetRect(&pictRect, 190, 10, 490, 310);

		DrawPicture(imageHandle, &pictRect);
	}

	UpdateDialog(_window, _window->visRgn);
}

pascal OSErr ProcessResponseEvent(AppleEvent* appleEvent, AppleEvent* reply, long refCon)
{
	_wifiLib.ProcessReply(appleEvent);
}