#include "Application.h"
#include "DDSTextureLoader.h"
#include "JSONLoader.h"

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

Application::Application()
{
    // Window Properties
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;

    _WindowHeight = 0;
    _WindowWidth = 0;

    // Global Settings
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
    _Transparency = nullptr;

	_pConstantBuffer = nullptr;

    // shaders and layout
	_pVertexLayout = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;

    // wire frame
    _wireFrame = nullptr;
    _enableWireFrame = false;
    _wireFrameDelay = 5;            // 5 second delay
    _wireFrameCount = 0;

    // back face culling
    _noCulling = nullptr;
    _enableCulling = false;

    // texturing
    _pSamplerLinear = nullptr;

    // [ B1 ]
    //cube
    _cubeNum = 0;       // number of cubes
    _uniCubeNum = 0;    // number of unique cubes

    _pCubeGO._pos = Vector3D(0.0f, 3.0f, 0.0f);
    _pCubeGO._scale = Vector3D(1.0f, 1.0f, 1.0f);

    // [ B1 ]
    //pyramid
    _pyramidNum = 0;
    _uniPyramidNum = 0;

    _pPyramidGO._pos = Vector3D(0.0f, -3.0f, -3.0f);
    _pPyramidGO._scale = Vector3D(1.0f, 1.0f, 1.0f);

    // [ B1 ]
    //solar system example
    _solarNum = 3;
    _solarGOs = new GameObject[_solarNum];

    _gTime = 0.0f;

    // [ B1 ]
    //gameObjects with loaded models
    _goCosmo._pos = Vector3D(3.0f, 3.0f, 3.0f);
    _goCosmo._scale = Vector3D(3.0f, 3.0f, 3.0f);

    // [ B1 ]
    _goTorusKnot._pos = Vector3D(3.0f, -3.0f, 3.0f);
    _goTorusKnot._scale = Vector3D(1.0f, 1.0f, 1.0f);

    // [ B1 ]
    _goDonut._pos = Vector3D(-3.0f, -3.0f, 3.0f);
    _goDonut._scale = Vector3D(1.0f, 1.0f, 1.0f);

    // [ B1 ]
    _goShip._pos = Vector3D(0.0f, -19.0f, -10.0f);
    _goShip._scale = Vector3D(0.0010f, 0.0010f, 0.0010f);

    //lighting
    // [ D1, D2, D3 ]
    _pLight = Lighting();
    _pLight.ambientLight = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
    _pLight.ambientMaterial = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);

    _pLight.SpecularLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    _pLight.SpecularMaterial = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    _pLight.SpecularPower = 4.0f;

    //keyboard inputs
    _keys = { false };
    _camSwitched = false;

    //mouse inputs
    _mouse = MouseDelta();
    _mouseLook = false;

    //setup random engine for cubes
    std::mt19937 rnd(randDevice());
    
    //debug
    TICKRIGHT = 5.0f;
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

// [ L1, L2, H1, H2 ]
HRESULT Application::InitCameras() {

    HRESULT hr = S_OK;

    json jsonCam = JSONLoader::Load(cameraFile);

    if (!jsonCam.contains("cameras"))
        return E_FAIL;

    Vector3D eye, at, up;

    _camSelected = 0;
    _camNum = jsonCam["cameras"].size();
    _cam = new Camera[_camNum];

    for (int i = 0; i < _camNum; i++) {
        //reset vars
        eye = Vector3D();
        at = Vector3D();
        up = Vector3D();

        //set vars from json
        eye = Vector3D(
            jsonCam["cameras"][i]["eye"][0].get<float>(),
            jsonCam["cameras"][i]["eye"][1].get<float>(),
            jsonCam["cameras"][i]["eye"][2].get<float>()
        );

        if (jsonCam["cameras"][i].contains("up")) {
            up = Vector3D(
                jsonCam["cameras"][i]["up"][0].get<float>(),
                jsonCam["cameras"][i]["up"][1].get<float>(),
                jsonCam["cameras"][i]["up"][2].get<float>()
            );
        } else up = Vector3D(0.0f, 1.0f, 0.0f);

        if (jsonCam["cameras"][i].contains("at")) {
            at = Vector3D(
                jsonCam["cameras"][i]["at"][0].get<float>(),
                jsonCam["cameras"][i]["at"][1].get<float>(),
                jsonCam["cameras"][i]["at"][2].get<float>()
            );
        } else at = Vector3D();

        _cam[i] = Camera(eye, at, up,
            (int)_WindowWidth, (int)_WindowHeight,
            jsonCam["cameras"][i]["near"].get<float>(),
            jsonCam["cameras"][i]["far"].get<float>()
        );

        if (jsonCam["cameras"][i].contains("UseLookTo")) {
            _cam[i].UseLookTo(
                jsonCam["cameras"][i]["UseLookTo"].get<bool>()
            );
        } else {
            _cam[i].UseLookTo(false);
        }

        if (_cam[i].UseLookTo()) {
            _cam[i].SetLookTo(Vector3D(
                jsonCam["cameras"][i]["to"][0].get<float>(),
                jsonCam["cameras"][i]["to"][1].get<float>(),
                jsonCam["cameras"][i]["to"][2].get<float>()
            ));
        }

        if (jsonCam["cameras"][i].contains("LookAtTarget")) {
            _cam[i]._lookAtTarget = true;
        } else {
            _cam[i]._lookAtTarget = false;
        }

        if (jsonCam["cameras"][i].contains("UseWayPoints")) {
            _cam[i].UseWayPoints(true);

            for (int j = 0; j < (int)jsonCam["cameras"][i]["waypoints"].size(); j++) {
                _cam[i].AddWayPoint(Vector3D(
                    jsonCam["cameras"][i]["waypoints"][j][0].get<float>(),
                    jsonCam["cameras"][i]["waypoints"][j][1].get<float>(),
                    jsonCam["cameras"][i]["waypoints"][j][2].get<float>()
                ));
            }
        } else {
            _cam[i].UseWayPoints(false);
        }

        if (jsonCam["cameras"][i].contains("follow")) {
            _cam[i]._followPlayer = jsonCam["cameras"][i]["follow"].get<bool>();
        } else {
            _cam[i]._followPlayer = false;
        }

        if (jsonCam["cameras"][i].contains("offset")) {
            _cam[i].SetOffset(Vector3D(
                jsonCam["cameras"][i]["offset"][0].get<float>(),
                jsonCam["cameras"][i]["offset"][1].get<float>(),
                jsonCam["cameras"][i]["offset"][2].get<float>()
            ));
        } else {
            _cam[i].SetOffset(Vector3D());
        }

        if(_cam[i]._followPlayer){
            float range = 10.0f;
            if (jsonCam["cameras"][i].contains("range")) {
                range = jsonCam["cameras"][i]["range"].get<float>();
            }

            // set initial cam position
            _cam[i].SetPos(_goShip._pos + _cam[i].GetOffset());

            // set initial lookAt vector
            Vector3D forward = _cam[i].GetForward(
                _cam[i].GetUp(),
                _cam[i].GetPos()
            );

            _cam[i].SetLookTo(forward);
            _cam[i].SetLookAt(forward * range);
        } 

        if (jsonCam["cameras"][i].contains("speed")) {
            _cam[i].SetSpeed(
                jsonCam["cameras"][i]["speed"]["rotate"].get<float>(),
                jsonCam["cameras"][i]["speed"]["translate"].get<float>()
            );
        }
    }
    
    eye = _cam[_camSelected].GetPos();
    if (eye == Vector3D())
        eye.z = 1.0f;

    return hr;
}

HRESULT Application::InitPlane() {
    HRESULT hr = S_OK;

    // [ G2 ]
    json jp = JSONLoader::Load(planeFile);
    _planeHNum = 0;
    _planeVNum = 0;

    if (!jp.contains("planeCount")) {
        return E_FAIL;
    } else {
        _planeVNum = jp["planeCount"].get<int>();

        if(jp.contains("planes"))
            _planeHNum = (int)jp["planes"].size();

        _planeVNum -= _planeHNum;
    }

    // build array of plane gameObjects
    //vertical
    _vertPlanes = vector<GameObject>();
    _vertPlanes.clear();
    _vertPlanes.resize(_planeVNum);

    //horizontal
    _horiPlanes = vector<GameObject>();
    _horiPlanes.clear();
    _horiPlanes.resize(_planeHNum);

    int tID = 0;
    int k = 0;
    Vector3D pos, scale, angle, dim;
    XMFLOAT2 detail{};
    string texturePath;
    bool isHori = false;

    for (int i = 0; i < int(_planeVNum + _planeHNum); i++) {
        //reset vars
        tID = 0;
        pos = Vector3D();
        scale = Vector3D();
        angle = Vector3D();
        dim = Vector3D();
        detail = XMFLOAT2{};
        isHori = false;
        texturePath = "";

        //set default vars
        pos = Vector3D(
            jp["default"]["position"][0].get<float>(),
            jp["default"]["position"][1].get<float>(),
            jp["default"]["position"][2].get<float>()
        );

        scale = Vector3D(
            jp["default"]["scale"][0].get<float>(),
            jp["default"]["scale"][1].get<float>(),
            jp["default"]["scale"][2].get<float>()
        );

        dim = Vector3D(
            jp["default"]["dimension"][0].get<float>(),
            jp["default"]["dimension"][1].get<float>(),
            jp["default"]["dimension"][2].get<float>()
        );

        detail = XMFLOAT2{
            jp["default"]["detail"][0].get<float>(),
            jp["default"]["detail"][1].get<float>(),
        };

        if (jp["default"].contains("angle")) {
            angle = Vector3D(
                jp["default"]["angle"][0].get<float>(),
                jp["default"]["angle"][1].get<float>(),
                jp["default"]["angle"][2].get<float>()
            );
        }

        isHori = jp["default"]["isHorizontal"].get<bool>();

        tID = jp["default"]["textureID"].get<int>();

        //set specific vars
        if (jp.contains("planes")) {
            for (int j = 0; j < _planeHNum; j++) {
                if (i == jp["planes"][j]["index"].get<int>()) {
                    pos = Vector3D(
                        jp["planes"][j]["position"][0].get<float>(),
                        jp["planes"][j]["position"][1].get<float>(),
                        jp["planes"][j]["position"][2].get<float>()
                    );

                    scale = Vector3D(
                        jp["planes"][j]["scale"][0].get<float>(),
                        jp["planes"][j]["scale"][1].get<float>(),
                        jp["planes"][j]["scale"][2].get<float>()
                    );

                    dim = Vector3D(
                        jp["planes"][j]["dimension"][0].get<float>(),
                        jp["planes"][j]["dimension"][1].get<float>(),
                        jp["planes"][j]["dimension"][2].get<float>()
                    );

                    detail = XMFLOAT2{
                        jp["planes"][j]["detail"][0].get<float>(),
                        jp["planes"][j]["detail"][1].get<float>(),
                    };

                    if (jp["planes"].contains("angle")) {
                        angle = Vector3D(
                            jp["planes"][j]["angle"][0].get<float>(),
                            jp["planes"][j]["angle"][1].get<float>(),
                            jp["planes"][j]["angle"][2].get<float>()
                        );
                    }

                    isHori = jp["planes"][j]["isHorizontal"].get<bool>();

                    tID = jp["planes"][j]["textureID"].get<int>();
                }
            }
        }

        //set texture path
        if (jp.contains("textures")) {
            texturePath = jp["textures"][tID].get<string>();
        }

        if (i < _planeVNum) {
           
            hr = _vertPlanes[i]._model->CreatePlane(
                *_pd3dDevice, dim, (int)detail.x, (int)detail.y, isHori
            );

            if (FAILED(hr))
                return hr;

            _vertPlanes[i].CreateTexture(*_pd3dDevice, texturePath);

            _vertPlanes[i]._pos = pos;
            _vertPlanes[i]._scale = scale;
            _vertPlanes[i]._angle = angle;

        } else {
            hr = _horiPlanes[k]._model->CreatePlane(
                *_pd3dDevice, dim, (int)detail.x, (int)detail.y, isHori
            );

            if (FAILED(hr))
                return hr;

            _horiPlanes[k].CreateTexture(*_pd3dDevice, texturePath);

            _horiPlanes[k]._pos = pos;
            _horiPlanes[k]._scale = scale;
            _horiPlanes[k]._angle = angle;

            k++;
        }
    }

    return hr;
}

HRESULT Application::InitCubeGO() {
    HRESULT hr = S_OK;

    // [ G2 ]
    json jsonCube = JSONLoader::Load(cubeFile);

    if (!jsonCube.contains("cubeCount")) {
        return E_FAIL;
    } else {
        _cubeNum = jsonCube["cubeCount"].get<int>();
        _uniCubeNum = (int)jsonCube["cubes"].size();
        _cubeNum -= _uniCubeNum;
    }

    if (
        !jsonCube.contains("vertices") &&
        !jsonCube.contains("uvs") &&
        !jsonCube.contains("indices")
        ) {
        return E_FAIL;
    }

    MeshArray cube{};

    if (jsonCube["vertices"].size() == jsonCube["uvs"].size()) {
        for (int i = 0; i < (int)jsonCube["vertices"].size(); i++) {
            cube.Vertices.push_back(Vertex(
                XMFLOAT3(
                    jsonCube["vertices"][i][0].get<float>(),
                    jsonCube["vertices"][i][1].get<float>(),
                    jsonCube["vertices"][i][2].get<float>()
                ),
                XMFLOAT2(
                    jsonCube["uvs"][i][0].get<float>(),
                    jsonCube["uvs"][i][1].get<float>()
                )
            ));
        }

        _pCubeGO._model->_mesh.Vertices = cube.Vertices;

    } else {
        return E_INVALIDARG;
    }

    hr = _pCubeGO._model->CreateVertexBuffer(*_pd3dDevice);

    if (FAILED(hr)) return hr;

    for (int i = 0; i < (int)jsonCube["indices"].size(); i++) {
        cube.Indices.push_back(
            jsonCube["indices"][i].get<unsigned short>()
        );
    }

    _pCubeGO._model->_mesh.Indices = cube.Indices;

    hr = _pCubeGO._model->CreateIndexBuffer(*_pd3dDevice);

    if (FAILED(hr)) return hr;

    // build array of cube gameObjects
    _cubeGOs = new GameObject[_cubeNum];
    _uniqueCubeGOs = new GameObject[_uniCubeNum];
    int tID = 0;
    int k = 0;
    Vector3D pos, scale, angle;
    string texturePath;

    for (int i = 0; i < int(_cubeNum+_uniCubeNum); i++) {
        tID = 0;
        pos = Vector3D();
        scale = Vector3D();
        angle = Vector3D();
        texturePath = "";

        pos = Vector3D(
            jsonCube["default"]["position"][0].get<float>(),
            jsonCube["default"]["position"][1].get<float>(),
            jsonCube["default"]["position"][2].get<float>()
        );

        scale = Vector3D(
            jsonCube["default"]["scale"][0].get<float>(),
            jsonCube["default"]["scale"][1].get<float>(),
            jsonCube["default"]["scale"][2].get<float>()
        );

        if (jsonCube["default"].contains("angle")) {
            angle = Vector3D(
                jsonCube["default"]["angle"][0].get<float>(),
                jsonCube["default"]["angle"][1].get<float>(),
                jsonCube["default"]["angle"][2].get<float>()
            );
        }

        tID = jsonCube["default"]["textureID"].get<int>();

        if (jsonCube.contains("cubes")) {
            for (int j = 0; j < _uniCubeNum; j++) {
                if (i == jsonCube["cubes"][j]["index"].get<int>()) {
                    pos = Vector3D(
                        jsonCube["cubes"][j]["position"][0].get<float>(),
                        jsonCube["cubes"][j]["position"][1].get<float>(),
                        jsonCube["cubes"][j]["position"][2].get<float>()
                    );

                    scale = Vector3D(
                        jsonCube["cubes"][j]["scale"][0].get<float>(),
                        jsonCube["cubes"][j]["scale"][1].get<float>(),
                        jsonCube["cubes"][j]["scale"][2].get<float>()
                    );

                    tID = jsonCube["cubes"][j]["textureID"].get<int>();
                }
            }
        }

        if (jsonCube.contains("textures")) {
            texturePath = jsonCube["textures"][tID].get<string>();
        }

        if (i == 0) {
            _pCubeGO.CreateTexture(
                *_pd3dDevice, 
                jsonCube["textures"][0].get<string>()
            );
        }

        if (i < _cubeNum) {
            _cubeGOs[i] = GameObject();
            _cubeGOs[i]._model = _pCubeGO._model;

            _cubeGOs[i].CreateTexture(*_pd3dDevice, texturePath);

            _cubeGOs[i]._pos = pos;
            _cubeGOs[i]._scale = scale;
            _cubeGOs[i]._angle = angle;
        } else {
            _uniqueCubeGOs[k] = GameObject();
            _uniqueCubeGOs[k]._model = _pCubeGO._model;

            _uniqueCubeGOs[k].CreateTexture(*_pd3dDevice, texturePath);

            _uniqueCubeGOs[k]._pos = pos;
            _uniqueCubeGOs[k]._scale = scale;
            _uniqueCubeGOs[k]._angle = angle;

            if (k < _uniCubeNum) k++;
        }
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

    // [ G2 ]
    json jPy = JSONLoader::Load(pyramidFile);

    if (!jPy.contains("pyramidCount")) {
        return E_FAIL;
    } else {
        _pyramidNum = jPy["pyramidCount"].get<int>();
        _uniPyramidNum = (int)jPy["pyramids"].size();
        _pyramidNum -= _uniPyramidNum;
    }

    if (
        !jPy.contains("vertices") &&
        !jPy.contains("uvs") &&
        !jPy.contains("indices")
        ) {
        return E_FAIL;
    }

    MeshArray pyramid{};

    if (jPy["vertices"].size() == jPy["uvs"].size()) {
        for (int i = 0; i < (int)jPy["vertices"].size(); i++) {
            pyramid.Vertices.push_back(Vertex(
                XMFLOAT3(
                    jPy["vertices"][i][0].get<float>(),
                    jPy["vertices"][i][1].get<float>(),
                    jPy["vertices"][i][2].get<float>()
                ),
                XMFLOAT2(
                    jPy["uvs"][i][0].get<float>(),
                    jPy["uvs"][i][1].get<float>()
                )
            ));
        }

        _pPyramidGO._model->_mesh.Vertices = pyramid.Vertices;

    } else {
        return E_INVALIDARG;
    }

    hr = _pPyramidGO._model->CreateVertexBuffer(*_pd3dDevice);

    if (FAILED(hr)) return hr;

    for (int i = 0; i < (int)jPy["indices"].size(); i++) {
        pyramid.Indices.push_back(
            jPy["indices"][i].get<unsigned short>()
        );
    }

    _pPyramidGO._model->_mesh.Indices = pyramid.Indices;

    hr = _pPyramidGO._model->CreateIndexBuffer(*_pd3dDevice);

    if (FAILED(hr)) return hr;

    // build array of cube gameObjects
    _pyramidGOs = new GameObject[_pyramidNum];
    _uniquePyramidGOs = new GameObject[_uniPyramidNum];
    int tID = 0;
    int k = 0;
    Vector3D pos, scale, angle;
    string texturePath;

    for (int i = 0; i < int(_pyramidNum + _uniPyramidNum); i++) {
        tID = 0;
        pos = Vector3D();
        scale = Vector3D();
        angle = Vector3D();
        texturePath = "";

        pos = Vector3D(
            jPy["default"]["position"][0].get<float>(),
            jPy["default"]["position"][1].get<float>(),
            jPy["default"]["position"][2].get<float>()
        );

        scale = Vector3D(
            jPy["default"]["scale"][0].get<float>(),
            jPy["default"]["scale"][1].get<float>(),
            jPy["default"]["scale"][2].get<float>()
        );

        if (jPy["default"].contains("angle")) {
            angle = Vector3D(
                jPy["default"]["angle"][0].get<float>(),
                jPy["default"]["angle"][1].get<float>(),
                jPy["default"]["angle"][2].get<float>()
            );
        }

        tID = jPy["default"]["textureID"].get<int>();

        if (jPy.contains("pyramids")) {
            for (int j = 0; j < _uniPyramidNum; j++) {
                if (i == jPy["pyramids"][j]["index"].get<int>()) {
                    pos = Vector3D(
                        jPy["pyramids"][j]["position"][0].get<float>(),
                        jPy["pyramids"][j]["position"][1].get<float>(),
                        jPy["pyramids"][j]["position"][2].get<float>()
                    );

                    scale = Vector3D(
                        jPy["pyramids"][j]["scale"][0].get<float>(),
                        jPy["pyramids"][j]["scale"][1].get<float>(),
                        jPy["pyramids"][j]["scale"][2].get<float>()
                    );

                    tID = jPy["pyramids"][j]["textureID"].get<int>();
                }
            }
        }

        if (jPy.contains("textures")) {
            texturePath = jPy["textures"][tID].get<string>();
        }

        if (i == 0) {
            _pPyramidGO.CreateTexture(
                *_pd3dDevice,
                jPy["textures"][0].get<string>()
            );
        }

        if (i < _pyramidNum) {
            _pyramidGOs[i] = GameObject();
            _pyramidGOs[i]._model = _pPyramidGO._model;

            _pyramidGOs[i].CreateTexture(*_pd3dDevice, texturePath);

            _pyramidGOs[i]._pos = pos;
            _pyramidGOs[i]._scale = scale;
            _pyramidGOs[i]._angle = angle;
        } else {
            _uniquePyramidGOs[k] = GameObject();
            _uniquePyramidGOs[k]._model = _pPyramidGO._model;

            _uniquePyramidGOs[k].CreateTexture(*_pd3dDevice, texturePath);

            _uniquePyramidGOs[k]._pos = pos;
            _uniquePyramidGOs[k]._scale = scale;
            _uniquePyramidGOs[k]._angle = angle;

            if (k < _uniPyramidNum) k++;
        }
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
    // Register window class
    WNDCLASSEX wcex{};
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

    // Register raw input class
    RAWINPUTDEVICE rid{};
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = 0;
    rid.hwndTarget = _hWnd;
    //rid.hwndTarget = nullptr;
    RegisterRawInputDevices(&rid, 1, sizeof(rid)); // don't care if it fails

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
    D3D11_TEXTURE2D_DESC depthStencilDesc{};

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
    D3D11_VIEWPORT vp{};
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

    // [ G1, G2, H1, H2, L1, L2 ]
    hr = InitCameras();
    
    if (FAILED(hr))
        return hr;

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
    _goTorusKnot._model->LoadOBJ("Models/3dsMax/torusKnot.obj", _pd3dDevice);
    _goTorusKnot.CreateTexture(*_pd3dDevice, imgpath);

    //Blender
    _goDonut._model->LoadOBJ("Models/Blender/donut.obj", _pd3dDevice, false);
    _goDonut.CreateTexture(*_pd3dDevice, imgpath);

    //Cosmo??
    _goCosmo._model->LoadOBJ("Models/Cosmo/OpticContainer.obj", _pd3dDevice, false);
    _goCosmo.CreateTexture(*_pd3dDevice, imgpath);

    //Player's Ship
    _goShip._model->LoadOBJ("Models/mk6_fighter.obj", _pd3dDevice, false);
    _goShip.CreateTexture(*_pd3dDevice, "Textures/SciFi_Fighter-MK6-diffuse.dds");
    
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

    // [ E1, E2, E3 ]
    UpdateInput(t);

    UpdateCameras(t);
    
    UpdateSolar(t, 
        &_solarGOs[0]._matrix,      // sun
        &_solarGOs[1]._matrix,      // planet
        &_solarGOs[2]._matrix       // moon
    );

    UpdatePlanes(t);
    
    UpdateBillBoards(t, _pyramidGOs, _pyramidNum);
    //UpdateBillBoards(t, _goVPlane, _planeVNum);

    UpdatePyramids(t);

    UpdateCubes(t);

    UpdateModels(t);

    //player
    _goShip.Update(t);

}

void Application::UpdateCameras(float t) {
    
    Vector3D eye = _cam[_camSelected].GetPos();
    if (eye == Vector3D()) {
        _cam = _cam;
        eye.z = 10.0f;
        return;
    }

    // Update chase cameras
    if (_cam[_camSelected]._lookAtTarget) {
        _cam[_camSelected].SetLookAt
            (_uniquePyramidGOs[_uniPyramidNum-1]._pos);
    }

    // Update player camera
    if (_cam[_camSelected]._followPlayer) {
        float angle = -_cam[_camSelected].GetAngle().y;
        Vector3D axis = _cam[_camSelected].GetUp();
        Vector3D offset = 
            _cam[_camSelected].Rotate(
                angle, axis,
                _cam[_camSelected].GetPos() - _cam[_camSelected].GetOffset()
            );
        
        _goShip._pos = offset;
        _goShip._angle = _cam[_camSelected].GetAngle();
    }

    _cam[_camSelected].Update();
}

void Application::UpdateInput(float t) {
    //toggle wire frame
    if (_wireFrameCount < 1) {
        if(_keys.WIRE){
            _enableWireFrame = (_enableWireFrame) ? false : true;
            _wireFrameCount = _wireFrameDelay;
        }

        //toggle mouse inputs
        if (_keys.MOUSELOOK) {
            _mouseLook = !_mouseLook;
            _wireFrameCount = _wireFrameDelay;
        }
    } else {
        if(_wireFrameCount > 0)
            _wireFrameCount -= t / 1000;
    }

    // camera switching
    if (!_camSwitched) {

        // next cam
        if (_keys.NEXTCAM) {
            if (_camSelected < _camNum)
                _camSelected++;
            else
                _camSelected = 0;

            _camSwitched = true;
        }

        // prev cam
        if (_keys.PREVCAM) {
            if (_camSelected > 0)
                _camSelected--;
            else
                _camSelected = _camNum;

            _camSwitched = true;
        }

        // default cam
        if (_keys.DEFAULTCAM) {
            _camSelected = 0;

            _camSwitched = true;
        }

        // switch between waypoint cam(s)
        if (_keys.WAYPOINTCAM) {
            int nextCam = _camSelected;
            int endCam = _camSelected;

            do {
                if (nextCam >= _camNum) nextCam = 0;
                else nextCam++;

                if (_cam[nextCam]._isUsingWayPoints) {
                    _camSelected = nextCam;
                    break;
                }
            } while (nextCam != endCam);

            _camSwitched = true;
        }

        // switch between player cam(s)
        if (_keys.PLAYERCAM) {
            int nextCam = _camSelected;
            int endCam = _camSelected;

            do {
                if (nextCam >= _camNum) nextCam = 0;
                else nextCam++;

                if (_cam[nextCam]._followPlayer) {
                    _camSelected = nextCam;
                    break;
                }
            } while (nextCam != endCam);

            _camSwitched = true;
        }

    } else {
        if (!_keys.NEXTCAM && !_keys.PREVCAM &&
            !_keys.WAYPOINTCAM && !_keys.PLAYERCAM) {
            _camSwitched = false;
        }
    }

    // [ E1, E2 ]
    // camera movement
    if (!_cam[_camSelected]._isUsingWayPoints) {
        //forward
        if(_keys.UP){
            _cam[_camSelected].MoveForward(t);
            //_cam[_camSelected].SetView();
        }

        //backward
        if(_keys.DOWN){
            _cam[_camSelected].MoveForward(-t);
            //_cam[_camSelected].SetView();
        }

        //left
        if(_keys.LEFT){
            _cam[_camSelected].MoveSidewards(-t);
        }
        
        //right
        if(_keys.RIGHT){
            _cam[_camSelected].MoveSidewards(t);
        }

        

        if (_mouseLook) {
            float angle = (t / 1000);
            int mx, my;
            Vector3D axis = Vector3D();
            Vector3D offset = Vector3D();

            _mouse.Diff(mx, my);

            //up first
            //set unit vector to rotate around
            if (mx < 0) {
                axis.y--;
                angle * -1.0f;
            } else {
                axis.y++;
            }

            offset = _cam[_camSelected].
                Rotate(angle, axis, _goShip._pos);

            _cam[_camSelected].SetPos(
                offset + _cam[_camSelected].GetOffset()
            );

            /*
            if (_mouse.y > 0) axis.x += angle;
            else if (_mouse.y < 0) axis.x -= angle;

            float ay = (_mouse.y == 0)? 0 : _mouse.y / _mouse.y;
            float ax = (_mouse.x == 0)? 0 : _mouse.x / _mouse.x;

            if (_mouse.y < 0) ay * -1.0f;
            if (_mouse.x < 0) ax * -1.0f;

            Vector3D axis = Vector3D(ax, ay, 0.0f);
                _mouse.x * (XM_PI * 180),
                _mouse.y * (XM_PI * 180),
                0.0f
            );

            axis.y += _mouse.x / _WindowWidth;
            axis.x += _mouse.y / _WindowHeight;

            _cam[_camSelected].SetPos(
                _cam[_camSelected].Rotate(
                    angle, axis,
                    _cam[_camSelected].GetPos()
                )
            );
            */


        }
    }
}

void Application::UpdateBillBoards(float t, GameObject* gObjs, int objCount) {
    
    for (int i = 0; i < objCount; i++) {
        gObjs[i]._pos.x = (i-(objCount*0.5f)) * (float)objCount * 0.5f;

        gObjs[i]._angle.y = atan2(
            gObjs[i]._pos.x - _cam[_camSelected].GetPos().x,
            gObjs[i]._pos.z - _cam[_camSelected].GetPos().z
        );

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

    for (int i = 0; i < _uniCubeNum; i++) {
        _uniqueCubeGOs[i].Update(t);
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
    vector<GameObject>::iterator it;
    // horizontal planes
    for (it = _horiPlanes.begin(); it != _horiPlanes.end(); ++it) {
        it->Update(t);
    }

    // vertical planes
    if (!_cam[_camSelected]._followPlayer) {
        float x = -(_planeVNum * 0.5f);
        for (it = _vertPlanes.begin(); it != _vertPlanes.end(); ++it) {
            it->_pos.x = x;
            x++;
        }
    } else {
        float tx = 0, tz = _planeVNum / 0.5f;
        for (int i = 0; i < _planeVNum; i++) {
            tz = float((i * 1.5f) - (_planeVNum / 2.0f));
            if (i % 2 == 0) {
                tx = -8.0f;
            } else {
                tx = 8.0f;
            }

            _vertPlanes[i]._pos.x = tx;
            _vertPlanes[i]._pos.z = tz;
        }
    }

    //billboarding
    for (it = _vertPlanes.begin(); it != _vertPlanes.end(); ++it) {
        it->_angle.y = atan2(
            it->_pos.x - _cam[_camSelected].GetPos().x,
            it->_pos.z - _cam[_camSelected].GetPos().z
        );

        it->Update(t);
    }
}

//
// Animate the pyramids
//
void Application::UpdatePyramids(float t) {
    //single pyramid
    _pPyramidGO._angle = Vector3D(0.0f, -t, -t);
    _pPyramidGO._pos.z = 10.0f;

    _pPyramidGO.Update(t);

    //multiple pyramids
    for (int i = 0; i < _uniPyramidNum; i++) {
        _uniquePyramidGOs[i]._angle.y = t;

        _uniquePyramidGOs[i]._pos.x = (i - (i * 0.5f)) * (float)i * 0.5f;

        if (_uniquePyramidGOs[i]._pos.y < -10.0f)
            _uniquePyramidGOs[i]._pos.y = 10.0f;
        else
            _uniquePyramidGOs[i]._pos.y -= t * 0.0005f;

        _uniquePyramidGOs[i].Update(t);
    }
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
    ConstantBufferLite cbl{};

    //matrices
    cbl.mWorld = XMMatrixTranspose(world);
    // use our camera class
    cbl.mViewProj = XMMatrixTranspose(_cam[_camSelected].GetViewProj());
    
    // [ D1, D2, D3 ]
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
    // Reader Player's Ship
    //
    _goShip.Draw(_pImmediateContext, _pConstantBuffer, cbl);

    //
    // Renders a pyramid
    //
    // [ C1 ]
    _pPyramidGO.Draw(_pImmediateContext, _pConstantBuffer, cbl);

    //
    // Render pyramid array
    //
    // [ C1 ]
    for (int i = 0; i < _pyramidNum; i++) {
        _pyramidGOs[i].Draw(_pImmediateContext, _pConstantBuffer, cbl);
    }

    //
    // Render cube array
    //
    for (int i = 0; i < _cubeNum; i++) {
        _cubeGOs[i].Draw(_pImmediateContext, _pConstantBuffer, cbl);
    }
    
    //
    // Render solar system simulation
    //
    // [ C1 ]
    for (int i = 0; i < _solarNum; i++) {
        _solarGOs[i].Draw(_pImmediateContext, _pConstantBuffer, cbl);
    }

    // plane arrays
    // [ C2 ]
    //_goHPlane.Draw(_pImmediateContext, _pConstantBuffer, cbl);
    vector<GameObject>::iterator it;
    for (it = _horiPlanes.begin(); it != _horiPlanes.end(); ++it)
        it->Draw(_pImmediateContext, _pConstantBuffer, cbl);

    for (it = _vertPlanes.begin(); it != _vertPlanes.end(); ++it) 
        it->Draw(_pImmediateContext, _pConstantBuffer, cbl);

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
    _pCubeGO.Draw(_pImmediateContext, _pConstantBuffer, cbl);
    // [ C1 ]
    _goDonut.Draw(_pImmediateContext, _pConstantBuffer, cbl);
    // [ C1 ]
    _goTorusKnot.Draw(_pImmediateContext, _pConstantBuffer, cbl);
    
    //
    // Draw more Pyramids
    //
    for (int i = 0; i < _uniPyramidNum; i++) {
        _uniquePyramidGOs[i].Draw(_pImmediateContext, _pConstantBuffer, cbl);
    }

    // Toggle backface culling
    if (!_enableWireFrame) {
        _pImmediateContext->RSSetState(_noCulling);
    }

    /* draw transparent objects with no back faces here */
    // [ C1 ]
    _goCosmo.Draw(_pImmediateContext, _pConstantBuffer, cbl);

    // pine plane
    // [ C1 ]
    /*
    for(int i=0;i<_planeVNum;i++)
        _goVPlane[i].Draw(_pImmediateContext, _pConstantBuffer, cbl);
    */
    //_goVPlane.Draw(_pImmediateContext, _pConstantBuffer, cbl);
    
    //
    // Render cube array
    //
    // [ C1 ]
    for (int i = 0; i < _uniCubeNum; i++) {
        _uniqueCubeGOs[i].Draw(_pImmediateContext, _pConstantBuffer, cbl);
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

void Application::OnKeyDown(MSG msg){
    //unsigned char repeat = (char)0x40000000;
    switch (msg.wParam) {
    // movement (cam and player)
    case 'W': _keys.UP = true; break;
    case 'A': _keys.LEFT = true; break;
    case 'S': _keys.DOWN = true; break;
    case 'D': _keys.RIGHT = true; break;

    case 'X': _keys.WIRE = true; break;         // toggle wireframe

    case 'Q': _keys.NEXTCAM = true; break;      // switch to next cam
    case 'E': _keys.PREVCAM = true; break;      // switch to previous cam
    case '1': _keys.DEFAULTCAM = true; break;   // switch to deafult cam
    case '2': _keys.WAYPOINTCAM = true; break;  // switch to way point cam
    case '0': _keys.PLAYERCAM = true; break;    // switch to player cam

    case VK_SPACE:_keys.MOUSELOOK = true; break;
    }
}
void Application::OnKeyUp(MSG msg) {
    switch (msg.wParam) {
    case 'W': _keys.UP = false; break;
    case 'A': _keys.LEFT = false; break;
    case 'S': _keys.DOWN = false; break;
    case 'D': _keys.RIGHT = false; break;

    case 'X': _keys.WIRE = false; break;

    case 'Q': _keys.NEXTCAM = false; break;     // switch to next cam
    case 'E': _keys.PREVCAM = false; break;     // switch to previous cam
    case '1': _keys.DEFAULTCAM = false; break;   // switch to deafult cam
    case '2': _keys.WAYPOINTCAM = false; break;  // switch to way point cam
    case '0': _keys.PLAYERCAM = false; break;    // switch to player cam

    case VK_SPACE:_keys.MOUSELOOK = false; break;
    }
}

void Application::OnMouse(LONG mouseX, LONG mouseY) {
    _mouse.Update((int)mouseX, (int)mouseY);
}