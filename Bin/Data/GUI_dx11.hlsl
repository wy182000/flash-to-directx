
//-----------------------------------------------------------------------------------------
// Textures and Samplers
//-----------------------------------------------------------------------------------------
Texture2D    g_txDiffuse : register( t0 );
//SamplerState g_samLinear : register( s0 );
SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};
//--------------------------------------------------------------------------------------
// shader input/output structure
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Position     : POSITION; // vertex position 
    float4 Color       	: COLOR;   // this color comes in per-vertex
    float2 TextureUV    : TEXCOORD;// vertex texture coords 
};

struct VS_OUTPUT
{
    float4 Position     : SV_POSITION; // vertex position 
    float4 Color      	: COLOR0;      // vertex diffuse color (note that COLOR0 is clamped from 0..1)
    float2 TextureUV    : TEXCOORD0;   // vertex texture coords 
};

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS( VS_INPUT input )
{
      return input;    
}

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
// color with diffuse material color
//--------------------------------------------------------------------------------------
float4 RenderScenePS( VS_OUTPUT In ) : SV_TARGET
{ 
    // Lookup mesh texture and modulate it with diffuse
    
	return g_txDiffuse.Sample( samLinear, In.TextureUV ) * In.Color;
	//return g_txDiffuse.Sample( g_samLinear, In.TextureUV ) * In.Diffuse;
	//return float4( 1.0f, 1.0f, 0.0f, 1.0f );
}
