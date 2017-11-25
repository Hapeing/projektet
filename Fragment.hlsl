//Diffuse texture
Texture2D diffuse : register(t0);

//TODO: send from app
SamplerState sampAni; //Default sampler

struct MaterialAttributes
{
	float KsR, KsG, KsB, Ns;			//specular color + power
	float KdR, KdG, KdB, UseTexture;	//Diffuse color + useTexture 'boolean'
	float KaR, KaG, KaB, pad2;			//Ambient color
};

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

cbuffer Material : register(b2)
{
	MaterialAttributes matAttr;
}

float ComputeDiffuseFactor(float3 lightVector, float3 normal)
{
	return dot(lightVector, normal);
}

//TODO: Specular
float4 PS_main(VS_OUT input) : SV_Target
{
	//normalize normal
	input.NorW.xyz = normalize(input.NorW.xyz);

	float3 lightPos = lightPosition.xyz;

	//normalized vector from fragment in world space to light
	float3 lightVector = normalize(lightPos - input.PosW.xyz );
	
	//---------------------------
	//             Specular
	//---------------------------

	float3 N = input.NorW.xyz;
	float3 L = normalize(lightPos - input.PosW);
	float3 V = normalize(CamPos.xyz - input.PosW.xyz);
	float3 R = 2 * dot(N, L) * N - L;


	//float3 R = normalize((2 * dot(input.NorW.xyz, lightVector) * input.NorW.xyz) - lightVector);

	float specularFactor = pow(max(dot(V, R), 0.0f), matAttr.Ns);

	//---------------------------

	//cos(angle) between light vector and normal of fragment (since normalized this is just dot prod)
	float diffuseFactor = max(ComputeDiffuseFactor(lightVector, input.NorW.xyz), 0.0f);
	
	float3 sampledColor;
	//get color of fragment from texture (if model uses texture)
	if (matAttr.UseTexture > 0.0)
	{
		sampledColor = diffuse.Sample(sampAni, float2(input.UV.x, 1.0f - input.UV.y)).xyz;
	}
	else
	{
		sampledColor = float3(matAttr.KdR, matAttr.KdG, matAttr.KdB);
	}
	//initialize final with ambient as ambientColor*lightcol*fragmentCol*0.1
	float3 ambientColor = max(float3(matAttr.KaR, matAttr.KaG, matAttr.KaB), float3(0.1, 0.1, 0.1)); //Never pitch black color
	float4 final = float4(ambientColor*lightColor*sampledColor * 0.3f, 1.0f);

	//calculate color from this light
	float3 Ks = float3(matAttr.KsR, matAttr.KsG, matAttr.KsB);
	float3 thisColor = 
		diffuseFactor * sampledColor * lightColor.xyz	//Diffuse factor
		+ (Ks * specularFactor);						//Specular factor

	//add it to final
	final += float4(thisColor, 1.0f);

	//clamp to 0.0-1.0 and return final
	return min(final, float4(1.0, 1.0, 1.0, 1.0));
};