#include<Windows.h>
#include<tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<vector>
#include<d3dcompiler.h>
#ifdef _DEBUG
#include<iostream>
#endif

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
using namespace std;
using namespace DirectX;

void DebugOutPutFormatStaring(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif // 
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//�E�B���h�E���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);//OS�ɑ΂��āu�������̃A�v���͏I���v�Ɠ`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);//����̏������s��
}

#ifdef _DEBUG
void EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	HRESULT result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	if (!SUCCEEDED(result))return;

	debugLayer->EnableDebugLayer(); //�f�o�b�O���C���[��L���ɂ���
	debugLayer->Release();          //�L����������C���^�[�t�F�C�X���J������
}
#endif _DEBUG

#ifdef _DEBUG
int main()
{
#else
int WINAOI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif
	const unsigned int window_width = 1280;
	const unsigned int window_height = 720;

	ID3D12Device* _dev = nullptr;
	IDXGIFactory6* _dxgiFactory = nullptr;
	ID3D12CommandAllocator* _cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* _cmdList = nullptr;
	ID3D12CommandQueue* _cmdQeue = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;

	//�E�B���h�E�N���X�̐������o�^
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;//�R�[���o�b�N�֐��̎w��
	w.lpszClassName = _T("DX12Sample");//�A�v���P�[�V�����̃N���X��
	w.hInstance = GetModuleHandle(nullptr);//�n���h���̎擾

	RegisterClassEx(&w);//�A�v���P�[�V�����N���X�i�E�B���h�E�̎w���OS�ɓ`����j

	RECT wrc = { 0,0,window_width,window_height };//�E�B���h�E�T�C�Y�����߂�

	//�֐����g���ăE�B���h�E�̃T�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	//�E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName, //�N���X���w��
		_T("DX12 �e�X�g"),                    //�^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW,                  //�^�C�g���o�[�Ƌ��E��������E�B���h�E
		CW_USEDEFAULT,                        //�\�� x ���W��OS�ɂ��C��
		CW_USEDEFAULT,                        //�\�� y ���W��OS�ɂ��C��
		wrc.right - wrc.left,                 //�E�B���h�E��
		wrc.bottom - wrc.top,                 //�E�B���h�E��
		nullptr,                              //�e�E�B���h�E
		nullptr,                              //���j���[�n���h��
		w.hInstance,                          //�Ăяo���A�v���P�[�V����
		nullptr);                             //�ǉ��p�����[�^�[

#ifdef  _DEBUG
	//�f�o�b�O���C���[���I����
	EnableDebugLayer();
#endif //  _DEBUG

	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

#ifdef _DEBUG
	auto result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
#else
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
#endif // _DEBUG

	//�A�_�v�^�[�̗񋓗p
	std::vector<IDXGIAdapter*>adapters;
	//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	IDXGIAdapter* tmpAdampter = nullptr;
	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdampter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		adapters.push_back(tmpAdampter);
	}
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);//�A�_�v�^�[�̐����I�u�W�F�N�g�擾
	std:wstring strDesc = adesc.Description;
		//�T�������A�_�v�^�[�̖��O���m�F
		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdampter = adpt;
			break;
		}
	}
	//Direct3D�@�f�o�C�X�̏�����
	D3D_FEATURE_LEVEL featureLevel;
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(tmpAdampter, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featureLevel = lv;
			break; //�����\�ȃo�[�W���������������烋�[�v��ł��؂�
		}
	}

	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&_cmdAllocator));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator, nullptr,
		IID_PPV_ARGS(&_cmdList));

	D3D12_COMMAND_QUEUE_DESC cmdQueueDese = {};
	cmdQueueDese.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDese.NodeMask = 0;
	cmdQueueDese.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDese.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	result = _dev->CreateCommandQueue(&cmdQueueDese, IID_PPV_ARGS(&_cmdQeue));

	HRESULT CreateSwapChainForHwnd(
		IUnknown * pDevice,
		HWND hWnd,
		const DXGI_SWAP_CHAIN_DESC1 * pDesc,
		const DXGI_SWAP_CHAIN_FULLSCREEN_DESC * pFullscreenDesc,
		IDXGIOutput * pRestrictToOutput,
		IDXGISwapChain1 * *ppSwapChain
	);

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;

	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQeue, hwnd,
		&swapchainDesc, nullptr, nullptr,
		(IDXGISwapChain1**)&_swapchain);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;   //�����_�[�^�[�Q�b�g�r���[�Ȃ̂Ł@RTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;                      //���\��2��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; //���Ɏw��Ȃ�

	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);

	std::vector<ID3D12Resource*>_backBuffers(swcDesc.BufferCount);
	for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
	{
		result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
		D3D12_CPU_DESCRIPTOR_HANDLE handle
			= rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);
	}

	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	//�E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);

	//���_�f�[�^�\����
	struct Vertex {
		XMFLOAT3 pos; //xyz���W
		XMFLOAT2 uv;  //uv���W
	};

	Vertex vertices[] = {
		{{-0.4f,-0.7f,0.0f},{0.0f,1.0f}},  //����
		{{-0.4f, +0.7f,0.0f }, {0.0f,0.0f}}, //����
		{{+0.4f,-0.7f,0.0f },{1.0f,1.0f}},  //�E��
		{{+0.4f,+0.7f,0.0f},{1.0f,0.0f}},  //�E��
	};

	//���_�C���f�b�N�X
	unsigned short indices[] = {
		0,1,2,
		2,1,3,
	};

	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);  //���_��񂪓��邾���̃T�C�Y
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* vertBuff = nullptr;

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	Vertex* vertMap = nullptr;  //�^��Vertex�ɕύX
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

	ID3D12Resource* idxbuff = nullptr;
	//�ݒ�́A�o�b�t�@�[�̃T�C�Y�ȊO�A���_�o�b�t�@�[�̐ݒ���g���܂킵�Ă��ǂ�
	resdesc.Width = sizeof(indices);
	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&idxbuff));

	//������o�b�t�@�[�ɃC���f�b�N�X�f�[�^���R�s�[
	unsigned short* mappedIdx = nullptr;
	idxbuff->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	idxbuff->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@�[�r���[���쐬
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation = idxbuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);

	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();  //�o�b�t�@�[�̉��z�A�h���X
	vbView.SizeInBytes = sizeof(vertices);  //�S�o�C�g��
	vbView.StrideInBytes = sizeof(vertices[0]); //1���_������̃o�C�g��

	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;

	ID3DBlob* errorBlob = nullptr;
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",  //�V�F�[�_�[��
		nullptr,  //define�͂Ȃ�
		D3D_COMPILE_STANDARD_FILE_INCLUDE,  //�C���N���[�h�̓f�t�H���g
		"BasicVS", "vs_5_0",  //�֐���BasicVS,�ΏۃV�F�[�_�[��vs_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,  //�f�o�b�O�p����эœK���Ȃ�
		0,
		&_vsBlob, &errorBlob);  //�G���[����errorBlob�Ƀ��b�Z�[�W������
	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			::OutputDebugStringA("�t�@�C������������܂���");
		}
		else
		{
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}

	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl",  //�V�F�[�_�[��
		nullptr,  //define�͂Ȃ�
		D3D_COMPILE_STANDARD_FILE_INCLUDE,  //�C���N���[�h�̓f�t�H���g
		"BasicPS", "ps_5_0",  //�֐���BasicVS,�ΏۃV�F�[�_�[��vs_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,  //�f�o�b�O�p����эœK���Ȃ�
		0,
		&_psBlob, &errorBlob);  //�G���[����errorBlob�Ƀ��b�Z�[�W������
	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			::OutputDebugStringA("�t�@�C������������܂���");
		}
		else
		{
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
		{//uv�i�ǉ��j
			"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,  //2�������������_
			0,D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		}
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipline = {};

	gpipline.pRootSignature = nullptr; //���ƂŐݒ肷��

	gpipline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	gpipline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipline.PS.BytecodeLength = _psBlob->GetBufferSize();

	//�f�t�H���g�̃T���v���}�X�N��\���萔�i0xffffffff�j
	gpipline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//�܂��A���`�G�C���A�X�͎g��Ȃ�����false
	gpipline.RasterizerState.MultisampleEnable = false;

	gpipline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  //�J�����O���Ȃ�
	gpipline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;  //���g��h��Ԃ�
	gpipline.RasterizerState.DepthClipEnable = true;  //�[�x�����̃N���b�s���O�͗L����

	gpipline.BlendState.AlphaToCoverageEnable = false;
	gpipline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	//�ЂƂ܂����Z���Z�⃿�u�����f�B���O�͎g�p���Ȃ�
	renderTargetBlendDesc.BlendEnable = false;
	//�ЂƂ܂��_�����Z�q�͎g�p���Ȃ�
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;
	gpipline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	gpipline.InputLayout.pInputElementDescs = inputLayout;  //���C���[�擪�A�h���X
	gpipline.InputLayout.NumElements = _countof(inputLayout);  //���C�A�E�g�z��̗v�f��

	gpipline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED; //�X�g���b�v���̃J�b�g�Ȃ�
	gpipline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;  //�O�p�`�ō\��

	gpipline.NumRenderTargets = 1;  //���͂P�̂�
	gpipline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;  //0�`1�ɐ��K������tPGBA

	gpipline.SampleDesc.Count = 1;  //�T���v�����O��1�s�N�Z���ɂ�1
	gpipline.SampleDesc.Quality = 0;  //�N�I���e�B�͍Œ�

	D3D12_ROOT_SIGNATURE_DESC rootSignaterDesc = {};
	rootSignaterDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descTblRange = {};
	descTblRange.NumDescriptors = 1;  //�e�N�X�`��1��
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;  //��ʂ̓e�N�X�`��
	descTblRange.BaseShaderRegister = 0;  //0�ԃX���b�g����
	descTblRange.OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;  //�������̌J��Ԃ�
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;  //�c�����̌J��Ԃ�
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;  //���s���̌J��Ԃ�
	samplerDesc.BorderColor =
		D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;  //�{�[�_�[�͍�
	//��Ԃ��Ȃ��i�j�A���X�g�l�C�o�[�@�F�ŋߖT���)
	//samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;  //���`���
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;  //�~�b�v�}�b�v�ő�l
	samplerDesc.MinLOD = 0.0f; //�~�b�v�}�b�v�ŏ��l
	samplerDesc.ShaderVisibility =
		D3D12_SHADER_VISIBILITY_PIXEL;  //�s�N�Z���V�F�[�_�[���猩����
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;  //���T���v�����O���Ȃ�

	rootSignaterDesc.pStaticSamplers = &samplerDesc;
	rootSignaterDesc.NumStaticSamplers = 1;

	D3D12_ROOT_PARAMETER rootparam = {};
	//�s�N�Z���V�F�[�_�[��������
	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	//�f�B�X�N���v�^�����W�̃A�h���X
	rootparam.DescriptorTable.pDescriptorRanges = &descTblRange;
	//�f�B�X�N���v�^�����W��
	rootparam.DescriptorTable.NumDescriptorRanges = 1;

	rootSignaterDesc.pParameters = &rootparam;  //���[�g�p�����[�^�̐퓬�A�h���X
	rootSignaterDesc.NumParameters = 1;  //���[�g�p�����[�^��

	ID3DBlob* rotSigBlob = nullptr;
	result = D3D12SerializeRootSignature(
		&rootSignaterDesc,  //���[�g�V�O�l�`���ݒ�
		D3D_ROOT_SIGNATURE_VERSION_1_0,  //���[�g�V�O�l�`���o�[�W����
		&rotSigBlob,  //�V�F�[�_��������Ƃ��Ɠ���
		&errorBlob);  //�G���[����������
	
	ID3D12RootSignature* rootSigunature = nullptr;
	result = _dev->CreateRootSignature(
		0,  //nodemask�B0�ł悢
		rotSigBlob->GetBufferPointer(), //�V�F�[�_�̎��Ɠ��l
		rotSigBlob->GetBufferSize(), //�V�F�[�_�̎��Ɠ��l
		IID_PPV_ARGS(&rootSigunature));
	rotSigBlob->Release(); //�s�v�ɂȂ����̂ŉ��

	gpipline.pRootSignature = rootSigunature;

	ID3D12PipelineState* _pipelinesture = nullptr;
	result = _dev->CreateGraphicsPipelineState(&gpipline, IID_PPV_ARGS(&_pipelinesture));

	D3D12_VIEWPORT viewport = {};
	viewport.Width = window_width;  //�o�͐�̕�(�s�N�Z����)
	viewport.Height = window_height;  //�o�͐�̍���(�s�N�Z����)
	viewport.TopLeftX = 0; //�o�͐�̍�����WX
	viewport.TopLeftY = 0; //�o�͐�̍�����WY
	viewport.MaxDepth = 1.0f; //�[�x�ő�l
	viewport.MinDepth = 0.0f; //�[�x�ŏ��l

	D3D12_RECT scissorrect = {};  //�r���[�|�[�g�̑S�Ă�\������ݒ�
	scissorrect.top = 0;  //�؂蔲������W
	scissorrect.left = 0;  //�؂蔲�������W
	scissorrect.right = scissorrect.left + window_width;  //�؂蔲������W
	scissorrect.bottom = scissorrect.top + window_height;  //�؂蔲�������W


	//WriteToSubresource�ő��M���邽�߂̃q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	//����Ȑݒ�Ȃ̂�DEFAULT�ł�UPLOAD�ł��Ȃ�
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	//���C�g�o�b�N
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	//�]����L0,�܂�CPU�����璼�ڍs��
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	//�P��A�_�v�^�[�̂���0
	texHeapProp.CreationNodeMask = 0;
	texHeapProp.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  //RGBA�t�H�[�}�b�g
	resDesc.Width = 256;  //��
	resDesc.Height = 256;  //����
	resDesc.DepthOrArraySize = 1;  //2D�z��ł��Ȃ��̂�1
	resDesc.SampleDesc.Count = 1;  //�ʏ�e�N�X�`���Ȃ̂ŃA���`�G�C���A�V���O���Ȃ�
	resDesc.SampleDesc.Quality = 0;  //�N�I���e�B�͍Œ�
	resDesc.MipLevels = 1;  //�~�j�}�b�v���Ȃ��̂Ń~�b�v����1��
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  //2D�e�N�X�`���p
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;  //���C�A�E�g�͌��肵�Ȃ�
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;  //���Ƀt���O�Ȃ�

	ID3D12Resource* texbuff = nullptr;
	result = _dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,  //���Ɏw��Ȃ�
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,  //�e�N�X�`���p�w��
		nullptr,
		IID_PPV_ARGS(&texbuff)
	);

	//�����_���ȃm�C�Y�摜��`��
	struct TexRGBA
	{
		unsigned char R, G, B, A;
	};

	std::vector<TexRGBA>texturedata(256 * 256);
	for (auto& rgba : texturedata)
	{
		rgba.R = rand() % 256;
		rgba.G = rand() % 256;
		rgba.B = rand() % 256;
		rgba.A = 255;  //����1.0�Ƃ���
	}

	result = texbuff->WriteToSubresource(
		0,
		nullptr,  //�S�̈�փR�s�[
		texturedata.data(),  //���f�[�^�A�h���X
		sizeof(TexRGBA) * 256,  //1���C���T�C�Y
		sizeof(TexRGBA) * (UINT)texturedata.size()  //�S�T�C�Y�u
	);

	ID3D12DescriptorHeap* texDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descHearDesc = {};
	//�V�F�[�_�[���猩����悤��
	descHearDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//�}�X�N��0
	descHearDesc.NumDescriptors = 0;
	//�r���[�͍��̂Ƃ���1����
	descHearDesc.NumDescriptors = 1;
	//�V�F�[�_�[���\�[�X�r���[�p
	descHearDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//����
	result = _dev->CreateDescriptorHeap(&descHearDesc, IID_PPV_ARGS(&texDescHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  //RGBA(0.0f~1.0f�ɐ��K��)
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;  //2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = 1;  //�~�b�v�}�b�v�͎g�p���Ȃ�

	_dev->CreateShaderResourceView(
		texbuff,  //�r���[�Ɗ֘A�t����o�b�t�@�[
		&srvDesc,  //��قǐݒ肵���e�N�X�`�����
		texDescHeap->GetCPUDescriptorHandleForHeapStart()  //�q�[�v�̂ǂ��Ɋ��蓖�Ă邩v
	);
		
	//�`�������W���p
	//float alpha = 0;
	while (true)
	{
		MSG msg ;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			//�A�v���P�[�V�������I���Ƃ���message��WM_QUIT�ɂȂ�
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//DirectX�̏���
		//�o�b�N�o�b�t�@�̃C���f�b�N�X���擾
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; //�J��
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;  //���ɂȂ�
		BarrierDesc.Transition.pResource = _backBuffers[bbIdx];  //�o�b�N�o�b�t�@�\���\�[�X
		BarrierDesc.Transition.Subresource = 0;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;  //���O��PRESENT���
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;  //������RT���
		_cmdList->ResourceBarrier(1, &BarrierDesc);  //�o���A�w����s

		_cmdList->SetPipelineState(_pipelinesture);


		//�����_�[�^�[�Q�b�g���w��
		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_cmdList->OMSetRenderTargets(1, &rtvH, 0,nullptr);

		//�`�������W���p
		//alpha = alpha + 0.01f;
		//if (alpha > 1.0f)
		//{
		//	alpha = 0.0f;
		//}

		//��ʃN���A
		float clearColor[] = { 0.0f,0.0f,0.3f,1.0f };
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		_cmdList->SetGraphicsRootSignature(rootSigunature);
		_cmdList->SetDescriptorHeaps(1, &texDescHeap);
		_cmdList->SetGraphicsRootDescriptorTable(
			0,  //���[�g�p�����[�^�C���f�b�N�X
			texDescHeap->GetGPUDescriptorHandleForHeapStart());  //�q�[�v�A�h���X

		_cmdList->RSSetViewports(1, &viewport);
		_cmdList->RSSetScissorRects(1, &scissorrect);

		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		_cmdList->IASetVertexBuffers(0, 1, &vbView);
		_cmdList->IASetIndexBuffer(&ibView);
		
		_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0); //6=�C���f�b�N�X��

		//�O�ゾ������ւ���
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		_cmdList->ResourceBarrier(1, &BarrierDesc);

		//���߂̃N���[�Y
		_cmdList->Close();

		//�R�}���h���X�g�̎��s
		ID3D12CommandList* cmdlists[] = { _cmdList };
		_cmdQeue->ExecuteCommandLists(1, cmdlists);
		//_swapchain->Present(1, 0);

		//�҂�
		_cmdQeue->Signal(_fence, ++_fenceVal);

		if (_fence->GetCompletedValue() != _fenceVal)
		{
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);  //�C�x���g�n���h���̎擾
			WaitForSingleObject(event, INFINITE);  //�C�x���g����������܂Ŗ����ɑ҂�
			CloseHandle(event);  //�C�x���g�n���h�������
		}

		_cmdList->Reset(_cmdAllocator, nullptr); //�L���[���N���A
		_swapchain->Present(1, 0); //�ĂуR�}���h���X�g�𗭂߂鏀��
	}
	//�����N���X�͎g��Ȃ��̂œo�^��������
	UnregisterClass(w.lpszClassName, w.hInstance);
	return 0;
	DebugOutPutFormatStaring("Show window test.");
	getchar();
	return 0;
}

