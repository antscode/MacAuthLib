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
		void Authenticate(AuthRequest request, function<void(AuthResponse)> onComplete);
		void HandleEvents(EventRecord *eventPtr);

	private:
		MacWifiLib* _wifiLib;
		AuthRequest _request;
		DialogRef _dialog;
		function<void(AuthResponse)> _onComplete;
		string _userCode, _deviceCode;
		enum UIState
		{
			PleaseWait,
			EnterCode
		};

		UIState _uiState;

		void CodeRequest();
		void CodeResponse(MacWifiResponse response);
		void StatusRequest();
		void StatusResponse(MacWifiResponse response);
		string GetResponseErrorMsg(MacWifiResponse response);
		void UpdateUI();
		void EraseStatusText();
		void CloseDialog();
		void Cancel();
};

#endif