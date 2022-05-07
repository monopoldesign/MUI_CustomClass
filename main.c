/******************************************************************************
* MUI_ARexx - ARexx-Test
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
#include <stdlib.h>
#include <string.h>

#include <exec/memory.h>
#include <libraries/easyrexx.h>
#include <libraries/gadtools.h>
#include <libraries/mui.h>
#include <proto/exec.h>

#include <pragma/muimaster_lib.h>
#include <pragma/commodities_lib.h>
#include <pragma/easyrexx_lib.h>

/******************************************************************************
* Macros
*******************************************************************************/
#define HOOKPROTONH(name, ret, obj, param) ret name(register __a2 obj, register __a1 param)
#define MakeHook(hookName, hookFunc) struct Hook hookName = {{NULL, NULL}, (HOOKFUNC)hookFunc, NULL, NULL}

/******************************************************************************
* Prototypes
*******************************************************************************/
HOOKPROTONH(ButtonFunc, ULONG, Object *obj, int *msg);
HOOKPROTONH(SliderFunc, ULONG, Object *obj, int *msg);
HOOKPROTONH(StringFunc, ULONG, Object *obj, char **msg);
HOOKPROTONH(CheckFunc, ULONG, Object *obj, int *msg);

BOOL arexxfuncQUIT(struct ARexxContext *c);
BOOL arexxfuncSETTEXT(struct ARexxContext *c);
BOOL arexxfuncSETSTRING(struct ARexxContext *c);
BOOL arexxfuncSETSLIDER(struct ARexxContext *c);

BOOL myHandleARexx(struct ARexxContext *c);
BOOL checkARexxPorts();
void init(void);
void end(void);
struct ObjApp *CreateApp(void);
void DisposeApp(struct ObjApp *ObjectApp);

/******************************************************************************
* Definitions
*******************************************************************************/
#define MAKE_ID(a, b, c, d) ((ULONG)(a) << 24 | (ULONG)(b) << 16 | (ULONG)(c) << 8 | (ULONG)(d))

struct ObjApp
{
	APTR	App;
	APTR	WI_label_0;
	APTR	TX_Receive;
	APTR	STR_Value;
	APTR	SL_Value2;
	APTR	CH_label_0;
	APTR	BT_Send;
	char *	STR_TX_Receive;
};

struct ARexxRetValues
{
	LONG result, resultlong;
	UBYTE *resultstring, *error;
};

/******************************************************************************
* Global Variables
*******************************************************************************/
struct IntuitionBase *IntuitionBase;
struct Library *MUIMasterBase;
struct Library *EasyRexxBase;

char buffer[40];
struct ObjApp *App = NULL;

MakeHook(hook_button, ButtonFunc);
MakeHook(hook_slider, SliderFunc);
MakeHook(hook_string, StringFunc);
MakeHook(hook_check, CheckFunc);

struct ARexxRetValues arexxReturn =
{
	0, 0,
	0, 0
};

struct ARexxContext *arexxContext;
char portName[20];

enum arcmd{AREXX_QUIT = 1, AREXX_SETTEXT, AREXX_SETSTRING, AREXX_SETSLIDER};

struct ARexxCommandTable commandTable[] =
{
	AREXX_QUIT, "QUIT", "",	(APTR)arexxfuncQUIT,
	AREXX_SETTEXT, "SETTEXT", "ITEM/A", (APTR)arexxfuncSETTEXT,
	AREXX_SETSTRING, "SETSTRING", "ITEM/A", (APTR)arexxfuncSETSTRING,
	AREXX_SETSLIDER, "SETSLIDER", "VALUE/A", (APTR)arexxfuncSETSLIDER,
	TABLE_END,
};

BOOL useHooks = TRUE;

/******************************************************************************
* Hook-Functions
*******************************************************************************/

/*-----------------------------------------------------------------------------
- ButtonFunc()
- Function for Button-Hook
------------------------------------------------------------------------------*/
HOOKPROTONH(ButtonFunc, ULONG, Object *obj, int *msg)
{
	LONG result;
	LONG val;
	char *line;

	get(App->STR_Value, MUIA_String_Contents, &line);
	sprintf(buffer, "SETTEXT %s", line);
	result = SendARexxCommand(buffer, ER_Portname, portName, ER_Context, arexxContext, ER_Asynch, TRUE, ER_String, TRUE, TAG_DONE);

	sprintf(buffer, "SETSTRING %s", line);
	result = SendARexxCommand(buffer, ER_Portname, portName, ER_Context, arexxContext, ER_Asynch, TRUE, ER_String, TRUE, TAG_DONE);

	get(App->SL_Value2, MUIA_Slider_Level, &val);
	sprintf(buffer, "SETSLIDER %ld", val);
	result = SendARexxCommand(buffer, ER_Portname, portName, ER_Context, arexxContext, ER_Asynch, TRUE, ER_String, TRUE, TAG_DONE);

	return 0;
}

/*-----------------------------------------------------------------------------
- SliderFunc()
- Function for Slider-Hook
------------------------------------------------------------------------------*/
HOOKPROTONH(SliderFunc, ULONG, Object *obj, int *msg)
{
	LONG result;

	if (useHooks)
	{
		sprintf(buffer, "SETSLIDER %ld", *msg);
		result = SendARexxCommand(buffer, ER_Portname, portName, ER_Context, arexxContext, ER_Asynch, TRUE, ER_String, TRUE, TAG_DONE);
	}

	return 0;
}

/*-----------------------------------------------------------------------------
- StringFunc()
- Function for String-Hook
------------------------------------------------------------------------------*/
HOOKPROTONH(StringFunc, ULONG, Object *obj, char **msg)
{
	LONG result;

	if (useHooks)
	{
		DoMethod(App->TX_Receive, MUIM_Set, MUIA_Text_Contents, *msg);

		sprintf(buffer, "SETTEXT %s", *msg);
		result = SendARexxCommand(buffer, ER_Portname, portName, ER_Context, arexxContext, ER_Asynch, TRUE, ER_String, TRUE, TAG_DONE);

		sprintf(buffer, "SETSTRING %s", *msg);
		result = SendARexxCommand(buffer, ER_Portname, portName, ER_Context, arexxContext, ER_Asynch, TRUE, ER_String, TRUE, TAG_DONE);
	}

	return 0;
}

/*-----------------------------------------------------------------------------
- CheckFunc()
- Function for String-Hook
------------------------------------------------------------------------------*/
HOOKPROTONH(CheckFunc, ULONG, Object *obj, int *msg)
{
	if (*msg)
	{
		useHooks = TRUE;
		DoMethod(App->BT_Send, MUIM_Set, MUIA_Disabled, TRUE);
	}
	else
	{
		useHooks = FALSE;
		DoMethod(App->BT_Send, MUIM_Set, MUIA_Disabled, FALSE);
	}

	return 0;
}

/*-----------------------------------------------------------------------------
- EmptyFunc()
- Function for String-Hook
------------------------------------------------------------------------------*/
HOOKPROTONH(EmptyFunc, ULONG, Object *obj, char *msg)
{
	return 0;
}

/******************************************************************************
* ARexx-Functions
*******************************************************************************/
/*-----------------------------------------------------------------------------
-
------------------------------------------------------------------------------*/
BOOL arexxfuncQUIT(struct ARexxContext *c)
{
	return FALSE;
}

/*-----------------------------------------------------------------------------
-
------------------------------------------------------------------------------*/
BOOL arexxfuncSETTEXT(struct ARexxContext *c)
{
	DoMethod(App->TX_Receive, MUIM_Set, MUIA_Text_Contents, ARGSTRING(c, 0));
	return TRUE;	
}

/*-----------------------------------------------------------------------------
-
------------------------------------------------------------------------------*/
BOOL arexxfuncSETSTRING(struct ARexxContext *c)
{
	DoMethod(App->STR_Value, MUIM_Set, MUIA_String_Contents, ARGSTRING(c, 0));
	return TRUE;
}

/*-----------------------------------------------------------------------------
-
------------------------------------------------------------------------------*/
BOOL arexxfuncSETSLIDER(struct ARexxContext *c)
{
	DoMethod(App->SL_Value2, MUIM_NoNotifySet, MUIA_Slider_Level, (LONG)atoi(ARGSTRING(c, 0)));
	return TRUE;
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
	ULONG signal, sigrcvd;

	/* Program initialisation : generated by GenCodeC */
	init();

	/* Create Object : generated by GenCodeC */
	if (!(App = CreateApp()))
	{
		printf("Can't Create App\n");
		end();
	}

	arexxContext = AllocARexxContext(ER_CommandTable, commandTable,
								ER_Author,      "M.Volkel",
								ER_Copyright,   "(C)2022 M.Volkel",
								ER_Version,     "MUI_ARexx V0.1",
								ER_Portname,    "MYAREXX",
								TAG_DONE);

	running = checkARexxPorts();

	while (running)
	{
		if (sigrcvd & ER_SIGNAL(arexxContext))
			running = myHandleARexx(arexxContext);
		else
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
		}

		if (running && signal)
			sigrcvd = Wait(signal | ER_SIGNAL(arexxContext));
	}

	FreeARexxContext(arexxContext);
	DisposeApp(App);
	end();
}

/*-----------------------------------------------------------------------------
- ARexx-Handler
------------------------------------------------------------------------------*/
BOOL myHandleARexx(struct ARexxContext *c)
{
	ARexxFunc func;
	UBYTE i = 0;

	LONG result = RC_OK, resultlong = ~0;
	UBYTE *resultstring = NULL, *error = NULL;
	BOOL running = TRUE;

	arexxReturn.result = RC_OK;
	arexxReturn.resultstring = NULL;
	arexxReturn.resultlong = ~0;
	arexxReturn.error = NULL;

	while (GetARexxMsg(c))
	{
		while (arexxContext->table[i].command)
		{
			if (arexxContext->table[i].id == arexxContext->id)
			{
				if (func = (ARexxFunc)(arexxContext->table[i].userdata))
					running = func(arexxContext);

				break;
			}
			else
				i++;
		}
		
		ReplyARexxMsg(arexxContext,
						ER_ReturnCode, arexxReturn.result,
						(arexxReturn.resultstring ? ER_ResultString : TAG_IGNORE), arexxReturn.resultstring,
 						(arexxReturn.resultlong != ~0 ? ER_ResultLong : TAG_IGNORE), arexxReturn.resultlong,
						(arexxReturn.error ? ER_ErrorMessage : TAG_IGNORE), arexxReturn.error,
						TAG_DONE);
	}
	return running;
}

/*-----------------------------------------------------------------------------
- checkARexxPorts()
------------------------------------------------------------------------------*/
BOOL checkARexxPorts()
{
	if (strcmp((UBYTE *)arexxContext->portname, "MYAREXX") == 0)
		sprintf(portName, "MYAREXX.1");
	else if (strcmp((UBYTE *)arexxContext->portname, "MYAREXX.1") == 0)
		sprintf(portName, "MYAREXX");
	else
		return FALSE;

	DoMethod(App->WI_label_0, MUIM_Set, MUIA_Window_Title, (UBYTE *)arexxContext->portname);

	return TRUE;
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

	if (!(EasyRexxBase = OpenLibrary((UBYTE *)EASYREXXNAME, (ULONG)EASYREXXVERSION)))
	{
		printf("Can't Open EasyRexx Library\n");
		CloseLibrary((struct Library *)MUIMasterBase);
		CloseLibrary((struct Library *)IntuitionBase);
		exit(20);
	}
}

/*-----------------------------------------------------------------------------
- end()
------------------------------------------------------------------------------*/
void end(void)
{
	CloseLibrary((struct Library *)EasyRexxBase);
	CloseLibrary((struct Library *)MUIMasterBase);
	CloseLibrary((struct Library *)IntuitionBase);
	exit(0);
}

/*-----------------------------------------------------------------------------
- CreateApp()
------------------------------------------------------------------------------*/
struct ObjApp *CreateApp(void)
{
	struct ObjApp * ObjectApp;

	APTR	GROUP_ROOT_0, GR_grp_1, LA_label_3, obj_aux0, obj_aux1, obj_aux2;
	APTR	obj_aux3, obj_aux4, obj_aux5;

	if (!(ObjectApp = AllocVec(sizeof(struct ObjApp),MEMF_CLEAR)))
		return(NULL);

	ObjectApp->STR_TX_Receive = NULL;

	LA_label_3 = Label("Received:");

	ObjectApp->TX_Receive = TextObject,
		MUIA_Background,	MUII_TextBack,
		MUIA_Frame,			MUIV_Frame_Text,
		MUIA_Text_Contents,	ObjectApp->STR_TX_Receive,
	End;

	GR_grp_1 = GroupObject,
		MUIA_HelpNode,		"GR_grp_1",
		MUIA_Group_Columns,	2,
		Child,				LA_label_3,
		Child,				ObjectApp->TX_Receive,
	End;

	ObjectApp->STR_Value = StringObject,
		MUIA_Frame,			MUIV_Frame_String,
		MUIA_HelpNode,		"STR_Value",
		MUIA_String_MaxLen,	16,
		MUIA_String_Accept, "0123456789",
	End;

	obj_aux1 = Label2("Value:");

	obj_aux0 = GroupObject,
		MUIA_Group_Columns,	2,
		Child,				obj_aux1,
		Child,				ObjectApp->STR_Value,
	End;

	ObjectApp->SL_Value2 = SliderObject,
		MUIA_HelpNode,		"SL_Value2",
		MUIA_Frame,			MUIV_Frame_Slider,
		MUIA_Slider_Min,	0,
		MUIA_Slider_Max,	100,
		MUIA_Slider_Level,	0,
	End;

	obj_aux3 = Label2("Value2:");

	obj_aux2 = GroupObject,
		MUIA_Group_Columns,	2,
		Child,				obj_aux3,
		Child,				ObjectApp->SL_Value2,
	End;

	ObjectApp->CH_label_0 = CheckMark(TRUE);

	obj_aux5 = Label2("Use Hooks:");

	obj_aux4 = GroupObject,
		MUIA_Group_Columns, 2,
		Child, obj_aux5,
		Child, ObjectApp->CH_label_0,
	End;

	ObjectApp->BT_Send = SimpleButton("Send ARexx");

	GROUP_ROOT_0 = GroupObject,
		MUIA_Group_Columns, 2,
		Child,	GR_grp_1,
		Child,	obj_aux0,
		Child,	obj_aux2,
		Child,	obj_aux4,
		Child,	ObjectApp->BT_Send,
	End;

	ObjectApp->WI_label_0 = WindowObject,
		MUIA_Window_Title,		"MUI_ARexx",
		MUIA_Window_ID,			MAKE_ID('0', 'W', 'I', 'N'),
		WindowContents,			GROUP_ROOT_0,
		MUIA_Window_SizeGadget,	TRUE,
	End;

	ObjectApp->App = ApplicationObject,
		MUIA_Application_Author,		"M.Volkel",
		MUIA_Application_Base,			"MUIAREXX",
		MUIA_Application_Title,			"MUI_ARexx",
		MUIA_Application_Version,		"$VER: MUI_ARexx V0.1",
		MUIA_Application_Copyright,		"(C)2022 M.Volkel",
		MUIA_Application_Description,	"ARexx-Test",
		MUIA_Application_UseRexx,		FALSE,
		SubWindow,						ObjectApp->WI_label_0,
	End;


	if (!ObjectApp->App)
	{
		FreeVec(ObjectApp);
		return(NULL);
	}

	// Window-Close-Method
	DoMethod(ObjectApp->WI_label_0, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, ObjectApp->App, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

	// Hook-Methods for Buttons
	DoMethod(ObjectApp->BT_Send, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Self, 2, MUIM_CallHook, &hook_button);

	// Hook-Method for Slider
	DoMethod(ObjectApp->SL_Value2, MUIM_Notify, MUIA_Slider_Level, MUIV_EveryTime, ObjectApp->App, 3, MUIM_CallHook, &hook_slider, MUIV_TriggerValue);

	// Hook-Method for String
	DoMethod(ObjectApp->STR_Value, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, ObjectApp->App, 3, MUIM_CallHook, &hook_string, MUIV_TriggerValue);

	// Hook-Method for Checkbox
	DoMethod(ObjectApp->CH_label_0, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, ObjectApp->App, 3, MUIM_CallHook, &hook_check, MUIV_TriggerValue);

	// Disable Send-Button
	DoMethod(ObjectApp->BT_Send, MUIM_Set, MUIA_Disabled, TRUE);

	// Window open
	set(ObjectApp->WI_label_0, MUIA_Window_Open, TRUE);

	return(ObjectApp);
}

/*-----------------------------------------------------------------------------
- DisposeApp()
------------------------------------------------------------------------------*/
void DisposeApp(struct ObjApp *ObjectApp)
{
	MUI_DisposeObject(ObjectApp->App);
	FreeVec(ObjectApp);
}
