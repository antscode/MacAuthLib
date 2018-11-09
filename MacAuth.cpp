#include "MacAuth.h"
#include "Util.h"
#include <Dialogs.h>
#include <json/json.h>

using namespace std::placeholders;
DialogRef _dialog;

MacAuth::MacAuth(MacWifiLib* wifiLib)
{
	_wifiLib = wifiLib;
}

void MacAuth::Authenticate(AuthRequest request, function<void(AuthResponse)> onComplete)
{
	_request = request;
	_onComplete = onComplete;

	_dialog = GetNewDialog(1028, 0, (WindowPtr)-1);

	_uiState = PleaseWait;
	UpdateUI();

	CodeRequest();
}

void MacAuth::HandleEvents(EventRecord *eventPtr)
{

	WindowPtr windowPtr;
	FindWindow(eventPtr->where, &windowPtr);

	if (windowPtr == _dialog)
	{
		switch (eventPtr->what)
		{
			case mouseDown:
				HandleMouseDown(eventPtr);
				break;
		}
	}
}

void MacAuth::HandleMouseDown(EventRecord *eventPtr)
{
	WindowPtr window;
	short int part;

	part = FindWindow(eventPtr->where, &window);

	switch (part)
	{
		case inContent:
			HandleInContent(eventPtr);
			break;

		case inDrag:
			DragWindow(window, eventPtr->where, &qd.screenBits.bounds);
			break;

		case inGoAway:
			if (TrackGoAway(window, eventPtr->where))
			{
				CloseDialog();
			}
			break;
	}
}

void MacAuth::HandleInContent(EventRecord *eventPtr)
{
	short item;

	if (DialogSelect(eventPtr, &_dialog, &item))
	{
		switch (item)
		{
			case 1:
				Cancel();
				break;

			case 2:
				CheckUserCode();
				break;
		}
	}
}

void MacAuth::Cancel()
{
	// TODO: in MacWifi Comms::Http.CancelRequest();
	AuthResponse response;

	response.Success = false;
	response.Code = "";
	response.Error = "User cancelled.";

	CloseDialog();
	_onComplete(response);
}

void MacAuth::UpdateUI()
{
	MacSetPort(_dialog);

	switch (_uiState)
	{
		case PleaseWait:
		{
			Util::FrameDefaultButton(_dialog, 2, false);

			TextSize(0);
			ForeColor(blackColor);
			MoveTo(10, 20);
			EraseStatusText();

			string msg = "Contacting MacAuth, please wait...";
			DrawString(Util::StrToPStr(msg));

			UpdateDialog(_dialog, _dialog->visRgn);
			break;
		}

		case EnterCode:
		{
			Util::FrameDefaultButton(_dialog, 2, true);
			
			TextSize(0);
			ForeColor(blackColor);
			MoveTo(10, 20);
			EraseStatusText();

			DrawString("\pVisit ");

			ForeColor(blueColor);
			DrawString("\p68k.io/login");

			ForeColor(blackColor);

			Point curPos;
			GetPen(&curPos);
			int maxWidth = _dialog->portRect.right - curPos.h;

			Util::DrawTextToWidth("on your smartphone and enter this code:", maxWidth, 15, 10);

			GetPen(&curPos);
			TextSize(24);

			string userCode = _userCode.c_str();
			const char* cUserCode = userCode.c_str();
			char* pUserCode = (char*)Util::CtoPStr((char*)cUserCode);

			short strWidth;
			strWidth = StringWidth((ConstStr255Param)pUserCode);
			int centrePos = (_dialog->portRect.right - strWidth) / 2;

			MoveTo(centrePos, curPos.v + 34);
			DrawString((ConstStr255Param)pUserCode);
			UpdateDialog(_dialog, _dialog->visRgn);

			TextSize(0);
			break;
		}
	}
}

void MacAuth::CloseDialog()
{
	DisposeDialog(_dialog);
	_dialog = 0;
}

void MacAuth::EraseStatusText()
{
	Rect rect;
	MacSetRect(&rect,
		_dialog->portRect.left,
		_dialog->portRect.top,
		_dialog->portRect.right,
		_dialog->portRect.bottom - 40);

	EraseRect(&rect);
}

void MacAuth::CodeRequest()
{
	string params = "ma_provider=" + _request.Provider + "&ma_client_id=" + _request.ClientId;

	for (map<string, string>::iterator it = _request.Params.begin(); it != _request.Params.end(); ++it)
	{
		params += "&" + it->first + "=" + it->second;
	}

	_wifiLib->Post("https://68k.io/code?" + params,
		"", 
		std::bind(&MacAuth::CodeResponse, this, _1));
}

void MacAuth::CodeResponse(MacWifiResponse response)
{
	string error = "";

	if (response.Success)
	{
		Json::Value root;
		Json::Reader reader;
		bool parseSuccess = reader.parse(response.Content.c_str(), root);

		if (parseSuccess)
		{
			if (root["user_code"].asString() != "")
			{
				_userCode = root["user_code"].asString();
				_deviceCode = root["device_code"].asString();

				_uiState = EnterCode;
				UpdateUI();
			}
			else
			{
				error = root["error"].asString(); 
			}
		}
	}
	else
	{
		error = GetResponseErrorMsg(response);
	}

	if (error != "")
	{
		ParamText(Util::StrToPStr(error), nil, nil, nil);
		StopAlert(1030, nil);

		AuthResponse response;

		response.Success = false;
		response.Code = "";
		response.Error = error;

		_onComplete(response);
		CloseDialog();
	}
}

void MacAuth::CheckUserCode()
{
	_uiState = PleaseWait;
	UpdateUI();

	StatusRequest();
}

void MacAuth::StatusRequest()
{
	string params = "ma_client_id=" + _request.ClientId + "&device_code=" + _deviceCode;

	_wifiLib->Post("https://68k.io/status",
		params,
		std::bind(&MacAuth::StatusResponse, this, _1));
}

void MacAuth::StatusResponse(MacWifiResponse response)
{
	string error = "";

	if (response.Success)
	{
		Json::Value root;
		Json::Reader reader;
		bool parseSuccess = reader.parse(response.Content.c_str(), root);

		if (parseSuccess)
		{
			string status = root["status"].asString();
			error = root["error"].asString();

			if (status == "pending")
			{
				NoteAlert(1029, nil);

				_uiState = EnterCode;
				UpdateUI();
			}
			else if (status == "complete")
			{
				string code = root["code"].asString();
				error = root["error"].asString();

				if (code != "")
				{
					AuthResponse response;

					response.Success = true;
					response.Code = code;
					response.Error = "";

					CloseDialog();
					_onComplete(response);
				}
			}
		}
	}
	else
	{
		error = GetResponseErrorMsg(response);
	}

	if (error != "")
	{
		ParamText(Util::StrToPStr(response.ErrorMsg), nil, nil, nil);
		StopAlert(1030, nil);

		_uiState = EnterCode;
		UpdateUI();
	}
}

string MacAuth::GetResponseErrorMsg(MacWifiResponse response)
{
	string err;

	if (response.Success)
	{
		err = "Server returned status code " + std::to_string(response.StatusCode) + ".";
	}
	else
	{
		err = response.ErrorMsg;
	}

	return err;
}