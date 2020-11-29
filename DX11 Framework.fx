//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    //float3 Norm : NORMAL;
    float4 Color : COLOR0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS( float4 Pos : POSITION, float3 NormalL : NORMAL)
//VS_OUTPUT VS(float4 Pos : POSITION, float4 Color : COLOR)
{
    VS_OUTPUT output = (VS_OUTPUT)0;

    output.Pos = mul( Pos, World );

    // Compute the vector from the vertex to the eye position
    float3 toEye = normalize(EyePosW - output.Pos.xyz);

    // Apply View and Projection transformations
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );

    // Convert normal from local space to world space 
    // W component of vector is 0 as vectors cannot be translated
    float3 normalW = mul(float4(NormalL, 0.0f), World).xyz;
    normalW = normalize(normalW);

    // Compute Colour
    //Compute the reflection vector
    float3 rVector = reflect(-LightVecW, normalW);
    
    // Determine how much *if any) specular light
    // makes it into the eye
    float specularAmount = pow(max(dot(rVector, toEye), 0.0f), SpecularPower);

    // Compute the ambient, diffuse and specular terms separately
    // Calculate Diffuse lighting
    float diffuseAmount = max(dot(LightVecW, normalW), 0.0f);
    float3 diffuse = diffuseAmount * (DiffuseMtrl * DiffuseLight).rgb;

    // Calculate Ambient lighting
    float3 ambient = AmbientMaterial * AmbientLight;

    // Calculate Specular lighting
    float3 specular = specularAmount * (SpecularMaterial * SpecularLight).rgb;

    // Sum all the lighting terms together
    output.Color.rgb = ambient + diffuse + specular;

    // Copy over the diffuse alpha
    output.Color.a = DiffuseMtrl.a;

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
    return input.Color;
}
