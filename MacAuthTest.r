#include "Dialogs.r"
#include "Processes.r"

resource 'SIZE' (-1) {
	dontSaveScreen,
	acceptSuspendResumeEvents,
	enableOptionSwitch,
	canBackground,
	multiFinderAware,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreChildDiedEvents,
	is32BitCompatible,
	isHighLevelEventAware,
	onlyLocalHLEvents,
	notStationeryAware,
	reserved,
	reserved,
	reserved,
	reserved,
	3000 * 1024,
	3000 * 1024
};

resource 'DLOG' (128, purgeable) {
	{ 0, 0, 316, 495 },
	noGrowDocProc,
	visible,
	goAway,
	0,
	128,
	"MacAuth Test Client",
	centerMainScreen
};

data 'dctb' (128, purgeable) {
   $"0000 0000 0000 FFFF"  /*use default colors*/
};

resource 'DITL' (128, purgeable) {
	{ 
		{ 10, 10, 30, 180 },
		Button { enabled, "Authenticate Spotify" };
	}
};