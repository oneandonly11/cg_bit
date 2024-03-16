#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include "NameVertices.h"
using namespace DirectX;

static float phi2 = 0.0f;

const D3D11_INPUT_ELEMENT_DESC GameApp::VertexPosNormalColor::inputLayout[3] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const D3D11_INPUT_ELEMENT_DESC GameApp::VertexPosNormalTex::inputLayout[3] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
};
GameApp::GameApp(HINSTANCE hInstance)
	: D3DApp(hInstance),
	m_VSConstantBuffer(),
	m_PSConstantBuffer(),
	m_DirLight(),
	m_PointLight(),
	m_SpotLight(),
	name(nullptr),// 初始化变量，置空、、、、、、、、、、、、、、、、、、、、、
	nameN(40),
	angle(0),
	Lightn(0),
	dx(0),
	dy(0),
	dz(0),
	viewPos(0.0f, 20.0f, 33.0f),
	viewRot(0.5f, 3.14f, 0.0f)
{
}

GameApp::~GameApp()
{
	delete name;// 释放内存、、、、、、、、、、、、、、、、、、、、、、、、、、
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

	if (!InitEffect())
		return false;

	if (!InitResource())
		return false;

	// 初始化鼠标，键盘不需要
	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);

	return true;
}

void GameApp::OnResize()
{
	D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt)
{
	phi2 += 0.01f;
	XMMATRIX texMat = XMMatrixTranslation(-0.5f, -0.5f, 0.0f) * XMMatrixRotationZ(phi2) * XMMatrixTranslation(0.5f, 0.5f, 0.0f);
	m_VSConstantBuffer.RotationMatrix = XMMatrixTranspose(texMat);//更新旋转矩阵

	angle += 1.5f * dt;
	
	// 获取鼠标状态
	Mouse::State mouseState = m_pMouse->GetState();
	Mouse::State lastMouseState = m_MouseTracker.GetLastState();
	// 获取键盘状态
	Keyboard::State keyState = m_pKeyboard->GetState();
	Keyboard::State lastKeyState = m_KeyboardTracker.GetLastState();

	// 更新鼠标按钮状态跟踪器，仅当鼠标按住的情况下才进行移动
	m_MouseTracker.Update(mouseState);
	m_KeyboardTracker.Update(keyState);

	XMFLOAT3 pos = { 0, 0, 0 };
	float moveSpeed = 40.0f;
	if (keyState.IsKeyDown(Keyboard::LeftShift)) moveSpeed *= 2.0f;
	if (keyState.IsKeyDown(Keyboard::W)) pos.z += moveSpeed * dt;
	if (keyState.IsKeyDown(Keyboard::S)) pos.z -= moveSpeed * dt;
	if (keyState.IsKeyDown(Keyboard::A)) pos.x -= moveSpeed * dt;
	if (keyState.IsKeyDown(Keyboard::D)) pos.x += moveSpeed * dt;
	if (keyState.IsKeyDown(Keyboard::Q)) viewRot.z += 0.5f * dt;
	if (keyState.IsKeyDown(Keyboard::E)) viewRot.z -= 0.5f * dt;

	if (mouseState.leftButton == true && m_MouseTracker.leftButton == m_MouseTracker.HELD) // 这两者似乎只有在鼠标按下的那一帧存在区别(此时左为true，右为false)?
	{
		viewRot.y += (mouseState.x - lastMouseState.x) * 0.5f * dt;
		viewRot.x += (mouseState.y - lastMouseState.y) * 0.5f * dt;
	}

	if (viewRot.x > 1.5f) viewRot.x = 1.5f;
	else if (viewRot.x < -1.5f) viewRot.x = -1.5f;
	if (viewRot.z > 0.7f) viewRot.z = 0.7f;
	else if (viewRot.z < -0.7f) viewRot.z = -0.7f;


	XMStoreFloat3(&pos, XMVector3Transform(XMLoadFloat3(&pos), XMMatrixRotationX(viewRot.x) * XMMatrixRotationZ(viewRot.z) * XMMatrixRotationY(viewRot.y)));
	viewPos.x += pos.x;
	//由于墙面影响整体观察故取消
	//设立墙面，防止穿墙
	/*if (viewPos.x > 30.0f)viewPos.x = 30.0f;
	else if (viewPos.x < -30.0f)viewPos.x = -30.0f;*/
	viewPos.y += pos.y;
	//设立地面，防止穿透地面
	/*if (viewPos.y < 0.0f)viewPos.y = 0.0f;*/
	viewPos.z += pos.z;
	//设立墙面，防止穿墙
	/*if (viewPos.z > 33.0f)viewPos.z = 33.0f;
	else if (viewPos.z < -33.0f)viewPos.z = -33.0f;*/
	m_VSConstantBuffer.view = XMMatrixTranspose(XMMatrixTranslation(-viewPos.x, -viewPos.y, -viewPos.z) * XMMatrixRotationY(-viewRot.y) * XMMatrixRotationZ(-viewRot.z) * XMMatrixRotationX(-viewRot.x));
	m_PSConstantBuffer.eyePos = { viewPos.x, viewPos.y, viewPos.z, 0.0f };

	if (m_KeyboardTracker.IsKeyPressed(Keyboard::D3))
	{
		m_PSConstantBuffer.dirLight = m_DirLight;
		m_PSConstantBuffer.pointLight = PointLight();
		m_PSConstantBuffer.spotLight = SpotLight();

	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D1))
	{
		m_PSConstantBuffer.dirLight = DirectionalLight();
		m_PSConstantBuffer.pointLight = m_PointLight;
		m_PSConstantBuffer.spotLight = SpotLight();
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D2))
	{
		m_PSConstantBuffer.dirLight = DirectionalLight();
		m_PSConstantBuffer.pointLight = PointLight();
		m_PSConstantBuffer.spotLight = m_SpotLight;
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::F))
	{
		m_IsWireframeMode = !m_IsWireframeMode;
		m_pd3dImmediateContext->RSSetState(m_IsWireframeMode ? m_pRSWireframe.Get() : nullptr);
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::OemPlus)) 
	{
		dx -= 0.5f;
		dy -= 0.5f;
		dz -= 0.5f;
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::OemMinus))
	{
		dx += 0.5f;
		dy += 0.5f;
		dz += 0.5f;
	}
	if (dz > 0.0f)
	{
		dz = 0.0f;
		dy = 0.0f;
		dx = 0.0f;
	}
	else if (dz < -5.0f)
	{ 
		dz = -5.0f;
		dy = -5.0f;
		dx = -5.0f;
	}
	m_PSConstantBuffer.addLight.lightIntensity.x = m_AddLight.lightIntensity.x -dx;
	m_PSConstantBuffer.addLight.lightIntensity.y = m_AddLight.lightIntensity.y -dy;
	m_PSConstantBuffer.addLight.lightIntensity.z = m_AddLight.lightIntensity.z -dz;
	m_PSConstantBuffer.pointLight.position.x = sinf(angle * 0.2f) * 20.0f;
	m_PSConstantBuffer.pointLight.position.z = sinf(angle * 0.5f) * 20.0f;



	m_PSConstantBuffer.spotLight.position = viewPos;
	XMFLOAT3 dir = { 0, 0, 1.0f };
	XMStoreFloat3(&m_PSConstantBuffer.spotLight.direction, XMVector3Transform(XMLoadFloat3(&dir), XMMatrixRotationX(viewRot.x) * XMMatrixRotationZ(viewRot.z) * XMMatrixRotationY(viewRot.y)));


}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);

	m_VSConstantBuffer.world = XMMatrixTranspose(XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	m_VSConstantBuffer.worldInvTranspose = XMMatrixInverse(nullptr, XMMatrixTranspose(m_VSConstantBuffer.world));


	DrawMirror();

	m_VSConstantBuffer.world = XMMatrixTranspose(XMMatrixScaling(10.0f, 10.0f, 10.0f) * XMMatrixTranslation(0.0f, 5.0f, 0.0f));
	m_VSConstantBuffer.worldInvTranspose = XMMatrixInverse(nullptr, XMMatrixTranspose(m_VSConstantBuffer.world));

	DrawName(1);

	

	
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);
	int N = 11;//修改了字符数量。
	for (int i = 1; i <= N; i++)
	{
		for (int j = 1; j <= N; j++)
		{
			//定义了相对四边的距离、、、、、、、、、、、
			int x = i > 6 ? 12 - i : i, y = j > 6 ? 12 - j : j, d = x + y;
			//修改了字符起伏规律、、、、、、、、、、、、、、
			float scale = (sinf(angle * 3.0f + (x * y) * 0.2f) + 0.1 * d + 1.4f) * 0.35f;
			//修改了字符大小，字符旋转速度与位置的关系、、、、、、、、、、、
			DirectX::XMMATRIX mScale = XMMatrixScaling((0.2 * d + 0.4) * scale, (0.2 * d + 0.4) * scale, (0.2 * d + 0.4) * scale);
			DirectX::XMMATRIX mRotate = XMMatrixRotationX((1.8 - 0.15 * d) * angle) * XMMatrixRotationY((1.8 - 0.15 * d) * angle);
			//使得修改数量后字符居中、、、、、、、、、、
			DirectX::XMMATRIX mTranslate = XMMatrixTranslation((i - 6.0f) * 4.25f, 0, (j - 6.0f) * 4.25f);
			m_VSConstantBuffer.world = XMMatrixTranspose(mScale * mRotate * mTranslate); // mul(vec, mat) 中为行向量，矩阵右乘，顺序SRT, 参考https://www.cnblogs.com/X-Jun/p/9808727.html#_lab2_1_1
			m_VSConstantBuffer.worldInvTranspose = XMMatrixInverse(nullptr, XMMatrixTranspose(m_VSConstantBuffer.world));

			DrawName(1);

			srand(i * j + 1);
			DirectX::XMMATRIX mScaleChild = XMMatrixScaling(0.15, 0.15, 0.15);
			DirectX::XMMATRIX mTranslateChild = XMMatrixTranslation(5.5f - 0.5 * d, 0, 0);
			for (int k = 1; k <= 2 + rand() % 3; k++)
			{
				DirectX::XMMATRIX mRotateChild = XMMatrixRotationX(rand() + angle) * XMMatrixRotationY(rand() + angle) * XMMatrixRotationY(rand() + angle);
				m_VSConstantBuffer.world = XMMatrixTranspose(mScaleChild * mTranslateChild * mScale * mRotateChild * mTranslate);
				m_VSConstantBuffer.worldInvTranspose = XMMatrixInverse(nullptr, XMMatrixTranspose(m_VSConstantBuffer.world));

				DrawName(1);
			}
		}
	}
	m_VSConstantBuffer.world = XMMatrixTranspose(XMMatrixScaling(1.0f, 2.0f, 2.0f) * XMMatrixTranslation(0.0f, 10.0f, 0.0f));
	m_VSConstantBuffer.worldInvTranspose = XMMatrixInverse(nullptr, XMMatrixTranspose(m_VSConstantBuffer.world));
	DrawPic(1);
	
	
	m_VSConstantBuffer.world = XMMatrixTranspose(XMMatrixScaling(10.0f, 10.0f, 10.0f) * XMMatrixTranslation(0.0f, 5.0f, 0.0f));
	m_VSConstantBuffer.worldInvTranspose = XMMatrixInverse(nullptr, XMMatrixTranspose(m_VSConstantBuffer.world));

	DrawName(0);

	m_VSConstantBuffer.world = XMMatrixTranspose(XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	m_VSConstantBuffer.worldInvTranspose = XMMatrixInverse(nullptr, XMMatrixTranspose(m_VSConstantBuffer.world));
	DrawMirror_back();
	
	
	for (int i = 1; i <= N; i++)
	{
		for (int j = 1; j <= N; j++)
		{
			//定义了相对四边的距离、、、、、、、、、、、
			int x = i > 6 ? 12 - i : i, y = j > 6 ? 12 - j : j, d = x + y;
			//修改了字符起伏规律、、、、、、、、、、、、、、
			float scale = (sinf(angle * 3.0f + (x * y) * 0.2f) + 0.1 * d + 1.4f) * 0.35f;
			//修改了字符大小，字符旋转速度与位置的关系、、、、、、、、、、、
			DirectX::XMMATRIX mScale = XMMatrixScaling((0.2 * d + 0.4) * scale, (0.2 * d + 0.4) * scale, (0.2 * d + 0.4) * scale);
			DirectX::XMMATRIX mRotate = XMMatrixRotationX((1.8 - 0.15 * d) * angle) * XMMatrixRotationY((1.8 - 0.15 * d) * angle);
			//使得修改数量后字符居中、、、、、、、、、、
			DirectX::XMMATRIX mTranslate = XMMatrixTranslation((i - 6.0f) * 4.25f, 0, (j - 6.0f) * 4.25f);
			m_VSConstantBuffer.world = XMMatrixTranspose(mScale * mRotate * mTranslate); // mul(vec, mat) 中为行向量，矩阵右乘，顺序SRT, 参考https://www.cnblogs.com/X-Jun/p/9808727.html#_lab2_1_1
			m_VSConstantBuffer.worldInvTranspose = XMMatrixInverse(nullptr, XMMatrixTranspose(m_VSConstantBuffer.world));

			DrawName(0);

			srand(i * j + 1);
			DirectX::XMMATRIX mScaleChild = XMMatrixScaling(0.15, 0.15, 0.15);
			DirectX::XMMATRIX mTranslateChild = XMMatrixTranslation(5.5f - 0.5 * d, 0, 0);
			for (int k = 1; k <= 2 + rand() % 3; k++)
			{
				DirectX::XMMATRIX mRotateChild = XMMatrixRotationX(rand() + angle) * XMMatrixRotationY(rand() + angle) * XMMatrixRotationY(rand() + angle);
				m_VSConstantBuffer.world = XMMatrixTranspose(mScaleChild * mTranslateChild * mScale * mRotateChild * mTranslate);
				m_VSConstantBuffer.worldInvTranspose = XMMatrixInverse(nullptr, XMMatrixTranspose(m_VSConstantBuffer.world));

				DrawName(0);
			}
		}
	}
	m_VSConstantBuffer.world = XMMatrixTranspose(XMMatrixScaling(1.0f, 2.0f, 2.0f) * XMMatrixTranslation(0, 10.0f, 0.0f));
	m_VSConstantBuffer.worldInvTranspose = XMMatrixInverse(nullptr, XMMatrixTranspose(m_VSConstantBuffer.world));
	m_PSConstantBuffer.material.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f);
	DrawPic(0);
	
	HR(m_pSwapChain->Present(0, 0));
}

void GameApp::DrawName(bool isReflection)
{
	

	if (isReflection) {
		// 开启反射绘制
		m_VSConstantBuffer.isReflection = true;
		// 绘制不透明物体，需要顺时针裁剪
		// 仅对模板值为1的镜面区域绘制
		m_pd3dImmediateContext->RSSetState(RenderStates::RSCullClockWise.Get());
		m_pd3dImmediateContext->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(), 1);
		m_pd3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	}
	else {
		//不透明绘制
		m_VSConstantBuffer.isReflection = false;
		m_pd3dImmediateContext->RSSetState(nullptr);
		m_pd3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
		m_pd3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	}

	m_pName.Draw(m_pd3dImmediateContext, m_VSConstantBuffer, m_PSConstantBuffer);
}
void GameApp::DrawPic(bool isReflection)
{

	if (isReflection)
	{
		m_VSConstantBuffer.isReflection = true;
		m_pd3dImmediateContext->RSSetState(RenderStates::RSNoCull.Get());
		m_pd3dImmediateContext->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(), 1);
		m_pd3dImmediateContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);

		m_pPic.Draw(m_pd3dImmediateContext, m_VSConstantBuffer, m_PSConstantBuffer);

	
		m_VSConstantBuffer.world = XMMatrixTranspose(XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(0.0f, 0.0f, 0.0f));
		m_VSConstantBuffer.worldInvTranspose = XMMatrixInverse(nullptr, XMMatrixTranspose(m_VSConstantBuffer.world));


		m_pMirror.Draw(m_pd3dImmediateContext, m_VSConstantBuffer, m_PSConstantBuffer);

	}
	else
	{
		m_VSConstantBuffer.isReflection = false;
		m_pd3dImmediateContext->RSSetState(RenderStates::RSNoCull.Get());
		m_pd3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
		m_pd3dImmediateContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
		
		m_pPic.Draw(m_pd3dImmediateContext, m_VSConstantBuffer, m_PSConstantBuffer);
	}
}
void GameApp::DrawMirror_back()
{


	
		//不透明绘制
		m_VSConstantBuffer.isReflection = false;
		m_pd3dImmediateContext->RSSetState(nullptr);
		m_pd3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
		m_pd3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	

		m_pMirror_back.Draw(m_pd3dImmediateContext, m_VSConstantBuffer, m_PSConstantBuffer);
}
void GameApp::DrawMirror() {
	m_pd3dImmediateContext->RSSetState(nullptr);
	m_pd3dImmediateContext->OMSetDepthStencilState(RenderStates::DSSWriteStencil.Get(), 1);
	m_pd3dImmediateContext->OMSetBlendState(RenderStates::BSNoColorWrite.Get(), nullptr, 0xFFFFFFFF);

	m_pMirror.Draw(m_pd3dImmediateContext, m_VSConstantBuffer, m_PSConstantBuffer);
}

bool GameApp::InitEffect()
{
	ComPtr<ID3DBlob> blob;

	//Name
	// 创建顶点着色器
	HR(CreateShaderFromFile(L"HLSL\\Light_VS.cso", L"HLSL\\Light_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));
	// 创建并绑定顶点布局
	HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalColor::inputLayout, ARRAYSIZE(VertexPosNormalColor::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));
	//创建几何着色器
	HR(CreateShaderFromFile(L"HLSL\\Mirror_GS.cso", L"HLSL\\Mirror_GS.hlsl", "GS", "gs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pGeometryShader.GetAddressOf()));
	// 创建像素着色器
	HR(CreateShaderFromFile(L"HLSL\\Light_PS.cso", L"HLSL\\Light_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));



	//Pic
	// 创建顶点着色器
	HR(CreateShaderFromFile(L"HLSL\\Tex_VS.cso", L"HLSL\\Tex_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexTexShader.GetAddressOf()));
	// 创建并绑定顶点布局
	HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout), blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexTexLayout.GetAddressOf()));
	//创建几何着色器
	HR(CreateShaderFromFile(L"HLSL\\Tex_GS.cso", L"HLSL\\Tex_GS.hlsl", "GS", "gs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pGeometryTexShader.GetAddressOf()));
	// 创建像素着色器
	HR(CreateShaderFromFile(L"HLSL\\Tex_PS.cso", L"HLSL\\Tex_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelTexShader.GetAddressOf()));

	//mirror
	//创建几何着色器
	HR(CreateShaderFromFile(L"HLSL\\Reflect_GS.cso", L"HLSL\\Reflect_GS.hlsl", "GS", "gs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pGeometryMirShader.GetAddressOf()));

	//mirror_back
	//创建几何着色器
	HR(CreateShaderFromFile(L"HLSL\\Back_GS.cso", L"HLSL\\Back_GS.hlsl", "GS", "gs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pGeometryBackShader.GetAddressOf()));

	return true;
}

bool GameApp::InitResource()
{
	// 创建对象，绘制类型为0、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、
	name = new NameVertices(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// ******************
	// 初始化游戏对象
	ComPtr<ID3D11ShaderResourceView> textureresource;
	Material material{};
	material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.specular = XMFLOAT4(0.4f, 0.2f, 0.2f, 16.0f);
	//初始化名字
	m_pName.is_Tex = false;
	m_pName.SetShaderLayout(m_pVertexShader, m_pGeometryShader, m_pPixelShader, m_pVertexLayout);
	m_pName.SetBuffer(m_pd3dDevice.Get(), name->GetNameVertices(), (UINT)name->GetVerticesCount(), name->GetNameIndices(),(UINT)name->GetIndexCount());
	m_pName.SetMaterial(material);
	

	//初始化镜子背面
	m_pMirror_back.is_Tex = true;
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"texture\\ice.dds", nullptr, textureresource.GetAddressOf()));
	m_pMirror_back.SetShaderLayout(m_pVertexTexShader, m_pGeometryBackShader, m_pPixelTexShader, m_pVertexTexLayout);
	m_pMirror_back.SetBuffer(m_pd3dDevice.Get(), name->GetMirrorVertices(), (UINT)4, name->GetMirrorIndices(), (UINT)6);
	m_pMirror_back.SetMaterial(material);
	m_pMirror_back.SetTexture(textureresource.Get());
	//更改材质
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	material.specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	//初始化图片
	m_pPic.is_Tex = true;
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"texture\\face.dds", nullptr, textureresource.GetAddressOf()));
	m_pPic.SetShaderLayout(m_pVertexTexShader, m_pGeometryTexShader, m_pPixelTexShader, m_pVertexTexLayout);
	m_pPic.SetBuffer(m_pd3dDevice.Get(), name->GetNameTexVertices(), (UINT)4, name->GetTexNameIndices(), (UINT)6);
	m_pPic.SetMaterial(material);
	m_pPic.SetTexture(textureresource.Get());
	
	//初始化镜子
	m_pMirror.is_Tex = true;
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"texture\\ice.dds", nullptr, textureresource.GetAddressOf()));
	m_pMirror.SetShaderLayout(m_pVertexTexShader, m_pGeometryMirShader, m_pPixelTexShader, m_pVertexTexLayout);
	m_pMirror.SetBuffer(m_pd3dDevice.Get(), name->GetMirrorVertices(), (UINT)4, name->GetMirrorIndices(), (UINT)6);
	m_pMirror.SetMaterial(material);
	m_pMirror.SetTexture(textureresource.Get());
	
	// 初始化采样器状态
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(m_pd3dDevice->CreateSamplerState(&sampDesc, m_pSamplerState.GetAddressOf()));
	// ******************
	// 设置常量缓冲区描述
	//
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(VSConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 新建用于VS和PS的常量缓冲区
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[0].GetAddressOf()));
	cbd.ByteWidth = sizeof(PSConstantBuffer);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[1].GetAddressOf()));

	m_PSConstantBuffer.reflect = XMMatrixTranspose(XMMatrixReflect(XMVectorSet(0.0f, 0.0f, -1.0f, -30.0f)));
	// ******************
	// 初始化默认光照
	//
	m_AddLight.direction = XMFLOAT3(0.577f, -0.577f, -0.577f);//调整光源方向以保障照片正面亮度
	m_AddLight.lightIntensity= XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	// 方向光
	m_DirLight.lightIntensity = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_DirLight.direction = XMFLOAT3(0.577f, -0.577f, -0.577f);
	// 点光
	m_PointLight.position = XMFLOAT3(0.0f, 0.0f, -10.0f);
	m_PointLight.lightIntensity = XMFLOAT4(0.4f, 0.8f, 0.4f, 1.0f);
	m_PointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_PointLight.range = 25.0f;
	// 聚光灯
	m_SpotLight.position = XMFLOAT3(0.0f, 0.0f, -5.0f);
	m_SpotLight.direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_SpotLight.lightIntensity = XMFLOAT4(0.4f, 0.4f, 0.8f, 1.0f);
	m_SpotLight.att = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_SpotLight.spot = 12.0f;
	m_SpotLight.range = 10000.0f;
	// 初始化用于VS的常量缓冲区的值
	m_VSConstantBuffer.world = XMMatrixIdentity();
	m_VSConstantBuffer.view = XMMatrixTranspose(XMMatrixTranslation(-viewPos.x, -viewPos.y, -viewPos.z) * XMMatrixRotationY(-viewRot.y) * XMMatrixRotationZ(-viewRot.z) * XMMatrixRotationX(-viewRot.x));
	m_VSConstantBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));
	m_VSConstantBuffer.worldInvTranspose = XMMatrixIdentity();
	m_VSConstantBuffer.RotationMatrix = XMMatrixIdentity();
	m_VSConstantBuffer.isReflection = false;

	// 初始化用于PS的常量缓冲区的值
	m_PSConstantBuffer.material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_PSConstantBuffer.material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_PSConstantBuffer.material.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 5.0f);
	// 使用默认平行光
	m_PSConstantBuffer.addLight = m_AddLight;
	m_PSConstantBuffer.dirLight = m_DirLight;
	// 注意不要忘记设置此处的观察位置，否则高亮部分会有问题
	m_PSConstantBuffer.eyePos = { viewPos.x, viewPos.y, viewPos.z, 0.0f };

	// 更新PS常量缓冲区资源
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_VSConstantBuffer, sizeof(PSConstantBuffer));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);

	// ******************
	// 初始化光栅化状态
	//
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;
	HR(m_pd3dDevice->CreateRasterizerState(&rasterizerDesc, m_pRSWireframe.GetAddressOf()));

	RenderStates::InitAll(m_pd3dDevice.Get());
	// ******************
	// 给渲染管线各个阶段绑定好所需资源
	//


	// 设置图元类型，设定输入布局
	m_pd3dImmediateContext->IASetPrimitiveTopology(name->GetTopology());// 通过对象设置图元类型、、、、、、、、、、、、、、、、、、、、、
	// VS常量缓冲区对应HLSL寄存于b0的常量缓冲区
	m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
	m_pd3dImmediateContext->VSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
	// GS常量缓冲区对应HLSL寄存于b0的常量缓冲区
	m_pd3dImmediateContext->GSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
	m_pd3dImmediateContext->GSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
	// PS常量缓冲区对应HLSL寄存于b1的常量缓冲区
	m_pd3dImmediateContext->PSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
	m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
	// PS采样器对应位图资源寄存于s0的采样器
	m_pd3dImmediateContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());
	
	// ******************
	// 设置调试对象名
	//
	return true;
}
template<class VertexType, class IndexType>
void GameApp::GameObject::SetBuffer(ID3D11Device* device, VertexType* vertices, UINT VertexCount, IndexType* indices, UINT IndexCount) {
	// 释放旧资源
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	// 设置顶点缓冲区描述
	m_VertexStride = sizeof(VertexType);
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = m_VertexStride * VertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	HR(device->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

	// 设置索引缓冲区描述
	m_IndexCount = IndexCount;
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = m_IndexCount * sizeof(IndexType);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 新建索引缓冲区
	InitData.pSysMem = indices;
	HR(device->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
}
void GameApp::GameObject::SetTexture(ID3D11ShaderResourceView* texture) {
	m_pTexture = texture;
}
void GameApp::GameObject::SetMaterial(const Material& material){
	m_pMaterial = material;
}
void GameApp::GameObject::SetShaderLayout(const ComPtr<ID3D11VertexShader> VS, const ComPtr<ID3D11GeometryShader> GS, const ComPtr<ID3D11PixelShader> PS, const ComPtr<ID3D11InputLayout> VL) {
	m_pGS = GS;
	m_pPS = PS;
	m_pVS = VS;
	m_pVL = VL;
}
void GameApp::GameObject::Draw(ComPtr<ID3D11DeviceContext> deviceContext, VSConstantBuffer& m_VSConstantBuffer, PSConstantBuffer& m_PSConstantBuffer)
{
	// 设置顶点/索引缓冲区
	UINT strides = m_VertexStride;
	UINT offsets = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offsets);
	deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	// 获取之前已经绑定到渲染管线上的常量缓冲区并进行修改
	ComPtr<ID3D11Buffer> cBuffer = nullptr;
	deviceContext->VSGetConstantBuffers(0, 1, cBuffer.GetAddressOf());
	// 更新常量缓冲区
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(deviceContext->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(VSConstantBuffer), &m_VSConstantBuffer, sizeof(VSConstantBuffer));
	deviceContext->Unmap(cBuffer.Get(), 0);

	deviceContext->PSGetConstantBuffers(1, 1, cBuffer.GetAddressOf());
	m_PSConstantBuffer.material = m_pMaterial;
	HR(deviceContext->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
	deviceContext->Unmap(cBuffer.Get(), 0);

	deviceContext->IASetInputLayout(m_pVL.Get());
	deviceContext->VSSetShader(m_pVS.Get(), nullptr, 0);
	deviceContext->GSSetShader(m_pGS.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_pPS.Get(), nullptr, 0);
	// 设置纹理
	if(is_Tex)
	deviceContext->PSSetShaderResources(0, 1, m_pTexture.GetAddressOf());
	// 可以开始绘制
	deviceContext->DrawIndexed(m_IndexCount, 0, 0);
}
