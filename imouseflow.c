#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>

#include <substrate.h>
#include <mach/mach.h>

static CFMessagePortRef ashikase_;
static CFDataRef cfTrue_;
static CFDataRef cfFalse_;

static BTN_RIEN = 0;
static BTN_TOUCH = 1;
static BTN_LOCK = 2;
static BTN_HOME = 4;

static Version = "0.0.1";

typedef struct {
    float x, y;
    int buttons;
    BOOL absolute;
} MouseEvent;

MouseEvent event_;
static CFDataRef cfEvent_;

typedef enum {
    MouseMessageTypeEvent,
    MouseMessageTypeSetEnabled
} MouseMessageType;

void writelog(char * txt){
	FILE* fp;
	fp = fopen("fichier.txt", "a" );
	fprintf(fp, txt);
	fprintf(fp, "\n");
	fclose(fp); 
}

void clearlog(){
	FILE* fp;
	fp = fopen("fichier.txt", "w" );
	fprintf(fp, "");
	fclose(fp); 
}


static bool Ashikase() {
    if (ashikase_ == NULL)
	ashikase_ = CFMessagePortCreateRemote(kCFAllocatorDefault, CFSTR("jp.ashikase.mousesupport"));
    if (ashikase_ != NULL)
	return true;

    return false;
}

static void AshikaseSendEvent(float x, float y, int buttons, bool abs) {
    event_.x = x;
    event_.y = y;
    event_.buttons = buttons;
    event_.absolute = abs?YES:NO;
//	 printf("\ne_  = %f, %f, %s\n", x, y, event_.absolute?"true":"false");
//	 printf("arg = %f, %f, %s\n", x, y, abs?"true":"false");

    CFMessagePortSendRequest(ashikase_, MouseMessageTypeEvent, cfEvent_, 0, 0, NULL, NULL);
}

static bool AshikaseSetEnabled(int enabled) {
    if (!Ashikase())
	return false;

    CFMessagePortSendRequest(ashikase_, MouseMessageTypeSetEnabled, enabled ? cfTrue_ : cfFalse_, 0, 0, NULL, NULL);
	return true;
}

void MouseGoto(float x, float y){
	AshikaseSendEvent(x, y, 0, true);
}

void MouseClick(int btn, float x, float y){
	AshikaseSendEvent(x, y, btn, true);
	AshikaseSendEvent(x, y, 0, true);
}

void MouseSlide(float x1, float y1, float x2, float y2){
	AshikaseSendEvent(x1, y1, BTN_TOUCH, true);
	AshikaseSendEvent(x2, y2, BTN_TOUCH, true);
	AshikaseSendEvent(x2, y2, 0, true);
}

int main(int argc, char ** argv)
{
	if(argc<4){
		printf("Version: %s\nUsage: %s cursor[0,1] command\nCommands:\n   move x y\n   click button[nothing, touch, lock, home = 0, 1, 2, 4]\n", Version, argv[0]);
		return 0;
	}

	bool value;value = true;cfTrue_ = CFDataCreate(kCFAllocatorDefault, &value, sizeof(value));
	value = false;cfFalse_ = CFDataCreate(kCFAllocatorDefault, &value, sizeof(value));
	cfEvent_ = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, &event_, sizeof(event_), kCFAllocatorNull);

	int cursor;
	sscanf(argv[1],"%i",&cursor);
	if(!AshikaseSetEnabled(cursor)){
		printf("jp.ashikase.mousesupport must be installed.\nExiting.");
		return 0;
	}

	if(strcmp(argv[2], "move") == 0){
		if(argc<5){printf("Not enough arguments for command 'move'.\nExiting.");}
		float x, y;
		sscanf(argv[3],"%f",&x);
		sscanf(argv[4],"%f",&y);
		MouseGoto(x, y);
	}else if(strcmp(argv[2], "click") == 0){
		if(argc<6){printf("Not enough arguments for command 'click'.\nExiting.");}
		int btn;
		float x, y;
		sscanf(argv[3],"%i",&btn);
		sscanf(argv[4],"%f",&x);
		sscanf(argv[5],"%f",&y);
		MouseClick(btn, x, y);
	}else if(strcmp(argv[2], "slide") == 0){
		if(argc<7){printf("Not enough arguments for command 'slide'.\nExiting.");}
		float x1, y1;
		float x2, y2;
		sscanf(argv[3],"%f",&x1);
		sscanf(argv[4],"%f",&y1);
		sscanf(argv[5],"%f",&x2);
		sscanf(argv[6],"%f",&y2);
		MouseSlide(x1, y1, x2, y2);
	}

	CFMessagePortInvalidate(ashikase_);
	CFRelease(ashikase_);

   return 0;
}

