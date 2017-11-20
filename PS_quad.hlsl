Texture2D tex : register(t0);
SamplerState sampAni; //Default sampler

struct PS_IN
{
	float4 position_cs : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

float4 PS_main(PS_IN input) : SV_TARGET
{
	float3 sampledColor = tex.Sample(sampAni, float2(input.texcoord.x, 1.0f - input.texcoord.y)).xyz;
	return float4(sampledColor, 1.0f);
}