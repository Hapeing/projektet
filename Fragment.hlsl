//Diffuse texture
Texture2D diffuse : register(t0);

//TODO: send from app
SamplerState sampAni; //Default sampler

struct VS_OUT
{
	float4 Pos  : SV_POSITION; //ignore
	float4 PosW : POSITION;	//Position of fragment in world space
	float4 NorW : NORMAL; //Normal of fragment in world space
	float2 UV   : TEXCOORD; //UV coordinates
};

//TODO: multiple lights
cbuffer PointLight : register(b0)
{
	float4 lightPosition; 
	float4 lightColor;	  //.a is intensity
}

cbuffer Camera : register(b1)
{
	float4 CamPos; //in world (ignore .w)
}

float ComputeDiffuseFactor(float3 lightVector, float3 normal)
{
	return dot(lightVector, normal);
}

//TODO: Specular
float4 PS_main(VS_OUT input) : SV_Target
{
	//normalize normal
	input.NorW = normalize(input.NorW);
	
	float3 lightPos = lightPosition.xyz;

	//normalized vector from fragment in world space to light
	float3 lightVector = normalize(lightPos - input.PosW.xyz);

	//cos(angle) between light vector and normal of fragment (since normalized this is just dot prod)
	float diffuseFactor = max(ComputeDiffuseFactor(lightVector, input.NorW.xyz), 0.0f);
	
	//get color of fragment from texture
	float3 sampledColor = diffuse.Sample(sampAni, float2(input.UV.x, 1.0f - input.UV.y)).xyz;
	
	//initialize final with ambient as lightcol*fragmentCol*0.1
	//TODO:? get from mtl
	float4 final = float4(lightColor*sampledColor * 0.1f, 1.0f);

	//calculate color from this light
	float3 thisColor = diffuseFactor * sampledColor * lightColor;

	//add it to final
	final += float4(thisColor, 1.0f);

	//clamp to 0.0-1.0 and return final
	return min(final, float4(1.0, 1.0, 1.0, 1.0));
};