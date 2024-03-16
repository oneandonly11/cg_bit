#include "Light.hlsli"

// 像素着色器
float4 PS(VertexOut pIn) : SV_Target
{
    
    // 标准化法向量
    pIn.NormalW = normalize(pIn.NormalW);

    // 顶点指向眼睛的向量
    float3 toEyeW = normalize(g_EyePosW - pIn.PosW);

    // 初始化为0 
    float4 ambient, diffuse, spec;
    float4 A, D, S;
    ambient = diffuse = spec = A = D = S = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    
    DirectionalLight dirlight=g_DirLight;
    PointLight pointlight = g_PointLight;
    SpotLight spotlight = g_SpotLight;
    AddLight addlight = g_AddLight;
    
    //反射变换
    if (g_IsReflection)
    {
        //[unroll]
        
        dirlight.Direction = mul(dirlight.Direction, (float3x3) (g_Reflection));
        addlight.Direction = mul(addlight.Direction, (float3x3) (g_Reflection));
        pointlight.Position = (float3) mul(float4(pointlight.Position, 1.0f), g_Reflection);

        spotlight.Position = (float3) mul(float4(spotlight.Position, 1.0f), g_Reflection);
        spotlight.Direction = mul(spotlight.Direction, (float3x3) g_Reflection);
    }
    
    ComputeAddLight(g_Material, addlight, pIn.NormalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;
    
    ComputeDirectionalLight(g_Material, dirlight, pIn.NormalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;

    ComputePointLight(g_Material, pointlight, pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;

    ComputeSpotLight(g_Material, spotlight, pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;

    float4 litColor = pIn.Color * (ambient + diffuse) + spec;
	
    litColor.a = g_Material.Diffuse.a * pIn.Color.a;
	
    return litColor;
}