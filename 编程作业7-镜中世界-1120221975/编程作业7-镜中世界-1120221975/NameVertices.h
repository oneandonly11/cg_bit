#pragma once

#include "GameApp.h"

class NameVertices
{
public:
	NameVertices(D3D11_PRIMITIVE_TOPOLOGY type);
	~NameVertices();

	// 获取顶点
	GameApp::VertexPosNormalTex* GetMirrorVertices();
	GameApp::VertexPosNormalColor* GetNameVertices();
	GameApp::VertexPosNormalTex* GetNameTexVertices();
	// 获取索引
	WORD* GetNameIndices();
	WORD* GetTexNameIndices();
	WORD* GetMirrorIndices();
	// 获取绘制图元类型
	D3D11_PRIMITIVE_TOPOLOGY GetTopology();
	// 获取顶点个数 
	UINT GetVerticesCount();
	// 获取索引个数
	UINT GetIndexCount();

private:
	GameApp::VertexPosNormalTex* nameTexVertices = new GameApp::VertexPosNormalTex[12]{//照片顶点
		DirectX::XMFLOAT3(25.0f,10.0f,10.0f),DirectX::XMFLOAT3(-1.0f,0.0f,0.0f), DirectX::XMFLOAT2(0.0f,0.0f),
		DirectX::XMFLOAT3(25.0f,10.0f,-10.0f),DirectX::XMFLOAT3(-1.0f,0.0f,0.0f), DirectX::XMFLOAT2(1.0f,0.0f),
		DirectX::XMFLOAT3(25.0f,-10.0f,10.0f),DirectX::XMFLOAT3(-1.0f,0.0f,0.0f), DirectX::XMFLOAT2(0.0f,1.0f),
		DirectX::XMFLOAT3(25.0f,-10.0f,-10.0f), DirectX::XMFLOAT3(-1.0f,0.0f,0.0f), DirectX::XMFLOAT2(1.0f,1.0f)
	};
	GameApp::VertexPosNormalTex* MirrorVertices = new GameApp::VertexPosNormalTex[12]{
		DirectX::XMFLOAT3(-30.0f,35.0f,-30.0f),DirectX::XMFLOAT3(0.0f,0.0f,1.0f), DirectX::XMFLOAT2(0.0f, 0.0f),
		DirectX::XMFLOAT3(-30.0f,-20.0f,-30.0f),DirectX::XMFLOAT3(0.0f,0.0f,1.0f), DirectX::XMFLOAT2(1.0f, 1.0f),
		DirectX::XMFLOAT3(80.0f,-20.0f,-30.0f),DirectX::XMFLOAT3(0.0f,0.0f,1.0f), DirectX::XMFLOAT2(0.0f, 1.0f),
		DirectX::XMFLOAT3(80.0f,35.0f,-30.0f),DirectX::XMFLOAT3(0.0f,0.0f,1.0f), DirectX::XMFLOAT2(1.0f,0.0f),
	};
	GameApp::VertexPosNormalColor* nameVertices; // 顶点
	WORD* nameIndices; // 索引
	WORD* nameTexIndices = new WORD[6]{//照片索引
		0,3,2,
		0,1,3,
	};

	WORD* mirrorIndices = new WORD[6]{
		0,2,1,
		0,3,2
	};
	D3D11_PRIMITIVE_TOPOLOGY topology;// 图元类型 
	UINT verticesCount; // 顶点个数
	UINT indexCount; // 索引个数
};