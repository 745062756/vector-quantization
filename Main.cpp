#include "Image.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <tuple>
#include <unordered_set>
#include <stdlib.h>
#include <time.h> 
#include <cmath>
#include <algorithm>

using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:
MyImage			inImage, outImage;
HINSTANCE		hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];
int side;

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void 				ColorPixel(char* imgBuf, int w, int h, int x, int y);


double dist_calc(vector<int>& vc1, vector<int>& vc2) {
	double res = 0;
	for (int k = 0; k < side * side; ++k) {
		res += pow(vc1[k] - vc2[k], 2);
	}
	return sqrt(res);
}

int mp=0;
int op(int x) {
	return round(x/(double)mp);
}

void err_reduce(vector<pair<vector<int>,int>> & inputV, vector<pair<vector<int>, unordered_set<int>>>& book) {
	unordered_set<string> pool;
	for (int i = 0; i < inputV.size();i++) {
		pair<int, double> minCodeV = { -1, 1000000 };
		for (int j = 0; j < book.size(); j++) {
			double dist = dist_calc(inputV[i].first, book[j].first);
			if (dist < minCodeV.second) {
				minCodeV = {j, dist };
			}
		}
		inputV[i].second=minCodeV.first;
		book[minCodeV.first].second.insert(i);

		string selectedVStr;
		for (int e = 0; e < side * side; ++e) {
			selectedVStr += to_string(book[minCodeV.first].first[e]) + "-";
		}
		pool.insert(selectedVStr);
	}

	// chk empty code
	for (int k = 0; k < book.size();++k) {
		if (book[k].second.size() == 0) {
			cout << "find empty code" << endl;

			//bool finish = false;
			//while (!finish) {
			//	int randomVectorId = rand() % inputV.size(); // 0~sz-1
			//	string selectedVStr = to_string(get<0>(inputV[randomVectorId])) + "-" + to_string(get<1>(inputV[randomVectorId]));
			//	if (pool.count(selectedVStr) == 1||get<2>(book[get<2>(inputV[randomVectorId])]).size()<2) continue;
			//	get<0>(book[k])= get<0>(inputV[randomVectorId]);
			//	get<1>(book[k])= get<1>(inputV[randomVectorId]);
			//	get<2>(book[k]).insert(randomVectorId);

			//	get<2>(book[get<2>(inputV[randomVectorId])]).erase(randomVectorId);
			//	get<2>(inputV[randomVectorId]) = k;

			//	pool.insert(selectedVStr);
			//	finish = true;
			//}
		}
	}
	// adjust code
	for (auto & each: book) {
		vector<int> temS(side*side, 0);

		mp = 0;
		for (int id : each.second) {
			for (int p = 0; p < side * side;++p) {
				temS[p] += inputV[id].first[p];
			}
			mp++;
		}
		transform(temS.begin(), temS.end(), temS.begin(), op);
		each.first = temS;
		each.second.clear();
	}
}


char* compression(int codeNum) {
	vector<pair<vector<int>, unordered_set<int>>> codeBook;
	vector<pair<vector<int>,int>> inputVectors;
	vector<vector<int>> prev;

	for (int i = 0; i * inImage.getWidth() * 3 * side < inImage.getWidth() * inImage.getHeight() * 3; ++i)
	{
		for (int j = 0; side * 3 * j < 3 * inImage.getWidth(); j++)
		{
			int final = i * inImage.getWidth() * 3 * side + side * 3 * j;
			vector<int> newV;
			for (int a = 0; a < side; ++a)
			{
				for (int b = 0; b < side; ++b)
				{
					int idx = final + a * 3 * inImage.getWidth() + 3 * b;
					newV.push_back((int)(unsigned char)inImage.getImageData()[idx]);
				}
			}
			inputVectors.push_back({ newV,-1});
		}
	}

	// initial code vector
	unordered_set<string> selectedVec;
	for (int i = 0; i < codeNum; i++) {
		bool finish = false;
		while (!finish) {
			int randomVectorId = rand() % inputVectors.size(); // 0~sz-1
			string selectedVStr;
			for (int e = 0; e < side * side; ++e) {
				selectedVStr += to_string(inputVectors[randomVectorId].first[e]) + "-";
			}
			if (selectedVec.count(selectedVStr) == 1) continue;
			codeBook.push_back({inputVectors[randomVectorId].first, unordered_set<int>()});
			prev.push_back(inputVectors[randomVectorId].first);
			selectedVec.insert(selectedVStr);
			finish = true;
		}
	}

	int rnd = 0;
	vector<int> totalDiff(side*side, 10);
	while (!(rnd > 100 || (all_of(totalDiff.begin(), totalDiff.end(), [](int i) {return i == 0;})))) {
		err_reduce(inputVectors, codeBook);
		cout << "round (max 100, please wait): " << rnd << endl;
		transform(totalDiff.begin(), totalDiff.end(), totalDiff.begin(), [](int z) {return 0;});

		for (int q = 0; q < codeBook.size();++q) {
			vector<int> temV(side * side, 0);
			for (int t = 0; t < side * side; ++t) {
				temV[t] = abs(codeBook[q].first[t] - prev[q][t]);
			}

			for_each(temV.begin(), temV.end(), [](int f) {cout << f << ", "; });
			cout << endl;

			prev[q] = codeBook[q].first;
			transform(totalDiff.begin(), totalDiff.end(), temV.begin(), totalDiff.begin(), [](int x, int c) {return x + c; });
		}
		rnd++;
	}

	char* outData = new char[inImage.getHeight() * inImage.getWidth() * 3];


	int cnt = 0;
	for (int i = 0; i * inImage.getWidth() * 3 * side < inImage.getWidth() * inImage.getHeight() * 3; ++i)
	{
		for (int j = 0; side * 3 * j < 3 * inImage.getWidth(); j++)
		{
			int final = i * inImage.getWidth() * 3 * side + side * 3 * j;

			vector<int> newV = codeBook[inputVectors[cnt].second].first;
			int mq = 0;
			for (int a = 0; a < side; ++a)
			{
				for (int b = 0; b < side; ++b)
				{
					int idx = final + a * 3 * inImage.getWidth() + 3 * b;
					outData[idx] = newV[mq];
					outData[idx+1] = newV[mq];
					outData[idx+2] = newV[mq];
					mq++;
				}
			}
			cnt++;
		}
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
	side = sqrt(size);
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


