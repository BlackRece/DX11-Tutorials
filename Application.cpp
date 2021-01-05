#include "Application.h"
#include "DDSTextureLoader.h"

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

        case WM_KEYDOWN:

            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pConstantBuffer = nullptr;

    _WindowHeight = 0;
    _WindowWidth = 0;

    _wireFrame = nullptr;
    _enableWireFrame = false;

    _noCulling = nullptr;
    _enableCulling = false;

    // [ B1 ]
    //cube
    _cubeNum = 5;       // number of cubes

    _pCubeGO._pos = Vector3D(0.0f, 3.0f, 0.0f);
    _pCubeGO._scale = Vector3D(1.0f, 1.0f, 1.0f);
    _cubeGOs = new GameObject[_cubeNum];                

    // [ B1 ]
    //pyramid
    _pPyramidGO._pos = Vector3D(0.0f, -3.0f, -3.0f);
    _pPyramidGO._scale = Vector3D(1.0f, 1.0f, 1.0f);
    _pyramidGOs = new GameObject[_cubeNum];

    // [ B1 ]
    //solar system example
    _solarNum = 3;
    _solarGOs = new GameObject[_solarNum];

    // [ B1 ]
    //gameObjects with planes
    //horizontal plane
    _goHPlane._pos = Vector3D(0.0f, -10.0f, 7.0f);
    _goHPlane._scale = Vector3D(1.0f, 1.0f, 1.0f);

    // [ B1 ]
    //vertical plane
    _goVPlane._pos = Vector3D(0.0f, 0.0f, 12.5f);
    _goVPlane._scale = Vector3D(1.0f, 1.0f, 1.0f);

    _gTime = 0.0f;

    // [ B1 ]
    //gameObjects with loaded models
    _goCosmo._pos = Vector3D(3.0f, 3.0f, 3.0f);
    _goCosmo._scale = Vector3D(3.0f, 3.0f, 3.0f);

    // [ B1 ]
    _goTorusKnot._pos = Vector3D(3.0f, -3.0f, 3.0f);
    _goTorusKnot._scale = Vector3D(3.0f, 3.0f, 3.0f);

    // [ B1 ]
    _goDonut._pos = Vector3D(-3.0f, -3.0f, 3.0f);
    _goDonut._scale = Vector3D(3.0f, 3.0f, 3.0f);

    //lighting
    // [ D3 ]
    _pLight = Lighting();
    _pLight.ambientLight = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
    _pLight.ambientMaterial = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);

    _pLight.SpecularLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    _pLight.SpecularMaterial = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    _pLight.SpecularPower = 10.0f;

    //camera
    _camNum = 6;
    _cam = new Camera[_camNum];
    _camSelected = 0;

    //setup random engine for cubes
    std::mt19937 rnd(randDevice());

    
    _pSamplerLinear = nullptr;
}

Application::~Application() {
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow) {
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	// Initialize the world origin matrix 
	XMStoreFloat4x4(&_world, XMMatrixIdentity());

    // Initialize the view matrix
    //default cam
    _cam[0] = Camera(
        Vector3D(0.0f, 0.0f, -10.0f),
        Vector3D(0.0f, 0.0f, 0.0f),
        Vector3D(0.0f, 1.0f, 0.0f),
        (float)_WindowWidth, (float)_WindowHeight,
        0.01f, 100.0f
    );
    _cam[0].UseWayPoints(false);

    //top cam
    _cam[1] = Camera(
        Vector3D(0.0f, -10.0f, 0.0f),
        Vector3D(0.0f, 0.0f, 0.0f),
        Vector3D(0.0f, 0.0f, 1.0f),
        (float)_WindowWidth, (float)_WindowHeight,
        0.01f, 100.0f
    );
    _cam[1].UseWayPoints(false);

    //right cam
    _cam[2] = Camera(
        Vector3D(-10.0f, 0.0f, 0.0f),
        Vector3D(0.0f, 0.0f, 0.0f),
        Vector3D(0.0f, 1.0f, 0.0f),
        (float)_WindowWidth, (float)_WindowHeight,
        0.01f, 100.0f
    );
    _cam[2].UseWayPoints(false);

    //far cam
    _cam[3] = Camera(
        Vector3D(0.0f, 0.0f, -20.0f),
        Vector3D(0.0f, 0.0f, 0.0f),
        Vector3D(0.0f, 1.0f, 0.0f),
        (float)_WindowWidth, (float)_WindowHeight,
        0.01f, 100.0f
    );
    _cam[3].SetLookTo(Vector3D(0.0f, 0.0f, 1.0f));
    _cam[3].UseLookTo(true);
    _cam[3].SetView();
    _cam[3].UseWayPoints(false);

    //chase cam
    _cam[4] = Camera(
        Vector3D(0.0f, 0.0f, -20.0f),
        Vector3D(0.0f, 0.0f, 0.0f),
        Vector3D(0.0f, 1.0f, 0.0f),
        (float)_WindowWidth, (float)_WindowHeight,
        0.01f, 100.0f
    );
    _cam[4].UseWayPoints(false);

    //waypoint cam
    _cam[5] = Camera(
        Vector3D(0.0f, 0.0f, -10.0f),
        Vector3D(0.0f, 0.0f, 0.0f),
        Vector3D(0.0f, 1.0f, 0.0f),
        (float)_WindowWidth, (float)_WindowHeight,
        0.01f, 100.0f
    );
    _cam[5].UseWayPoints(true);
    //set waypoints
    _cam[5].AddWayPoint(Vector3D(0.0f, 0.0f, 10.0f));
    _cam[5].AddWayPoint(Vector3D(-10.0f, 0.0f, 0.0f));
    _cam[5].AddWayPoint(Vector3D(0.0f, 0.0f, -10.0f));
    _cam[5].AddWayPoint(Vector3D(10.0f, 0.0f, 0.0f));

	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"VSBlob: The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"PSBlob: The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	return hr;
}

HRESULT Application::InitPlane() {
    HRESULT hr = S_OK;

    hr = _goHPlane._model->CreatePlane(*_pd3dDevice, Vector3D(20,20,20), 4, 4);
    if (FAILED(hr))
        return hr;

    hr = _goVPlane._model->CreatePlane(*_pd3dDevice, Vector3D(10,10,10), 4, 4, false);
    if (FAILED(hr))
        return hr;

    _goVPlane.CreateTexture(*_pd3dDevice, "Textures/Pine Tree.dds");

    return hr;
}

HRESULT Application::InitCubeGO() {
    HRESULT hr = S_OK;

    VertexTextures cubeVerTexC[] = {
        // back square (0-3)
        { XMFLOAT3(-1.0f, 1.0f, -1.0f),     XMFLOAT3(0.0f, 0.0f, 1.0f),     XMFLOAT2(0.0f, 0.0f) }, //0
        { XMFLOAT3(1.0f, 1.0f, -1.0f),      XMFLOAT3(0.0f, 1.0f, 0.0f),     XMFLOAT2(1.0f, 0.0f) }, //1
        { XMFLOAT3(-1.0f, -1.0f, -1.0f),    XMFLOAT3(0.0f, 1.0f, 1.0f),     XMFLOAT2(0.0f, 1.0f) }, //2
        { XMFLOAT3(1.0f, -1.0f, -1.0f),     XMFLOAT3(1.0f, 0.0f, 0.0f),     XMFLOAT2(1.0f, 1.0f) }, //3

        // front square (4-7)
        { XMFLOAT3(-1.0f, 1.0f, 1.0f),      XMFLOAT3(0.0f, 0.0f, 1.0f),     XMFLOAT2(0.0f, 0.0f) }, //4
        { XMFLOAT3(1.0f, 1.0f, 1.0f),       XMFLOAT3(0.0f, 1.0f, 0.0f),     XMFLOAT2(1.0f, 0.0f) }, //5
        { XMFLOAT3(-1.0f, -1.0f, 1.0f),     XMFLOAT3(0.0f, 1.0f, 1.0f),     XMFLOAT2(0.0f, 1.0f) }, //6
        { XMFLOAT3(1.0f, -1.0f, 1.0f),      XMFLOAT3(1.0f, 0.0f, 0.0f),     XMFLOAT2(1.0f, 1.0f) }, //7

        // bottom (for texturing) (8-11)
        { XMFLOAT3(1.0f, -1.0f, -1.0f),     XMFLOAT3(0.0f, 1.0f, 0.0f),     XMFLOAT2(0.0f, 0.0f) }, //2 lbb
        { XMFLOAT3(-1.0f, -1.0f, -1.0f),    XMFLOAT3(0.0f, 1.0f, 1.0f),     XMFLOAT2(1.0f, 0.0f) }, //3 rbb
        { XMFLOAT3(-1.0f, -1.0f, 1.0f),     XMFLOAT3(0.0f, 1.0f, 1.0f),     XMFLOAT2(1.0f, 1.0f) }, //6 rbf
        { XMFLOAT3(1.0f, -1.0f, 1.0f),      XMFLOAT3(1.0f, 0.0f, 0.0f),     XMFLOAT2(0.0f, 1.0f) }, //7 lbf

        // right (for texturing) (12-15)
        { XMFLOAT3(1.0f, 1.0f, -1.0f),      XMFLOAT3(0.0f, 1.0f, 0.0f),     XMFLOAT2(1.0f, 0.0f) }, //1 rtb
        { XMFLOAT3(1.0f, 1.0f, 1.0f),       XMFLOAT3(0.0f, 1.0f, 0.0f),     XMFLOAT2(0.0f, 0.0f) }, //5 rtf
        { XMFLOAT3(1.0f, -1.0f, -1.0f),     XMFLOAT3(0.0f, 1.0f, 1.0f),     XMFLOAT2(1.0f, 1.0f) }, //3 rbb
        { XMFLOAT3(1.0f, -1.0f, 1.0f),      XMFLOAT3(1.0f, 0.0f, 0.0f),     XMFLOAT2(0.0f, 1.0f) }, //7 rbf

        // left (for texturing) (16-19)
        { XMFLOAT3(-1.0f, 1.0f, 1.0f),      XMFLOAT3(0.0f, 0.0f, 1.0f),     XMFLOAT2(0.0f, 0.0f) }, // ltf
        { XMFLOAT3(-1.0f, -1.0f, -1.0f),    XMFLOAT3(0.0f, 0.0f, 1.0f),     XMFLOAT2(1.0f, 1.0f) }, // lbb
        { XMFLOAT3(-1.0f, -1.0f, 1.0f),     XMFLOAT3(0.0f, 1.0f, 1.0f),     XMFLOAT2(0.0f, 1.0f) }, // lbf
        { XMFLOAT3(-1.0f, 1.0f, -1.0f),     XMFLOAT3(0.0f, 1.0f, 0.0f),     XMFLOAT2(1.0f, 0.0f) }, // ltb

        // top (for texturing) (20-23)
        { XMFLOAT3(-1.0f, 1.0f, 1.0f),      XMFLOAT3(0.0f, 0.0f, 1.0f),     XMFLOAT2(0.0f, 1.0f) }, //4 ltf
        { XMFLOAT3(1.0f, 1.0f, 1.0f),       XMFLOAT3(0.0f, 1.0f, 0.0f),     XMFLOAT2(1.0f, 1.0f) }, //5 rtf
        { XMFLOAT3(-1.0f, 1.0f, -1.0f),     XMFLOAT3(0.0f, 0.0f, 1.0f),     XMFLOAT2(0.0f, 0.0f) }, //0 ltb
        { XMFLOAT3(1.0f, 1.0f, -1.0f),      XMFLOAT3(0.0f, 1.0f, 0.0f),     XMFLOAT2(1.0f, 0.0f) }, //1 rtb
    };  // 24 in total

    _pCubeGO._model->ImportVertices(cubeVerTexC, sizeof(cubeVerTexC));

    hr = _pCubeGO._model->CreateVertexBuffer(*_pd3dDevice);

    if (FAILED(hr))
        return hr;

    // Create textured index buffer with
    WORD cubeIndTex[] = {
       0,1,2,      2,1,3,      // back
       6,7,5,      5,4,6,      // front
       11,9,8,     10,9,11,    // bottom
       15,12,13,   14,12,15,   // right
       16,19,18,   18,19,17,   // left
       20,21,23,   23,22,20    // top
    };

    _pCubeGO._model->ImportIndices(cubeIndTex, sizeof(cubeIndTex));

    hr = _pCubeGO._model->CreateIndexBuffer(*_pd3dDevice);

    if (FAILED(hr))
        return hr; 

    _pCubeGO.CreateTexture(*_pd3dDevice, "Textures/Crate_COLOR.dds");

    // Duplicate for array
    for (int i = 0; i < (int)_cubeNum; i++) {
        _cubeGOs[i]._model = _pCubeGO._model;

        _cubeGOs[i]._pos = _pCubeGO._pos;
        _cubeGOs[i]._scale = _pCubeGO._scale;

        _cubeGOs[i].CreateTexture(*_pd3dDevice, "Textures/Crate_COLOR.dds");

    }

    // Duplicate for solar array
    //planet
    _solarGOs[1]._model = _pCubeGO._model;

    _solarGOs[1]._pos = _pCubeGO._pos;
    _solarGOs[1]._scale = _pCubeGO._scale;

    _solarGOs[1].CreateTexture(*_pd3dDevice, "Textures/asphalt.dds");

    return hr;
}

HRESULT Application::InitPyramidGO() {
    HRESULT hr = S_OK;

    VertexTextures pyramidVertTex[] = {
        // bottom square (0-3)
        { XMFLOAT3(-1.0f, -1.0f, -1.0f),    XMFLOAT3(0.0f, 0.0f, 1.0f),     XMFLOAT2(0.0f, 1.0f) }, //0
        { XMFLOAT3(1.0f, -1.0f, -1.0f),     XMFLOAT3(0.0f, 1.0f, 0.0f),     XMFLOAT2(1.0f, 1.0f) }, //1
        { XMFLOAT3(1.0f, -1.0f, 1.0f),      XMFLOAT3(0.0f, 1.0f, 1.0f),     XMFLOAT2(1.0f, 0.0f) }, //2
        { XMFLOAT3(-1.0f, -1.0f, 1.0f),     XMFLOAT3(1.0f, 1.0f, 0.0f),     XMFLOAT2(0.0f, 0.0f) }, //3

        // top point first - front face (4-6)
        { XMFLOAT3(0.0f, 1.0f, 0.0f),       XMFLOAT3(1.0f, 0.0f, 0.0f),     XMFLOAT2(0.0f, 0.0f) }, //4
        { XMFLOAT3(1.0f, -1.0f, -1.0f),     XMFLOAT3(0.0f, 1.0f, 0.0f),     XMFLOAT2(1.0f, 1.0f) }, //1
        { XMFLOAT3(-1.0f, -1.0f, -1.0f),    XMFLOAT3(0.0f, 0.0f, 1.0f),     XMFLOAT2(0.0f, 1.0f) }, //0

        // right face (7-9)
        { XMFLOAT3(0.0f, 1.0f, 0.0f),       XMFLOAT3(1.0f, 0.0f, 0.0f),     XMFLOAT2(0.0f, 0.0f) }, //4
        { XMFLOAT3(1.0f, -1.0f, 1.0f),      XMFLOAT3(0.0f, 1.0f, 1.0f),     XMFLOAT2(1.0f, 1.0f) }, //2
        { XMFLOAT3(1.0f, -1.0f, -1.0f),     XMFLOAT3(0.0f, 1.0f, 0.0f),     XMFLOAT2(0.0f, 1.0f) }, //1

        // back face (10-12)
        { XMFLOAT3(0.0f, 1.0f, 0.0f),       XMFLOAT3(1.0f, 0.0f, 0.0f),     XMFLOAT2(0.0f, 0.0f) }, //4
        { XMFLOAT3(-1.0f, -1.0f, 1.0f),     XMFLOAT3(1.0f, 1.0f, 0.0f),     XMFLOAT2(1.0f, 1.0f) }, //3
        { XMFLOAT3(1.0f, -1.0f, 1.0f),      XMFLOAT3(0.0f, 1.0f, 1.0f),     XMFLOAT2(0.0f, 1.0f) }, //2

        // left face (13-15)
        { XMFLOAT3(0.0f, 1.0f, 0.0f),       XMFLOAT3(1.0f, 0.0f, 0.0f),     XMFLOAT2(0.0f, 0.0f) }, //4
        { XMFLOAT3(-1.0f, -1.0f, -1.0f),    XMFLOAT3(0.0f, 0.0f, 1.0f),     XMFLOAT2(1.0f, 1.0f) }, //0
        { XMFLOAT3(-1.0f, -1.0f, 1.0f),     XMFLOAT3(1.0f, 1.0f, 0.0f),     XMFLOAT2(0.0f, 1.0f) }, //3
    };

    _pPyramidGO._model->ImportVertices(pyramidVertTex, sizeof(pyramidVertTex));
    
    hr = _pPyramidGO._model->CreateVertexBuffer(*_pd3dDevice);

    if (FAILED(hr))
        return hr;

    WORD pyramidIndTex[] = {
        0,1,3,  1,2,3,  // bottom
        5,6,4,          // front
        8,9,7,          // right
        11,12,10,       // back
        14,15,13        // left
    };

    _pPyramidGO._model->ImportIndices(pyramidIndTex, sizeof(pyramidIndTex));

    hr = _pPyramidGO._model->CreateIndexBuffer(*_pd3dDevice);

    if (FAILED(hr))
        return hr;

    //_pPyramidGO.CreateTexture(*_pd3dDevice, "Textures/ChainLink.dds");
    _pPyramidGO.CreateTexture(*_pd3dDevice, "Textures/Crate_COLOR.dds");

    // Duplicate for array
    for (int i = 0; i < (int)_cubeNum; i++) {
        _pyramidGOs[i]._model = new ModelObject();
        _pyramidGOs[i]._model = _pPyramidGO._model;

        _pyramidGOs[i]._pos = _pPyramidGO._pos;
        _pyramidGOs[i]._scale = _pPyramidGO._scale;

        _pyramidGOs[i].CreateTexture(*_pd3dDevice, "Textures/Crate_COLOR.dds");
    }

    // Duplicate for solar system
    //sun
    _solarGOs[0]._model = new ModelObject();
    _solarGOs[0]._model = _pPyramidGO._model;

    _solarGOs[0]._pos = _pPyramidGO._pos;
    _solarGOs[0]._scale = _pPyramidGO._scale;

    _solarGOs[0].CreateTexture(*_pd3dDevice, "Textures/2k_mars.dds");

    //moon
    _solarGOs[2]._model = new ModelObject();
    _solarGOs[2]._model = _pPyramidGO._model;

    _solarGOs[2]._pos = _pPyramidGO._pos;
    _solarGOs[2]._scale = _pPyramidGO._scale;

    _solarGOs[2].CreateTexture(*_pd3dDevice, "Textures/2k_mars.dds");

    return hr;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
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
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    //RECT rc = {0, 0, 640, 480};
    RECT rc = { 0, 0, 1280, 960};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _WindowWidth;
    sd.BufferDesc.Height = _WindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

    // Setup the depth buffer
    // [ C4 ]
    D3D11_TEXTURE2D_DESC depthStencilDesc;

    depthStencilDesc.Width = _WindowWidth;
    depthStencilDesc.Height = _WindowHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    _pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
    _pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

    _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

    hr = InitPlane();

    if (FAILED(hr))
        return hr;
    
    hr = InitCubeGO();

    if (FAILED(hr))
        return hr;

    hr = InitPyramidGO();
    
    if (FAILED(hr))
        return hr;

    // Initialize a loaded objects
    string imgpath = "Models/Cosmo/OpticContainer.dds";
    //3ds Max
    //objMeshDataA = OBJLoader::Load("Models/3dsMax/torusKnot.obj", _pd3dDevice);
    _goTorusKnot._model->LoadOBJ("Models/3dsMax/torusKnot.obj", _pd3dDevice);
    _goTorusKnot.CreateTexture(*_pd3dDevice, imgpath);

    //Blender
    //objMeshDataB = OBJLoader::Load("Models/Blender/donut.obj", _pd3dDevice, false);
    _goDonut._model->LoadOBJ("Models/Blender/donut.obj", _pd3dDevice, false);
    _goDonut.CreateTexture(*_pd3dDevice, imgpath);

    //Cosmo??
    //_goCosmo = OBJLoader::Load("Models/Cosmo/OpticContainer.obj", _pd3dDevice, false);
    _goCosmo._model->LoadOBJ("Models/Cosmo/OpticContainer.obj", _pd3dDevice, false);
    _goCosmo.CreateTexture(*_pd3dDevice, imgpath);
    
    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBufferLite);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

    if (FAILED(hr))
        return hr;

    // Create wireframe rasterizer render state
    D3D11_RASTERIZER_DESC wfdesc;
    ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
    wfdesc.FillMode = D3D11_FILL_WIREFRAME;
    wfdesc.CullMode = D3D11_CULL_NONE;
    hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireFrame);

    if (FAILED(hr))
        return hr;

    // Create no backface culling render state
    D3D11_RASTERIZER_DESC bfcdesc;
    ZeroMemory(&bfcdesc, sizeof(D3D11_RASTERIZER_DESC));
    bfcdesc.FillMode = D3D11_FILL_SOLID;
    bfcdesc.CullMode = D3D11_CULL_NONE;
    hr = _pd3dDevice->CreateRasterizerState(&bfcdesc, &_noCulling);

    if (FAILED(hr))
        return hr;

    //
    // Texturing and Sampling
    //
    
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

    hr = _pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);

    _pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);

    if (FAILED(hr))
        return hr;

    // Transparency Blending Equation
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(blendDesc));

    D3D11_RENDER_TARGET_BLEND_DESC rtbd;
    ZeroMemory(&rtbd, sizeof(rtbd));

    rtbd.BlendEnable = true;
    rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
    rtbd.DestBlend = D3D11_BLEND_BLEND_FACTOR;
    rtbd.BlendOp = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0] = rtbd;

    hr = _pd3dDevice->CreateBlendState(&blendDesc, &_Transparency);

    if (FAILED(hr))
        return hr;

    return S_OK;
}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();

    if (_pConstantBuffer) _pConstantBuffer->Release();
    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();

    if (_depthStencilView) _depthStencilView->Release();
    if (_depthStencilBuffer) _depthStencilBuffer->Release();

    if (_wireFrame) _wireFrame->Release();
    if (_noCulling) _noCulling->Release();

    if (_Transparency) _Transparency->Release();
}

float Application::GetDeltaTime() {
    float t = 0.0f;

    if (_driverType == D3D_DRIVER_TYPE_REFERENCE) {
        t += (float)XM_PI * 0.0125f;
    } else {
        static ULONGLONG dwTimeStart = 0;
        ULONGLONG dwTimeCur = GetTickCount64();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

    return t;
}

void Application::Update() {
    // Update our time
    static float t = 0.0f;
    t = GetDeltaTime();

    //update constant buffer
    _gTime = t;

    UpdateInput(t);
    
    UpdateSolar(t, 
        &_solarGOs[0]._matrix,      // sun
        &_solarGOs[1]._matrix,      // planet
        &_solarGOs[2]._matrix       // moon
    );

    UpdatePlanes(t);
    
    UpdateBillBoards(t, _pyramidGOs, _cubeNum);

    UpdatePyramids(t);

    UpdateCubes(t);

    UpdateModels(t);

    // Update our cameras
    if (_camSelected == 4) {
        _cam[_camSelected].SetView(_cubeGOs[1]._matrix);
    }
    _cam[_camSelected].Update();
}

void Application::UpdateInput(float t) {
    //keyboard input
    // should be handled by window not graphics
    if (GetKeyState('Q') & 0x8000) {
        _enableWireFrame = (_enableWireFrame) ? false : true;
    }

    // camera switching
    if (GetKeyState('1') & 0x8000) {
        _camSelected = 0;
    }

    if (GetKeyState('2') & 0x8000) {
        _camSelected = 1;
    }

    if (GetKeyState('3') & 0x8000) {
        _camSelected = 2;
    }

    if (GetKeyState('4') & 0x8000) {
        _camSelected = 3;
    }

    if (GetKeyState('5') & 0x8000) {
        _camSelected = 4;
    }

    if (GetKeyState('6') & 0x8000) {
        _camSelected = 5;
    }

    // camera movement
    if (GetKeyState('W') & 0x8000) {
        _cam[_camSelected].MoveForward(t);
        _cam[_camSelected].SetView();
    }
    if (GetKeyState('S') & 0x8000) {
        _cam[_camSelected].MoveForward(-t);
        _cam[_camSelected].SetView();
    }
    if (GetKeyState('A') & 0x8000) {
        _cam[_camSelected].MoveSidewards(-t);
    }
    if (GetKeyState('D') & 0x8000) {
        _cam[_camSelected].MoveSidewards(t);
    }

    if (GetKeyState(' ') & 0x8000) {
        _cam[_camSelected].SetView();
    }
}

void Application::UpdateBillBoards(float t, GameObject* gObjs, int objCount) {
    //theta = cos^-1 [(A dot B) / (|A| |B|)]
    //float angle = _pPyramidGO._pos.dot_product(_cam[_camSelected].GetPos());
    //_pPyramidGO._angle.y = angle;
    //_pPyramidGO._angle.y = _cam[_camSelected].GetAngle().y;
    //XMFLOAT4X4 target = _cam[_camSelected].GetView4x4();
    //Vector3D angle = _pPyramidGO.GetRotation(target);
    //_pPyramidGO._angle.y = angle.y;

    for (int i = 0; i < objCount; i++) {
        gObjs[i]._pos.x = i * (float)objCount * 0.5f;

        //gObjs[i].LookTo(_cam[_camSelected].GetTranslation());
        gObjs[i]._angle.y = -(_cam[_camSelected].GetRotation().y);

        gObjs[i].Update(t);
    }
}

//
// Animate the cubes
//
void Application::UpdateCubes(float t) {
    float tx = 0.0f;
    float ty = 0.0f;
    float rotx = 0.0f;
    float roty = 0.0f;
    Vector3D tmp;

    //single cube
    _pCubeGO.Update(t);

    //multiple cubes
    for (int i = 0; i < _cubeNum; i++) {
        if (i % 2 == 0) {
            ty = float((i * 1.5f) - (_cubeNum / 2.0f));
            tx = -2;
            rotx = -i * t;
            roty = t;
        } else {
            tx = 2;
            rotx = t;
            roty = i * t;
        }

        _cubeGOs[i]._pos.x = tx;
        _cubeGOs[i]._pos.y = ty;
        _cubeGOs[i]._pos.z = -5.0f;

        _cubeGOs[i]._angle.x = rotx;
        _cubeGOs[i]._angle.y = roty;

        _cubeGOs[i].Update(t);
    }
}

//
// Update Loaded Models
//
void Application::UpdateModels(float t) {
    _goCosmo.Update(t);
    _goDonut.Update(t);
    _goTorusKnot.Update(t);
}

//
// Update Planes GameObjects
//
void Application::UpdatePlanes(float t) {
    _goHPlane.Update(t);
    _goVPlane.Update(t);
}

//
// Animate the pyramids
//
void Application::UpdatePyramids(float t) {
    _pPyramidGO._angle = Vector3D(0.0f, -t, -t);
    _pPyramidGO._pos.z = 10.0f;

    _pPyramidGO.Update(t);
}

//
// Animate solar system simulation
//
void Application::UpdateSolar(float t, XMFLOAT4X4* sun, XMFLOAT4X4* planet, XMFLOAT4X4* moon) {
    XMMATRIX tSun, tPlanet, tMoon;             // define some matrices

    // sun
    tSun = XMMatrixIdentity();                  // reset sun to identity matrix

    tSun = XMMatrixMultiply(                    // replace sun matrix...
        XMMatrixRotationZ(t),                   // ... adding a rotation
        tSun                                    // ... with this matrix, after...
    );

    XMStoreFloat4x4(
        sun,
        XMMatrixMultiply(                       // replace sun matrix...
            XMMatrixRotationZ(t),               // ... adding a rotation
            tSun
        )
    );                                          // set a cube to the sun matrix

    // planet 1
    tPlanet = XMMatrixIdentity();               // reset planet to identity matrix

    tPlanet = XMMatrixMultiply(                 // replace planet matrix...
        XMMatrixTranslation(5.0f, 0.0f, 0.0),   // ... adding translation by 4 units
        tSun                                    // ... with this matrix, after...
    );
    XMStoreFloat4x4(planet, tPlanet);           // set a cube to the planet matrix

    // moon 1
    tMoon = XMMatrixIdentity();                 // reset moon to identity matrix

    tMoon = XMMatrixMultiply(                   // replace moon matrix...
        XMMatrixTranslation(4.0f, 0.0f, 0.0) *  // ... adding translation by 4 units
        XMMatrixRotationZ(t),                   // ... adding a rotation
        tPlanet                                 // ... with this matrix, after...
    );
    XMStoreFloat4x4(moon, tMoon);               // set a cube to the moon matrix
}

/// <summary>
/// 
/// </summary>
void Application::Draw()
{
    //
    // Clear the back buffer
    //
    float ClearColor[4] = {0.0f, 0.125f, 0.3f, 1.0f}; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
    // [ C4 ]
    _pImmediateContext->ClearDepthStencilView(
        _depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0
    );

	//
    // Update variables
    //
    XMMATRIX world = XMLoadFloat4x4(&_world);

    //use efficient constant buffer
    //by pre multiplying values on cpu
    ConstantBufferLite cbl;

    //matrices
    cbl.mWorld = XMMatrixTranspose(world);
    // use our camera class
    cbl.mViewProj = XMMatrixTranspose(_cam[_camSelected].GetViewProj());
    
    // [ D3 ]
    //lighting
    //ambient
    cbl.mAmbient = XMFLoat4Multiply(_pLight.ambientMaterial, _pLight.ambientMaterial);
    
    //diffuse
    cbl.mDiffuse = XMFLoat4Multiply(_pLight.diffuseMaterial, _pLight.diffuseLight);
    cbl.mDiffuseAlpha = _pLight.diffuseMaterial.w;
    cbl.mLightVecW = _pLight.lightDirection;

    //specular
    cbl.mSpecular = XMFLoat4Multiply(_pLight.SpecularMaterial, _pLight.SpecularLight);
    cbl.SpecularPower = _pLight.SpecularPower;
    
    // use camera class
    Vector3D cam = _cam[_camSelected].GetPos();
    cbl.EyePosW = XMFLOAT3{ cam.x, cam.y, cam.z };

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
    _pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
    
    //update directX with cb lite
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cbl, 0, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    // "fine-tune" the blending equation
    float blendFactor[] = { 0.75f, 0.75f, 0.75f, 1.0f };
    
    // Toggle wireframe
    // [ C3 ]
    if (_enableWireFrame) {
        _pImmediateContext->RSSetState(_wireFrame);
    } else {
        _pImmediateContext->RSSetState(nullptr);
    }

    //
    // Transparency option (DEFAULT)
    //
    // Set the default blend state (no blending) for opaque objects
    _pImmediateContext->OMSetBlendState(0, 0, 0xffffffff);

    // Render opaque objects //

    //
    // Renders a pyramid
    //
    // [ C1 ]
    _pPyramidGO.Draw(_pImmediateContext, _pConstantBuffer, cbl);

    //
    // Render pyramid array
    //
    // [ C1 ]
    for (int i = 0; i < _cubeNum; i++) {
        _pyramidGOs[i].Draw(_pImmediateContext, _pConstantBuffer, cbl);
    }

    //
    // Render solar system simulation
    //
    // [ C1 ]
    for (int i = 0; i < _solarNum; i++) {
        _solarGOs[i].Draw(_pImmediateContext, _pConstantBuffer, cbl);
    }

    // quad floor
    // [ C2 ]
    _goHPlane.Draw(_pImmediateContext, _pConstantBuffer, cbl);

    //
    // Transparency option (CUSTOM)
    //
    // Set the blend state for transparent objects
    _pImmediateContext->OMSetBlendState(_Transparency, blendFactor, 0xffffffff);

    // Render transparent objects //

    //
    // Draw Loaded Objects
    //
    // [ C1 ]
    _goDonut.Draw(_pImmediateContext, _pConstantBuffer, cbl);
    // [ C1 ]
    _goTorusKnot.Draw(_pImmediateContext, _pConstantBuffer, cbl);

    // Toggle backface culling
    if (!_enableWireFrame) {
        _pImmediateContext->RSSetState(_noCulling);
    }

    /* draw transparent objects with no back faces here */
    // [ C1 ]
    _goCosmo.Draw(_pImmediateContext, _pConstantBuffer, cbl);

    // pine plane
    // [ C1 ]
    _goVPlane.Draw(_pImmediateContext, _pConstantBuffer, cbl);
    
    //
    // Render cube array
    //
    // [ C1 ]
    _pCubeGO.Draw(_pImmediateContext, _pConstantBuffer, cbl);

    // [ C1 ]
    for (int i = 0; i < _cubeNum; i++) {
        _cubeGOs[i].Draw(_pImmediateContext, _pConstantBuffer, cbl);
    }

    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);
}

XMMATRIX Application::InverseTranspose(CXMMATRIX M) {
    XMMATRIX A = M;
    A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    XMVECTOR det = XMMatrixDeterminant(A);
    return XMMatrixTranspose(XMMatrixInverse(&det, A));
}

XMFLOAT4 Application::XMFLoat4Multiply(XMFLOAT4& lhs, XMFLOAT4& rhs) {
    XMFLOAT4 result;
    XMVECTOR tLhs = XMLoadFloat4(&lhs);
    XMVECTOR tRhs = XMLoadFloat4(&rhs);
    XMStoreFloat4(&result, tLhs * tRhs);

    return result;
}