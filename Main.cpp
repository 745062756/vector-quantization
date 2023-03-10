#include "Image.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <tuple>
#include <unordered_set>
#include <stdlib.h>
#include <time.h> 
#include <cmath>

using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:
MyImage			inImage, outImage;
HINSTANCE		hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void 				ColorPixel(char* imgBuf, int w, int h, int x, int y);


double dist_calc(pair<int, int> pt1, pair<int, int> pt2) {
	return sqrt(pow(pt1.first-pt2.first,2)+pow(pt1.second - pt2.second,2));
}

void err_reduce(vector<tuple<int, int, int>> & inputV, vector<tuple<int, int, unordered_set<int>>>& book) {
	unordered_set<string> pool;
	for (int i = 0; i < inputV.size();i++) {
		pair<int, double> minCodeV = { -1, 1000000 };
		for (int j = 0; j < book.size(); j++) {
			double dist = dist_calc({ get<0>(inputV[i]), get<1>(inputV[i]) }, { get<0>(book[j]) , get<1>(book[j]) });
			if (dist < minCodeV.second) {
				minCodeV = {j, dist };
			}
		}
		get<2>(inputV[i])=minCodeV.first;
		get<2>(book[minCodeV.first]).insert(i);
		pool.insert(to_string(get<0>(book[minCodeV.first]))+"-"+to_string(get<1>(book[minCodeV.first])));
	}

	// chk empty code
	for (int k = 0; k < book.size();++k) {
		if (get<2>(book[k]).size() == 0) {
			cout << "find empty code" << endl;

			bool finish = false;
			while (!finish) {
				int randomVectorId = rand() % inputV.size(); // 0~sz-1
				string selectedVStr = to_string(get<0>(inputV[randomVectorId])) + "-" + to_string(get<1>(inputV[randomVectorId]));
				if (pool.count(selectedVStr) == 1||get<2>(book[get<2>(inputV[randomVectorId])]).size()<2) continue;
				get<0>(book[k])= get<0>(inputV[randomVectorId]);
				get<1>(book[k])= get<1>(inputV[randomVectorId]);
				get<2>(book[k]).insert(randomVectorId);

				get<2>(book[get<2>(inputV[randomVectorId])]).erase(randomVectorId);
				get<2>(inputV[randomVectorId]) = k;

				pool.insert(selectedVStr);
				finish = true;
			}
		}
	}

	// adjust code
	for (auto & each: book) {
		int m = 0;
		int sumX = 0;
		int sumY = 0;
		for (int id : get<2>(each)) {
			sumX+=get<0>(inputV[id]);
			sumY+=get<1>(inputV[id]);
			m++;
		}
		get<0>(each) = round(sumX / (double)m);
		get<1>(each) = round(sumY / (double)m);
		get<2>(each).clear();
	}
}


char* compression(int codeNum) {
	vector<tuple<int, int, unordered_set<int>>> codeBook;
	vector<tuple<int, int, int>> inputVectors;
	vector<pair<int, int>> prev;

	for (int i = 0; i < inImage.getHeight() * inImage.getWidth()*3; i=i+6)
	{
		inputVectors.push_back({(int)(unsigned char)inImage.getImageData()[i], (int)(unsigned char)inImage.getImageData()[i + 3], -1});
	}
	// initial code vector
	unordered_set<string> selectedVec;
	for (int i = 0; i < codeNum; i++) {
		bool finish = false;
		while (!finish) {
			int randomVectorId = rand() % inputVectors.size(); // 0~sz-1
			string selectedVStr = to_string(get<0>(inputVectors[randomVectorId]))+"-"+to_string(get<1>(inputVectors[randomVectorId]));
			if (selectedVec.count(selectedVStr) == 1) continue;
			codeBook.push_back({get<0>(inputVectors[randomVectorId]), get<1>(inputVectors[randomVectorId]), unordered_set<int>()});
			prev.push_back({get<0>(inputVectors[randomVectorId]), get<1>(inputVectors[randomVectorId])});
			selectedVec.insert(selectedVStr);
			finish = true;
		}
	}

	int rnd = 0;
	int diffA = 10;
	int diffB = 10;
	while (!(rnd > 100 || (diffA ==0 && diffB ==0))) {
		err_reduce(inputVectors, codeBook);
		cout << "round (max 100, please wait): " << rnd << endl;
		diffA = 0;
		diffB = 0;
		for (int q = 0; q < codeBook.size();++q) {
			int diffX = abs(get<0>(codeBook[q]) - prev[q].first);
			int diffY = abs(get<1>(codeBook[q]) - prev[q].second);
			cout << diffX << "," << diffY << endl;
			prev[q] = { get<0>(codeBook[q]) ,get<1>(codeBook[q]) };
			diffA += diffX;
			diffB += diffY;
		}
		rnd++;
	}

	char* outData = new char[inImage.getHeight() * inImage.getWidth() * 3];

	for (int i = 0; i < inImage.getHeight() * inImage.getWidth() * 3; i = i + 6) {
		int newIdx = i / 6;
		outData[i] = get<0>(codeBook[get<2>(inputVectors[newIdx])]);
		outData[i + 1] = outData[i];
		outData[i + 2] = outData[i];
		outData[i+3] = get<1>(codeBook[get<2>(inputVectors[newIdx])]);
		outData[i + 4] = outData[i + 3];
		outData[i + 5] = outData[i + 3];
	}
	return outData;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;
	srand(time(NULL));
	// Read in a parameter from the command line
	stringstream stream(lpCmdLine);
	int size;
	int codeNum;
	string filePath;
	stream >> filePath >> size >> codeNum;
	// Create a separate console window to display output to stdout
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	cout << "file: " << filePath << endl << "codebook size: " << codeNum << endl;

	// Set up the input images
	int w = 352;
	int h = 288;
	inImage.setWidth(w);
	inImage.setHeight(h);
	inImage.setImagePath(filePath.c_str());
	inImage.ReadImage();

	// Set up the output img
	outImage = inImage;
	outImage.setImageData(compression(codeNum));

	// Initialize 
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_IMAGE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_IMAGE);
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}

void ColorPixel(char* imgBuf, int w, int h, int x, int y)
{
	imgBuf[(3 * y * w) +  3 * x] = 0;
	imgBuf[(3 * y * w) +  3 * x + 1] = 0;
	imgBuf[(3 * y * w) +  3 * x + 2] = 0;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_IMAGE);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_IMAGE;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance;

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR szHello[MAX_LOADSTRING];
	LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);
	RECT rt;
	GetClientRect(hWnd, &rt);

	switch (message) 
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			switch (wmId)
			{
				case IDM_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case ID_MODIFY_IMAGE:
				   outImage.Modify();
				   InvalidateRect(hWnd, &rt, false); 
				   break;
				case IDM_EXIT:
				   DestroyWindow(hWnd);
				   break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_PAINT:
			{
				hdc = BeginPaint(hWnd, &ps);
				// TO DO: Add any drawing code here...
				char text[1000];
				strcpy(text, "Original image (Left) and image after compression (Right)\n");
				DrawText(hdc, text, strlen(text), &rt, DT_LEFT);

				BITMAPINFO bmi;
				CBitmap bitmap;
				memset(&bmi,0,sizeof(bmi));
				bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
				bmi.bmiHeader.biWidth = inImage.getWidth();
				bmi.bmiHeader.biHeight = -inImage.getHeight();  // Use negative height.  DIB is top-down.
				bmi.bmiHeader.biPlanes = 1;
				bmi.bmiHeader.biBitCount = 24;
				bmi.bmiHeader.biCompression = BI_RGB;
				bmi.bmiHeader.biSizeImage = inImage.getWidth()*inImage.getHeight();

				SetDIBitsToDevice(hdc,
								  10,50,inImage.getWidth(),inImage.getHeight(),
								  0,0,0,inImage.getHeight(),
								  inImage.getImageData(),&bmi,DIB_RGB_COLORS);

				SetDIBitsToDevice(hdc,
								  outImage.getWidth()+50,50,outImage.getWidth(),outImage.getHeight(),
								  0,0,0,outImage.getHeight(),
								  outImage.getImageData(),&bmi,DIB_RGB_COLORS);


				EndPaint(hWnd, &ps);
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}




// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}


