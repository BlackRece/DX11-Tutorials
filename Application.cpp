#include "Application.h"

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

    _cubeNum = 10;       // number of cubes
    _cubes = nullptr;

    _pVertexCount = 0;
    _pIndexCount = 0;

    _WindowHeight = 0;
    _WindowWidth = 0;

    _wireFrame = nullptr;
    _enableWireFrame = false;

    //pyramid
    _pPyramidMesh = MeshData();
    _pPyramidVB = nullptr;		//VertexBuffer;
    _pPyramidIB = nullptr;		//IndexBuffer;
    _pPyramidVC = 0;    		//VertexCount;
    _pPyramidIC = 0;            //IndexCount;

    //cube
    _pCubeMesh = MeshData();
    _pCubeVB = nullptr;		    //VertexBuffer;
    _pCubeIB = nullptr;		    //IndexBuffer;
    _pCubeVC = 0;    		    //VertexCount;
    _pCubeIC = 0;               //IndexCount;

    //quad
    _pQuadGen = new PlaneGenerator();
    _pQuadDims = { 4,4 };       //vertices width and height
    _pQuadArea = { 10,10 };     //size of area for plane
    _pQuadGen->position = Vector3D(0.0f, -10.0f, 7.0f);

    _pQuadVB = nullptr;		    //VertexBuffer;
    _pQuadIB = nullptr;		    //IndexBuffer;
    _pQuadVC = 0;    		    //VertexCount;
    _pQuadIC = 0;               //IndexCount;

    _gTime = 0.0f;

    //lighting
    _pLight = Lighting();
    _pLight.ambientLight = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
    _pLight.ambientMaterial = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);

    _pLight.SpecularLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    _pLight.SpecularMaterial = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    _pLight.SpecularPower = 10.0f;

    //camera
    _cam = Camera();

    //setup random engine for cubes
    std::mt19937 rnd(randDevice());

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
    // Forward
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f);    //cam pos
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);      //cam dir
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);      //cam up

    // Backward
    XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, 10.0f, 0.0f);    //cam pos
    XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);      //cam dir
    XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);      //cam up

	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(Eye, At, Up));
    */

    _cam.Eye = XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f);    //cam pos
    _cam.At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);      //cam dir
    _cam.Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);      //cam up

    XMStoreFloat4x4(&_view, XMMatrixLookAtLH(_cam.Eye, _cam.At, _cam.Up));

    // Initialize the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _WindowWidth / (FLOAT) _WindowHeight, 0.01f, 100.0f));

    // Initialize the matrices of cubes
    _cubes = new XMFLOAT4X4[_cubeNum];
    for (UINT i = 0; i < _cubeNum; i++) {
        XMStoreFloat4x4(&_cubes[i], XMMatrixIdentity());
    }

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
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        //{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, _pIndexCount, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, _pIndexCount, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        //{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, _pPyramidIC, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

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

    for (int i = 0; i < (_pCubeVC / sizeof(SimpleVertex)); i++) {
        _pCubeMesh.Vertices.push_back(Vertex(cubeVertices[i].Pos));
    }
    
    
    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = _pCubeVC;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = cubeVertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pCubeVB);

    if (FAILED(hr))
        return hr;
    
    // create pyramid buffer
    SimpleVertex pyramidVertices[] = {
        // bottom square
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },  
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },   
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) }, 
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },  

        // top point
        { XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }
    };

    _pPyramidVC = sizeof(pyramidVertices);

    for (int i = 0; i < (_pPyramidVC / sizeof(SimpleVertex)); i++) {
        _pPyramidMesh.Vertices.push_back(Vertex(pyramidVertices[i].Pos));
    }

    
    D3D11_BUFFER_DESC pbd;
    ZeroMemory(&pbd, sizeof(pbd));
    pbd.Usage = D3D11_USAGE_DEFAULT;
    pbd.ByteWidth = _pPyramidVC;
    pbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    pbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA pInitData;
    ZeroMemory(&pInitData, sizeof(pInitData));
    pInitData.pSysMem = pyramidVertices;

    hr = _pd3dDevice->CreateBuffer(&pbd, &pInitData, &_pPyramidVB);

    if (FAILED(hr))
        return hr;
    
	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

    // Create index buffer
    WORD cubeIndices[] =
    {
        0,1,2,  2,1,3,  // front
        2,3,7,  7,6,2,  // bottom
        1,5,7,  7,3,1,  // right
        6,7,5,  5,4,6,  // back
        4,0,2,  2,6,4,  // left
        4,5,1,  1,0,4   // top
    };

    _pIndexCount = sizeof(cubeIndices)/sizeof(WORD);

    for (int i = 0; i < _pIndexCount; i++) {
        _pCubeMesh.Indices.push_back(cubeIndices[i]);
    }

    int sizeofindices = _pCubeMesh.Indices.size();
    
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(cubeIndices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = cubeIndices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pCubeIB);

    if (FAILED(hr))
        return hr;

    // Create pyramid index buffer
    WORD pyramidIndices[] =
    {
        0,1,3,  1,2,3,  // bottom
        1,0,4,          // front
        2,1,4,          // right
        3,2,4,          // back
        0,3,4           // left
    };

    _pPyramidIC = sizeof(pyramidIndices) / sizeof(WORD);

    for (int i = 0; i < _pPyramidIC; i++) {
        _pPyramidMesh.Indices.push_back(pyramidIndices[i]);
    }
    
    D3D11_BUFFER_DESC pbd;
    ZeroMemory(&pbd, sizeof(pbd));

    pbd.Usage = D3D11_USAGE_DEFAULT;
    pbd.ByteWidth = sizeof(pyramidIndices);
    pbd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    pbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA pInitData;
    ZeroMemory(&pInitData, sizeof(pInitData));
    pInitData.pSysMem = pyramidIndices;
    hr = _pd3dDevice->CreateBuffer(&pbd, &pInitData, &_pPyramidIB);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitPlane() {
    HRESULT hr = S_OK;

    // generate plane 
    _pQuadGen->CreateGrid(
        (UINT)_pQuadArea.x, (UINT)_pQuadArea.y,     //size of area for plane
        (UINT)_pQuadDims.x, (UINT)_pQuadDims.y,     //number of vertices per width and height
        _pQuadGen->_meshData                        //the generated vertices and indices of plane
    );

    /*
    // DEBUG: variable size check
    int sizeVertex = sizeof(Vertex);            //48
    int sizeMeshData = sizeof(MeshData);        //32
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

HRESULT Application::InitCubeNormals() {
    HRESULT hr = S_OK;

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

    for (int i = 0; i < (_pCubeVC / sizeof(SimpleVertex)); i++) {
        _pCubeMesh.Vertices.push_back(Vertex(cubeVertices[i].Pos));
    }

    // Create index buffer
    WORD cubeIndices[] =
    {
        0,1,2,  2,1,3,  // front
        2,3,7,  7,6,2,  // bottom
        1,5,7,  7,3,1,  // right
        6,7,5,  5,4,6,  // back
        4,0,2,  2,6,4,  // left
        4,5,1,  1,0,4   // top
    };

    _pIndexCount = sizeof(cubeIndices) / sizeof(WORD);

    for (int i = 0; i < _pIndexCount; i++) {
        _pCubeMesh.Indices.push_back(cubeIndices[i]);
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

HRESULT Application::InitPyramidNormals() {
    HRESULT hr = S_OK;

    // create pyramid buffer
    SimpleVertex pyramidVertices[] = {
        // bottom square
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },

        // top point
        { XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }
    };

    _pPyramidVC = sizeof(pyramidVertices);

    for (int i = 0; i < (_pPyramidVC / sizeof(SimpleVertex)); i++) {
        _pPyramidMesh.Vertices.push_back(Vertex(pyramidVertices[i].Pos));
    }

    // Create pyramid index buffer
    WORD pyramidIndices[] =
    {
        0,1,3,  1,2,3,  // bottom
        1,0,4,          // front
        2,1,4,          // right
        3,2,4,          // back
        0,3,4           // left
    };

    _pPyramidIC = sizeof(pyramidIndices) / sizeof(WORD);

    for (int i = 0; i < _pPyramidIC; i++) {
        _pPyramidMesh.Indices.push_back(pyramidIndices[i]);
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

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
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

    /*
	InitVertexBuffer();
	InitIndexBuffer();
    */

    InitPlane();
    InitCubeNormals();
    InitPyramidNormals();
    
    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
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

    if (_pPyramidVB) _pPyramidVB->Release();
    if (_pPyramidIB) _pPyramidIB->Release();

    if (_pCubeVB) _pCubeVB->Release();
    if (_pCubeIB) _pCubeIB->Release();
    if (_pQuadGen) {
        _pQuadGen = nullptr; 
        delete _pQuadGen;
    }
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

    //
    // Animate the cube
    //
	XMStoreFloat4x4(&_world, 
        XMMatrixScaling(4.0f,4.0f,4.0f) *
        XMMatrixRotationZ(-t) * 
        XMMatrixRotationY(-t) *
        XMMatrixTranslation(0.0f,0.0f,10.0f)
    );

    /*
    //
    // Animate multpile cubes
    //
    float tx = 0.0f;
    float ty = 0.0f;
    float rotx = 0.0f;
    float roty = 0.0f;
    
    for (int i = 0; i < _cubeNum; i++) {
        
        if (i % 2 == 0) {
            ty = float((i*1.5f)-(_cubeNum/2));
            tx = -2;
            rotx = -i * t;
            roty = t;
        } else {
            tx = 2;
            rotx = t;
            roty = i * t;
        }

        XMStoreFloat4x4(&_cubes[i],
            XMMatrixRotationRollPitchYaw(rotx, roty, 0) *
            XMMatrixTranslation(tx, ty, 0.0f)
        );
    }
    */

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
    // Position plane
    //
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
    XMMATRIX view = XMLoadFloat4x4(&_view);
    XMMATRIX projection = XMLoadFloat4x4(&_projection);

    ConstantBuffer cb;
    //matrices
	cb.mWorld = XMMatrixTranspose(world);
	cb.mView = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(projection);

    //animation time (for animations performed by shader)
    cb.gTime = _gTime;

    //lighting
    //diffuse
    cb.DiffuseLight = _pLight.diffuseLight;
    cb.DiffuseMtrl = _pLight.diffuseMaterial;
    cb.LightVecW = _pLight.lightDirection;

    //ambient
    cb.AmbientLight = _pLight.ambientLight;
    cb.AmbientMaterial = _pLight.ambientMaterial;

    //specular
    cb.SpecularLight = _pLight.SpecularLight;
    cb.SpecularMaterial = _pLight.SpecularMaterial;
    cb.SpecularPower = _pLight.SpecularPower;
    cb.EyePosW = _cam.EyeToFloat3();

    //update directX with cb
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

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
    // Renders a pyramid
    //
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
        cb.mWorld = XMMatrixTranspose(world);
        _pImmediateContext->IASetVertexBuffers(0, 1, &_pCubeVB, &stride, &offset);
        //_pImmediateContext->IASetIndexBuffer(_pCubeIB, DXGI_FORMAT_R16_UINT, 0);
        _pImmediateContext->IASetIndexBuffer(_pCubeIB, DXGI_FORMAT_R32_UINT, 0);
        _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

        _pImmediateContext->DrawIndexed(_pIndexCount, 0, 0);
    }

    /* multiple cubes
    for (int i = 0; i < _cubeNum; i++) {
        world = XMLoadFloat4x4(&_cubes[i]);
        cb.mWorld = XMMatrixTranspose(world);

        if (i % 2 == 0) {
            _pImmediateContext->IASetVertexBuffers(0, 1, &_pCubeVB, &stride, &offset);
            _pImmediateContext->IASetIndexBuffer(_pCubeIB, DXGI_FORMAT_R16_UINT, 0);
            _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
            
            _pImmediateContext->DrawIndexed(_pIndexCount, 0, 0);
        } else {
            _pImmediateContext->IASetVertexBuffers(0, 1, &_pPyramidVB, &stride, &offset);
            _pImmediateContext->IASetIndexBuffer(_pPyramidIB, DXGI_FORMAT_R16_UINT, 0);
            _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

            _pImmediateContext->DrawIndexed(_pPyramidIC, 0, 0);
        }
    }
    */

    // quad floor wireframe
    //_pImmediateContext->RSSetState(_wireFrame);
    
    cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&_pPlane));
    cb.gTime = 0;

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
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    _pImmediateContext->DrawIndexed(_pQuadGen->_indexCount, 0, 0);

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