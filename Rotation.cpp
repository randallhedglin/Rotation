// REQ'D DEFINES //

#define ID_APPICON NULL
#define ID_APPCURSOR NULL
#define ID_APPMENU NULL
#define ID_WINDOWTITLE "Rotation"
#define ID_WINDOWCLASS "ROTATION_CLS"

#define APP_HINST hAppInst
#define APP_HWND hAppWnd

#define CALL_INIT_PROC Initialize
#define CALL_MAIN_PROC AppMain
#define CALL_REST_PROC Restore

#define ENCODE_DATA
#define MOUSE_TRACKING
//#define INIT_DIRECT3D
//#define INIT_SOUND

typedef long fixed;
#define FPROT 8
#define FPMPLR 256
#define FPSET(a) (((fixed)a)<<FPROT)
#define FPMUL(a,b) ((((fixed)a)*((fixed)b))>>FPROT)
#define FPDIV(a,b) ((((fixed)a)<<FPROT)/((fixed)b))
#define FPREAD(a) (((fixed)a)>>FPROT)
#define FPDSET(a) ((fixed)(a*FPMPLR))
#define FPDREAD(a) (((double)(a))/FPMPLR)
#define FPSQUARED(a) ((fixed)(FPMUL((fixed)a,(fixed)a)))

// CONSTANTS //

#define SCRWD 640
#define SCRHT 480
#define SCRBP 16
#define NUMBACKBUFFERS 2

// INCLUDES //

#include"directx.h"
#include<math.h>

// MACROS //

// PROTOTYPES //

void EncodeDataFiles(void);

// GLOBALS //

CDirectDraw *DirectDraw;
DIRECTDRAWINITSTRUCT ddis;
#ifdef INIT_DIRECT3D
CDirect3D *Direct3D;
#endif
CDirectInput *DirectInput;
BYTE ActualBPP;
BYTE ActualNBB;
LPSTR gbuffer;
DWORD gpitch;
FONT_DATA Font;
fixed sintab[256],costab[256];
HGLOBAL zbuffermem=NULL;
LPBYTE zbuffer=NULL;

// CLASSES //

// FUNCTIONS //

BOOL Initialize(void)
{
	long count;
	
	while(ShowCursor(FALSE)>=0)
	{}
	srand(GetTickCount());
	//SetDefaultFilePath("c:\\windows\\msremote.sfs\\program files\\3d modeler\\");
	//SetDefaultFilePath("c:\\windows\\desktop\\randy's folder\\3d modeler\\");
	SetDefaultFilePath("c:\\randy's folder\\3d modeler\\");
	if(!(DirectDraw=new CDirectDraw()))
	{
		DisplayErrorMessage("Failed to create CDirectDraw class instance.",
							"Error - Initialize()",
							MB_OK|MB_ICONSTOP,
							NULL);
		return(FALSE);
	}
	ddis.dwScreenWidth=SCRWD;
	ddis.dwScreenHeight=SCRHT;
	ddis.dwScreenBitsPerPixel=SCRBP;
	ddis.dwBackBufferCount=NUMBACKBUFFERS;
	if(!DirectDraw->Initialize(&ddis))
		return(FALSE);
	ActualBPP=(BYTE)ddis.dwScreenBitsPerPixel;
	ActualNBB=(BYTE)ddis.dwBackBufferCount;
#ifdef INIT_DIRECT3D
	if(!(Direct3D=new CDirect3D))
	{
		DisplayErrorMessage("Failed to create CDirect3D class instance.",
							"Error - Initialize()",
							MB_OK|MB_ICONSTOP,
							NULL);
		return(FALSE);
	}
	if(!Direct3D->Initialize(&ddis,DirectDraw))
		return(FALSE);
#endif
	if(!(DirectInput=new CDirectInput()))
	{
		DisplayErrorMessage("Failed to create CDirectInput class instance.",
							"Error - Initialize()",
							MB_OK|MB_ICONSTOP,
							DirectDraw);
		return(FALSE);
	}
	EncodeDataFiles();
	if(!VerifyDataFiles("rotation.rec",DirectDraw))
		return(FALSE);
	OpenDataFile("rotation.dat",DirectDraw,FALSE);
	DirectDraw->LoadFontFromDataFile(&Font);
	CloseDataFile(DirectDraw);
	DirectDraw->SetDirectDrawPaletteEntry(255,255,255,255);
	DirectDraw->SetFontColor(&Font,255);
	if(!DirectInput->Initialize(DirectDraw))
		return(FALSE);
	DirectInput->DetectJoysticks();
	if(JoystickCount)
		DirectInput->SelectJoystick(0);
	for(count=0;count<256;count++)
	{
		double r=(double)count;
		r/=128;
		r*=(double)3.14159;
		double c=cos(r)*FPMPLR;
		double s=sin(r)*FPMPLR;
		costab[count]=(fixed)c;
		sintab[count]=(fixed)s;
	}
	zbuffermem=GlobalAlloc(GHND,SCRWD*SCRHT);
	if(!zbuffermem)
	{
		DisplayErrorMessage("Failed to allocate memory for z-buffer.",
							"Error - Initialize()",
							MB_OK|MB_ICONSTOP,
							DirectDraw);
		return(FALSE);
	}
	zbuffer=(LPBYTE)GlobalLock(zbuffermem);
	if(!zbuffer)
	{
		DisplayErrorMessage("Failed to lock memory for z-buffer.",
							"Error - Initialize()",
							MB_OK|MB_ICONSTOP,
							DirectDraw);
		return(FALSE);
	}
	return(TRUE);
}

void AppMain(void)
{
	static DWORD fpsTickCount=GetTickCount();
	static DWORD frame=0;
	static DWORD fps=0;
	DWORD TickCount;
	DWORD ProgramComplete=FALSE;
	
	TickCount=GetTickCount();

// BEGIN FRAME RENDER //////////////////////////////////////////////////	

	CONTROL_DATA UserInput;
	CONTROL_DATA UserInputNew;

	DirectInput->GetUserInput(&UserInput,&UserInputNew);
	if(UserInputNew.esc)
		ProgramComplete=TRUE;

	DirectDraw->ClearSecondaryDirectDrawSurface();

	sprintf(GlobalStr,"%d",fps);
	DirectDraw->BufferString(&Font,GlobalStr,0,0,NULL,FALSE);

// END FRAME RENDER ////////////////////////////////////////////////////

	while(GetTickCount()<TickCount+33)
	{}
	DirectDraw->PerformPageFlip();
	frame++;
	if(GetTickCount()>=(fpsTickCount+1000))
	{
		fpsTickCount=GetTickCount();
		frame=0;
	}
		fps=frame;
	if(ProgramComplete)
	{
		PostMessage(hAppWnd,WM_QUIT,NULL,NULL);
		return;
	}
}

void Restore(void)
{
	if(zbuffer)
		GlobalUnlock(zbuffermem);
	if(zbuffermem)
		if(GlobalFree(zbuffermem))
			DisplayErrorMessage("Failed to free memory.\nSystem may become unstable.",
								"Error - Restore()",
								MB_OK|MB_ICONINFORMATION,
								DirectDraw);
	DirectDraw->ReleaseFont(&Font);
	DirectInput->Release();
	delete DirectInput;
	DirectDraw->ClearPrimaryDirectDrawSurface();
#ifdef INIT_DIRECT3D
	Direct3D->Release();
	delete Direct3D;
#endif
	DirectDraw->Release();
	delete DirectDraw;
	while(ShowCursor(TRUE)<0)
	{}
}

void EncodeDataFiles(void)
{
#ifdef ENCODE_DATA
	OpenRecorder("rotation.rec");
		OpenEncoder("rotation.dat");
			EncodeFont("cronos.ftd",DirectDraw);
		CloseEncoder();
	CloseRecorder();
#endif
}

// END //

