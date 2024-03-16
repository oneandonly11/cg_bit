#include "Light.hlsli"

// 旋转角度，单位为弧度

// 像素着色器
float4 PS(VertexOutTex pIn) : SV_Target
{
    float4 texColor = g_Tex.Sample(g_SamLinear, pIn.Tex);
    clip(texColor.a - 0.1f);
    // 标准化法向量
    pIn.NormalW = normalize(pIn.NormalW);

    // 顶点指向眼睛的向量
    float3 toEyeW = normalize(g_EyePosW - pIn.PosW);

    // 初始化为0 
    float4 ambient, diffuse, spec;
    float4 A, D, S;
    ambient = diffuse = spec = A = D = S = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    
    DirectionalLight dirlight=g_DirLight;
    AddLight addlight = g_AddLight;
    PointLight pointlight = g_PointLight;
    SpotLight spotlight = g_SpotLight;
    
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

    ComputeDirectionalLight(g_Material, dirlight, pIn.NormalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;

    ComputeAddLight(g_Material, addlight, pIn.NormalW, toEyeW, A, D, S);
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

    //float4 texColor = g_Tex.Sample(g_SamLinear, pIn.Tex);
   float4 litColor = texColor * (ambient + diffuse) + spec;
    litColor.a = texColor.a  * g_Material.Diffuse.a;
	
    return litColor;
}