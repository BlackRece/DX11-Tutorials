#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <random>
#include "resource.h"
#include "PlaneGenerator.h"

using namespace DirectX;

struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	float gTime;
};

class Application
{
private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;
	ID3D11Buffer*           _pVertexBuffer;
	ID3D11Buffer*           _pIndexBuffer;
	ID3D11Buffer*           _pConstantBuffer;
	XMFLOAT4X4              _world;
	XMFLOAT4X4              _view;
	XMFLOAT4X4              _projection;

	XMFLOAT4X4*				_cubes;
	UINT					_cubeNum;

	UINT					_pIndexCount, _pVertexCount;

	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D*		_depthStencilBuffer;

	ID3D11RasterizerState*	_wireFrame;
	bool					_enableWireFrame;

	//pyramid
	ID3D11Buffer*			_pPyramidVB;		//VertexBuffer;
	ID3D11Buffer*			_pPyramidIB;		//IndexBuffer;
	UINT					_pPyramidVC;		//VertexCount;
	UINT					_pPyramidIC;		//IndexCount;

	//cube
	ID3D11Buffer*			_pCubeVB;		//VertexBuffer;
	ID3D11Buffer*			_pCubeIB;		//IndexBuffer;
	UINT					_pCubeVC;		//VertexCount;
	UINT					_pCubeIC;		//IndexCount;

	//plain
	PlaneGenerator*			_pQuadGen;
	XMFLOAT4X4				_pPlane;
	ID3D11Buffer*			_pQuadVB;		//VertexBuffer;
	ID3D11Buffer*			_pQuadIB;		//IndexBuffer;
	UINT					_pQuadVC;		//VertexCount;
	UINT					_pQuadIC;		//IndexCount;
	XMFLOAT2				_pQuadDims;		//dimensions of quad

	float					_gTime;

	//randomiser util
	std::random_device		randDevice;	
	std::normal_distribution<float> nd;

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();
	HRESULT InitPlane();

	UINT _WindowHeight;
	UINT _WindowWidth;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};

