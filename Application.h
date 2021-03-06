#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <random>
#include "resource.h"
#include "PlaneGenerator.h"
#include "Structures.h"
#include "Lighting.h"
#include "OBJLoader.h"
#include "Camera.h"
#include "GameObject.h"

using namespace DirectX;

class Application {
private:
	// config file paths
	const string cameraFile =	"cameras.json";
	const string cubeFile =		"cubes.json";
	const string planeFile =	"planes.json";
	const string pyramidFile =	"pyramids.json";

	HINSTANCE					_hInst;
	HWND						_hWnd;
	D3D_DRIVER_TYPE				_driverType;
	D3D_FEATURE_LEVEL			_featureLevel;
	ID3D11Device*				_pd3dDevice;
	ID3D11DeviceContext*		_pImmediateContext;
	IDXGISwapChain*				_pSwapChain;
	ID3D11RenderTargetView*		_pRenderTargetView;
	ID3D11VertexShader*			_pVertexShader;
	ID3D11PixelShader*			_pPixelShader;
	ID3D11InputLayout*			_pVertexLayout;
	ID3D11Buffer*				_pConstantBuffer;
	ID3D11SamplerState*			_pSamplerLinear;	//Texturing Sampler
	ID3D11BlendState*			_Transparency;		//transparency

	XMFLOAT4X4					_world;

	Camera*						_cam;				// array of cameras
	int							_camNum;			// number of cameras
	int							_camSelected;		// index of selected camera
	bool						_camSwitched;		// has cam been switched?
	bool						_mouseLook;

	ID3D11DepthStencilView*		_depthStencilView;
	ID3D11Texture2D*			_depthStencilBuffer;

	ID3D11RasterizerState*		_wireFrame;
	bool						_enableWireFrame;
	float						_wireFrameDelay;
	float						_wireFrameCount;

	ID3D11RasterizerState*		_noCulling;
	bool						_enableCulling;

	//pyramid
	int							_pyramidNum;
	int							_uniPyramidNum;
	GameObject					_pPyramidGO;
	GameObject*					_pyramidGOs;
	GameObject*					_uniquePyramidGOs;

	//cube
	int							_cubeNum;
	int							_uniCubeNum;
	GameObject					_pCubeGO;
	GameObject*					_cubeGOs;
	GameObject*					_uniqueCubeGOs;

	//mixed array of game objects
	GameObject*					_solarGOs;
	int							_solarNum;

	//plane
	int							_planeHNum;
	int							_planeVNum;
	GameObject					_goHPlane;
	GameObject					_goVPlane;
	vector<GameObject>			_vertPlanes;
	vector<GameObject>			_horiPlanes;
	
	float						_gTime;

	// [ D1, D2, D3 ]
	//lighting
	Lighting					_pLight;

	//OBJ loader
	GameObject					_goCosmo;
	GameObject					_goTorusKnot;
	GameObject					_goDonut;
	GameObject					_goShip;

	//randomiser util
	std::random_device			randDevice;	
	std::normal_distribution<float> nd;

	//debug
	float	TICKRIGHT;

public:
	KeyboardFlags				_keys;
	MouseDelta					_mouse;

private:
	

	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitCameras();
	HRESULT InitShadersAndInputLayout();
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();

	HRESULT InitPlane();
	
	HRESULT InitCubeGO();
	HRESULT InitPyramidGO();
	
	float GetDeltaTime();
	void UpdateInput(float t);
	void UpdateBillBoards(float t, GameObject* gObjs, int objCount);
	void UpdateCameras(float t);
	void UpdateCubes(float t);
	void UpdateModels(float t);
	void UpdatePlanes(float t);
	void UpdatePyramids(float t);
	void UpdateSolar(float t, XMFLOAT4X4* sun, XMFLOAT4X4* planet, XMFLOAT4X4* moon);

	void Cleanup();

	UINT _WindowHeight;
	UINT _WindowWidth;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	//TODO: move to maths helper class
	static XMMATRIX InverseTranspose(CXMMATRIX M);
	static XMFLOAT4 XMFLoat4Multiply(XMFLOAT4& lhs, XMFLOAT4& rhs);

	void Update();
	void Draw();

	void OnKeyDown(MSG msg);
	void OnKeyUp(MSG msg);
	void Application::OnMouse(LONG mouseX, LONG mouseY);
};

