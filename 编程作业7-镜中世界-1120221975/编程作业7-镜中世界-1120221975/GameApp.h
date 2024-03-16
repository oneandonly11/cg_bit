#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "LightHelper.h"
#include "RenderStates.h"

class NameVertices;// 声明类， 避免头文件与类之间的相互依赖、、、、、、、、、、、、、、、、、、、、、、、、

class GameApp : public D3DApp
{
public:
	struct VertexPosNormalColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT4 color;
		static const D3D11_INPUT_ELEMENT_DESC inputLayout[3];
	};
	struct VertexPosNormalTex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 tex;
		static const D3D11_INPUT_ELEMENT_DESC inputLayout[3];
	};
	struct VSConstantBuffer
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX proj;
		DirectX::XMMATRIX worldInvTranspose;
		DirectX::XMMATRIX RotationMatrix;
		bool isReflection;
	};

	struct PSConstantBuffer
	{
		AddLight addLight;
		DirectionalLight dirLight;
		PointLight pointLight;
		SpotLight spotLight;
		Material material;

		DirectX::XMFLOAT4 eyePos;
		DirectX::XMMATRIX reflect;
	};

	class GameObject
	{
	public:
		/*GameObject();*/

		// 设置缓冲区
		template<class VertexType, class IndexType>
		void SetBuffer(ID3D11Device* device, VertexType* vertices, UINT VertexCount, IndexType* indices, UINT IndexCount);
		//设置着色器
		void SetShaderLayout(const ComPtr<ID3D11VertexShader> VS, const ComPtr<ID3D11GeometryShader> GS, const ComPtr<ID3D11PixelShader> PS, const ComPtr<ID3D11InputLayout> VL);
		// 设置纹理
		void SetTexture(ID3D11ShaderResourceView* texture);
		// 绘制
		void Draw(ComPtr<ID3D11DeviceContext> deviceContext, VSConstantBuffer& m_VSConstantBuffer, PSConstantBuffer& m_PSConstantBuffer);
		// 设置材质
		void SetMaterial(const Material& material);
		//是否有纹理
		bool is_Tex;
	private:
		ComPtr<ID3D11ShaderResourceView> m_pTexture;        // 纹理
		Material m_pMaterial;								// 物体材质
		ComPtr<ID3D11Buffer> m_pVertexBuffer;               // 顶点缓冲区
		ComPtr<ID3D11Buffer> m_pIndexBuffer;                // 索引缓冲区
		ComPtr<ID3D11InputLayout> m_pVL;	    // 顶点输入布局
		ComPtr<ID3D11VertexShader> m_pVS;	    // 顶点着色器
		ComPtr<ID3D11PixelShader> m_pPS;		// 像素着色器
		ComPtr<ID3D11GeometryShader> m_pGS; // 几何着色器
		
		UINT m_VertexStride;                                // 顶点字节大小
		UINT m_IndexCount;                                  // 索引数目    
	};

public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();


private:
	bool InitEffect();
	bool InitResource();
	void DrawName(bool);
	void DrawPic(bool);
	void DrawMirror_back();
	void DrawMirror();

private:
	ComPtr<ID3D11InputLayout>m_pVertexMirrorLayout; // 镜子顶点输入布局
	ComPtr<ID3D11InputLayout>m_pVertexTexLayout;    // 面元顶点输入布局
	ComPtr<ID3D11InputLayout> m_pVertexLayout;	    // 顶点输入布局
	ComPtr<ID3D11Buffer> m_pVertexBuffer;		// 顶点缓冲区
	ComPtr<ID3D11Buffer> m_pIndexBuffer;			// 索引缓冲区
	ComPtr<ID3D11Buffer> m_pConstantBuffers[2];	    // 常量缓冲区

	ComPtr<ID3D11VertexShader> m_pVertexShader;	    // 顶点着色器
	ComPtr<ID3D11PixelShader> m_pPixelShader;		// 像素着色器
	ComPtr<ID3D11GeometryShader> m_pGeometryShader; // 几何着色器

	//含纹理的着色器
	ComPtr<ID3D11VertexShader> m_pVertexTexShader;	    // 顶点着色器
	ComPtr<ID3D11PixelShader> m_pPixelTexShader;		// 像素着色器
	ComPtr<ID3D11GeometryShader> m_pGeometryTexShader; // 几何着色器
	//镜子专属着色器
	ComPtr<ID3D11GeometryShader> m_pGeometryMirShader; //几何着色器
	ComPtr<ID3D11GeometryShader> m_pGeometryBackShader;

	GameObject m_pPic;			    // 照片
	GameObject m_pName;				// 名字
	GameObject m_pMirror;           //镜子
	GameObject m_pMirror_back;		//镜子背面

	VSConstantBuffer m_VSConstantBuffer;			// 用于修改用于VS的GPU常量缓冲区的变量
	PSConstantBuffer m_PSConstantBuffer;			// 用于修改用于PS的GPU常量缓冲区的变量

	ComPtr<ID3D11ShaderResourceView> m_pFace;			    // 脸纹理
	ComPtr<ID3D11SamplerState> m_pSamplerState;				    // 采样器状态

	AddLight m_AddLight;                // 添加光
	DirectionalLight m_DirLight;					// 默认环境光
	PointLight m_PointLight;						// 默认点光
	SpotLight m_SpotLight;						    // 默认汇聚光

	ComPtr<ID3D11RasterizerState> m_pRSWireframe;	// 光栅化状态: 线框模式
	bool m_IsWireframeMode;							// 当前是否为线框模式

	NameVertices* name;// 存储绘制名字的顶点、类型、、、、、、、、、、、、、、、、、、、、、、
	int nameN;
	float angle;
	float dx, dy, dz; //光强变化量
	int Lightn;       //灯光数
	DirectX::XMFLOAT3 viewPos;
	DirectX::XMFLOAT3 viewRot;
};


#endif