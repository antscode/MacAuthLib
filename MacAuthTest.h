#include <Dialogs.h>

bool _run = true;

int main();
void InitToolBox();
void ShowMainWindow();
void EventLoop();
void DoEvent(EventRecord *eventPtr);
void HandleMouseDown(EventRecord *eventPtr);
void HandleInContent(EventRecord *eventPtr);
void HandleUpdate(EventRecord *eventPtr);
void HandleActivate(EventRecord *eventPtr);
void HandleOSEvt(EventRecord *eventPtr);
void HandleResponse(AuthResponse response);
pascal OSErr ProcessResponseEvent(AppleEvent* appleEvent, AppleEvent* reply, long refCon);