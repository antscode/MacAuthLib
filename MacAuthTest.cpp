#include "MacAuth.h"
#include "MacAuthTest.h"
#include "Util.h"
#include "Keys.h"

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
	MacSetPort(_window);
	MoveTo(50, 50);
	Util::DebugStr(response.Code);
	DrawString(Util::CtoPStr((char*)response.Code.c_str()));
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