Texture2D NormalTexture   : register(t0);
Texture2D DiffuseTexture  : register(t1);
Texture2D SpecularTexture : register(t2);
Texture2D PositionTexture : register(t3);

void GetAttributes
(
	in float2 screenPosition,
	out float3 normal,
	out float3 position,
	out float3 diffuse,
	out float3 specular,
	out float specularPower
)
{
	int3 sampledIndex = int3(screenPosition, 0);

	normal = NormalTexture.Load(sampledIndex.xyz).xyz;
	position = PositionTexture.Load(sampledIndex.xyz).xyz;
	diffuse = DiffuseTexture.Load(sampledIndex.xyz).xyz;
	float4 spec = SpecularTexture.Load(sampledIndex.xyz);
	
	specular = spec.xyz;
	specularPower = spec.w;
}

float ComputeDiffuseFactor(float3 lightVector, float3 normal)
{
	return dot(lightVector, normal);
}

cbuffer PointLight : register(b0)
{
	float4 lightPosition;
	float4 lightColor;	  //.a is intensity
}

cbuffer Camera : register(b1)
{
	float4 CamPos; //in world (ignore .w)
}

float4 PS_main(in float4 screenPos : SV_Position) : SV_TARGET0
{

	float3 NorW;
	float3 PosW;
	float3 diffuse;
	float3 specular;
	float specularPower;

	GetAttributes(screenPos.xy, NorW, PosW, diffuse, specular, specularPower);
	//normalize normal
	NorW.xyz = normalize(NorW.xyz);

	float3 lightPos = lightPosition.xyz;

	//normalized vector from fragment in world space to light
	float3 lightVector = normalize(lightPos - PosW.xyz);

	//---------------------------
	//             Specular
	//---------------------------

	float3 N = NorW.xyz;
	float3 L = normalize(lightPos - PosW);
	float3 V = normalize(CamPos.xyz - PosW.xyz);
	float3 R = 2 * dot(N, L) * N - L;


	//float3 R = normalize((2 * dot(input.NorW.xyz, lightVector) * input.NorW.xyz) - lightVector);

	float specularFactor = pow(max(dot(V, R), 0.0f), specularPower);

	//---------------------------

	//cos(angle) between light vector and normal of fragment (since normalized this is just dot prod)
	float diffuseFactor = max(ComputeDiffuseFactor(lightVector, NorW.xyz), 0.0f);


	//initialize final with ambient as ambientColor*lightcol*fragmentCol*0.1
	float3 ambientColor = diffuse * float3(0.2, 0.2, 0.2); //Never pitch black color
	float4 final = float4(ambientColor*lightColor*diffuse * 0.3f, 1.0f);

	//calculate color from this light
	float3 thisColor =
	diffuseFactor * diffuse * lightColor.xyz	//Diffuse factor
	+ (specular * specularFactor);						//Specular factor

													//add it to final
	final += float4(thisColor, 1.0f);

	//clamp to 0.0-1.0 and return final
	return min(final, float4(1.0, 1.0, 1.0, 1.0));
};