#include "Light.hlsli"
[maxvertexcount(6)]
void GS(
	triangle VertexIn input[3]: POSITION,
	inout TriangleStream< VertexOut > output
)
{
	matrix viewProj = mul(g_View, g_Proj);
	VertexOut main = (VertexOut)0;
	VertexOut mirror = (VertexOut)0;
	[unroll]
	for (uint i = 0; i < 3; i++)
	{
		VertexIn vIn = input[i];
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
		main.Color = vIn.Color;
		output.Append(main);
	}
	output.RestartStrip();
	[unroll]
	for (uint i = 0; i < 3; i++)
	{
		VertexIn vIn = input[i];
		vIn.NormalL.x = -vIn.NormalL.x;
		float4 posW = mul(float4(vIn.PosL, 1.0f), g_World);
		float3 normalW = mul(vIn.NormalL, (float3x3) g_WorldInvTranspose);
		if (g_IsReflection)
    		{
			posW = mul(posW, g_Reflection);
			normalW = mul(normalW, (float3x3) g_Reflection);
    		}
		posW.x = 50 - posW.x;
		mirror.PosH = mul(posW, viewProj);
		mirror.PosW = posW.xyz;
        mirror.NormalW = normalW;
		mirror.Color = float4(1.0, 0.0, 0.0, 0.5);
		output.Append(mirror);
	}
}