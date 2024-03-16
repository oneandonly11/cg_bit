#include "Light.hlsli"
[maxvertexcount(6)]
void GS(
	triangle VertexInTex input[3] : POSITION,
	inout TriangleStream<VertexOutTex> output
)
{
	matrix viewProj = mul(g_View, g_Proj);
	VertexOutTex main = (VertexOutTex)0;
	VertexOutTex mirror = (VertexOutTex)0;
	[unroll];
	for (uint i = 0; i < 3; i++)
	{
		VertexInTex vIn = input[i];
		float4 posW = mul(float4(vIn.PosL, 1.0f), g_World);
		float3 normalW = mul(vIn.NormalL, (float3x3) g_WorldInvTranspose);
		if (g_IsReflection)
		{
			posW = mul(posW, g_Reflection);
			normalW = mul(normalW, (float3x3) g_Reflection);
		}
		main.PosH = mul(posW, viewProj);
		main.PosW = posW.xyz;
		main.NormalW = normalW;
        main.Tex = mul(float4(vIn.Tex, 0.0f, 1.0f), (float4x4) g_RotationMatrix);//利用矩阵乘法实现图片旋转
		output.Append(main);
	}
	 output.RestartStrip();
	[unroll];
        int a[3]={1,0,2};
	for (uint i = 0; i < 3; i++)
	{
		VertexInTex vIn = input[a[i]];
		float4 posW = mul(float4(vIn.PosL, 1.0f), g_World);
		vIn.NormalL.x = -vIn.NormalL.x;
		float3 normalW = mul(vIn.NormalL, (float3x3) g_WorldInvTranspose);

		posW.x = 50 - posW.x;
        if (g_IsReflection)
        {
            posW = mul(posW, g_Reflection);
            normalW = mul(normalW, (float3x3) g_Reflection);
        }
		mirror.PosH = mul(posW, viewProj);
                mirror.PosW = posW.xyz;
		
        	mirror.NormalW = normalW;
        mirror.Tex = mul(float4(vIn.Tex, 0.0f, 1.0f), (float4x4) g_RotationMatrix);
        	output.Append(mirror);
    }
}