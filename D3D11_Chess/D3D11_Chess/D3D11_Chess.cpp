//--------------------------------------------------------------------------------------
//
//                  2014-2015 Bahar Dönemi BIL320 Bilgisayar Grafikleri I
//                  Dönem Projesi  : "DirectX 11 ile 3D Satranç Oyunu"
//					Son Güncelleme : 28.4.2015 -oynanabilir halde-þah tehlikesi gitmemiþse yine oyuncuda
//
//--------------------------------------------------------------------------------------

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include "resource.h"

#include <DXGI.h>
#include <dinput.h>

#include <fstream>
#include <sstream>
using namespace std;

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
	XMFLOAT3 Normal;
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 vEyePos;
	XMFLOAT4 vLightPos[2];
	XMFLOAT4 vMeshColor;
	XMFLOAT4 vLightColor[2];
}cBuffer;

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE                           g_hInst = NULL;
HWND                                g_hWnd = NULL;
D3D_DRIVER_TYPE                     g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL                   g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*                       g_pd3dDevice = NULL;
ID3D11DeviceContext*                g_pImmediateContext = NULL;
IDXGISwapChain*                     g_pSwapChain = NULL;
ID3D11RenderTargetView*             g_pRenderTargetView = NULL;
ID3D11Texture2D*                    g_pDepthStencil = NULL;
ID3D11DepthStencilView*             g_pDepthStencilView = NULL;
ID3D11VertexShader*                 g_pVertexShader = NULL;
ID3D11PixelShader*                  g_pPixelShader = NULL;
ID3D11PixelShader*                  g_pPixelShader_Textured = NULL;
ID3D11PixelShader*                  g_pPixelShader_Solid = NULL;
ID3D11InputLayout*                  g_pVertexLayout = NULL;
ID3D11Buffer*                       g_pVertexBuffer_Pointer = NULL;
ID3D11Buffer*                       g_pVertexBuffer_CrossHair = NULL;
ID3D11Buffer*                       g_pVertexBuffer_chessBoard = NULL;


ID3D11Buffer*                       g_pVertexBuffer_piyon = NULL;
ID3D11Buffer*                       g_pVertexBuffer_kale = NULL;
ID3D11Buffer*                       g_pVertexBuffer_at = NULL;
ID3D11Buffer*                       g_pVertexBuffer_fil = NULL;
ID3D11Buffer*                       g_pVertexBuffer_vezir = NULL;
ID3D11Buffer*                       g_pVertexBuffer_sah = NULL;


ID3D11Buffer*                       g_pVertexBuffer_Light = NULL;
ID3D11Buffer*                       g_pIndexBuffer = NULL;
ID3D11Buffer*                       g_pConstantBuffer = NULL;
ID3D11ShaderResourceView*           g_pTexturepiyon = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerWhite = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerOneri_White = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerOneri_Black = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerBlack = NULL;
ID3D11ShaderResourceView*           g_pTexturechessBoard = NULL;
ID3D11SamplerState*                 g_pSamplerLinear = NULL;
XMMATRIX                            g_World;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;
XMFLOAT3 translateP; //cell point almak için global deðiþken
int vertexCount_piyon;
int vertexCount_kale;
int vertexCount_at;
int vertexCount_fil;
int vertexCount_vezir;
int vertexCount_sah;
int vertexCountpointer;
int vertexCountchessBoard;
int vertexCountlight;
int vertexCountcrosshair;

SimpleVertex* verticespointer;
SimpleVertex* verticeschessBoard;
SimpleVertex* vertices_piyon;
SimpleVertex* vertices_kale;
SimpleVertex* vertices_at;
SimpleVertex* vertices_fil;
SimpleVertex* vertices_vezir;
SimpleVertex* vertices_sah;
SimpleVertex* verticesLight;
SimpleVertex* verticesCrossHair;

XMVECTOR Ro;
XMVECTOR Rd;

// Oyuncu sýrasý (true:beyaz, false:siyah)
bool turn = true;


//-------------------------------------------------------------------------------------
// DIRECT INPUT
//-------------------------------------------------------------------------------------
IDirectInputDevice8* DIKeyboard;
IDirectInputDevice8* DIMouse;

DIMOUSESTATE mouseLastState;
LPDIRECTINPUT8 DirectInput;

float rot = 0.01f;
double countsPerSecond = 0.0;
__int64 CounterStart = 0;
int frameCount = 0;
int fps = 0;
__int64 frameTimeOld = 0;
double frameTime;

bool InitDirectInput(HINSTANCE hInstance);
void DetectInput(double time);
void UpdateScene(double time);
void StartTimer();
double GetTime();
double GetFrameTime();
void tas_render(ID3D11Buffer* tas_VertexBuffer, int tas_vertexCount, UINT  stride, UINT  offset, double r_1, double r_2, double r_3, double r_4, double l_x, double l_y, double l_z);
void pointer_oneri_render(UINT  stride, UINT  offset, double l_x, double l_z, int siyah_beyaz);
void onerileri_sifirla();
void Yenilme_tehlikesi_yap();
bool sah_yenilme_durumu();
int Black_or_White[8][8] =
{
	0, 1, 0, 1, 0, 1, 0, 1,
	1, 0, 1, 0, 1, 0, 1, 0,
	0, 1, 0, 1, 0, 1, 0, 1,
	1, 0, 1, 0, 1, 0, 1, 0,
	0, 1, 0, 1, 0, 1, 0, 1,
	1, 0, 1, 0, 1, 0, 1, 0,
	0, 1, 0, 1, 0, 1, 0, 1,
	1, 0, 1, 0, 1, 0, 1, 0
};

int Taslar_Konumu[8][8] =
{
	7, 8, 9, 10, 11, 9, 8, 7,
	12, 12, 12, 12, 12, 12, 12, 12,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	6, 6, 6, 6, 6, 6, 6, 6,
	1, 2, 3, 4, 5, 3, 2, 1
};

int Oneri_Matrisi[8][8] =
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
};
int Yenilme_tehlikesi[8][8] =
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
};


int Yenen_beyaz_taslar[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int Yenen_siyah_taslar[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int Secilen_Tas_Translate_Olucak = 0;
int from_translate_olan_i, from_translate_olan_j;
int i_tas_konum_matis_icin, j_tas_konum_matis_icin;
int hamle_sirai = 2;


//-------------------------------------------------------------------------------------
// FIRST PERSON CAMERA
//-------------------------------------------------------------------------------------

XMVECTOR Eye;
XMVECTOR At;
XMVECTOR Up;

XMVECTOR DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
XMVECTOR camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

XMVECTOR DefaultForward2 = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);

XMVECTOR crosshairPosition = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

XMMATRIX camRotationMatrix;
XMMATRIX groundWorld;

float moveLeftRight = 0.0f;
float moveBackForward = 0.0f;

float camYaw = 0.0f;
float camPitch = 0.0f;

void UpdateCamera();

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void Render(double time);

XMVECTOR testIntersection(XMVECTOR rayOrigin, XMVECTOR rayDir, SimpleVertex* verticesModel, int vertexCount, bool transform, XMMATRIX& g_World);

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	if (FAILED(InitDevice()))
	{
		CleanupDevice();
		return 0;
	}

	if (!InitDirectInput(hInstance))
	{
		MessageBox(0, L"Direct Input Initialization - Failed", L"Error", MB_OK);
		return 0;
	}

	// Main message loop
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			frameCount++;
			if (GetTime() > 1.0f)
			{
				fps = frameCount;
				frameCount = 0;
				StartTimer();
			}

			frameTime = GetFrameTime();

			DetectInput(frameTime);

			Render(frameTime);
		}
	}

	CleanupDevice();

	return (int)msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, 1280, 720 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(L"TutorialWindowClass", L"DirectX11 ile 3D Satranç Oyunu", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

bool InitDirectInput(HINSTANCE hInstance)
{
	HRESULT HR = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DirectInput, NULL);
	HR = DirectInput->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL);
	HR = DirectInput->CreateDevice(GUID_SysMouse, &DIMouse, NULL);
	HR = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	HR = DIKeyboard->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	HR = DIMouse->SetDataFormat(&c_dfDIMouse);
	HR = DIMouse->SetCooperativeLevel(g_hWnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
	return true;
}

void DetectInput(double time)
{
	DIMOUSESTATE mouseCurrState;
	BYTE keyboardState[256];
	DIKeyboard->Acquire();
	DIMouse->Acquire();

	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
	DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);
	if (keyboardState[DIK_ESCAPE] & 0x80) PostMessage(g_hWnd, WM_DESTROY, 0, 0);

	float speed = 15.0F * time;

	if (keyboardState[DIK_A] & 0x80)
	{
		moveLeftRight -= speed;
	}

	if (keyboardState[DIK_D] & 0x80)
	{
		moveLeftRight += speed;
	}

	if (keyboardState[DIK_W] & 0x80)
	{
		moveBackForward += speed;
	}

	if (keyboardState[DIK_S] & 0x80)
	{
		moveBackForward -= speed;
	}
	if (keyboardState[DIK_C] & 0x80)
	{
		////translate
		//Taslar_Konumu[7][0] = 0;
		//Taslar_Konumu[4][4] = 1;


		//// öneriler
		//Oneri_Matrisi[4][4] = 1;
		//Oneri_Matrisi[4][5] = 1;
		//Oneri_Matrisi[4][6] = 1;
	}

	if (mouseCurrState.rgbButtons[0] & 0x80)
	{

		int z, x;

		// z için i
		z = translateP.z;
		x = translateP.x;
		if (z > 0){

			i_tas_konum_matis_icin = 3 - (z / 5);
		}
		else{
			i_tas_konum_matis_icin = 4 + ((-1 * z) / 5);

		}

		// x için j
		if (x > 0){

			j_tas_konum_matis_icin = 4 + (x / 5);

		}
		else{
			j_tas_konum_matis_icin = 3 + ((x / 5));

		}

		std::wostringstream outs;
		outs << " =  Suan burdayým :  " << translateP.x << " , " << translateP.y << " , " << translateP.z << "  i ,j = " << i_tas_konum_matis_icin << "," << j_tas_konum_matis_icin << "   Tas : " << Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin];
		SetWindowText(g_hWnd, outs.str().c_str());







		//burda hangi taþ ise olasý hamleleri verilecek ve kendisienden farklý bir öneri cell ine týklamýþ ise oraya translate edilcek :D
		bool hamleden_once_sah_yenilme_tehlisesi;
		hamleden_once_sah_yenilme_tehlisesi = sah_yenilme_durumu();

		int kopya_Taslar_Konumu[8][8];
		int kopya_Oneri_Matrisi[8][8];
		int kopya_Yenilme_tehlikesi[8][8];
		int kopya_Secilen_Tas_Translate_Olucak=Secilen_Tas_Translate_Olucak;
		int kopya_from_translate_olan_i=from_translate_olan_i;
		int kopya_from_translate_olan_j=from_translate_olan_j;
		int kopya_Yenen_beyaz_taslar[16];
		int kopya_Yenen_siyah_taslar[16];
		// kopyalama 
		for (int i = 0; i < 8; i++)
		{

			for (int j = 0; j < 8; j++)
			{
				kopya_Taslar_Konumu[i][j] = Taslar_Konumu[i][j];
				kopya_Oneri_Matrisi[i][j] = Oneri_Matrisi[i][j];
				kopya_Yenilme_tehlikesi[i][j] = Yenilme_tehlikesi[i][j];

			}
			kopya_Yenen_beyaz_taslar[i] = Yenen_beyaz_taslar[i];
			kopya_Yenen_siyah_taslar[i] = Yenen_siyah_taslar[i];
		}


		if (hamle_sirai % 2 == 0){
			// Beyaz oyuncular oynar




			/// BEYAZZ ÞAH YEME DURUMU VAR MI

			std::wostringstream outs;
			outs << " =  camera :  " << moveBackForward << " move back < " << "move rig" << moveLeftRight << "camyaw " << camYaw << "cam pitch" << camPitch;
			SetWindowText(g_hWnd, outs.str().c_str());


			// bir taþ seçilmiþtir ve önerilerden birisine týklamýþtýr oda 0 olamlýdýr çünkü boþ bir yere trasnlate olabilir  Oneri_Matrisi[i_tas_konum_matis_icin][j_tas_konum_matis_icin] != 0 öneri verilmiþ mi?
			if (Secilen_Tas_Translate_Olucak > 0 && Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 0 && Secilen_Tas_Translate_Olucak < 7 && Oneri_Matrisi[i_tas_konum_matis_icin][j_tas_konum_matis_icin] != 0){

				// seçilen bir taþ var mý? var trasnlete olacak nokta boþ mu? boþ 

				////translate
				Taslar_Konumu[from_translate_olan_i][from_translate_olan_j] = 0;
				Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] = Secilen_Tas_Translate_Olucak;
				hamle_sirai++;



				turn = false;

				if (turn)
					Eye = XMVectorSet(0.0f, 30.0f, -50.0f, 0.0f);
				else
					Eye = XMVectorSet(0.0f, 30.0f, 50.0f, 0.0f);

				camPitch = 0;
				camYaw = 0;


			}
			// yeme olaylarý baþka bir taþýn üstüne gelme siyah taþlarýn üstüne gidebilir-----SÝYAH TAÞ YENÝYOR  && Oneri_Matrisi[i_tas_konum_matis_icin][j_tas_konum_matis_icin] != 0 öneri var mý
			if (Secilen_Tas_Translate_Olucak > 0 && Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] > 6 && Secilen_Tas_Translate_Olucak < 7 && Oneri_Matrisi[i_tas_konum_matis_icin][j_tas_konum_matis_icin] != 0){

				// seçilen bir taþ var mý? var trasnlete olacak nokta boþ mu? boþ 
				for (int index = 0; index < 16; index++)
				{
					if (Yenen_siyah_taslar[index] == 0){
						Yenen_siyah_taslar[index] = Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin];
						break;
					}
				}
				////translate
				Taslar_Konumu[from_translate_olan_i][from_translate_olan_j] = 0;
				Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] = Secilen_Tas_Translate_Olucak;
				hamle_sirai++;



				turn = false;

				if (turn)
					Eye = XMVectorSet(0.0f, 30.0f, -50.0f, 0.0f);
				else
					Eye = XMVectorSet(0.0f, 30.0f, 50.0f, 0.0f);

				camPitch = 0;
				camYaw = 0;


			}

			if (Secilen_Tas_Translate_Olucak == 6 && Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] > 6) {//piyon ise bunun yeme iþlemleri ve siyah bir taþ varsa seçilen
				if (from_translate_olan_i - 1 == i_tas_konum_matis_icin && from_translate_olan_j - 1 == j_tas_konum_matis_icin || from_translate_olan_j + 1 == j_tas_konum_matis_icin){

					// seçilen bir taþ var mý? var trasnlete olacak nokta boþ mu? boþ 
					for (int index = 0; index < 16; index++)
					{
						if (Yenen_siyah_taslar[index] == 0){
							Yenen_siyah_taslar[index] = Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin];
							break;
						}
					}
					////translate
					Taslar_Konumu[from_translate_olan_i][from_translate_olan_j] = 0;
					Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] = Secilen_Tas_Translate_Olucak;
					hamle_sirai++;



					turn = false;

					if (turn)
						Eye = XMVectorSet(0.0f, 30.0f, -50.0f, 0.0f);
					else
						Eye = XMVectorSet(0.0f, 30.0f, 50.0f, 0.0f);

					camPitch = 0;
					camYaw = 0;

				}

			}




			Secilen_Tas_Translate_Olucak = Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin];
			from_translate_olan_i = i_tas_konum_matis_icin;
			from_translate_olan_j = j_tas_konum_matis_icin;

		}
		else{///////////////////////////////////////// ELSEE //////////////////////////////

			// siyah oyuncu oynar
			// bir taþ seçilmiþtir ve önerilerden birisine týklamýþtýr oda 0 olamlýdýr çünkü boþ bir yere trasnlate olabilir  && Oneri_Matrisi[i_tas_konum_matis_icin][j_tas_konum_matis_icin] != 0 öneri var mý?
			if (Secilen_Tas_Translate_Olucak > 0 && Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 0 && Secilen_Tas_Translate_Olucak > 6 && Oneri_Matrisi[i_tas_konum_matis_icin][j_tas_konum_matis_icin] != 0){

				// seçilen bir taþ var mý? var trasnlete olacak nokta boþ mu? boþ 
				////translate
				Taslar_Konumu[from_translate_olan_i][from_translate_olan_j] = 0;
				Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] = Secilen_Tas_Translate_Olucak;
				hamle_sirai++;


				turn = true;

				if (turn)
					Eye = XMVectorSet(0.0f, 30.0f, -50.0f, 0.0f);
				else
					Eye = XMVectorSet(0.0f, 30.0f, 50.0f, 0.0f);

				camPitch = 0;
				camYaw = 0;



			}
			// aþaðýdaki yere piyon ise farklý yeme stili olacak 
			//beyaz taþlar yenicek 
			if (Secilen_Tas_Translate_Olucak > 0 && Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] <7 && Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] > 0 && Secilen_Tas_Translate_Olucak > 6 && Oneri_Matrisi[i_tas_konum_matis_icin][j_tas_konum_matis_icin] != 0){

				//yemem iþelemlerþi
				for (int index = 0; index < 16; index++)
				{
					if (Yenen_beyaz_taslar[index] == 0){
						Yenen_beyaz_taslar[index] = Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin];
						break;
					}
				}
				Taslar_Konumu[from_translate_olan_i][from_translate_olan_j] = 0;
				Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] = Secilen_Tas_Translate_Olucak;
				hamle_sirai++;


				turn = true;

				if (turn)
					Eye = XMVectorSet(0.0f, 30.0f, -50.0f, 0.0f);
				else
					Eye = XMVectorSet(0.0f, 30.0f, 50.0f, 0.0f);

				camPitch = 0;
				camYaw = 0;



			}
			/// siyah piyonun yeme olaylarý
			if (Secilen_Tas_Translate_Olucak == 12 && Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] <7 && Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] >0) {//piyon ise bunun yeme iþlemleri ve siyah bir taþ varsa seçilen
				// +1 olacak ters yönde
				if (from_translate_olan_i + 1 == i_tas_konum_matis_icin && from_translate_olan_j - 1 == j_tas_konum_matis_icin || from_translate_olan_j + 1 == j_tas_konum_matis_icin){

					//yemem iþelemlerþi
					for (int index = 0; index < 16; index++)
					{
						if (Yenen_beyaz_taslar[index] == 0){
							Yenen_beyaz_taslar[index] = Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin];
							break;
						}
					}
					Taslar_Konumu[from_translate_olan_i][from_translate_olan_j] = 0;
					Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] = Secilen_Tas_Translate_Olucak;
					hamle_sirai++;


					turn = true;

					if (turn)
						Eye = XMVectorSet(0.0f, 30.0f, -50.0f, 0.0f);
					else
						Eye = XMVectorSet(0.0f, 30.0f, 50.0f, 0.0f);

					camPitch = 0;
					camYaw = 0;

				}

			}






			Secilen_Tas_Translate_Olucak = Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin];
			from_translate_olan_i = i_tas_konum_matis_icin;
			from_translate_olan_j = j_tas_konum_matis_icin;


		}

		hamle_sirai++;
		if (hamleden_once_sah_yenilme_tehlisesi == true && sah_yenilme_durumu()==true){
			// tehlike ortan kalmamkmýþsa geri alýyoruz 

			Secilen_Tas_Translate_Olucak = kopya_Secilen_Tas_Translate_Olucak;
			from_translate_olan_i = kopya_from_translate_olan_i;
			from_translate_olan_j = kopya_from_translate_olan_j;
			for (int i = 0; i < 8; i++)
			{

				for (int j = 0; j < 8; j++)
				{
					Taslar_Konumu[i][j] = kopya_Taslar_Konumu[i][j];
					Oneri_Matrisi[i][j] = kopya_Oneri_Matrisi[i][j];
					Yenilme_tehlikesi[i][j] = kopya_Yenilme_tehlikesi[i][j];

				}
				Yenen_beyaz_taslar[i] = kopya_Yenen_beyaz_taslar[i];
				Yenen_siyah_taslar[i] = kopya_Yenen_siyah_taslar[i];
			}
			if (turn == false){
				turn = true;
			}
			else{
				turn = false;
			}

			if (turn)
				Eye = XMVectorSet(0.0f, 30.0f, -50.0f, 0.0f);
			else
				Eye = XMVectorSet(0.0f, 30.0f, 50.0f, 0.0f);

			camPitch = 0;
			camYaw = 0;

		}
		else{
			hamle_sirai++;
		}




		////// ÖNERÝLER 
		onerileri_sifirla();
		if (hamle_sirai % 2 == 0){
			// BEYAZ KALE
			if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 1)
			{

				/*Oneri_Matrisi[i_tas_konum_matis_icin][j_tas_konum_matis_icin] = 1;*/
				for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

					if (i >= 0 && i < 8){

						if (Taslar_Konumu[i][j_tas_konum_matis_icin]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j_tas_konum_matis_icin]>6){
								Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
						}

					}

				}
				for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

					if (i >= 0 && i < 8){

						if (Taslar_Konumu[i][j_tas_konum_matis_icin]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j_tas_konum_matis_icin] > 6){
								Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
						}

					}
				}

				for (int j = j_tas_konum_matis_icin + 1; j < 8; j++){

					if (j >= 0 && j < 8){

						if (Taslar_Konumu[i_tas_konum_matis_icin][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i_tas_konum_matis_icin][j]>6){
								Oneri_Matrisi[i_tas_konum_matis_icin][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i_tas_konum_matis_icin][j] = 1;
						}

					}

				}
				for (int j = j_tas_konum_matis_icin - 1; j >= 0; j--){

					if (j >= 0 && j < 8){

						if (Taslar_Konumu[i_tas_konum_matis_icin][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i_tas_konum_matis_icin][j] > 6){
								Oneri_Matrisi[i_tas_konum_matis_icin][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i_tas_konum_matis_icin][j] = 1;
						}

					}
				}



			}
			// BEYAZ AT
			else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 2)
			{

				int i = i_tas_konum_matis_icin, j = j_tas_konum_matis_icin;

				i = i - 2;
				j = j + 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j] < 7 && Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
					{


					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}

				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i - 2;
				j = j - 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j] < 7 && Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
					{


					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i - 1;
				j = j - 2;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j] < 7 && Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
					{


					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i - 1;
				j = j + 2;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j] < 7 && Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
					{


					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i + 1;
				j = j - 2;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j] < 7 && Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
					{


					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i + 1;
				j = j + 2;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j] < 7 && Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
					{


					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i + 2;
				j = j + 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j] < 7 && Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
					{


					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}


				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i + 2;
				j = j - 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j] < 7 && Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
					{


					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}


			}
			// BEYAZ FÝL
			else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 3)
			{
				int j = j_tas_konum_matis_icin + 1;
				for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j]>6){
								Oneri_Matrisi[i][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j] = 1;
						}

					}
					j++;
				}
				j = j_tas_konum_matis_icin - 1;
				for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j]>6){
								Oneri_Matrisi[i][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j] = 1;
						}

					}
					j--;
				}




				j = j_tas_konum_matis_icin + 1;
				for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j]>6){
								Oneri_Matrisi[i][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j] = 1;
						}

					}
					j++;
				}
				j = j_tas_konum_matis_icin - 1;
				for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j]>6){
								Oneri_Matrisi[i][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j] = 1;
						}

					}
					j--;
				}





			}
			//BEYAZ VEZÝR
			else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 4)
			{
				/*Oneri_Matrisi[i_tas_konum_matis_icin][j_tas_konum_matis_icin] = 1;*/
				for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

					if (i >= 0 && i < 8){

						if (Taslar_Konumu[i][j_tas_konum_matis_icin]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j_tas_konum_matis_icin]>6){
								Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
						}

					}

				}
				for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

					if (i >= 0 && i < 8){

						if (Taslar_Konumu[i][j_tas_konum_matis_icin]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j_tas_konum_matis_icin] > 6){
								Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
						}

					}
				}

				for (int j = j_tas_konum_matis_icin + 1; j < 8; j++){

					if (j >= 0 && j < 8){

						if (Taslar_Konumu[i_tas_konum_matis_icin][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i_tas_konum_matis_icin][j]>6){
								Oneri_Matrisi[i_tas_konum_matis_icin][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i_tas_konum_matis_icin][j] = 1;
						}

					}

				}
				for (int j = j_tas_konum_matis_icin - 1; j >= 0; j--){

					if (j >= 0 && j < 8){

						if (Taslar_Konumu[i_tas_konum_matis_icin][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i_tas_konum_matis_icin][j] > 6){
								Oneri_Matrisi[i_tas_konum_matis_icin][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i_tas_konum_matis_icin][j] = 1;
						}

					}
				}



				int j = j_tas_konum_matis_icin + 1;
				for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j]>6){
								Oneri_Matrisi[i][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j] = 1;
						}

					}
					j++;
				}
				j = j_tas_konum_matis_icin - 1;
				for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j]>6){
								Oneri_Matrisi[i][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j] = 1;
						}

					}
					j--;
				}




				j = j_tas_konum_matis_icin + 1;
				for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j]>6){
								Oneri_Matrisi[i][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j] = 1;
						}

					}
					j++;
				}
				j = j_tas_konum_matis_icin - 1;
				for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j]>6){
								Oneri_Matrisi[i][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j] = 1;
						}

					}
					j--;
				}

			}
			// BEYAZ ÞAH
			else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 5)
			{
				int i = i_tas_konum_matis_icin, j = j_tas_konum_matis_icin;

				i = i - 1;
				j = j - 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
					{
						if (Taslar_Konumu[i][j]>6){
							Oneri_Matrisi[i][j] = 1;
						}

					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}

				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i - 1;
				j = j;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
					{
						if (Taslar_Konumu[i][j]>6){
							Oneri_Matrisi[i][j] = 1;
						}

					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i - 1;
				j = j + 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
					{

						if (Taslar_Konumu[i][j]>6){
							Oneri_Matrisi[i][j] = 1;
						}
					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i;
				j = j - 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
					{
						if (Taslar_Konumu[i][j]>6){
							Oneri_Matrisi[i][j] = 1;
						}

					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i;
				j = j + 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
					{
						if (Taslar_Konumu[i][j]>6){
							Oneri_Matrisi[i][j] = 1;
						}

					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i + 1;
				j = j - 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
					{
						if (Taslar_Konumu[i][j]>6){
							Oneri_Matrisi[i][j] = 1;
						}

					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i + 1;
				j = j;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
					{
						if (Taslar_Konumu[i][j]>6){
							Oneri_Matrisi[i][j] = 1;
						}

					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}


				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i + 1;
				j = j + 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
					{
						if (Taslar_Konumu[i][j]>6){
							Oneri_Matrisi[i][j] = 1;
						}

					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}




			}
			// BEYAZ PÝYON
			else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 6)
			{
				int i = i_tas_konum_matis_icin - 1;
				int durma = i - 1;

				for (i = i; i >= durma; i--){

					if (i >= 0 && i < 8){

						if (Taslar_Konumu[i][j_tas_konum_matis_icin]>0)// herhangi bir  taþ rastlanmýþsa orda kes
						{
							break;
						}
						else{

							Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
							if (durma == i && i_tas_konum_matis_icin != 6){
								Oneri_Matrisi[i][j_tas_konum_matis_icin] = 0;
							}
						}

					}

				}




			}



		}
		else{

			// SÝYAH KALE
			if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 7)
			{

				for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

					if (i >= 0 && i < 8){

						if (Taslar_Konumu[i][j_tas_konum_matis_icin] >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j_tas_konum_matis_icin] < 7){

								Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
						}

					}

				}
				for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

					if (i >= 0 && i < 8){

						if (Taslar_Konumu[i][j_tas_konum_matis_icin]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j_tas_konum_matis_icin] < 7){

								Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
						}

					}
				}

				for (int j = j_tas_konum_matis_icin + 1; j < 8; j++){

					if (j >= 0 && j < 8){

						if (Taslar_Konumu[i_tas_konum_matis_icin][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i_tas_konum_matis_icin][j] < 7){

								Oneri_Matrisi[i_tas_konum_matis_icin][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i_tas_konum_matis_icin][j] = 1;
						}

					}

				}
				for (int j = j_tas_konum_matis_icin - 1; j >= 0; j--){

					if (j >= 0 && j < 8){

						if (Taslar_Konumu[i_tas_konum_matis_icin][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i_tas_konum_matis_icin][j] < 7){

								Oneri_Matrisi[i_tas_konum_matis_icin][j] = 1;
							}

							break;

						}
						else{

							Oneri_Matrisi[i_tas_konum_matis_icin][j] = 1;
						}

					}
				}



			}
			// SÝYAH AT
			else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 8)
			{

				int i = i_tas_konum_matis_icin, j = j_tas_konum_matis_icin;

				i = i - 2;
				j = j + 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j] >6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
					{


					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}

				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i - 2;
				j = j - 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j] >6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
					{


					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i - 1;
				j = j - 2;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j] >6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
					{


					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i - 1;
				j = j + 2;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j] >6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
					{


					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i + 1;
				j = j - 2;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j] >6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
					{


					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i + 1;
				j = j + 2;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j] >6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
					{


					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i + 2;
				j = j + 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j] >6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
					{


					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}


				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i + 2;
				j = j - 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j] >6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
					{


					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}


			}
			// SÝYAH FÝL
			else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 9)
			{
				int j = j_tas_konum_matis_icin + 1;
				for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j] < 7){

								Oneri_Matrisi[i][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j] = 1;
						}

					}
					j++;
				}
				j = j_tas_konum_matis_icin - 1;
				for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j] < 7){

								Oneri_Matrisi[i][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j] = 1;
						}

					}
					j--;
				}




				j = j_tas_konum_matis_icin + 1;
				for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j] < 7){

								Oneri_Matrisi[i][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j] = 1;
						}

					}
					j++;
				}
				j = j_tas_konum_matis_icin - 1;
				for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j] < 7){

								Oneri_Matrisi[i][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j] = 1;
						}

					}
					j--;
				}





			}
			//SÝYAH VEZÝR
			else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 10)
			{
				/*Oneri_Matrisi[i_tas_konum_matis_icin][j_tas_konum_matis_icin] = 1;*/
				for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

					if (i >= 0 && i < 8){

						if (Taslar_Konumu[i][j_tas_konum_matis_icin]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j_tas_konum_matis_icin] < 7){

								Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
						}

					}

				}
				for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

					if (i >= 0 && i < 8){

						if (Taslar_Konumu[i][j_tas_konum_matis_icin] >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j_tas_konum_matis_icin] < 7){

								Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
						}

					}
				}

				for (int j = j_tas_konum_matis_icin + 1; j < 8; j++){

					if (j >= 0 && j < 8){

						if (Taslar_Konumu[i_tas_konum_matis_icin][j] >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i_tas_konum_matis_icin][j] < 7){

								Oneri_Matrisi[i_tas_konum_matis_icin][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i_tas_konum_matis_icin][j] = 1;
						}

					}

				}
				for (int j = j_tas_konum_matis_icin - 1; j >= 0; j--){

					if (j >= 0 && j < 8){

						if (Taslar_Konumu[i_tas_konum_matis_icin][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i_tas_konum_matis_icin][j] < 7){

								Oneri_Matrisi[i_tas_konum_matis_icin][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i_tas_konum_matis_icin][j] = 1;
						}

					}
				}



				int j = j_tas_konum_matis_icin + 1;
				for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j] < 7){

								Oneri_Matrisi[i][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j] = 1;
						}

					}
					j++;
				}
				j = j_tas_konum_matis_icin - 1;
				for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j] < 7){

								Oneri_Matrisi[i][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j] = 1;
						}

					}
					j--;
				}




				j = j_tas_konum_matis_icin + 1;
				for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j] < 7){

								Oneri_Matrisi[i][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j] = 1;
						}

					}
					j++;
				}
				j = j_tas_konum_matis_icin - 1;
				for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j] < 7){

								Oneri_Matrisi[i][j] = 1;
							}
							break;

						}
						else{

							Oneri_Matrisi[i][j] = 1;
						}

					}
					j--;
				}

			}
			// SÝYAH ÞAH
			else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 11)
			{
				int i = i_tas_konum_matis_icin, j = j_tas_konum_matis_icin;

				i = i - 1;
				j = j - 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
					{
						if (Taslar_Konumu[i][j] < 7){

							Oneri_Matrisi[i][j] = 1;
						}

					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}

				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i - 1;
				j = j;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
					{
						if (Taslar_Konumu[i][j] < 7){

							Oneri_Matrisi[i][j] = 1;
						}

					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i - 1;
				j = j + 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
					{

						if (Taslar_Konumu[i][j] < 7){

							Oneri_Matrisi[i][j] = 1;
						}
					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i;
				j = j - 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
					{
						if (Taslar_Konumu[i][j] < 7){

							Oneri_Matrisi[i][j] = 1;
						}

					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i;
				j = j + 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
					{
						if (Taslar_Konumu[i][j] < 7){

							Oneri_Matrisi[i][j] = 1;
						}

					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i + 1;
				j = j - 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
					{
						if (Taslar_Konumu[i][j] < 7){

							Oneri_Matrisi[i][j] = 1;
						}

					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}



				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i + 1;
				j = j;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
					{
						if (Taslar_Konumu[i][j] < 7){

							Oneri_Matrisi[i][j] = 1;
						}

					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}


				i = i_tas_konum_matis_icin;
				j = j_tas_konum_matis_icin;
				i = i + 1;
				j = j + 1;
				if (i >= 0 && i < 8 && j >= 0 && j < 8){

					if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
					{
						if (Taslar_Konumu[i][j] < 7){

							Oneri_Matrisi[i][j] = 1;
						}

					}
					else{

						Oneri_Matrisi[i][j] = 1;
					}

				}




			}
			// SÝYAH PÝYON
			else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 12)
			{
				int i = i_tas_konum_matis_icin + 1;
				int durma = i + 1;

				for (i = i; i <= durma; i++){

					if (i >= 0 && i < 8){

						if (Taslar_Konumu[i][j_tas_konum_matis_icin] >0)// herhangi bir taþ rastlanmýþsa orda kes
						{
							break;
						}
						else{

							Oneri_Matrisi[i][j_tas_konum_matis_icin] = 1;
							if (durma == i && i_tas_konum_matis_icin != 1){
								Oneri_Matrisi[i][j_tas_konum_matis_icin] = 0;
							}
						}

					}

				}


			}



		}


		Yenilme_tehlikesi_yap();

	}

	if (mouseCurrState.rgbButtons[1] & 0x80)
	{


		std::wostringstream outs;
		outs << "þah yenilme tehlikesi :  " << sah_yenilme_durumu();
		SetWindowText(g_hWnd, outs.str().c_str());



	}

	if ((mouseCurrState.lX != mouseLastState.lX) || (mouseCurrState.lY != mouseLastState.lY))
	{
		camYaw += mouseLastState.lX * 0.001f;

		if (turn)
			camPitch += mouseCurrState.lY * 0.001f;
		else
			camPitch -= mouseCurrState.lY * 0.001f;

		mouseLastState = mouseCurrState;
	}



	UpdateCamera();

	return;
}

bool sah_yenilme_durumu(){

	int sah_numarasý = 0;
	if (hamle_sirai % 2 == 0){
		// Beyaz oyuncular oynar
		sah_numarasý = 5;
	}
	else{
		sah_numarasý = 11;
	}


	for (int i = 0; i < 8; i++){

		for (int j = 0; j < 8; j++){
			if (Yenilme_tehlikesi[i][j] == 1 && Taslar_Konumu[i][j] == sah_numarasý){
				// þah yenilme tehlikesindedir 
				return true;
			}
		}
	}

	return false;

}
void onerileri_sifirla()
{
	for (int i = 0; i < 8; i++)
	{

		for (int j = 0; j < 8; j++)
		{
			Oneri_Matrisi[i][j] = 0;

		}
	}
}
void StartTimer()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);
	countsPerSecond = double(frequencyCount.QuadPart);
	QueryPerformanceCounter(&frequencyCount);
	CounterStart = frequencyCount.QuadPart;
}

double GetTime()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	return double(currentTime.QuadPart - CounterStart) / countsPerSecond;
}

double GetFrameTime()
{
	LARGE_INTEGER currentTime;
	__int64 tickCount;
	QueryPerformanceCounter(&currentTime);

	tickCount = currentTime.QuadPart - frameTimeOld;
	frameTimeOld = currentTime.QuadPart;

	if (tickCount < 0.0f)
		tickCount = 0.0f;

	return float(tickCount) / countsPerSecond;
}

void UpdateScene(double time)
{
}
void UpdateCamera()
{
	camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0);
	if (turn)
		At = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
	else
		At = XMVector3TransformCoord(DefaultForward2, camRotationMatrix);

	At = XMVector3Normalize(At);

	XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = XMMatrixRotationY(camYaw);

	camRight = XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);
	Up = XMVector3TransformCoord(Up, RotateYTempMatrix);

	if (turn)
		camForward = XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);
	else
		camForward = XMVector3TransformCoord(DefaultForward2, RotateYTempMatrix);

	if (turn)
		Eye += moveLeftRight * camRight;
	else
		Eye -= moveLeftRight * camRight;

	Eye += moveBackForward * camForward;

	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;

	At = Eye + At;

	crosshairPosition = Eye + 0.5 * XMVector3Normalize(At - Eye);

	g_View = XMMatrixLookAtLH(Eye, At, Up);
}



//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);
	if (FAILED(hr))
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
	if (FAILED(hr))
		return hr;

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

	// Compile the vertex shader
	ID3DBlob* pVSBlob = NULL;
	hr = CompileShaderFromFile(L"D3D11_Chess.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },

	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(L"D3D11_Chess.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))	return hr;

	// Compile the pixel shader Textured
	ID3DBlob* pPStextureBlob = NULL;
	hr = CompileShaderFromFile(L"D3D11_Chess.fx", "PS_Textured", "ps_4_0", &pPStextureBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPStextureBlob->GetBufferPointer(), pPStextureBlob->GetBufferSize(), NULL, &g_pPixelShader_Textured);
	pPStextureBlob->Release();
	if (FAILED(hr))	return hr;


	// Compile the pixel shader Solid
	ID3DBlob* pPSsolidBlob = NULL;
	hr = CompileShaderFromFile(L"D3D11_Chess.fx", "PS_Solid", "ps_4_0", &pPSsolidBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSsolidBlob->GetBufferPointer(), pPSsolidBlob->GetBufferSize(), NULL, &g_pPixelShader_Solid);
	pPSsolidBlob->Release();
	if (FAILED(hr))	return hr;

	ifstream fin;

	// SATRANÇ TAHTASINI YÜKLE
	fin.open("chessBoard.txt");
	fin >> vertexCountchessBoard;
	verticeschessBoard = new SimpleVertex[vertexCountchessBoard];

	for (int i = 0; i < vertexCountchessBoard; i++)
	{
		fin >> verticeschessBoard[i].Pos.x >> verticeschessBoard[i].Pos.y >> verticeschessBoard[i].Pos.z;
		fin >> verticeschessBoard[i].Tex.x >> verticeschessBoard[i].Tex.y;
		fin >> verticeschessBoard[i].Normal.x >> verticeschessBoard[i].Normal.y >> verticeschessBoard[i].Normal.z;
	}
	fin.close();

	// Buradaki genel setlemelerin bazýlarý aþaðýdaki diðer setlemelerde kullanýlmayabilir.
	// Diðer setlemeler burdaki gibi uzun deðil, mesela piyon için bd.ByteWidth... ile  if(FAILED(hr))... arasý yani sadece 5 satýr. 
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex)* vertexCountchessBoard;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = verticeschessBoard;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_chessBoard);
	if (FAILED(hr))     return hr;
	/////////////////

	// PIYON'u YÜKLE
	fin.open("piyon.txt");
	fin >> vertexCount_piyon;
	vertices_piyon = new SimpleVertex[vertexCount_piyon];

	for (int i = 0; i < vertexCount_piyon; i++)
	{
		fin >> vertices_piyon[i].Pos.x >> vertices_piyon[i].Pos.y >> vertices_piyon[i].Pos.z;
		fin >> vertices_piyon[i].Tex.x >> vertices_piyon[i].Tex.y;
		fin >> vertices_piyon[i].Normal.x >> vertices_piyon[i].Normal.y >> vertices_piyon[i].Normal.z;
	}
	fin.close();

	bd.ByteWidth = sizeof(SimpleVertex)* vertexCount_piyon;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices_piyon;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_piyon);
	if (FAILED(hr))     return hr;
	/////////////////



	// KALE YÜKLE
	fin.open("kale.txt");
	fin >> vertexCount_kale;
	vertices_kale = new SimpleVertex[vertexCount_kale];

	for (int i = 0; i < vertexCount_kale; i++)
	{
		fin >> vertices_kale[i].Pos.x >> vertices_kale[i].Pos.y >> vertices_kale[i].Pos.z;
		fin >> vertices_kale[i].Tex.x >> vertices_kale[i].Tex.y;
		fin >> vertices_kale[i].Normal.x >> vertices_kale[i].Normal.y >> vertices_kale[i].Normal.z;
	}
	fin.close();

	bd.ByteWidth = sizeof(SimpleVertex)* vertexCount_kale;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices_kale;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_kale);
	if (FAILED(hr))     return hr;
	/////////////////


	// AT YÜKLE
	fin.open("at.txt");
	fin >> vertexCount_at;
	vertices_at = new SimpleVertex[vertexCount_at];

	for (int i = 0; i < vertexCount_at; i++)
	{
		fin >> vertices_at[i].Pos.x >> vertices_at[i].Pos.y >> vertices_at[i].Pos.z;
		fin >> vertices_at[i].Tex.x >> vertices_at[i].Tex.y;
		fin >> vertices_at[i].Normal.x >> vertices_at[i].Normal.y >> vertices_at[i].Normal.z;
	}
	fin.close();

	bd.ByteWidth = sizeof(SimpleVertex)* vertexCount_at;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices_at;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_at);
	if (FAILED(hr))     return hr;
	/////////////////



	// FÝL YÜKLE
	fin.open("fil.txt");
	fin >> vertexCount_fil;
	vertices_fil = new SimpleVertex[vertexCount_fil];

	for (int i = 0; i < vertexCount_fil; i++)
	{
		fin >> vertices_fil[i].Pos.x >> vertices_fil[i].Pos.y >> vertices_fil[i].Pos.z;
		fin >> vertices_fil[i].Tex.x >> vertices_fil[i].Tex.y;
		fin >> vertices_fil[i].Normal.x >> vertices_fil[i].Normal.y >> vertices_fil[i].Normal.z;
	}
	fin.close();

	bd.ByteWidth = sizeof(SimpleVertex)* vertexCount_fil;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices_fil;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_fil);
	if (FAILED(hr))     return hr;
	/////////////////


	// VEZÝR YÜKLE
	fin.open("vezir.txt");
	fin >> vertexCount_vezir;
	vertices_vezir = new SimpleVertex[vertexCount_vezir];

	for (int i = 0; i < vertexCount_vezir; i++)
	{
		fin >> vertices_vezir[i].Pos.x >> vertices_vezir[i].Pos.y >> vertices_vezir[i].Pos.z;
		fin >> vertices_vezir[i].Tex.x >> vertices_vezir[i].Tex.y;
		fin >> vertices_vezir[i].Normal.x >> vertices_vezir[i].Normal.y >> vertices_vezir[i].Normal.z;
	}
	fin.close();

	bd.ByteWidth = sizeof(SimpleVertex)* vertexCount_vezir;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices_vezir;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_vezir);
	if (FAILED(hr))     return hr;
	/////////////////



	// SAH YÜKLE
	fin.open("sah.txt");
	fin >> vertexCount_sah;
	vertices_sah = new SimpleVertex[vertexCount_sah];

	for (int i = 0; i < vertexCount_sah; i++)
	{
		fin >> vertices_sah[i].Pos.x >> vertices_sah[i].Pos.y >> vertices_sah[i].Pos.z;
		fin >> vertices_sah[i].Tex.x >> vertices_sah[i].Tex.y;
		fin >> vertices_sah[i].Normal.x >> vertices_sah[i].Normal.y >> vertices_sah[i].Normal.z;
	}
	fin.close();

	bd.ByteWidth = sizeof(SimpleVertex)* vertexCount_sah;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices_sah;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_sah);
	if (FAILED(hr))     return hr;
	/////////////////





	fin.open("Light.txt");
	int vertexCountlight;
	fin >> vertexCountlight;
	verticesLight = new SimpleVertex[vertexCountlight];

	for (int i = 0; i < vertexCountlight; i++)
	{
		fin >> verticesLight[i].Pos.x >> verticesLight[i].Pos.y >> verticesLight[i].Pos.z;
		fin >> verticesLight[i].Tex.x >> verticesLight[i].Tex.y;
		fin >> verticesLight[i].Normal.x >> verticesLight[i].Normal.y >> verticesLight[i].Normal.z;
	}

	fin.close();

	bd.ByteWidth = sizeof(SimpleVertex)* vertexCountlight;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = verticesLight;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_Light);
	if (FAILED(hr))     return hr;


	fin.open("pointer.txt");

	fin >> vertexCountpointer;
	verticespointer = new SimpleVertex[vertexCountpointer];

	for (int i = 0; i < vertexCountpointer; i++)
	{
		fin >> verticespointer[i].Pos.x >> verticespointer[i].Pos.y >> verticespointer[i].Pos.z;
		fin >> verticespointer[i].Tex.x >> verticespointer[i].Tex.y;
		fin >> verticespointer[i].Normal.x >> verticespointer[i].Normal.y >> verticespointer[i].Normal.z;
	}

	fin.close();

	bd.ByteWidth = sizeof(SimpleVertex)* vertexCountpointer;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = verticespointer;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_Pointer);
	if (FAILED(hr))     return hr;



	fin.open("CrossHair.txt");

	fin >> vertexCountcrosshair;
	verticesCrossHair = new SimpleVertex[vertexCountcrosshair];

	for (int i = 0; i < vertexCountcrosshair; i++)
	{
		fin >> verticesCrossHair[i].Pos.x >> verticesCrossHair[i].Pos.y >> verticesCrossHair[i].Pos.z;
		fin >> verticesCrossHair[i].Tex.x >> verticesCrossHair[i].Tex.y;
		fin >> verticesCrossHair[i].Normal.x >> verticesCrossHair[i].Normal.y >> verticesCrossHair[i].Normal.z;
	}

	fin.close();

	bd.ByteWidth = sizeof(SimpleVertex)* vertexCountcrosshair;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = verticesCrossHair;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_CrossHair);
	if (FAILED(hr))     return hr;

	// Set primitive topology
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	// Create the constant buffers
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pConstantBuffer);
	if (FAILED(hr))	return hr;




	// Load the Texture pointer White  oneri
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_White_oneri.jpg", NULL, NULL, &g_pTexturepointerOneri_White, NULL);
	if (FAILED(hr))	return hr;

	// Load the Texture pointer Black 0neri
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_Black_oneri.jpg", NULL, NULL, &g_pTexturepointerOneri_Black, NULL);
	if (FAILED(hr))	return hr;


	// Load the Texture pointer White
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_White.jpg", NULL, NULL, &g_pTexturepointerWhite, NULL);
	if (FAILED(hr))	return hr;

	// Load the Texture pointer Black
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_Black.jpg", NULL, NULL, &g_pTexturepointerBlack, NULL);
	if (FAILED(hr))	return hr;

	// Load the Texture chessBoard
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"chessBoard.jpg", NULL, NULL, &g_pTexturechessBoard, NULL);
	if (FAILED(hr))	return hr;

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
	if (FAILED(hr))	return hr;

	// Initialize the world matrices
	g_World = XMMatrixIdentity();

	// Initialize the view matrix
	if (turn)
		Eye = XMVectorSet(0.0f, 30.0f, -50.0f, 0.0f);
	else
		Eye = XMVectorSet(0.0f, 30.0f, 50.0f, 0.0f);

	At = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	g_View = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 1000.0f);

	cBuffer.mProjection = XMMatrixTranspose(g_Projection);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0);


	Ro = Eye;
	Rd = XMVector3Normalize(At - Eye);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
	if (g_pImmediateContext)		g_pImmediateContext->ClearState();
	if (g_pSamplerLinear)			g_pSamplerLinear->Release();
	if (g_pTexturepiyon)			g_pTexturepiyon->Release();
	if (g_pTexturechessBoard)		g_pTexturechessBoard->Release();
	if (g_pConstantBuffer)			g_pConstantBuffer->Release();
	if (g_pVertexBuffer_CrossHair) g_pVertexBuffer_CrossHair->Release();
	if (g_pVertexBuffer_piyon)	g_pVertexBuffer_piyon->Release();
	if (g_pVertexBuffer_Light)		g_pVertexBuffer_Light->Release();
	if (g_pIndexBuffer)			g_pIndexBuffer->Release();
	if (g_pVertexLayout)			g_pVertexLayout->Release();
	if (g_pVertexShader)			g_pVertexShader->Release();
	if (g_pPixelShader)			g_pPixelShader->Release();
	if (g_pPixelShader)			g_pPixelShader_Solid->Release();
	if (g_pDepthStencil)			g_pDepthStencil->Release();
	if (g_pDepthStencilView)		g_pDepthStencilView->Release();
	if (g_pRenderTargetView)		g_pRenderTargetView->Release();
	if (g_pSwapChain)				g_pSwapChain->Release();
	if (g_pImmediateContext)		g_pImmediateContext->Release();
	if (g_pd3dDevice)				g_pd3dDevice->Release();
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render(double time)
{
	// Update our time
	rot += 1.0f * time;
	if (rot > 6.28f) rot = 0.0f;

	Ro = Eye;
	Rd = XMVector3Normalize(At - Eye);


	// Clear the back buffer
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);

	// Clear the depth buffer to 1.0 (max depth)
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


	cBuffer.mView = XMMatrixTranspose(g_View);
	XMStoreFloat4(&cBuffer.vEyePos, Eye);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0);

	XMFLOAT4 vLightPos[2] =
	{
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)
	};

	XMFLOAT4 vLightColors[2] =
	{
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)
	};

	cBuffer.vLightColor[0] = vLightColors[0];
	cBuffer.vLightColor[1] = vLightColors[1];
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_Light, &stride, &offset);

	XMMATRIX mRotate;
	XMMATRIX mTranslate1, mTranslate2, mTranslate3;

	mTranslate1 = XMMatrixTranslation(-5.0f, 0.0f, 0.0f);
	mTranslate2 = XMMatrixTranslation(-10.0f, 20.0f, 10.0f);
	mRotate = XMMatrixRotationY(rot);

	XMVECTOR vLightPos0 = XMLoadFloat4(&vLightPos[0]);
	vLightPos0 = XMVector3Transform(vLightPos0, mTranslate1);
	vLightPos0 = XMVector3Transform(vLightPos0, mRotate);
	vLightPos0 = XMVector3Transform(vLightPos0, mTranslate2);
	XMStoreFloat4(&vLightPos[0], vLightPos0);

	cBuffer.mWorld = XMMatrixTranspose(mTranslate1 * mRotate * mTranslate2);
	cBuffer.vLightPos[0] = vLightPos[0];
	cBuffer.vMeshColor = vLightColors[0];
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0);

	// Render White Light
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetShader(g_pPixelShader_Solid, NULL, 0);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->Draw(vertexCountlight, 0);

	mTranslate1 = XMMatrixTranslation(5.0f, 0.0f, 0.0f);
	mTranslate3 = XMMatrixTranslation(10.0f, 20.0f, -10.0f);

	XMVECTOR vLightPos1 = XMLoadFloat4(&vLightPos[1]);
	vLightPos1 = XMVector3Transform(vLightPos1, mTranslate1);
	vLightPos1 = XMVector3Transform(vLightPos1, mRotate);
	vLightPos1 = XMVector3Transform(vLightPos1, mTranslate3);
	XMStoreFloat4(&vLightPos[1], vLightPos1);

	cBuffer.mWorld = XMMatrixTranspose(mTranslate1 * mRotate * mTranslate3);
	cBuffer.vLightPos[1] = vLightPos[1];
	cBuffer.vMeshColor = vLightColors[1];
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0);

	// Render Yellow Light
	g_pImmediateContext->Draw(vertexCountlight, 0);


	// Render CrossHair
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_CrossHair, &stride, &offset);
	XMFLOAT4 sPosition;
	XMStoreFloat4(&sPosition, crosshairPosition);
	XMMATRIX mTranslate = XMMatrixTranslation(sPosition.x, sPosition.y, sPosition.z);
	cBuffer.mWorld = XMMatrixTranspose(mTranslate);
	cBuffer.vMeshColor = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0);
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetShader(g_pPixelShader_Solid, NULL, 0);
	g_pImmediateContext->Draw(vertexCountcrosshair, 0);

	g_World = XMMatrixIdentity();
	cBuffer.mWorld = XMMatrixTranspose(g_World);


	// Render ChessBoard
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_chessBoard, &stride, &offset);
	cBuffer.vMeshColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0);
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetShader(g_pPixelShader_Textured, NULL, 0);
	g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturechessBoard);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->Draw(vertexCountchessBoard, 0);


	// Render Pointer
	XMVECTOR translatePointer = testIntersection(Ro, Rd, verticeschessBoard, vertexCountchessBoard, false, cBuffer.mWorld);

	XMStoreFloat3(&translateP, translatePointer);
	if (translateP.x != 0.0F)
	{
		int x, y;

		x = (translateP.x + 20) / 5.0f;
		y = (translateP.z + 20) / 5.0f;

		if (Black_or_White[x][y] == 1)
			g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerWhite);
		else
			g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerBlack);

		XMMATRIX mTranslatePointer = XMMatrixTranslation(translateP.x, 0, translateP.z);
		cBuffer.mWorld = XMMatrixTranspose(mTranslatePointer);
		g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_Pointer, &stride, &offset);
		cBuffer.vMeshColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0);
		g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
		g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pConstantBuffer);
		g_pImmediateContext->PSSetShader(g_pPixelShader_Textured, NULL, 0);
		g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pConstantBuffer);
		g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
		g_pImmediateContext->Draw(vertexCountpointer, 0);
	}

	double X, Z;

	Z = 17.5F; // i --
	for (int i = 0; i < 8; i++){

		X = -17.5F; // j ++
		for (int j = 0; j < 8; j++){



			// BEYAZ KALE
			if (Taslar_Konumu[i][j] == 1)
			{
				tas_render(g_pVertexBuffer_kale, vertexCount_kale, stride, offset, 1.0f, 1.0f - Yenilme_tehlikesi[i][j], 1.0f - Yenilme_tehlikesi[i][j], 1.0f, X, 0, Z);
			}
			// BEYAZ AT
			else if (Taslar_Konumu[i][j] == 2)
			{
				tas_render(g_pVertexBuffer_at, vertexCount_at, stride, offset, 1.0f, 1.0f - Yenilme_tehlikesi[i][j], 1.0f - Yenilme_tehlikesi[i][j], 1.0f, X, 0, Z);
			}
			// BEYAZ FÝL
			else if (Taslar_Konumu[i][j] == 3)
			{
				tas_render(g_pVertexBuffer_fil, vertexCount_fil, stride, offset, 1.0f, 1.0f - Yenilme_tehlikesi[i][j], 1.0f - Yenilme_tehlikesi[i][j], 1.0f, X, 0, Z);
			}
			//BEYAZ VEZÝR
			else if (Taslar_Konumu[i][j] == 4)
			{
				tas_render(g_pVertexBuffer_vezir, vertexCount_vezir, stride, offset, 1.0f, 1.0f - Yenilme_tehlikesi[i][j], 1.0f - Yenilme_tehlikesi[i][j], 1.0f, X, 0, Z);
			}
			// BEYAZ ÞAH
			else if (Taslar_Konumu[i][j] == 5)
			{
				tas_render(g_pVertexBuffer_sah, vertexCount_sah, stride, offset, 1.0f, 1.0f - Yenilme_tehlikesi[i][j], 1.0f - Yenilme_tehlikesi[i][j], 1.0f, X, 0, Z);
			}
			// BEYAZ PÝYON
			else if (Taslar_Konumu[i][j] == 6)
			{
				tas_render(g_pVertexBuffer_piyon, vertexCount_piyon, stride, offset, 1.0f, 1.0f - Yenilme_tehlikesi[i][j], 1.0f - Yenilme_tehlikesi[i][j], 1.0f, X, 0, Z);
			}
			//SÝYAH KALE
			else if (Taslar_Konumu[i][j] == 7)
			{
				tas_render(g_pVertexBuffer_kale, vertexCount_kale, stride, offset, Yenilme_tehlikesi[i][j] + 0.0f, 0.0f, 0.0f, 1.0f, X, 0, Z);
			}
			//SÝYAH AT
			else if (Taslar_Konumu[i][j] == 8)
			{
				tas_render(g_pVertexBuffer_at, vertexCount_at, stride, offset, Yenilme_tehlikesi[i][j] + 0.0f, 0.0f, 0.0f, 1.0f, X, 0, Z);
			}
			//SÝYAH FÝL
			else if (Taslar_Konumu[i][j] == 9)
			{
				tas_render(g_pVertexBuffer_fil, vertexCount_fil, stride, offset, Yenilme_tehlikesi[i][j] + 0.0f, 0.0f, 0.0f, 1.0f, X, 0, Z);
			}
			//SÝYAH VEZÝR
			else if (Taslar_Konumu[i][j] == 10)
			{
				tas_render(g_pVertexBuffer_vezir, vertexCount_vezir, stride, offset, Yenilme_tehlikesi[i][j] + 0.0f, 0.0f, 0.0f, 1.0f, X, 0, Z);
			}
			//SÝYAH ÞAH
			else if (Taslar_Konumu[i][j] == 11)
			{
				tas_render(g_pVertexBuffer_sah, vertexCount_sah, stride, offset, Yenilme_tehlikesi[i][j] + 0.0f, 0.0f, 0.0f, 1.0f, X, 0, Z);
			}
			//SÝYAH PÝYON
			else if (Taslar_Konumu[i][j] == 12)
			{
				tas_render(g_pVertexBuffer_piyon, vertexCount_piyon, stride, offset, Yenilme_tehlikesi[i][j] + 0.0f, 0.0f, 0.0f, 1.0f, X, 0, Z);
			}


			if (Taslar_Konumu[i][j] == Secilen_Tas_Translate_Olucak && Secilen_Tas_Translate_Olucak != 0 && i_tas_konum_matis_icin == i && j_tas_konum_matis_icin == j){

				// aþaðýdaki ifde sadece kendi taþlarýna pointer verebilir oyuncu
				if (hamle_sirai % 2 == 0 && Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] < 7 && Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] >= 0 || hamle_sirai % 2 == 1 && Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] > 6){

					// secilen taþýn altýnda çember
					XMVECTOR translatePointer = testIntersection(Ro, Rd, verticeschessBoard, vertexCountchessBoard, false, cBuffer.mWorld);

					XMStoreFloat3(&translateP, translatePointer);

					if (Black_or_White[i][j] == 0)
						g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerWhite);
					else
						g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerBlack);

					XMMATRIX mTranslatePointer = XMMatrixTranslation(X, 0, Z);
					cBuffer.mWorld = XMMatrixTranspose(mTranslatePointer);
					g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_Pointer, &stride, &offset);
					cBuffer.vMeshColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
					g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0);
					g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
					g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pConstantBuffer);
					g_pImmediateContext->PSSetShader(g_pPixelShader_Textured, NULL, 0);
					g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pConstantBuffer);
					g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
					g_pImmediateContext->Draw(vertexCountpointer, 0);
				}

			}

			//öneriler render ediliyor
			if (Oneri_Matrisi[i][j] == 1)
			{
				pointer_oneri_render(stride, offset, X, Z, Black_or_White[i][j]);

			}


			X = X + 5.0F;
		}

		Z = Z - 5.0F;

	}

	//yenen taþlar için
	Z = 17.5F; // i --
	X = -22.5F; // j

	for (int i = 0; i < 16; i++){

		if (i == 8){ Z = 17.5F; X = -27.5F; }

		// BEYAZ KALE
		if (Yenen_beyaz_taslar[i] == 1)
		{
			tas_render(g_pVertexBuffer_kale, vertexCount_kale, stride, offset, 1.0f, 1.0f, 1.0f, 1.0f, -1 * X, 0, -1 * Z);
		}
		// BEYAZ AT
		else if (Yenen_beyaz_taslar[i] == 2)
		{
			tas_render(g_pVertexBuffer_at, vertexCount_at, stride, offset, 1.0f, 1.0f, 1.0f, 1.0f, -1 * X, 0, -1 * Z);
		}
		// BEYAZ FÝL
		else if (Yenen_beyaz_taslar[i] == 3)
		{
			tas_render(g_pVertexBuffer_fil, vertexCount_fil, stride, offset, 1.0f, 1.0f, 1.0f, 1.0f, -1 * X, 0, -1 * Z);
		}
		//BEYAZ VEZÝR
		else if (Yenen_beyaz_taslar[i] == 4)
		{
			tas_render(g_pVertexBuffer_vezir, vertexCount_vezir, stride, offset, 1.0f, 1.0f, 1.0f, 1.0f, -1 * X, 0, -1 * Z);
		}
		// BEYAZ ÞAH
		else if (Yenen_beyaz_taslar[i] == 5)
		{
			tas_render(g_pVertexBuffer_sah, vertexCount_sah, stride, offset, 1.0f, 1.0f, 1.0f, 1.0f, -1 * X, 0, -1 * Z);
		}
		// BEYAZ PÝYON
		else if (Yenen_beyaz_taslar[i] == 6)
		{
			tas_render(g_pVertexBuffer_piyon, vertexCount_piyon, stride, offset, 1.0f, 1.0f, 1.0f, 1.0f, -1 * X, 0, -1 * Z);
		}



		//SÝYAH KALE
		if (Yenen_siyah_taslar[i] == 7)
		{
			tas_render(g_pVertexBuffer_kale, vertexCount_kale, stride, offset, 0.0f, 0.0f, 0.0f, 1.0f, X, 0, Z);
		}
		//SÝYAH AT
		else if (Yenen_siyah_taslar[i] == 8)
		{
			tas_render(g_pVertexBuffer_at, vertexCount_at, stride, offset, 0.0f, 0.0f, 0.0f, 1.0f, X, 0, Z);
		}
		//SÝYAH FÝL
		else if (Yenen_siyah_taslar[i] == 9)
		{
			tas_render(g_pVertexBuffer_fil, vertexCount_fil, stride, offset, 0.0f, 0.0f, 0.0f, 1.0f, X, 0, Z);
		}
		//SÝYAH VEZÝR
		else if (Yenen_siyah_taslar[i] == 10)
		{
			tas_render(g_pVertexBuffer_vezir, vertexCount_vezir, stride, offset, 0.0f, 0.0f, 0.0f, 1.0f, X, 0, Z);
		}
		//SÝYAH ÞAH
		else if (Yenen_siyah_taslar[i] == 11)
		{
			tas_render(g_pVertexBuffer_sah, vertexCount_sah, stride, offset, 0.0f, 0.0f, 0.0f, 1.0f, X, 0, Z);
		}
		//SÝYAH PÝYON
		else if (Yenen_siyah_taslar[i] == 12)
		{
			tas_render(g_pVertexBuffer_piyon, vertexCount_piyon, stride, offset, 0.0f, 0.0f, 0.0f, 1.0f, X, 0, Z);
		}



		Z = Z - 5.0F;


	}








	// Print
	/*std::wostringstream outs;
	outs << " Cell Position = (" << translateP.x << ", " << translateP.y << ", " << translateP.z << ")";
	SetWindowText(g_hWnd, outs.str().c_str());*/

	//UINT stride = sizeof(SimpleVertex);
	////UINT offset = 0;
	//pointer_oneri_render(stride, offset, 2.5F, 2.5F, 1);





	// Present our back buffer to our front buffer
	g_pSwapChain->Present(0, 0);



}

void Yenilme_tehlikesi_yap(){

	for (int i_tas_konum_matis_icin = 0; i_tas_konum_matis_icin < 8; i_tas_konum_matis_icin++)
	{
		for (int j_tas_konum_matis_icin = 0; j_tas_konum_matis_icin < 8; j_tas_konum_matis_icin++)
		{
			Yenilme_tehlikesi[i_tas_konum_matis_icin][j_tas_konum_matis_icin] = 0;
		}
	}


	for (int i_tas_konum_matis_icin = 0; i_tas_konum_matis_icin < 8; i_tas_konum_matis_icin++)
	{
		for (int j_tas_konum_matis_icin = 0; j_tas_konum_matis_icin < 8; j_tas_konum_matis_icin++)
		{

			if (hamle_sirai % 2 != 0){
				// BEYAZ KALE
				if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 1)
				{

					/*Yenilme_tehlikesi[i_tas_konum_matis_icin][j_tas_konum_matis_icin] = 1;*/
					for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

						if (i >= 0 && i < 8){

							if (Taslar_Konumu[i][j_tas_konum_matis_icin]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
							{
								//bur
								if (Taslar_Konumu[i][j_tas_konum_matis_icin]>6){
									Yenilme_tehlikesi[i][j_tas_konum_matis_icin] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j_tas_konum_matis_icin] = 1;
							}

						}

					}
					for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

						if (i >= 0 && i < 8){

							if (Taslar_Konumu[i][j_tas_konum_matis_icin]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j_tas_konum_matis_icin] > 6){
									Yenilme_tehlikesi[i][j_tas_konum_matis_icin] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j_tas_konum_matis_icin] = 1;
							}

						}
					}

					for (int j = j_tas_konum_matis_icin + 1; j < 8; j++){

						if (j >= 0 && j < 8){

							if (Taslar_Konumu[i_tas_konum_matis_icin][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i_tas_konum_matis_icin][j]>6){
									Yenilme_tehlikesi[i_tas_konum_matis_icin][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i_tas_konum_matis_icin][j] = 1;
							}

						}

					}
					for (int j = j_tas_konum_matis_icin - 1; j >= 0; j--){

						if (j >= 0 && j < 8){

							if (Taslar_Konumu[i_tas_konum_matis_icin][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i_tas_konum_matis_icin][j] > 6){
									Yenilme_tehlikesi[i_tas_konum_matis_icin][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i_tas_konum_matis_icin][j] = 1;
							}

						}
					}



				}
				// BEYAZ AT
				else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 2)
				{

					int i = i_tas_konum_matis_icin, j = j_tas_konum_matis_icin;

					i = i - 2;
					j = j + 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j] < 7 && Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}

					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i - 2;
					j = j - 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j] < 7 && Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i - 1;
					j = j - 2;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j] < 7 && Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i - 1;
					j = j + 2;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j] < 7 && Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i + 1;
					j = j - 2;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j] < 7 && Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i + 1;
					j = j + 2;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j] < 7 && Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i + 2;
					j = j + 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j] < 7 && Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}


					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i + 2;
					j = j - 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j] < 7 && Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}


				}
				// BEYAZ FÝL
				else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 3)
				{
					int j = j_tas_konum_matis_icin + 1;
					for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

						if (i >= 0 && i < 8 && j >= 0 && j < 8){

							if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j]>6){
									Yenilme_tehlikesi[i][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						j++;
					}
					j = j_tas_konum_matis_icin - 1;
					for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

						if (i >= 0 && i < 8 && j >= 0 && j < 8){

							if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j]>6){
									Yenilme_tehlikesi[i][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						j--;
					}




					j = j_tas_konum_matis_icin + 1;
					for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

						if (i >= 0 && i < 8 && j >= 0 && j < 8){

							if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j]>6){
									Yenilme_tehlikesi[i][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						j++;
					}
					j = j_tas_konum_matis_icin - 1;
					for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

						if (i >= 0 && i < 8 && j >= 0 && j < 8){

							if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j]>6){
									Yenilme_tehlikesi[i][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						j--;
					}





				}
				//BEYAZ VEZÝR
				else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 4)
				{
					/*Yenilme_tehlikesi[i_tas_konum_matis_icin][j_tas_konum_matis_icin] = 1;*/
					for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

						if (i >= 0 && i < 8){

							if (Taslar_Konumu[i][j_tas_konum_matis_icin]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j_tas_konum_matis_icin]>6){
									Yenilme_tehlikesi[i][j_tas_konum_matis_icin] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j_tas_konum_matis_icin] = 1;
							}

						}

					}
					for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

						if (i >= 0 && i < 8){

							if (Taslar_Konumu[i][j_tas_konum_matis_icin]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j_tas_konum_matis_icin] > 6){
									Yenilme_tehlikesi[i][j_tas_konum_matis_icin] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j_tas_konum_matis_icin] = 1;
							}

						}
					}

					for (int j = j_tas_konum_matis_icin + 1; j < 8; j++){

						if (j >= 0 && j < 8){

							if (Taslar_Konumu[i_tas_konum_matis_icin][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i_tas_konum_matis_icin][j]>6){
									Yenilme_tehlikesi[i_tas_konum_matis_icin][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i_tas_konum_matis_icin][j] = 1;
							}

						}

					}
					for (int j = j_tas_konum_matis_icin - 1; j >= 0; j--){

						if (j >= 0 && j < 8){

							if (Taslar_Konumu[i_tas_konum_matis_icin][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i_tas_konum_matis_icin][j] > 6){
									Yenilme_tehlikesi[i_tas_konum_matis_icin][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i_tas_konum_matis_icin][j] = 1;
							}

						}
					}



					int j = j_tas_konum_matis_icin + 1;
					for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

						if (i >= 0 && i < 8 && j >= 0 && j < 8){

							if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j]>6){
									Yenilme_tehlikesi[i][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						j++;
					}
					j = j_tas_konum_matis_icin - 1;
					for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

						if (i >= 0 && i < 8 && j >= 0 && j < 8){

							if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j]>6){
									Yenilme_tehlikesi[i][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						j--;
					}




					j = j_tas_konum_matis_icin + 1;
					for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

						if (i >= 0 && i < 8 && j >= 0 && j < 8){

							if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j]>6){
									Yenilme_tehlikesi[i][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						j++;
					}
					j = j_tas_konum_matis_icin - 1;
					for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

						if (i >= 0 && i < 8 && j >= 0 && j < 8){

							if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j]>6){
									Yenilme_tehlikesi[i][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						j--;
					}

				}
				// BEYAZ ÞAH
				else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 5)
				{
					int i = i_tas_konum_matis_icin, j = j_tas_konum_matis_icin;

					i = i - 1;
					j = j - 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j]>6){
								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}

					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i - 1;
					j = j;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j]>6){
								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i - 1;
					j = j + 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j]>6){
								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i;
					j = j - 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j]>6){
								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i;
					j = j + 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j]>6){
								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i + 1;
					j = j - 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j]>6){
								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i + 1;
					j = j;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{

							if (Taslar_Konumu[i][j]>6){
								Yenilme_tehlikesi[i][j] = 1;
							}
						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}


					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i + 1;
					j = j + 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j]>6){
								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}




				}
				// BEYAZ PÝYON
				else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 6)
				{
					int i = i_tas_konum_matis_icin - 1;
					int jsol = j_tas_konum_matis_icin - 1;
					int jsag = j_tas_konum_matis_icin + 1;
					if (i >= 0 && i < 8 && jsag >= 0 && jsag < 8 && jsol >= 0 && jsol < 8){
						if (Taslar_Konumu[i][jsol] < 7 && Taslar_Konumu[i][jsol]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
						}
						else{
							Yenilme_tehlikesi[i][jsol] = 1;
						}


						if (Taslar_Konumu[i][jsag] < 7 && Taslar_Konumu[i][jsag]>0)// herhangi bir beyaz taþ rastlanmýþsa orda kes
						{
						}
						else{
							Yenilme_tehlikesi[i][jsag] = 1;
						}
					}

				}



			}
			else{

				// SÝYAH KALE
				if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 7)
				{

					for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

						if (i >= 0 && i < 8){

							if (Taslar_Konumu[i][j_tas_konum_matis_icin] >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j_tas_konum_matis_icin] < 7){

									Yenilme_tehlikesi[i][j_tas_konum_matis_icin] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j_tas_konum_matis_icin] = 1;
							}

						}

					}
					for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

						if (i >= 0 && i < 8){

							if (Taslar_Konumu[i][j_tas_konum_matis_icin]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j_tas_konum_matis_icin] < 7){

									Yenilme_tehlikesi[i][j_tas_konum_matis_icin] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j_tas_konum_matis_icin] = 1;
							}

						}
					}

					for (int j = j_tas_konum_matis_icin + 1; j < 8; j++){

						if (j >= 0 && j < 8){

							if (Taslar_Konumu[i_tas_konum_matis_icin][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i_tas_konum_matis_icin][j] < 7){

									Yenilme_tehlikesi[i_tas_konum_matis_icin][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i_tas_konum_matis_icin][j] = 1;
							}

						}

					}
					for (int j = j_tas_konum_matis_icin - 1; j >= 0; j--){

						if (j >= 0 && j < 8){

							if (Taslar_Konumu[i_tas_konum_matis_icin][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
							{

								if (Taslar_Konumu[i_tas_konum_matis_icin][j] < 7){

									Yenilme_tehlikesi[i_tas_konum_matis_icin][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i_tas_konum_matis_icin][j] = 1;
							}

						}
					}



				}
				// SÝYAH AT
				else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 8)
				{

					int i = i_tas_konum_matis_icin, j = j_tas_konum_matis_icin;

					i = i - 2;
					j = j + 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j] >6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}

					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i - 2;
					j = j - 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j] >6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i - 1;
					j = j - 2;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j] >6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i - 1;
					j = j + 2;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j] >6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i + 1;
					j = j - 2;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j] >6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i + 1;
					j = j + 2;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j] >6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i + 2;
					j = j + 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j] >6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}


					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i + 2;
					j = j - 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j] >6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}


				}
				// SÝYAH FÝL
				else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 9)
				{
					int j = j_tas_konum_matis_icin + 1;
					for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

						if (i >= 0 && i < 8 && j >= 0 && j < 8){

							if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j] < 7){

									Yenilme_tehlikesi[i][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						j++;
					}
					j = j_tas_konum_matis_icin - 1;
					for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

						if (i >= 0 && i < 8 && j >= 0 && j < 8){

							if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j] < 7){

									Yenilme_tehlikesi[i][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						j--;
					}




					j = j_tas_konum_matis_icin + 1;
					for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

						if (i >= 0 && i < 8 && j >= 0 && j < 8){

							if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j] < 7){

									Yenilme_tehlikesi[i][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						j++;
					}
					j = j_tas_konum_matis_icin - 1;
					for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

						if (i >= 0 && i < 8 && j >= 0 && j < 8){

							if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j] < 7){

									Yenilme_tehlikesi[i][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						j--;
					}





				}
				//SÝYAH VEZÝR
				else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 10)
				{
					/*Yenilme_tehlikesi[i_tas_konum_matis_icin][j_tas_konum_matis_icin] = 1;*/
					for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

						if (i >= 0 && i < 8){

							if (Taslar_Konumu[i][j_tas_konum_matis_icin]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j_tas_konum_matis_icin] < 7){

									Yenilme_tehlikesi[i][j_tas_konum_matis_icin] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j_tas_konum_matis_icin] = 1;
							}

						}

					}
					for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

						if (i >= 0 && i < 8){

							if (Taslar_Konumu[i][j_tas_konum_matis_icin] >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j_tas_konum_matis_icin] < 7){

									Yenilme_tehlikesi[i][j_tas_konum_matis_icin] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j_tas_konum_matis_icin] = 1;
							}

						}
					}

					for (int j = j_tas_konum_matis_icin + 1; j < 8; j++){

						if (j >= 0 && j < 8){

							if (Taslar_Konumu[i_tas_konum_matis_icin][j] >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i_tas_konum_matis_icin][j] < 7){

									Yenilme_tehlikesi[i_tas_konum_matis_icin][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i_tas_konum_matis_icin][j] = 1;
							}

						}

					}
					for (int j = j_tas_konum_matis_icin - 1; j >= 0; j--){

						if (j >= 0 && j < 8){

							if (Taslar_Konumu[i_tas_konum_matis_icin][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i_tas_konum_matis_icin][j] < 7){

									Yenilme_tehlikesi[i_tas_konum_matis_icin][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i_tas_konum_matis_icin][j] = 1;
							}

						}
					}



					int j = j_tas_konum_matis_icin + 1;
					for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

						if (i >= 0 && i < 8 && j >= 0 && j < 8){

							if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j] < 7){

									Yenilme_tehlikesi[i][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						j++;
					}
					j = j_tas_konum_matis_icin - 1;
					for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

						if (i >= 0 && i < 8 && j >= 0 && j < 8){

							if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j] < 7){

									Yenilme_tehlikesi[i][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						j--;
					}




					j = j_tas_konum_matis_icin + 1;
					for (int i = i_tas_konum_matis_icin - 1; i >= 0; i--){

						if (i >= 0 && i < 8 && j >= 0 && j < 8){

							if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j] < 7){

									Yenilme_tehlikesi[i][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						j++;
					}
					j = j_tas_konum_matis_icin - 1;
					for (int i = i_tas_konum_matis_icin + 1; i < 8; i++){

						if (i >= 0 && i < 8 && j >= 0 && j < 8){

							if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
							{
								if (Taslar_Konumu[i][j] < 7){

									Yenilme_tehlikesi[i][j] = 1;
								}
								break;

							}
							else{

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						j--;
					}

				}
				// SÝYAH ÞAH
				else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 11)
				{
					int i = i_tas_konum_matis_icin, j = j_tas_konum_matis_icin;

					i = i - 1;
					j = j - 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j] < 7){

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}

					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i - 1;
					j = j;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{

							if (Taslar_Konumu[i][j] < 7){

								Yenilme_tehlikesi[i][j] = 1;
							}
						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i - 1;
					j = j + 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j] < 7){

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i;
					j = j - 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j] < 7){

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i;
					j = j + 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j] < 7){

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i + 1;
					j = j - 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j] < 7){

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}



					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i + 1;
					j = j;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j] < 7){

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}


					i = i_tas_konum_matis_icin;
					j = j_tas_konum_matis_icin;
					i = i + 1;
					j = j + 1;
					if (i >= 0 && i < 8 && j >= 0 && j < 8){

						if (Taslar_Konumu[i][j]  >0)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{
							if (Taslar_Konumu[i][j] < 7){

								Yenilme_tehlikesi[i][j] = 1;
							}

						}
						else{

							Yenilme_tehlikesi[i][j] = 1;
						}

					}




				}
				// SÝYAH PÝYON
				else if (Taslar_Konumu[i_tas_konum_matis_icin][j_tas_konum_matis_icin] == 12)
				{
					int i = i_tas_konum_matis_icin + 1;
					int jsol = j_tas_konum_matis_icin - 1;
					int jsag = j_tas_konum_matis_icin + 1;
					if (i >= 0 && i < 8 && jsag >= 0 && jsag < 8 && jsol >= 0 && jsol < 8){


						if (Taslar_Konumu[i][jsol] > 6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][jsol] = 1;
						}
						if (Taslar_Konumu[i][jsag] > 6)// herhangi bir SÝYAH taþ rastlanmýþsa orda kes
						{


						}
						else{

							Yenilme_tehlikesi[i][jsag] = 1;
						}

					}



				}
			}
		}


	}
}



void pointer_oneri_render(UINT  stride, UINT  offset, double l_x, double l_z, int siyah_beyaz){

	XMVECTOR translatePointer = testIntersection(Ro, Rd, verticeschessBoard, vertexCountchessBoard, false, cBuffer.mWorld);

	XMStoreFloat3(&translateP, translatePointer);

	if (siyah_beyaz == 0)
		g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerOneri_White);
	else
		g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerOneri_Black);

	XMMATRIX mTranslatePointer = XMMatrixTranslation(l_x, 0, l_z);
	cBuffer.mWorld = XMMatrixTranspose(mTranslatePointer);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_Pointer, &stride, &offset);
	cBuffer.vMeshColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0);
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetShader(g_pPixelShader_Textured, NULL, 0);
	g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->Draw(vertexCountpointer, 0);

}
void tas_render(ID3D11Buffer* tas_VertexBuffer, int tas_vertexCount, UINT  stride, UINT  offset, double r_1, double r_2, double r_3, double r_4, double l_x, double l_y, double l_z)
{

	g_pImmediateContext->IASetVertexBuffers(0, 1, &tas_VertexBuffer, &stride, &offset);
	cBuffer.vMeshColor = XMFLOAT4(r_1, r_2, r_3, r_4); // Renk 
	cBuffer.mWorld = XMMatrixTranspose(XMMatrixTranslation(l_x, l_y, l_z));
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0);
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pConstantBuffer);
	g_pImmediateContext->Draw(tas_vertexCount, 0);
}

XMVECTOR testIntersection(XMVECTOR Ro, XMVECTOR Rd, SimpleVertex* verticesModel, int vertexCount, bool transform, XMMATRIX& g_World)
{
	for (int i = 0; i < vertexCount; i += 3)
	{
		XMVECTOR V0 = XMLoadFloat3(&verticesModel[i].Pos);
		XMVECTOR V1 = XMLoadFloat3(&verticesModel[i + 1].Pos);
		XMVECTOR V2 = XMLoadFloat3(&verticesModel[i + 2].Pos);

		if (transform)
		{
			V0 = XMVector3Transform(V0, g_World);
			V1 = XMVector3Transform(V1, g_World);
			V2 = XMVector3Transform(V2, g_World);
		}

		XMVECTOR Edge1 = V1 - V0;
		XMVECTOR Edge2 = V2 - V0;
		XMVECTOR Normal = XMVector3Cross(Edge1, Edge2);

		//Backface test
		XMVECTOR toEye = Eye - V0;
		if (XMVectorGetX(XMVector3Dot(Normal, toEye)) < 0.0F) continue;

		float D = -XMVectorGetX(XMVector3Dot(Normal, V0));

		float t = -(XMVectorGetX(XMVector3Dot(Normal, Ro)) + D) / XMVectorGetX(XMVector3Dot(Normal, Rd));

		if (t > 0.0F)
		{
			XMVECTOR R = Ro + t * Rd;

			XMVECTOR Edge3 = V2 - V1;
			XMVECTOR Edge4 = R - V0;
			XMVECTOR Edge5 = R - V1;

			float S = XMVectorGetX(XMVector3Length(XMVector3Cross(Edge1, Edge2)));
			float s1 = XMVectorGetX(XMVector3Length(XMVector3Cross(Edge4, Edge2)));
			float s2 = XMVectorGetX(XMVector3Length(XMVector3Cross(Edge4, Edge1)));
			float s3 = XMVectorGetX(XMVector3Length(XMVector3Cross(Edge5, Edge3)));

			float fark = abs(S - (s1 + s2 + s3));

			float epsilon = 0.005F;

			if (fark <= epsilon)
			{
				XMVECTOR Edge = V2 - V1;
				if (XMVectorGetX(XMVector3Length(Edge)) > 6.0F)
					return XMVECTOR((V1 + V2) / 2);
				else
					return XMVECTOR((V0 + V1) / 2);
			}
		}
	}
	return XMVECTOR();
}