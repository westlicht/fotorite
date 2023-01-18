[[vk::binding(0)]] StructuredBuffer<float> buffer0;
[[vk::binding(1)]] StructuredBuffer<float> buffer1;
[[vk::binding(2)]] RWStructuredBuffer<float> result;

[[vk::push_constant]] struct {
    uint count;
} constants;

[numthreads(256, 1, 1)]
void main(uint3 thread_id : SV_DISPATCHTHREADID)
{
    const uint i = thread_id.x;
    if (i >= constants.count)
        return;
    result[i] = buffer0[i] + buffer1[i];
    // result[i] = 333.f;
}
