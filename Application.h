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
	ID3D11Buffer*				_pVertexBuffer;
	ID3D11Buffer*				_pIndexBuffer;
	ID3D11Buffer*				_pConstantBuffer;
	XMFLOAT4X4					_world;
	XMFLOAT4X4					_view;
	XMFLOAT4X4					_projection;

	Camera*						_cam;				// array of cameras
	int							_camNum;			// number of cameras
	int							_camSelected;		// index of selected camera

	UINT						_pIndexCount, _pVertexCount;

	ID3D11DepthStencilView*		_depthStencilView;
	ID3D11Texture2D*			_depthStencilBuffer;

	ID3D11RasterizerState*		_wireFrame;
	bool						_enableWireFrame;

	ID3D11RasterizerState*		_noCulling;
	bool						_enableCulling;

	//pyramid
	GameObject					_pPyramidGO;
	GameObject*					_pyramidGOs;

	//cube
	int							_cubeNum;
	GameObject					_pCubeGO;
	GameObject*					_cubeGOs;

	//mixed array of game objects
	GameObject*					_solarGOs;
	int							_solarNum;

	//plane
	PlaneGenerator*				_pQuadGen;
	XMFLOAT4X4					_pPlane;
	ID3D11Buffer*				_pQuadVB;			//VertexBuffer;
	ID3D11Buffer*				_pQuadIB;			//IndexBuffer;
	XMFLOAT2					_pQuadDims;			//vertex counts of quad
	XMFLOAT2					_pQuadArea;			//size of plane

	//pine tree plane
	PlaneGenerator*				_pPineGen;
	XMFLOAT4X4					_pPine;
	ID3D11Buffer*				_pPineVB;			//VertexBuffer;
	ID3D11Buffer*				_pPineIB;			//IndexBuffer;
	XMFLOAT2					_pPineDims;			//vertex counts of quad
	XMFLOAT2					_pPineArea;			//size of plane
	
	float						_gTime;

	//lighting
	Lighting					_pLight;

	//texturing
	ID3D11ShaderResourceView*	_pTextureRV;		//Crate Texture
	ID3D11ShaderResourceView*	_pContainerRV;		//Cosmo Texture
	ID3D11ShaderResourceView*	_pPineRV;			//Pine Tree Texture
	ID3D11SamplerState*			_pSamplerLinear;	//Sampler

	//OBJ loader
	MeshData					objMeshDataA;
	MeshData					objMeshDataB;
	MeshData					objContainerMesh;

	//randomiser util
	std::random_device			randDevice;	
	std::normal_distribution<float> nd;

	//transparency
	ID3D11BlendState*			_Transparency;

private:

	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();

	HRESULT InitPlane();
	HRESULT InitVerticalPlane();
	
	HRESULT InitCubeGO();
	HRESULT InitPyramidGO();
	
	float GetDeltaTime();
	void UpdateInput(float t);
	void UpdateBillBoards(float t, GameObject* gObjs, int objCount);
	void UpdateCubes(float t);
	void UpdatePlanes();
	void UpdatePyramids();
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
};

