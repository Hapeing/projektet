// Declare the input and output resources
//Texture2D<float4> InputMap : register(t0);
RWTexture2D<float4> OutputMap : register(u0);
// Group size
#define size_x 32
#define size_y 32

float Luma(float3 rgb)
{
	return rgb.y * (0.587 / 0.299) + rgb.x;
}

[numthreads(size_x, size_y, 1)]
void main( uint3 DispatchThreadID : SV_DispatchThreadID )
{
	//Declare the pixels used for edge detection (north, south, east, west, middle)
	float3 rgbN = OutputMap.Load(DispatchThreadID + uint3(0, 1, 0)).xyz;
	float3 rgbS = OutputMap.Load(DispatchThreadID + uint3(0, -1, 0)).xyz;
	float3 rgbE = OutputMap.Load(DispatchThreadID + uint3(1, 0, 0)).xyz;
	float3 rgbW = OutputMap.Load(DispatchThreadID + uint3(0, -1, 0)).xyz;
	float3 rgbM = OutputMap.Load(DispatchThreadID).xyz;

	//Get their luma values
	float lumaN = Luma(rgbN);
	float lumaS = Luma(rgbS);
	float lumaE = Luma(rgbE);
	float lumaW = Luma(rgbW);
	float lumaM = Luma(rgbM);

	//TODO: check for low contrast


}