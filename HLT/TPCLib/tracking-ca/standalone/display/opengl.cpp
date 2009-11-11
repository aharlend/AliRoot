/*
 *		This Code Was Created By Jeff Molofee 2000
 *		A HUGE Thanks To Fredric Echols For Cleaning Up
 *		And Optimizing This Code, Making It More Flexible!
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit My Site At nehe.gamedev.net
 */

#include "AliHLTTPCCADef.h"

#ifdef R__WIN32
#include <windows.h>		// Header File For Windows
#include <winbase.h>
#include <windowsx.h>

HDC			hDC=NULL;		// Private GDI Device Context
HGLRC		hRC=NULL;		// Permanent Rendering Context
HWND		hWnd=NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

bool	active=TRUE;		// Window Active Flag Set To TRUE By Default
bool	fullscreen=TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default

HANDLE semLockDisplay = NULL;
#else
#include <GL/glx.h> // This includes the necessary X headers
#include <pthread.h>

Display *g_pDisplay = NULL;
Window   g_window;
bool     g_bDoubleBuffered = GL_TRUE;
GLuint   g_textureID = 0;

float g_fSpinX           = 0.0f;
float g_fSpinY           = 0.0f;
int   g_nLastMousePositX = 0;
int   g_nLastMousePositY = 0;
bool  g_bMousing         = false;

pthread_mutex_t semLockDisplay = PTHREAD_MUTEX_INITIALIZER;
#endif
#include <GL/gl.h>			// Header File For The OpenGL32 Library
#include <GL/glu.h>			// Header File For The GLu32 Library

#include "AliHLTTPCCAStandaloneFramework.h"
#include "AliHLTTPCCATrackerFramework.h"
#include "AliHLTTPCCATracker.h"
#include "AliHLTTPCCASliceData.h"
#include "AliHLTTPCCATrack.h"
#include "AliHLTTPCCAMergerOutput.h"
#include "include.h"

#define fgkNSlices 36

bool	keys[256];			// Array Used For The Keyboard Routine

float rotateX = 0, rotateY = 0;
float mouseDnX, mouseDnY;
float mouseMvX, mouseMvY;
bool mouseDn = false;
bool mouseDnR = false;
int mouseWheel = 0;
volatile int buttonPressed = 0;

GLfloat currentMatrice[16];

int drawClusters = true;
int drawLinks = false;
int drawSeeds = false;
int drawInitLinks = false;
int drawTracklets = false;
int drawTracks = false;
int drawFinal = false;

float4* globalPos = NULL;
int maxClusters = 0;
int currentClusters = 0;

volatile int displayEventNr = 0;
int currentEventNr = -1;

int glDLrecent = 0;

volatile int resetScene = 0;

inline void SetColorClusters() {glColor3f(0, 0.7, 1.0);}
inline void SetColorInitLinks() {glColor3f(0.5, 0.5, 0.5);}
inline void SetColorLinks() {glColor3f(1, 0.3, 0.3);}
inline void SetColorSeeds() {glColor3f(1.0, 0.4, 0);}
inline void SetColorTracklets() {glColor3f(0, 0.8, 0.2);}
inline void SetColorTracks() {glColor3f(0.6, 1, 0);}
inline void SetColorFinal() {glColor3f(1, 1, 1);}

void ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	static int init = 1;
	GLfloat tmp[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, tmp);

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix

	if (init)
	{
		glTranslatef(0, 0, -16);
		init = 0;
	}
	else
	{
		glLoadMatrixf(tmp);
	}

	glGetFloatv(GL_MODELVIEW_MATRIX, currentMatrice);
}

int InitGL()										// All Setup For OpenGL Goes Here
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	return(true);										// Initialization Went OK
}

inline void drawPointLinestrip(int cid, int id)
{
	glVertex3f(globalPos[cid].x, globalPos[cid].y, globalPos[cid].z);
	globalPos[cid].w = id;
}

void DrawClusters(AliHLTTPCCAStandaloneFramework& hlt, int id)
{
	glBegin(GL_POINTS);
	for (int i = 0;i < currentClusters;i++)
	{
		if (globalPos[i].w == id)
		{
			glVertex3f(globalPos[i].x, globalPos[i].y, globalPos[i].z);
		}
	}
	glEnd();
}

void DrawLinks(AliHLTTPCCAStandaloneFramework& hlt, int id)
{
	glBegin(GL_LINES);
	for (int iSlice = 0;iSlice < fgkNSlices;iSlice++)
	{
		AliHLTTPCCATracker& tracker = hlt.fTracker.fCPUTrackers[iSlice];
		for (int i = 0;i < tracker.Param().NRows() - 2;i++)
		{
			const AliHLTTPCCARow &row = tracker.Data().Row(i);
			const AliHLTTPCCARow &rowUp = tracker.Data().Row(i + 2);
			for (int j = 0;j < row.NHits();j++)
			{
				if (tracker.Data().HitLinkUpData(row, j) != -1)
				{
					const int cid1 = tracker.ClusterData()->Id(tracker.Data().ClusterDataIndex(row, j));
					const int cid2 = tracker.ClusterData()->Id(tracker.Data().ClusterDataIndex(rowUp, tracker.Data().HitLinkUpData(row, j)));
					drawPointLinestrip(cid1, id);
					drawPointLinestrip(cid2, id);
				}
			}
		}
	}
	glEnd();
}

void DrawSeeds(AliHLTTPCCAStandaloneFramework& hlt)
{
	for (int iSlice = 0;iSlice < fgkNSlices;iSlice++)
	{
		AliHLTTPCCATracker& tracker = hlt.fTracker.fCPUTrackers[iSlice];
		for (int i = 0;i < *tracker.NTracklets();i++)
		{
			const AliHLTTPCCAHitId &hit = tracker.TrackletStartHit(i);
			glBegin(GL_LINE_STRIP);
			int ir = hit.RowIndex();
			int ih = hit.HitIndex();
			do
			{
				const AliHLTTPCCARow &row = tracker.Data().Row(ir);
				const int cid = tracker.ClusterData()->Id(tracker.Data().ClusterDataIndex(row, ih));
				drawPointLinestrip(cid, 3);
				ir += 2;
				ih = tracker.Data().HitLinkUpData(row, ih);
			} while (ih != -1);
			glEnd();
		}
	}
}

void DrawTracklets(AliHLTTPCCAStandaloneFramework& hlt)
{
	for (int iSlice = 0;iSlice < fgkNSlices;iSlice++)
	{
		AliHLTTPCCATracker& tracker = hlt.fTracker.fCPUTrackers[iSlice];
		for (int i = 0;i < *tracker.NTracklets();i++)
		{
			const AliHLTTPCCATracklet &tracklet = tracker.Tracklet(i);
			if (tracklet.NHits() == 0) continue;
			glBegin(GL_LINE_STRIP);
			for (int j = tracklet.FirstRow();j <= tracklet.LastRow();j++)
			{
				if (tracklet.RowHit(j) != -1)
				{
					const AliHLTTPCCARow &row = tracker.Data().Row(j);
					const int cid = tracker.ClusterData()->Id(tracker.Data().ClusterDataIndex(row, tracklet.RowHit(j)));
					drawPointLinestrip(cid, 4);
				}
			}
			glEnd();
		}
	}
}

void DrawTracks(AliHLTTPCCAStandaloneFramework& hlt)
{
	for (int iSlice = 0;iSlice < fgkNSlices;iSlice++)
	{
		AliHLTTPCCATracker& tracker = hlt.fTracker.fCPUTrackers[iSlice];	
		for (int i = 0;i < *tracker.NTracks();i++)
		{
			AliHLTTPCCATrack &track = tracker.Tracks()[i];
			glBegin(GL_LINE_STRIP);
			for (int j = 0;j < track.NHits();j++)
			{
				const AliHLTTPCCAHitId &hit = tracker.TrackHits()[track.FirstHitID() + j];
				const AliHLTTPCCARow &row = tracker.Data().Row(hit.RowIndex());
				const int cid = tracker.ClusterData()->Id(tracker.Data().ClusterDataIndex(row, hit.HitIndex()));
				drawPointLinestrip(cid, 5);
			}
			glEnd();
		}
	}
}

void DrawFinal(AliHLTTPCCAStandaloneFramework& hlt)
{
	const AliHLTTPCCAMerger &merger = hlt.Merger();
	const AliHLTTPCCAMergerOutput &mergerOut = *merger.Output();
	for (int i = 0;i < mergerOut.NTracks();i++)
	{
		const AliHLTTPCCAMergedTrack &track = mergerOut.Track(i);
		glBegin(GL_LINE_STRIP);
		for (int j = 0;j < track.NClusters();j++)
		{
			int cid = mergerOut.ClusterId(track.FirstClusterRef() + j);
			drawPointLinestrip(cid, 6);
		}
		glEnd();
	}
}

int DrawGLScene()									// Here's Where We Do All The Drawing
{
	static float fpsscale = 1;

	static int framesDone = 0;
	static unsigned long long int startTime, displayFpsTime, timeFreq;

	static float pointSize = 2.0;

	static GLuint glDLlines[6];
	static GLuint glDLpoints[7];
	static int glDLcreated = 0;

	AliHLTTPCCAStandaloneFramework &hlt = AliHLTTPCCAStandaloneFramework::Instance();

	//Initialize
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();									// Reset The Current Modelview Matrix

	int mouseWheelTmp = mouseWheel;
	mouseWheel = 0;

	//Calculate rotation / translation scaling factors
	float scalefactor = keys[16] ? 0.2 : 1.0;

	float sqrdist = sqrt(sqrt(currentMatrice[12] * currentMatrice[12] + currentMatrice[13] * currentMatrice[13] + currentMatrice[14] * currentMatrice[14]) / 50) * 0.8;
	if (sqrdist < 0.2) sqrdist = 0.2;
	if (sqrdist > 5) sqrdist = 5;
	scalefactor *= sqrdist;

	//Perform new rotation / translation
	float moveZ = scalefactor * ((float) mouseWheelTmp / 150 + (float) (keys['W'] - keys['S']) * 0.2 * fpsscale);
	float moveX = scalefactor * ((float) (keys['A'] - keys['D']) * 0.2 * fpsscale);

	glTranslatef(moveX, 0, moveZ);

	if (mouseDnR && mouseDn)
	{
		glTranslatef(0, 0, -scalefactor * ((float) mouseMvY - (float)mouseDnY) / 4);
		glRotatef(scalefactor * ((float)mouseMvX - (float)mouseDnX), 0, 0, 1);
	}
	else if (mouseDnR)
	{
		glTranslatef(-scalefactor * 0.5 * ((float) mouseDnX - (float)mouseMvX) / 4, -scalefactor * 0.5 * ((float) mouseMvY - (float)mouseDnY) / 4, 0);
	}
	else if (mouseDn)
	{
		glRotatef(scalefactor * 0.5 * ((float)mouseMvX - (float)mouseDnX), 0, 1, 0);
		glRotatef(scalefactor * 0.5 * ((float)mouseMvY - (float)mouseDnY), 1, 0, 0);
	}
	if (keys['E'] ^ keys['F'])
	{
		glRotatef(scalefactor * fpsscale * 2, 0, 0, keys['E'] - keys['F']);
	}
	mouseDnX = mouseMvX;
	mouseDnY = mouseMvY;

	//Apply standard translation / rotation
	glMultMatrixf(currentMatrice);

	//Grafichs Options
	pointSize += (float) (keys[107] - keys[109] + keys[187] - keys[189]) * fpsscale * 0.05;
	if (pointSize <= 0.01) pointSize = 0.01;

	//Reset position
	if (resetScene)
	{
		glLoadIdentity();
		glTranslatef(0, 0, -16);

		AliHLTTPCCAStandaloneFramework::StandaloneQueryTime(&startTime);
		displayFpsTime = startTime;
		framesDone = 0;

		pointSize = 2.0;

		resetScene = 0;
	}

	//Store position
	glGetFloatv(GL_MODELVIEW_MATRIX, currentMatrice);

	//Make sure event gets not overwritten during display
#ifdef R__WIN32
	WaitForSingleObject(semLockDisplay, INFINITE);
#else
	pthread_mutex_lock( &semLockDisplay );
#endif

	//Open GL Default Values
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPointSize(pointSize);
	glLineWidth(1.5);

	//Extract global cluster information
	if (displayEventNr != currentEventNr)
	{
		currentClusters = 0;
		for (int iSlice = 0;iSlice < fgkNSlices;iSlice++)
		{
			currentClusters += hlt.fTracker.fCPUTrackers[iSlice].NHitsTotal();
		}

		if (maxClusters < currentClusters)
		{
			if (globalPos) delete[] globalPos;
			maxClusters = currentClusters;
			globalPos = new float4[maxClusters];
		}

		for (int iSlice = 0;iSlice < fgkNSlices;iSlice++)
		{
			const AliHLTTPCCAClusterData &cdata = hlt.fClusterData[iSlice];
			for (int i = 0;i < cdata.NumberOfClusters();i++)
			{
				const int cid = cdata.Id(i);
				if (cid >= maxClusters)
				{
					printf("Cluster Buffer Size exceeded\n");
					exit(1);
				}
				float4 *ptr = &globalPos[cid];
				hlt.fTracker.fCPUTrackers[iSlice].Param().Slice2Global(cdata.X(i), cdata.Y(i), cdata.Z(i), &ptr->x, &ptr->y, &ptr->z);
				ptr->x /= 50;
				ptr->y /= 50;
				ptr->z /= 50;
				ptr->w = 1;
			}
		}

		currentEventNr = displayEventNr;

		AliHLTTPCCAStandaloneFramework::StandaloneQueryFreq(&timeFreq);
		AliHLTTPCCAStandaloneFramework::StandaloneQueryTime(&startTime);
		displayFpsTime = startTime;
		framesDone = 0;
		glDLrecent = 0;
	}

	//Prepare Event
	if (!glDLrecent)
	{
		if (glDLcreated)
		{
			for (int i = 0;i < 6;i++) glDeleteLists(glDLlines[i], 1);
			for (int i = 0;i < 7;i++) glDeleteLists(glDLpoints[i], 1);
		}
		else
		{
			for (int i = 0;i < 6;i++) glDLlines[i] = glGenLists(1);
			for (int i = 0;i < 7;i++) glDLpoints[i] = glGenLists(1);
		}

		for (int i = 0;i < currentClusters;i++) globalPos[i].w = 0;

		glNewList(glDLlines[0], GL_COMPILE);
		if (drawInitLinks)
		{
			char* tmpMem[fgkNSlices];
			for (int i = 0;i < fgkNSlices;i++)
			{
				AliHLTTPCCATracker& tracker = hlt.fTracker.fCPUTrackers[i];
				tmpMem[i] = tracker.Data().Memory();
				tracker.SetGPUSliceDataMemory(tracker.fLinkTmpMemory, tracker.Data().Rows());
				tracker.SetPointersSliceData(tracker.ClusterData());
			}
			DrawLinks(hlt, 1);
			for (int i = 0;i < fgkNSlices;i++)
			{
				AliHLTTPCCATracker& tracker = hlt.fTracker.fCPUTrackers[i];
				tracker.SetGPUSliceDataMemory(tmpMem[i], tracker.Data().Rows());
				tracker.SetPointersSliceData(tracker.ClusterData());
			}
		}
		glEndList();
		
		glNewList(glDLlines[1], GL_COMPILE);
		DrawLinks(hlt, 2);
		glEndList();
		
		glNewList(glDLlines[2], GL_COMPILE);
		DrawSeeds(hlt);
		glEndList();
		
		glNewList(glDLlines[3], GL_COMPILE);
		DrawTracklets(hlt);
		glEndList();
		
		glNewList(glDLlines[4], GL_COMPILE);
		DrawTracks(hlt);
		glEndList();
		
		glNewList(glDLlines[5], GL_COMPILE);
		DrawFinal(hlt);
		glEndList();

		for (int i = 0;i < 7;i++)
		{
			glNewList(glDLpoints[i], GL_COMPILE);
			DrawClusters(hlt, i);
			glEndList();
		}

		int errCode;
		if ((errCode = glGetError()) != GL_NO_ERROR)
		{
			printf("Error creating OpenGL display list: %s\n", gluErrorString(errCode));
			resetScene = 1;
		}
		else
		{
			glDLrecent = 1;
		}
	}

	++framesDone;
	unsigned long long int tmpTime;
	AliHLTTPCCAStandaloneFramework::StandaloneQueryTime(&tmpTime);
	if (tmpTime - displayFpsTime > timeFreq)
	{
		displayFpsTime = tmpTime;
		float fps = (double) framesDone * (double) timeFreq / (double) (tmpTime - startTime);
		printf("FPS: %f (%d frames, 1:Clusters %d, 2:Prelinks %d, 3:Links %d, 4:Seeds %d, 5:Tracklets %d, 6:Tracks %d, 7:Merger %d)\n", fps, framesDone, drawClusters, drawInitLinks, drawLinks, drawSeeds, drawTracklets, drawTracks, drawFinal);
		fpsscale = 60 / fps;
	}

	//Draw Event
	if (glDLrecent)
	{
		
		if (drawClusters)
		{
			SetColorClusters();
			glCallList(glDLpoints[0]);

			if (drawInitLinks) SetColorInitLinks();
			glCallList(glDLpoints[1]);

			if (drawLinks) SetColorLinks();
			glCallList(glDLpoints[2]);

			if (drawSeeds) SetColorSeeds();
			glCallList(glDLpoints[3]);

			glColor3f(0, 0.7, 1.0);
			if (drawTracklets) SetColorTracklets();
			glCallList(glDLpoints[4]);

			if (drawTracks) SetColorTracks();
			glCallList(glDLpoints[5]);

			if (drawFinal) SetColorFinal();
			glCallList(glDLpoints[6]);
		}

		if (drawInitLinks) {SetColorInitLinks();glCallList(glDLlines[0]);}
		if (drawLinks) {SetColorLinks();glCallList(glDLlines[1]);}
		if (drawSeeds) {SetColorSeeds();glCallList(glDLlines[2]);}
		if (drawTracklets) {SetColorTracklets();glCallList(glDLlines[3]);}
		if (drawTracks) {SetColorTracks();glCallList(glDLlines[4]);}
		if (drawFinal) {SetColorFinal();glCallList(glDLlines[5]);}
	}

	//Free event
#ifdef R__WIN32
	ReleaseSemaphore(semLockDisplay, 1, NULL);
#else
	pthread_mutex_unlock( &semLockDisplay );
#endif

	return true;										// Keep Going
}

#ifdef R__WIN32

void KillGLWindow()										// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
 
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	
	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
		case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			if (!HIWORD(wParam))					// Check Minimization State
			{
				active=TRUE;						// Program Is Active
			}
			else
			{
				active=FALSE;						// Program Is No Longer Active
			}

			return 0;								// Return To The Message Loop
		}

		case WM_SYSCOMMAND:							// Intercept System Commands
		{
			switch (wParam)							// Check System Calls
			{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;									// Exit
		}

		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}

		case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			keys[wParam] = TRUE;					// If So, Mark It As TRUE
			return 0;								// Jump Back
		}

		case WM_KEYUP:								// Has A Key Been Released?
		{
			keys[wParam] = FALSE;					// If So, Mark It As FALSE

			if (wParam == 13 || wParam == 'N') buttonPressed = 1;
			if (wParam == 'Q') exit(0);
			if (wParam == 'R') resetScene = 1;

			if (wParam == '1') drawClusters ^= 1;
			else if (wParam == '2') drawInitLinks ^= 1; 
			else if (wParam == '3') drawLinks ^= 1;
			else if (wParam == '4') drawSeeds ^= 1;
			else if (wParam == '5') drawTracklets ^= 1;
			else if (wParam == '6') drawTracks ^= 1;
			else if (wParam == '7') drawFinal ^= 1;

			//printf("Key: %d\n", wParam);
			return 0;								// Jump Back
		}

		case WM_SIZE:								// Resize The OpenGL Window
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}

		case WM_LBUTTONDOWN:
		{
			mouseDnX = GET_X_LPARAM(lParam); 
			mouseDnY = GET_Y_LPARAM(lParam);
			mouseDn = true;
			return 0;
		}

		case WM_LBUTTONUP:
		{
			mouseDn = false;
			return 0;
		}

		case WM_RBUTTONDOWN:
		{
			mouseDnX = GET_X_LPARAM(lParam); 
			mouseDnY = GET_Y_LPARAM(lParam);
			mouseDnR = true;
			return 0;
		}

		case WM_RBUTTONUP:
		{
			mouseDnR = false;
			return 0;
		}

		case WM_MOUSEMOVE:
		{
			mouseMvX = GET_X_LPARAM(lParam); 
			mouseMvY = GET_Y_LPARAM(lParam);
			return 0;
		}

		case WM_MOUSEWHEEL:
		{
			mouseWheel += GET_WHEEL_DELTA_WPARAM(wParam);
			return 0;
		}
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

DWORD WINAPI OpenGLMain(LPVOID tmp)
{
	MSG		msg;									// Windows Message Structure
	BOOL	done=FALSE;								// Bool Variable To Exit Loop

	// Ask The User Which Screen Mode They Prefer
	fullscreen=FALSE;								// Windowed Mode

	// Create Our OpenGL Window
	if (!CreateGLWindow("Alice HLT TPC CA Event Display",1024,768,32,fullscreen))
	{
		return 0;									// Quit If Window Was Not Created
	}

	while(!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done=TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if (active)								// Program Active?
			{
				if (keys[VK_ESCAPE])				// Was ESC Pressed?
				{
					done=TRUE;						// ESC Signalled A Quit
				}
				else								// Not Time To Quit, Update Screen
				{
					DrawGLScene();					// Draw The Scene
					SwapBuffers(hDC);				// Swap Buffers (Double Buffering)
				}
			}

			/*if (keys[VK_F1])						// Is F1 Being Pressed?
			{
				keys[VK_F1]=FALSE;					// If So Make Key FALSE
				KillGLWindow();						// Kill Our Current Window
				fullscreen=!fullscreen;				// Toggle Fullscreen / Windowed Mode
				// Recreate Our OpenGL Window
				if (!CreateGLWindow("NeHe's OpenGL Framework",640,480,16,fullscreen))
				{
					return 0;						// Quit If Window Was Not Created
				}
			}*/
		}
	}

	// Shutdown
	KillGLWindow();									// Kill The Window
	return (msg.wParam);							// Exit The Program
}

#else

void render(void);
void init(void);

void* OpenGLMain( void* ptr )
{
    XSetWindowAttributes windowAttributes;
    XVisualInfo *visualInfo = NULL;
    XEvent event;
    Colormap colorMap;
    GLXContext glxContext;
    int errorBase;
	int eventBase;

    // Open a connection to the X server
    g_pDisplay = XOpenDisplay( NULL );

    if( g_pDisplay == NULL )
    {
        fprintf(stderr, "glxsimple: %s\n", "could not open display");
        exit(1);
    }

    // Make sure OpenGL's GLX extension supported
    if( !glXQueryExtension( g_pDisplay, &errorBase, &eventBase ) )
    {
        fprintf(stderr, "glxsimple: %s\n", "X server has no OpenGL GLX extension");
        exit(1);
    }

    // Find an appropriate visual

    int doubleBufferVisual[]  =
    {
        GLX_RGBA,           // Needs to support OpenGL
        GLX_DEPTH_SIZE, 16, // Needs to support a 16 bit depth buffer
        GLX_DOUBLEBUFFER,   // Needs to support double-buffering
        None                // end of list
    };

    int singleBufferVisual[] =
    {
        GLX_RGBA,           // Needs to support OpenGL
        GLX_DEPTH_SIZE, 16, // Needs to support a 16 bit depth buffer
        None                // end of list
    };

    // Try for the double-bufferd visual first
    visualInfo = glXChooseVisual( g_pDisplay, DefaultScreen(g_pDisplay), doubleBufferVisual );

    if( visualInfo == NULL )
    {
    	// If we can't find a double-bufferd visual, try for a single-buffered visual...
        visualInfo = glXChooseVisual( g_pDisplay, DefaultScreen(g_pDisplay), singleBufferVisual );

        if( visualInfo == NULL )
        {
            fprintf(stderr, "glxsimple: %s\n", "no RGB visual with depth buffer");
            exit(1);
        }

        g_bDoubleBuffered = false;
    }

    // Create an OpenGL rendering context
    glxContext = glXCreateContext( g_pDisplay, 
                                   visualInfo, 
                                   NULL,      // No sharing of display lists
                                   GL_TRUE ); // Direct rendering if possible
                           
    if( glxContext == NULL )
    {
        fprintf(stderr, "glxsimple: %s\n", "could not create rendering context");
        exit(1);
    }

    // Create an X colormap since we're probably not using the default visual 
    colorMap = XCreateColormap( g_pDisplay, 
                                RootWindow(g_pDisplay, visualInfo->screen), 
                                visualInfo->visual, 
                                AllocNone );

    windowAttributes.colormap     = colorMap;
    windowAttributes.border_pixel = 0;
    windowAttributes.event_mask   = ExposureMask           |
                                    VisibilityChangeMask   |
                                    KeyPressMask           |
                                    KeyReleaseMask         |
                                    ButtonPressMask        |
                                    ButtonReleaseMask      |
                                    PointerMotionMask      |
                                    StructureNotifyMask    |
                                    SubstructureNotifyMask |
                                    FocusChangeMask;
    
    // Create an X window with the selected visual
    g_window = XCreateWindow( g_pDisplay, 
                              RootWindow(g_pDisplay, visualInfo->screen), 
                              0, 0,     // x/y position of top-left outside corner of the window
                              640, 480, // Width and height of window
                              0,        // Border width
                              visualInfo->depth,
                              InputOutput,
                              visualInfo->visual,
                              CWBorderPixel | CWColormap | CWEventMask,
                              &windowAttributes );

    XSetStandardProperties( g_pDisplay,
                            g_window,
                            "GLX Sample",
                            "GLX Sample",
                            None,
                            NULL,
                            0,
                            NULL );

    // Bind the rendering context to the window
    glXMakeCurrent( g_pDisplay, g_window, glxContext );

    // Request the X window to be displayed on the screen
    XMapWindow( g_pDisplay, g_window );

    // Init OpenGL...
    init();
 
    //
    // Enter the render loop and don't forget to dispatch X events as
    // they occur.
    //

    while(1)
    {
        do
        {
            XNextEvent( g_pDisplay, &event );

            switch( event.type )
            {
                case ButtonPress:
                {
            	    if( event.xbutton.button == 1 )
            		{
						g_nLastMousePositX = event.xmotion.x;
				        g_nLastMousePositY = event.xmotion.y;
						g_bMousing = true;
					}
                }
                break;

                case ButtonRelease:
                {
                	if( event.xbutton.button == 1 )
                		g_bMousing = false;
                }
                break;
                
                case KeyPress:
                {
                    fprintf( stderr, "KeyPress event\n" );
                }
                break;

                case KeyRelease:
                {
                    fprintf( stderr, "KeyRelease event\n" );
                }
                break;

                case MotionNotify:
                {
                    if( g_bMousing )
                    {
	                    g_fSpinX -= (event.xmotion.x - g_nLastMousePositX);
						g_fSpinY -= (event.xmotion.y - g_nLastMousePositY);
					
						g_nLastMousePositX = event.xmotion.x;
					    g_nLastMousePositY = event.xmotion.y;
                    }
                }
                break;

                case Expose:
                {
                    fprintf( stderr, "Expose event\n" );
                }
                break;

                case ConfigureNotify:
                {
                    glViewport( 0, 0, event.xconfigure.width, event.xconfigure.height );
                }
            }
        }
        while( XPending(g_pDisplay) ); // Loop to compress events

        render();
    }
}

//-----------------------------------------------------------------------------
// Name: init()
// Desc: Init OpenGL context for rendering
//-----------------------------------------------------------------------------
void init( void )
{
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glEnable( GL_TEXTURE_2D );

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);
}

//-----------------------------------------------------------------------------
// Name: getBitmapImageData()
// Desc: Simply image loader for 24 bit BMP files.
//-----------------------------------------------------------------------------

void render( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	

    if( g_bDoubleBuffered )
        glXSwapBuffers( g_pDisplay, g_window ); // Buffer swap does implicit glFlush
    else
        glFlush(); // Explicit flush for single buffered case 
}



#endif
