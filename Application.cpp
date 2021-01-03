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
	_pVertexBuffer = nullptr;
	_pIndexBuffer = nullptr;
	_pConstantBuffer = nullptr;

    _pVertexCount = 0;
    _pIndexCount = 0;

    _WindowHeight = 0;
    _WindowWidth = 0;

    _wireFrame = nullptr;
    _enableWireFrame = false;

    _noCulling = nullptr;
    _enableCulling = false;

    //cube
    _pCubeMesh = MeshArray();
    _pCubeVB = nullptr;		    //VertexBuffer;
    _pCubeIB = nullptr;		    //IndexBuffer;
    _pCubeVC = 0;    		    //VertexCount;
    _pCubeIC = 0;               //IndexCount;

    _cubeNum = 5;       // number of cubes
    _cubes = nullptr;

    _pCubeGO._pos = Vector3D(0.0f, 3.0f, 0.0f);
    _pCubeGO._scale = Vector3D(1.0f, 1.0f, 1.0f);
    _cubeGOs = new GameObject[_cubeNum];

    //pyramid
    _pPyramidMesh = MeshArray();
    _pPyramidVB = nullptr;		//VertexBuffer;
    _pPyramidIB = nullptr;		//IndexBuffer;
    _pPyramidVC = 0;    		//VertexCount;
    _pPyramidIC = 0;            //IndexCount;

    _pPyramidGO._pos = Vector3D(0.0f, -3.0f, -3.0f);
    _pPyramidGO._scale = Vector3D(1.0f, 1.0f, 1.0f);
    _pyramidGOs = new GameObject[_cubeNum];

    //quad
    _pQuadGen = new PlaneGenerator();
    _pQuadDims = { 4,4 };       //vertices width and height
    _pQuadArea = { 10,10 };     //size of area for plane
    _pQuadGen->position = Vector3D(0.0f, -10.0f, 7.0f);

    _pQuadVB = nullptr;		    //VertexBuffer;
    _pQuadIB = nullptr;		    //IndexBuffer;
    
    //pine tree plane
    _pPineGen = new PlaneGenerator();
    _pPineDims = { 4,4 };       //vertices width and height
    _pPineArea = { 10,10 };     //size of area for plane
    _pPineGen->position = Vector3D(0.0f, 0.0f, 12.5f);

    _pPineVB = nullptr;		    //VertexBuffer;
    _pPineIB = nullptr;		    //IndexBuffer;

    _gTime = 0.0f;

    //lighting
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

    //texturing
    _pTextureRV = nullptr;
    _pContainerRV = nullptr;
    _pPineRV = nullptr;
    _pSamplerLinear = nullptr;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
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

	// Initialize the world matrix
	XMStoreFloat4x4(&_world, XMMatrixIdentity());

    // Initialize the view matrix
    /*
    _cam.Eye = XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f);    //cam pos
    _cam.At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);      //cam dir
    _cam.Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);      //cam up
    */
    // use new camera class
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

    // done automatically in Camera class
    //XMStoreFloat4x4(&_view, XMMatrixLookAtLH(_cam.Eye, _cam.At, _cam.Up));

    // Initialize the projection matrix
    // done automatically in Camera class
	//XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _WindowWidth / (FLOAT) _WindowHeight, 0.01f, 100.0f));

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

    // generate plane 
    _pQuadGen->CreateGrid(
        _pQuadArea.x, _pQuadArea.y,                 //size of area for plane
        (UINT)_pQuadDims.x, (UINT)_pQuadDims.y,     //number of vertices per width and height
        _pQuadGen->_meshData                        //the generated vertices and indices of plane
    );

    /*
    // DEBUG: variable size check
    int sizeVertex = sizeof(Vertex);            //48
    int sizeMeshData = sizeof(MeshArray);        //32
    int sizeVertices = sizeof(_pQuadGen->_meshData.Vertices);   //16
    int sizeIndices = sizeof(_pQuadGen->_meshData.Indices);     //16
    int sizeUnsignedInt= sizeof(unsigned int);                  //4
    */

    // create plane vertex buffer
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_DEFAULT;
    //vbd.ByteWidth = sizeof(PlaneGenerator::Vertex) * _pQuadGen->_vertexCount;   //48*4 = 192
    vbd.ByteWidth = (sizeof(Vertex) * _pQuadGen->_vertexCount);   //384
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData;
    ZeroMemory(&vInitData, sizeof(vInitData));
    vInitData.pSysMem = &_pQuadGen->_meshData.Vertices[0];

    hr = _pd3dDevice->CreateBuffer(&vbd, &vInitData, &_pQuadVB);

    if (FAILED(hr))
        return hr;

    // create plane index buffer
    _pQuadGen->CreateIndices(_pQuadGen->_meshData);

    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));

    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(unsigned int) * _pQuadGen->_indexCount;      //32
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData;
    ZeroMemory(&iInitData, sizeof(iInitData));
    iInitData.pSysMem = &_pQuadGen->_meshData.Indices[0];
    hr = _pd3dDevice->CreateBuffer(&ibd, &iInitData, &_pQuadIB);

    if (FAILED(hr))
        return hr;

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pQuadVB, &stride, &offset);
    _pImmediateContext->IASetIndexBuffer(_pQuadIB, DXGI_FORMAT_R32_UINT, 0);

    return hr;
}

HRESULT Application::InitVerticalPlane() {
    HRESULT hr = S_OK;

    // generate plane 
    _pPineGen->CreateVerticalGrid(
        _pPineArea.x, _pPineArea.y, 0.0f,                 //size of area for plane
        (UINT)_pPineDims.x, (UINT)_pPineDims.y,     //number of vertices per width and height
        _pPineGen->_meshData                        //the generated vertices and indices of plane
    );

    // create plane vertex buffer
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_DEFAULT;
    //vbd.ByteWidth = sizeof(PlaneGenerator::Vertex) * _pPineGen->_vertexCount;   //48*4 = 192
    vbd.ByteWidth = (sizeof(Vertex) * _pPineGen->_vertexCount);   //384
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData;
    ZeroMemory(&vInitData, sizeof(vInitData));
    vInitData.pSysMem = &_pPineGen->_meshData.Vertices[0];

    hr = _pd3dDevice->CreateBuffer(&vbd, &vInitData, &_pPineVB);

    if (FAILED(hr))
        return hr;

    // create plane index buffer
    _pPineGen->CreateIndices(_pPineGen->_meshData);

    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));

    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(unsigned int) * _pPineGen->_indexCount;      //32
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData;
    ZeroMemory(&iInitData, sizeof(iInitData));
    iInitData.pSysMem = &_pPineGen->_meshData.Indices[0];
    hr = _pd3dDevice->CreateBuffer(&ibd, &iInitData, &_pPineIB);

    if (FAILED(hr))
        return hr;

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pPineVB, &stride, &offset);
    _pImmediateContext->IASetIndexBuffer(_pPineIB, DXGI_FORMAT_R32_UINT, 0);

    return hr;
}

HRESULT Application::InitCubeNormals() {
    HRESULT hr = S_OK;

    /* old code for normals
    // Create vertex buffer
    SimpleVertex cubeVertices[] =
    {
        // back square
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },  //tl
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },   //tr
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) }, //bl
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },  //br

        // front square
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },  //tl
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },   //tr
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) }, //bl
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },  //br
    };

    _pCubeVC = sizeof(cubeVertices);        //224 bytes

    for (unsigned int i = 0; i < (_pCubeVC / sizeof(SimpleVertex)); i++) {
        _pCubeMesh.Vertices.push_back(Vertex(cubeVertices[i].Pos));
    }
    */

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

    _pCubeVC = sizeof(cubeVerTexC);        //224 bytes

    for (unsigned int i = 0; i < (_pCubeVC / sizeof(VertexTextures)); i++) {
        _pCubeMesh.Vertices.push_back( Vertex(
            cubeVerTexC[i].Pos, 
            cubeVerTexC[i].TexC
        ));
    }

    /*
    * Texturing pseudo code
    * foreach of CUBE face (pair of triangles)
    *   find the four corner vectors
    *   assign the the correct texture cords to those corners
    *   foreach duplicated corner vectors
    *       add an extra vector to accommodate the additional texture coord
    */

    // Create index buffer
    WORD cubeIndices[] = {
        0,1,2,  2,1,3,      // back
        2,3,7,  7,6,2,      // bottom
        1,5,7,  7,3,1,      // right
        6,7,5,  5,4,6,      // front
        4,0,2,  2,6,4,      // left
        4,5,1,  1,0,4       // top
    };

    WORD cubeIndTex[] = {
        0,1,2,      2,1,3,      // back
        6,7,5,      5,4,6,      // front
        11,9,8,     10,9,11,    // bottom
        15,12,13,   14,12,15,   // right

        16,19,18,   18,19,17,   // left

        20,21,23,   23,22,20    // top
    };

    //_pIndexCount = sizeof(cubeIndices) / sizeof(WORD);
    _pCubeIC = sizeof(cubeIndTex) / sizeof(WORD);

    for (unsigned int i = 0; i < _pCubeIC; i++) {
        //_pCubeMesh.Indices.push_back(cubeIndices[i]);
        _pCubeMesh.Indices.push_back(cubeIndTex[i]);
    }

    PlaneGenerator::CalcNormals(_pCubeMesh.Vertices, _pCubeMesh.Indices);

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    //bd.ByteWidth = _pCubeVC;
    //bd.ByteWidth = sizeof(unsigned int) * _pCubeMesh.Vertices.size();
    bd.ByteWidth = sizeof(Vertex) * _pCubeMesh.Vertices.size();

    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = &_pCubeMesh.Vertices[0];

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pCubeVB);

    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));

    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = _pCubeMesh.Indices.size() * sizeof(unsigned int);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData;
    ZeroMemory(&iInitData, sizeof(iInitData));
    iInitData.pSysMem = &_pCubeMesh.Indices[0];
    hr = _pd3dDevice->CreateBuffer(&ibd, &iInitData, &_pCubeIB);

    if (FAILED(hr))
        return hr;

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

    hr = _pCubeGO._model->CreateVertexBuffer(*_pd3dDevice);

    if (FAILED(hr))
        return hr;

    hr = _pCubeGO._model->CreateIndexBuffer(*_pd3dDevice);

    if (FAILED(hr))
        return hr; 

    _pCubeGO.CreateTexture(*_pd3dDevice, "Textures/Crate_COLOR.dds");

    for (int i = 0; i < (int)_cubeNum; i++) {
        //_pCubeGO.CopyObject(*_pd3dDevice, _cubeGOs[i]);
        _cubeGOs[i]._model = _pCubeGO._model;

        _cubeGOs[i]._pos = _pCubeGO._pos;
        _cubeGOs[i]._scale = _pCubeGO._scale;

        _cubeGOs[i].CreateTexture(*_pd3dDevice, "Textures/Crate_COLOR.dds");

    }

    return hr;
}

HRESULT Application::InitPyramidNormals() {
    HRESULT hr = S_OK;

    /*
    // create pyramid buffer
    SimpleVertex pyramidVertices[] = {
        // bottom square
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },    lbf
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },     
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },

        // top point
        { XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }
    };

    _pPyramidVC = sizeof(pyramidVertices);

    for (unsigned int i = 0; i < (_pPyramidVC / sizeof(SimpleVertex)); i++) {
        _pPyramidMesh.Vertices.push_back(Vertex(pyramidVertices[i].Pos));
    }
    */

    // create pyramid buffer
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

    _pPyramidVC = sizeof(pyramidVertTex);

    for (unsigned int i = 0; i < (_pPyramidVC / sizeof(VertexTextures)); i++) {
        _pPyramidMesh.Vertices.push_back(Vertex(
            pyramidVertTex[i].Pos,
            pyramidVertTex[i].TexC
        ));
    }

    // Create pyramid index buffer
    /*
    WORD pyramidIndices[] = {
        0,1,3,  1,2,3,  // bottom
        1,0,4,          // front
        2,1,4,          // right
        3,2,4,          // back
        0,3,4           // left
    };
    */
    WORD pyramidIndTex[] = {
        0,1,3,  1,2,3,  // bottom
        5,6,4,          // front
        8,9,7,          // right
        11,12,10,       // back
        14,15,13        // left
    };

    _pPyramidIC = sizeof(pyramidIndTex) / sizeof(WORD);

    for (unsigned int i = 0; i < _pPyramidIC; i++) {
        _pPyramidMesh.Indices.push_back(pyramidIndTex[i]);
    }

    PlaneGenerator::CalcNormals(_pPyramidMesh.Vertices, _pPyramidMesh.Indices);

    D3D11_BUFFER_DESC pbd;
    ZeroMemory(&pbd, sizeof(pbd));
    pbd.Usage = D3D11_USAGE_DEFAULT;
    //pbd.ByteWidth = _pPyramidVC;
    pbd.ByteWidth = sizeof(Vertex) * _pPyramidMesh.Vertices.size();
    pbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    pbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA pInitData;
    ZeroMemory(&pInitData, sizeof(pInitData));
    pInitData.pSysMem = &_pPyramidMesh.Vertices[0];

    hr = _pd3dDevice->CreateBuffer(&pbd, &pInitData, &_pPyramidVB);

    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));

    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = _pPyramidMesh.Indices.size() * sizeof(unsigned int);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData;
    ZeroMemory(&iInitData, sizeof(iInitData));
    iInitData.pSysMem = &_pPyramidMesh.Indices[0];
    hr = _pd3dDevice->CreateBuffer(&ibd, &iInitData, &_pPyramidIB);

    if (FAILED(hr))
        return hr;

    return S_OK;
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

    _pPyramidGO.CreateTexture(*_pd3dDevice, "Textures/ChainLink.dds");
    //_pPyramidGO.CreateTexture(*_pd3dDevice, "Textures/Crate_COLOR.dds");

    for (int i = 0; i < (int)_cubeNum; i++) {
        //_pPyramidGO.CopyObject(*_pd3dDevice, _pyramidGOs[i]);
        _pyramidGOs[i]._model = new ModelObject();
        _pyramidGOs[i]._model = _pPyramidGO._model;

        _pyramidGOs[i]._pos = _pPyramidGO._pos;
        _pyramidGOs[i]._scale = _pPyramidGO._scale;

        _pyramidGOs[i].CreateTexture(*_pd3dDevice, "Textures/Crate_COLOR.dds");
    }

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
    //ID3D11DepthStencilView* _depthStencilView;
    //ID3D11Texture2D* _depthStencilBuffer;
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

    InitPlane();
    InitVerticalPlane();
    InitCubeNormals();
    InitPyramidNormals();

    hr = InitCubeGO();

    if (FAILED(hr))
        return hr;

    // Initialize the matrices of cubes
    _cubes = new XMFLOAT4X4[_cubeNum];
    for (UINT i = 0; i < _cubeNum; i++) {
        XMStoreFloat4x4(&_cubes[i], XMMatrixIdentity());
    }

    hr = InitPyramidGO();

    // Initialize a loaded object
    //3ds Max
    objMeshDataA = OBJLoader::Load("Models/3dsMax/torusKnot.obj", _pd3dDevice);

    //Blender
    objMeshDataB = OBJLoader::Load("Models/Blender/donut.obj", _pd3dDevice, false);

    //Cosmo??
    objContainerMesh = OBJLoader::Load("Models/Cosmo/OpticContainer.obj", _pd3dDevice, false);
    
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

    // Load crate texture from file
    CreateDDSTextureFromFile(_pd3dDevice, L"Textures/Crate_COLOR.dds", nullptr, &_pTextureRV);
    // Load cosmo texture from file
    CreateDDSTextureFromFile(_pd3dDevice, L"Models/Cosmo/OpticContainer.dds", nullptr, &_pContainerRV);
    // Load pine tree texture from file
    CreateDDSTextureFromFile(_pd3dDevice, L"Textures/Pine Tree.dds", nullptr, &_pPineRV);

    // Select texture to use in pixel shader
    _pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV);
    
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
    if (_pVertexBuffer) _pVertexBuffer->Release();
    if (_pIndexBuffer) _pIndexBuffer->Release();
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

    if (_pPyramidVB) _pPyramidVB->Release();
    if (_pPyramidIB) _pPyramidIB->Release();

    if (_pCubeVB) _pCubeVB->Release();
    if (_pCubeIB) _pCubeIB->Release();
    if (_pQuadGen) {
        _pQuadGen = nullptr; 
        delete _pQuadGen;
    }

    if (_Transparency) _Transparency->Release();
}

void Application::Update()
{
    // Update our time
    static float t = 0.0f;

    if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float) XM_PI * 0.0125f;
    }
    else
    {
        static ULONGLONG dwTimeStart = 0;
        ULONGLONG dwTimeCur = GetTickCount64();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

    //update constant buffer
    _gTime = t;

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

    //
    // Animate the pyramid
    //
	XMStoreFloat4x4(&_world, 
        XMMatrixScaling(4.0f,4.0f,4.0f) *
        XMMatrixRotationZ(-t) * 
        XMMatrixRotationY(-t) *
        XMMatrixTranslation(0.0f,0.0f,10.0f)
    );

   //
    // Animate solar system simulation
    //
    XMMATRIX sun, planet, moon;             // define some matrices

    // sun
    sun = XMMatrixIdentity();               // reset sun to identity matrix

    sun = XMMatrixMultiply(                 // replace sun matrix...
        XMMatrixRotationZ(t),                // ... adding a rotation
        sun                                // ... with this matrix, after...
    );
    XMStoreFloat4x4(
        &_cubes[0],
        XMMatrixMultiply(                 // replace sun matrix...
            XMMatrixRotationZ(t),                // ... adding a rotation
            sun
            )
    );       // set a cube to the sun matrix

    // planet 1
    planet = XMMatrixIdentity();            // reset planet to identity matrix

    planet = XMMatrixMultiply(              // replace planet matrix...
        XMMatrixTranslation(5.0f,0.0f,0.0),  // ... adding translation by 4 units
        sun                                // ... with this matrix, after...
    );
    XMStoreFloat4x4(&_cubes[1], planet);    // set a cube to the planet matrix

    // moon 1
    moon = XMMatrixIdentity();              // reset moon to identity matrix

    moon = XMMatrixMultiply(                // replace moon matrix...
        XMMatrixTranslation(4.0f, 0.0f, 0.0) * // ... adding translation by 4 units
        XMMatrixRotationZ(t),                 // ... adding a rotation
        planet                                   // ... with this matrix, after...
    );
    XMStoreFloat4x4(&_cubes[2], moon);    // set a cube to the moon matrix

    //
    // Position planes
    //
    //quad
    XMStoreFloat4x4(&_pPlane, 
        XMMatrixMultiply(
            XMMatrixIdentity(),
            XMMatrixTranslation(
                _pQuadGen->position.show_X(),
                _pQuadGen->position.show_Y(),
                _pQuadGen->position.show_Z()
            )
        )
    );

    //pine
    XMStoreFloat4x4(&_pPine,
        XMMatrixMultiply(
            XMMatrixIdentity(),
            XMMatrixTranslation(
                _pPineGen->position.show_X(),
                _pPineGen->position.show_Y(),
                _pPineGen->position.show_Z()
            )
        )
    );

    _pPyramidGO.LookTo(_cam[_camSelected].GetPos());
    _pPyramidGO.Update(t);

    for (int i = 0; i < _cubeNum; i++) {
        //_pyramidGOs[i].LookTo(_cam[_camSelected].GetPos());
        // this line made other cubes not render!?
        // too many objects to render?
        _pyramidGOs[i]._pos.x = i * (float)_cubeNum * 0.5f;

        _pyramidGOs[i].Update(t);
    }

    _pCubeGO.Update(t);

   //
   // Animate multpile cubes
   //
    float tx = 0.0f;
    float ty = 0.0f;
    float rotx = 0.0f;
    float roty = 0.0f;
    Vector3D tmp;
    for (int i = 0; i < _cubeNum; i++) {
        if (i % 2 == 0) {
            ty = float((i * 1.5f) - (_cubeNum / 2));
            tx = -2;
            rotx = -i * t;
            roty = t;
        } else {
            tx = 2;
            rotx = t;
            roty = i * t;
        }
        
        //_cubeGOs[i]._pos.x += t;
        _cubeGOs[i]._pos.x = tx;
        _cubeGOs[i]._pos.y = ty;
        _cubeGOs[i]._pos.z = -5.0f;

        _cubeGOs[i]._angle.x = rotx;
        _cubeGOs[i]._angle.y = roty;

        _cubeGOs[i].Update(t);
    }

    // Update our cameras
    if (_camSelected == 4) {
        _cam[_camSelected].SetView(_cubes[1]);
    }
    _cam[_camSelected].Update();
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

    //update directX with cb lite
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cbl, 0, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    
    // Set vertex buffer
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pPyramidVB, &stride, &offset);

    // Set index buffer
    _pImmediateContext->IASetIndexBuffer(_pPyramidIB, DXGI_FORMAT_R32_UINT, 0);

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Toggle wireframe
    if (_enableWireFrame) {
        _pImmediateContext->RSSetState(_wireFrame);
    } else {
        _pImmediateContext->RSSetState(nullptr);
    }

    //
    // Transparency option (DEFAULT)
    //
    // "fine-tune" the blending equation
    float blendFactor[] = { 0.75f, 0.75f, 0.75f, 1.0f };

    // Set the default blend state (no blending) for opaque objects
    _pImmediateContext->OMSetBlendState(0, 0, 0xffffffff);

    // Render opaque objects //

    //
    // Renders a pyramid
    //
    _pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV);
	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
    _pImmediateContext->DrawIndexed(_pPyramidIC, 0, 0);

    //
    // Render cube array
    //
    /* solar system simulation */
    for (int i = 0; i < 3; i++) {
        world = XMLoadFloat4x4(&_cubes[i]);
        cbl.mWorld = XMMatrixTranspose(world);
        _pImmediateContext->IASetVertexBuffers(0, 1, &_pCubeVB, &stride, &offset);
        _pImmediateContext->IASetIndexBuffer(_pCubeIB, DXGI_FORMAT_R32_UINT, 0);
        _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cbl, 0, 0);

        _pImmediateContext->DrawIndexed(_pCubeIC, 0, 0);
    }

    // quad floor wireframe
    cbl.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&_pPlane));

    stride = sizeof(Vertex);
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pQuadVB, &stride, &offset);
    /*
    * due to DirectX fuckery! 
    * c++ intrinsic int = 4 bytes = 32bits
    * whereas <Window.h> UINT = 2 bytes = 16bits (aka SHORT!!)
    * fucking Micro$haft!
    */
    //_pImmediateContext->IASetIndexBuffer(_pQuadIB, DXGI_FORMAT_R16_UINT, 0);
    _pImmediateContext->IASetIndexBuffer(_pQuadIB, DXGI_FORMAT_R32_UINT, 0);
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cbl, 0, 0);

    _pImmediateContext->DrawIndexed(_pQuadGen->_indexCount, 0, 0);

    

    _pPyramidGO.Draw(_pImmediateContext, _pConstantBuffer, cbl);

    for (int i = 0; i < _cubeNum; i++) {
        _pyramidGOs[i].Draw(_pImmediateContext, _pConstantBuffer, cbl);
    }

    //
    // Transparency option (CUSTOM)
    //
    // Set the blend state for transparent objects
    _pImmediateContext->OMSetBlendState(_Transparency, blendFactor, 0xffffffff);

    // Render transparent objects //

    //
    // Draw Loaded Objects
    //
    // object A
    cbl.mWorld = XMMatrixTranspose(
        XMMatrixMultiply(world, XMMatrixTranslation(3.0f,0.0f,0.0f))
    );
    _pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV);
    _pImmediateContext->IASetVertexBuffers(0, 1, &objMeshDataA.VertexBuffer, &objMeshDataA.VBStride, &objMeshDataA.VBOffset);
    _pImmediateContext->IASetIndexBuffer(objMeshDataA.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cbl, 0, 0);
    _pImmediateContext->DrawIndexed(objMeshDataA.IndexCount, 0, 0);

    // object b
    cbl.mWorld = XMMatrixTranspose(
        XMMatrixMultiply(world, XMMatrixTranslation(-3.0f, 0.0f, 0.0f))
    );
    _pImmediateContext->IASetVertexBuffers(0, 1, &objMeshDataB.VertexBuffer, &objMeshDataB.VBStride, &objMeshDataB.VBOffset);
    _pImmediateContext->IASetIndexBuffer(objMeshDataB.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cbl, 0, 0);
    _pImmediateContext->DrawIndexed(objMeshDataB.IndexCount, 0, 0);

    // Toggle backface culling
    if (!_enableWireFrame) {
        _pImmediateContext->RSSetState(_noCulling);
    }

    /* draw transparent objects with no back faces here */
    cbl.mWorld = XMMatrixTranspose(
        XMMatrixMultiply(XMMatrixIdentity(), XMMatrixTranslation(0.0f, -1.0f, -5.0f))
    );
    _pImmediateContext->PSSetShaderResources(0, 1, &_pContainerRV); //set texture
    _pImmediateContext->IASetVertexBuffers(0, 1, &objContainerMesh.VertexBuffer, &objContainerMesh.VBStride, &objContainerMesh.VBOffset);
    _pImmediateContext->IASetIndexBuffer(objContainerMesh.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cbl, 0, 0);
    _pImmediateContext->DrawIndexed(objContainerMesh.IndexCount, 0, 0);

    // pine plane
    cbl.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&_pPine));

    stride = sizeof(Vertex);
    _pImmediateContext->PSSetShaderResources(0, 1, &_pPineRV); //set texture
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pPineVB, &stride, &offset);
    _pImmediateContext->IASetIndexBuffer(_pPineIB, DXGI_FORMAT_R32_UINT, 0);
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cbl, 0, 0);

    _pImmediateContext->DrawIndexed(_pPineGen->_indexCount, 0, 0);

    /*
    _pPyramidGO.Draw(_pImmediateContext, _pConstantBuffer, cbl);

    for (int i = 0; i < _cubeNum; i++) {
        _pyramidGOs[i].Draw(_pImmediateContext, _pConstantBuffer, cbl);
    }
    */

    _pCubeGO.Draw(_pImmediateContext, _pConstantBuffer, cbl);

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