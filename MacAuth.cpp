#include "MacAuth.h"
#include "MacAuthUtils.h"
#include <gason/gason.hpp>

using namespace std::placeholders;
using namespace gason;

MacAuth::MacAuth(MacWifiLib* wifiLib)
{
	_wifiLib = wifiLib;
}

AuthResponse MacAuth::Authenticate(AuthRequest request)
{
	EventRecord event;

	_request = request;
	_authDialog = GetNewDialog(1028, 0, (WindowPtr)-1);
	_uiState = PleaseWait;
	_run = true;

	_response.Success = false;
	_response.Error = "";
	_response.Code = "";

	UpdateUI();
	CodeRequest();

	while (_run)
	{
		if (WaitNextEvent(everyEvent, &event, 0, NULL))
		{
			HandleEvents(&event);
		}
	}

	return 	_response;
}

void MacAuth::HandleEvents(EventRecord *eventPtr)
{
	WindowPtr windowPtr;
	short item;

	FindWindow(eventPtr->where, &windowPtr);

	switch (eventPtr->what)
	{
		case kHighLevelEvent:
			AEProcessAppleEvent(eventPtr);
			break;

		case mouseDown:
		{
			if (windowPtr == _authDialog)
			{
				if (DialogSelect(eventPtr, &_authDialog, &item))
				{
					switch (item)
					{
					case 1:
						Cancel();
						break;

					case 2:
						StatusRequest();
						break;
					}
				}
			}
			break;
		}

		case updateEvt:
			HandleUpdate(eventPtr);
			break;
	}
}

void MacAuth::HandleUpdate(EventRecord *eventPtr)
{
	WindowPtr windowPtr = (WindowPtr)eventPtr->message;

	BeginUpdate(windowPtr);
	EndUpdate(windowPtr);
}

void MacAuth::Cancel()
{
	_response.Success = false;
	_response.Code = "";
	_response.Error = "User cancelled.";

	CloseAuthDialog();
	_run = false;
}

void MacAuth::UpdateUI()
{
	MacSetPort(_authDialog);

	switch (_uiState)
	{
		case PleaseWait:
		{
			MacAuthUtils::FrameDefaultButton(_authDialog, 2, false);

			TextSize(0);
			ForeColor(blackColor);
			MoveTo(10, 20);
			EraseStatusText();

			DrawString("\pPlease wait...");

			UpdateDialog(_authDialog, _authDialog->visRgn);
			break;
		}

		case EnterCode:
		{
			MacAuthUtils::FrameDefaultButton(_authDialog, 2, true);
			
			TextSize(0);
			ForeColor(blackColor);
			MoveTo(10, 20);
			EraseStatusText();

			DrawString("\pVisit ");

			ForeColor(blueColor);
			DrawString("\p68k.io/login ");

			ForeColor(blackColor);

			Point curPos;
			GetPen(&curPos);
			int maxWidth = _authDialog->portRect.right - curPos.h;

			MacAuthUtils::DrawTextToWidth("on your smartphone and enter this code:", maxWidth, 15, 10);

			GetPen(&curPos);
			TextSize(24);

			string userCode = _userCode.c_str();
			const char* cUserCode = userCode.c_str();
			char* pUserCode = (char*)MacAuthUtils::CtoPStr((char*)cUserCode);

			short strWidth;
			strWidth = StringWidth((ConstStr255Param)pUserCode);
			int centrePos = (_authDialog->portRect.right - strWidth) / 2;

			MoveTo(centrePos, curPos.v + 34);
			DrawString((ConstStr255Param)pUserCode);
			
			UpdateDialog(_authDialog, _authDialog->visRgn);
			break;
		}
	}
}

void MacAuth::CloseAuthDialog()
{
	CloseDialog(_authDialog);
}

void MacAuth::EraseStatusText()
{
	Rect rect;
	MacSetRect(&rect,
		_authDialog->portRect.left,
		_authDialog->portRect.top,
		_authDialog->portRect.right,
		_authDialog->portRect.bottom - 40);

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
		JsonAllocator allocator;
		JsonValue root;
		JsonParseStatus status = jsonParse((char*)response.Content.c_str(), root, allocator);
		
		if (status == JSON_PARSE_OK)
		{
			_userCode = root("user_code").toString();
			_deviceCode = root("device_code").toString();

			if (_userCode != "")
			{
				_uiState = EnterCode;
				UpdateUI();
			}
			else
			{
				error = root("error").toString(); 
			}
		}
	}
	else
	{
		error = GetResponseErrorMsg(response);
	}

	if (error != "")
	{
		ParamText(MacAuthUtils::StrToPStr(error), nil, nil, nil);
		StopAlert(1030, nil);

		_response.Success = false;
		_response.Code = "";
		_response.Error = error;

		CloseAuthDialog();
		_run = false;
	}
}

void MacAuth::StatusRequest()
{
	_uiState = PleaseWait;
	UpdateUI();

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
		JsonAllocator allocator;
		JsonValue root;
		JsonParseStatus status = jsonParse((char*)response.Content.c_str(), root, allocator);
		
		if (status == JSON_PARSE_OK)
		{
			if (root("error").isString())
			{
				error = root("error").toString();
			}
			
			if (root("status").isString())
			{
				string status = root("status").toString();

				if (status == "pending")
				{
					NoteAlert(1029, nil);

					_uiState = EnterCode;
					UpdateUI();
					return;
				}
				else if (status == "complete")
				{
					if (root("code").isString())
					{
						string code = root("code").toString();

						if (code != "")
						{
							_response.Success = true;
							_response.Code = code;
							_response.Error = "";

							CloseAuthDialog();
							_run = false;
							return;
						}
					}
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
		ParamText(MacAuthUtils::StrToPStr(response.ErrorMsg), nil, nil, nil);
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