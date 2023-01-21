[[vk::binding(0)]] Texture2D<float4> src;
[[vk::binding(1)]] RWTexture2D<float4> dst;

[[vk::push_constant]] struct {
    uint2 res;
} constants;

[numthreads(32, 32, 1)]
void main(uint3 thread_id : SV_DISPATCHTHREADID)
{
    const uint2 pixel = thread_id.xy;
    if (any(pixel >= constants.res))
        return;
    dst[pixel] = src[pixel] * 2.f + 1.f;
}
