/******************************************************************************
* MUI_CustomClass - Simple CustomClass
* (C)2022 M.Volkel (mario.volkel@outlook.com)
*******************************************************************************/

// Comment templates

/******************************************************************************
*
*******************************************************************************/

/*-----------------------------------------------------------------------------
-
------------------------------------------------------------------------------*/

/******************************************************************************
* Header-Files
*******************************************************************************/
#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include <math.h>

#include <exec/memory.h>
#include <libraries/mui.h>
#include <proto/exec.h>

#include <pragma/graphics_lib.h>
#include <pragma/muimaster_lib.h>

/******************************************************************************
* Macros
*******************************************************************************/
#define HOOKPROTONH(name, ret, obj, param) ret name(register __a2 obj, register __a1 param)
#define MakeHook(hookName, hookFunc) struct Hook hookName = {{NULL, NULL}, (HOOKFUNC)hookFunc, NULL, NULL}

/******************************************************************************
* Prototypes
*******************************************************************************/
LONG MouseArrowDispatcher (register __a0 Class *cl, register __a2 Object *obj, register __a1 Msg msg);

void init(void);
void end(void);
struct ObjApp * CreateApp(void);
void DisposeApp(struct ObjApp * ObjectApp);

/******************************************************************************
* Definitions
*******************************************************************************/
#define MAKE_ID(a, b, c, d) ((ULONG)(a) << 24 | (ULONG)(b) << 16 | (ULONG)(c) << 8 | (ULONG)(d))

struct ObjApp
{
	APTR	App;
	APTR	WI_label_0;
	APTR	BT_label_0;
};

/******************************************************************************
* Global Variables
*******************************************************************************/
struct IntuitionBase *IntuitionBase;
struct Library *MUIMasterBase;

char buffer[40];
struct ObjApp *App = NULL;

struct MUI_CustomClass *MouseArrowClass;

/******************************************************************************
* MUI-Custom-Class
*******************************************************************************/
/*-----------------------------------------------------------------------------
- Definitions
------------------------------------------------------------------------------*/
#define SIZE 160
#define RADIUS (SIZE/2-2)

struct MouseArrow
{
	short DeltaX;
	short DeltaY;
	struct MUI_EventHandlerNode EHNode;
};

/*-----------------------------------------------------------------------------
- mAskMinMax
------------------------------------------------------------------------------*/
LONG mAskMinMax (Class *cl, Object *obj, struct MUIP_AskMinMax *msg)
{
	DoSuperMethodA(cl, obj, (Msg)msg);

	msg->MinMaxInfo->MinWidth += SIZE;
	msg->MinMaxInfo->DefWidth += SIZE;
	msg->MinMaxInfo->MaxWidth += SIZE;
	msg->MinMaxInfo->MinHeight += SIZE;
	msg->MinMaxInfo->DefHeight += SIZE;
	msg->MinMaxInfo->MaxHeight += SIZE;

	return 0;
}

/*-----------------------------------------------------------------------------
- mDraw
------------------------------------------------------------------------------*/
LONG mDraw (Class *cl, Object *obj, struct MUIP_Draw *msg)
{
	short big_radius;
	long dx, dy;
	struct RastPort *rp = _rp(obj);
	struct MouseArrow *data = INST_DATA(cl,obj);

	DoSuperMethodA(cl, obj, (Msg)msg);
	big_radius = (short)sqrt((double)(data->DeltaX * data->DeltaX + data->DeltaY * data->DeltaY));
	if (big_radius != 0)
	{
		dx = (data->DeltaX * RADIUS) / big_radius;
		dy = (data->DeltaY * RADIUS) / big_radius;
	}
	else
	{
		dx = 0;
		dy = 0;
	}
	SetAPen(rp, 1);
	Move(rp, _mleft(obj) + SIZE/2 + dx, _mtop(obj) + SIZE/2 + dy);
	Draw(rp, _mleft(obj) + SIZE/2 - dx, _mtop(obj) + SIZE/2 - dy);
	SetAPen(rp, 2);
	WritePixel(rp, _mleft(obj) + SIZE/2 + dx, _mtop(obj) + SIZE/2 + dy);
	WritePixel(rp, _mleft(obj) + SIZE/2+1 + dx, _mtop(obj) + SIZE/2 + dy);
	WritePixel(rp, _mleft(obj) + SIZE/2 + dx, _mtop(obj) + SIZE/2+1 + dy);
	WritePixel(rp, _mleft(obj) + SIZE/2-1 + dx, _mtop(obj) + SIZE/2 + dy);
	WritePixel(rp, _mleft(obj) + SIZE/2 + dx, _mtop(obj) + SIZE/2-1 + dy);

	return 0;
}

/*-----------------------------------------------------------------------------
- mSetup
------------------------------------------------------------------------------*/
LONG mSetup (Class *cl, Object *obj, Msg msg)
{
	struct MouseArrow *data = INST_DATA(cl,obj);

	if (DoSuperMethodA(cl, obj, msg))
	{
		data->EHNode.ehn_Priority = 0;
		data->EHNode.ehn_Flags = 0;
		data->EHNode.ehn_Object = obj;
		data->EHNode.ehn_Class = cl;
		data->EHNode.ehn_Events = IDCMP_MOUSEMOVE;
		DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->EHNode);
		return TRUE;
	}
	return FALSE;
}

/*-----------------------------------------------------------------------------
- mCleanup
------------------------------------------------------------------------------*/
LONG mCleanup (Class *cl, Object *obj, Msg msg)
{
	struct MouseArrow *data = INST_DATA(cl, obj);

	DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->EHNode);
	return 0;
}

/*-----------------------------------------------------------------------------
- mHandleEvent
------------------------------------------------------------------------------*/
LONG mHandleEvent (Class *cl, Object *obj, struct MUIP_HandleEvent *msg)
{
	struct MouseArrow *data = INST_DATA(cl, obj);

	if (msg->imsg)
	{
		if (msg->imsg->Class == IDCMP_MOUSEMOVE)
		{
			data->DeltaX = msg->imsg->MouseX - _mleft(obj) - SIZE/2;
			data->DeltaY = msg->imsg->MouseY - _mtop(obj) - SIZE/2;
			MUI_Redraw(obj, MADF_DRAWOBJECT);
		}
	}
	return 0;
}

/*-----------------------------------------------------------------------------
- Dispatcher
------------------------------------------------------------------------------*/
LONG MouseArrowDispatcher(register __a0 Class *cl, register __a2 Object *obj, register __a1 Msg msg)
{
	switch (msg->MethodID)
	{
		case MUIM_Setup:
			return (mSetup(cl, obj, msg));

		case MUIM_Cleanup:
			return (mCleanup(cl, obj, msg));

		case MUIM_AskMinMax:
			return (mAskMinMax(cl, obj, (struct MUIP_AskMinMax*)msg));

		case MUIM_Draw:
			return (mDraw(cl, obj, (struct MUIP_Draw*)msg));

		case MUIM_HandleEvent:
			return (mHandleEvent(cl, obj, (struct MUIP_HandleEvent*)msg));

		default:
			return (DoSuperMethodA(cl, obj, msg));
	}
}

/******************************************************************************
* Main-Program
*******************************************************************************/

/*-----------------------------------------------------------------------------
- main()
------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	BOOL running = TRUE;
	ULONG signal;

	init();

	if (MouseArrowClass = MUI_CreateCustomClass(NULL, MUIC_Area, NULL, sizeof (struct MouseArrow), MouseArrowDispatcher))
	{
		if (!(App = CreateApp()))
		{
			printf("Can't Create App\n");
			end();
		}

		while (running)
		{
			switch (DoMethod(App->App, MUIM_Application_NewInput, &signal))
			{
				// Window close
				case MUIV_Application_ReturnID_Quit:
					if ((MUI_RequestA(App->App, 0, 0, "Quit?", "_Yes|_No", "\33cAre you sure?", 0)) == 1)
						running = FALSE;
				break;

				default:
					break;
			}

			if (running && signal)
				Wait(signal);
		}
	}

	DisposeApp(App);

	MUI_DeleteCustomClass(MouseArrowClass);
 	end();
}

/*-----------------------------------------------------------------------------
- init()
------------------------------------------------------------------------------*/
void init(void)
{
	if (!(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 37)))
	{
		printf("Can't Open Intuition Library\n");
		exit(20);
	}

	if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME, MUIMASTER_VMIN)))
	{
		printf("Can't Open MUIMaster Library\n");
		CloseLibrary((struct Library *)IntuitionBase);
		exit(20);
	}
}

/*-----------------------------------------------------------------------------
- end()
------------------------------------------------------------------------------*/
void end(void)
{
	CloseLibrary((struct Library *)MUIMasterBase);
	CloseLibrary((struct Library *)IntuitionBase);
	exit(0);
}

/*-----------------------------------------------------------------------------
- CreateApp()
------------------------------------------------------------------------------*/
struct ObjApp * CreateApp(void)
{
	struct ObjApp * ObjectApp;

	APTR GROUP_ROOT_0;
	APTR rect, mcc1;

	if (!(ObjectApp = AllocVec(sizeof(struct ObjApp), MEMF_CLEAR)))
		return(NULL);

	ObjectApp->BT_label_0 = SimpleButton("Exit");

	rect = RectangleObject,
	End;

	mcc1 = NewObject(MouseArrowClass->mcc_Class, NULL, MUIA_Frame, MUIV_Frame_Text, MUIA_Background, MUII_TextBack, TAG_END);

	GROUP_ROOT_0 = GroupObject,
		MUIA_Group_Columns,		1,
		MUIA_Window_SizeGadget, TRUE,
		Child,					rect,
		Child,					mcc1,
		Child,					ObjectApp->BT_label_0,
	End;

	ObjectApp->WI_label_0 = WindowObject,
		MUIA_Window_Title,	"MUI_CClass",
		MUIA_Window_ID,		MAKE_ID('0', 'W', 'I', 'N'),
		WindowContents,		GROUP_ROOT_0,
	End;

	ObjectApp->App = ApplicationObject,
		MUIA_Application_Author,		"M.Volkel",
		MUIA_Application_Base,			"MUICCLASS",
		MUIA_Application_Title,			"MUI_CustomClass",
		MUIA_Application_Version,		"$VER: MUI_CustomClass V0.1",
		MUIA_Application_Copyright,		"(C)2022 M.Volkel",
		MUIA_Application_Description,	"MUI-CustomClass",
		SubWindow,						ObjectApp->WI_label_0,
	End;


	if (!ObjectApp->App)
	{
		FreeVec(ObjectApp);
		return(NULL);
	}

	// Window-Close-Method
	DoMethod(ObjectApp->WI_label_0, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, ObjectApp->App, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

	// Exit-Button
	DoMethod(ObjectApp->BT_label_0, MUIM_Notify, MUIA_Pressed, MUIV_EveryTime, ObjectApp->App, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

	// Open Window
	set(ObjectApp->WI_label_0, MUIA_Window_Open, TRUE);

	return(ObjectApp);
}

/*-----------------------------------------------------------------------------
- DisposeApp()
------------------------------------------------------------------------------*/
void DisposeApp(struct ObjApp * ObjectApp)
{
	MUI_DisposeObject(ObjectApp->App);
	FreeVec(ObjectApp);
}
