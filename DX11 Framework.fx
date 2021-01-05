//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
/*
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
    
    float4 DiffuseMtrl;
    float4 DiffuseLight;
    float3 LightVecW;

    float gTime;

    float4 AmbientMaterial;
    float4 AmbientLight;

    float4 SpecularMaterial;
    float4 SpecularLight;
    float SpecularPower;
    float3 EyePosW;			// Camera position in world space
}
*/

cbuffer ConstantBufferLite : register (b0) {
    matrix World;
    matrix ViewProjT;        // View and Projection matrices pre-multiplied

    float4 DiffuseT;         // Diffuse Material and Light pre-multiplied
    float DiffuseMtrlAlpha;
    //float4 DiffuseMtrl;
    //float4 DiffuseLight;
    float3 LightVecW;

    float4 AmbientT;         // Ambient Material and Light pre-multiplied
    //float4 AmbientMaterial;
    //float4 AmbientLight;

    float4 SpecularT;        // Specular Material and Light pre-multiplied
    //float4 SpecularMaterial;
    //float4 SpecularLight;
    float SpecularPower;
    float3 EyePosW;			// Camera position in world space

    //float gTime;
}

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    //float3 Norm : NORMAL;
    float4 Color : COLOR0;
    float2 Tex : TEXCOORD0;
};

struct VS_INPUT {
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
//VS_OUTPUT VS( float4 Pos : POSITION, float3 NormalL : NORMAL, VS_INPUT input)
//VS_OUTPUT VS(float4 Pos : POSITION, float4 Color : COLOR)
VS_OUTPUT VS(VS_INPUT input)
{
    // [ D1, D2, D3, F2 ]
    VS_OUTPUT output = (VS_OUTPUT)0;

    output.Pos = mul( input.Pos, World );

    // Compute the vector from the vertex to the eye position
    float3 toEye = normalize(EyePosW - output.Pos.xyz);

    // Apply View and Projection transformations
    output.Pos = mul(output.Pos, ViewProjT);
    //output.Pos = mul( output.Pos, View );
    //output.Pos = mul( output.Pos, Projection );

    // Convert normal from local space to world space 
    // W component of vector is 0 as vectors cannot be translated
    float3 normalW = normalize(mul(float4(input.Norm, 0.0f), World).xyz);
    //normalW = normalize(normalW);

    // Compute Colour
    //Compute the reflection vector
    float3 rVector = reflect(-LightVecW, normalW);
    
    // Determine how much (if any) specular light
    // makes it into the eye
    float specularAmount = pow(max(dot(rVector, toEye), 0.0f), SpecularPower);

    // Compute the ambient, diffuse and specular terms separately
    // Calculate Diffuse lighting
    float diffuseAmount = max(dot(LightVecW, normalW), 0.0f);
    //float3 diffuse = diffuseAmount * (DiffuseMtrl * DiffuseLight).rgb;
    float3 diffuse = diffuseAmount * DiffuseT.rgb;

    // Calculate Ambient lighting
    //float3 ambient = AmbientMaterial * AmbientLight;

    // Calculate Specular lighting
    //float3 specular = specularAmount * (SpecularMaterial * SpecularLight).rgb;
    float3 specular = specularAmount * SpecularT.rgb;

    // Sum all the lighting terms together
    output.Color.rgb = AmbientT.rgb + diffuse + specular;

    // Copy over the diffuse alpha
    //output.Color.a = DiffuseMtrl.a;
    output.Color.a = DiffuseMtrlAlpha;

    // Pass texture info
    output.Tex = input.Tex;

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target{
    float4 textureColour = txDiffuse.Sample(samLinear, input.Tex);

    // alpha clipping
    clip(textureColour.a - 0.25f);

    //return input.Color;
    return input.Color + textureColour;
}
