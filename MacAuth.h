#ifndef __MACAUTH__
#define __MACAUTH__

#include <string>
#include <map>
#include <functional>
#include <macwifi/MacWifiLib.h>
#include <Dialogs.h>

using namespace std;

class AuthRequest
{
	public:
		string Provider;
		string ClientId;
		map<string, string> Params;
};

class AuthResponse
{
	public:
		bool Success;
		string Error;
		string Code;
};

class MacAuth
{
	public:
		MacAuth(MacWifiLib* wifiLib);
		AuthResponse Authenticate(AuthRequest request);
		

	private:
		MacWifiLib* _wifiLib;
		AuthRequest _request;
		DialogRef _authDialog;
		AuthResponse _response;
		bool _run;
		string _userCode, _deviceCode;
		enum UIState
		{
			PleaseWait,
			EnterCode
		};

		UIState _uiState;

		void HandleEvents(EventRecord *eventPtr);
		void HandleUpdate(EventRecord *eventPtr);
		void CodeRequest();
		void CodeResponse(MacWifiResponse response);
		void StatusRequest();
		void StatusResponse(MacWifiResponse response);
		string GetResponseErrorMsg(MacWifiResponse response);
		void UpdateUI();
		void EraseStatusText();
		void CloseAuthDialog();
		void Cancel();
};

#endif