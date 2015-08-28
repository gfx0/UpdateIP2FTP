#include <Windows.h>
#include <stdio.h>
#include <wininet.h>
#include <iostream>
#include <time.h>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "wininet")

#define ONE_MINUTE 60000

#ifdef _DEBUG
int testCommunicationsValue = 5;
#else
int testCommunicationsValue = 0;
#endif

#define TIMELIMIT_FOR_RETRANSMISSION_MS 15 * ONE_MINUTE

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void coreLoop();
void updateExternalIP();

DWORD timeSinceLastUpdate = 0;
bool isHeartBeating = false;

HWND hWndTimestampOfLastSentIP = NULL;
HWND hWndLastSentIP = NULL;

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
{

#ifdef _DEBUG
	MessageBox(0, "Running in debug mode! Test data will be sent!",0,0);
#endif

	char launchMessage[512] = { 0 };
	sprintf_s(launchMessage, "Launching UpdateIP2FTP with update interval: %d", TIMELIMIT_FOR_RETRANSMISSION_MS);
	OutputDebugStringA(launchMessage);

	MSG msg = { 0 };
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = "UpdateIP2FTP";
	if (!RegisterClass(&wc))
		return 1;

	HWND mainHWND = CreateWindowA(wc.lpszClassName,
									"UpdateIP2FTP",
									WS_OVERLAPPEDWINDOW | WS_VISIBLE,
									0, 0, 320, 240, 0, 0, hInstance, NULL);
	if ( !mainHWND )
		return 2;

	hWndTimestampOfLastSentIP = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), TEXT("Last IP Sent at:"),
		WS_CHILD | WS_VISIBLE | ES_READONLY, 0, 0, 300, 20, mainHWND, NULL, NULL, NULL);
	
	hWndLastSentIP = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), TEXT("Waiting to send first ip..."),
		WS_CHILD | WS_VISIBLE | ES_READONLY, 0, 20, 300, 20, mainHWND, NULL, NULL, NULL);
	
	timeSinceLastUpdate = timeGetTime();
	isHeartBeating = true;

	while (isHeartBeating)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) //Or use an if statement
		{
			//TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		coreLoop();
	}

	return 0;
}

void coreLoop()
{
	if ( !isHeartBeating )
		return;

	DWORD timeNow = timeGetTime();
	DWORD timeDeltaMS = timeNow - timeSinceLastUpdate;
	char msgg[512] = { 0 };
	if (timeDeltaMS > TIMELIMIT_FOR_RETRANSMISSION_MS)
	{
		sprintf_s(msgg, "Updating IP: lastUpdate: %d timeNow: %d timeDeltaMS: %d\n", timeNow, timeSinceLastUpdate, timeDeltaMS);
		timeSinceLastUpdate = timeGetTime();
		OutputDebugStringA(msgg);

		updateExternalIP();

	} else {

		//sprintf_s(msgg, "not yet time to update!:( timeSinceLastUpdate: %d timeNow : %d timeDeltaMS : %d\n", timeNow, timeSinceLastUpdate, timeDeltaMS);
		//OutputDebugStringA(msgg);
	}
}

void timeStamp()
{
	SYSTEMTIME st, lt;

	GetSystemTime(&st);
	GetLocalTime(&lt);

	char timeStampMsg[512] = { 0 };
	sprintf_s(timeStampMsg,"Last IP Sent at: %02d:%02d\n", st.wHour, st.wMinute);
	SetWindowText(hWndTimestampOfLastSentIP, timeStampMsg);
}

void updateExternalIP()
{
	HINTERNET hInternet, hFile;
	DWORD rSize;
	char buffer[512];

	hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	hFile = InternetOpenUrlA(hInternet, "http://www.janimakinen.com/ipcheck/echouserip.php", NULL, 0, INTERNET_FLAG_RELOAD, 0);
	InternetReadFile(hFile, &buffer, sizeof(buffer), &rSize);
	buffer[rSize] = '\0';

	InternetCloseHandle(hFile);

	OutputDebugStringA("Sending this via http post:\n");



	Sleep(1000);

	char newMsg[1024] = { 0 };
	if (testCommunicationsValue == 5)
		sprintf_s(buffer, "55.55.5.5");

	if (testCommunicationsValue == 4)
		sprintf_s(buffer, "444.44.4.44.444.4.4.");

	if (testCommunicationsValue == 3)
		sprintf_s(buffer, "33.3.3.33.3.3.3333.3.");

	if (testCommunicationsValue == 2)
		sprintf_s(buffer, "22.2.2.2.22.2..2.2.2");

	if (testCommunicationsValue == 1)
		sprintf_s(buffer, "1.1.1.1");

	SetWindowText(hWndLastSentIP, buffer);

	sprintf_s(newMsg, "http://www.janimakinen.com/ipcheck/updateip.php?%s\0", buffer);

	OutputDebugStringA(newMsg);
	OutputDebugStringA("\n");

	hFile = InternetOpenUrlA(hInternet, newMsg, NULL, 0, INTERNET_FLAG_RELOAD, 0);

	InternetCloseHandle(hFile);
	InternetCloseHandle(hInternet);

	timeStamp();

	testCommunicationsValue--;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_KEYUP:
		{
			switch (wParam)
			{
				case VK_ESCAPE:
				{
					isHeartBeating = false;
					PostQuitMessage(0);
				}
				break;
			}
			break;
		}
		break;

	case WM_DESTROY:
		{
			PostQuitMessage(0);
			isHeartBeating = false;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;

}

#undef ONE_MINUTE
#undef TIMELIMIT_FOR_RETRANSMISSION_MS

/********************************************************
 *
 * Interesting time consistency test for windows (53ms)
 *	(requires Winmm.lib)
 *
 ********************************************************/

/*
int i = 100;
DWORD lastTick = 0;
DWORD lastTime = 0;
while (--i)
{
	DWORD tick = GetTickCount();
	DWORD time = timeGetTime();
	char xxxx[512] = { 0 };
	sprintf_s(xxxx, "Tick: %d, Time: %d, dTick: %3d, dTime: %3d\n", tick, time, tick - lastTick, time - lastTime);
	OutputDebugStringA(xxxx);
	lastTick = tick;
	lastTime = time;
	Sleep(53);
}
return 0;
*/

/********************************************************
*
* Pseudocode
*
********************************************************/

//program start

	//wait for timelimit

		//find out current external IP address ( http://www.janimakinen.com/ipcheck/echouserip.php )

		//save current external IP address to a string

		//send the external IP through PHP to be stored into a logfile ( http://www.janimakinen.com/ipcheck/updateip.php?ipaddresshere ) where ipaddresshere is the ip...

//program shutdown