#include <Dialogs.h>

bool _run = true;
string _albumName = "", _artistName = "";
char* _image = 0;

int main();
void InitToolBox();
void ShowMainWindow();
void EventLoop();
void DoUpdate();
void HandleMouseDown(EventRecord *eventPtr);
void HandleInContent(EventRecord *eventPtr);
void HandleUpdate(EventRecord *eventPtr);
void HandleActivate(EventRecord *eventPtr);
void HandleOSEvt(EventRecord *eventPtr);
void TokenResponse(MacWifiResponse response);
void BrowseResponse(MacWifiResponse response);
void ImageResponse(MacWifiResponse response);
pascal OSErr ProcessResponseEvent(AppleEvent* appleEvent, AppleEvent* reply, long refCon);