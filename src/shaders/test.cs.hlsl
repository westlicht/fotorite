[[vk::binding(0)]] Buffer<float> buffer0;
[[vk::binding(1)]] Buffer<float> buffer1;
[[vk::binding(2)]] RWBuffer<float> result;

[[vk::push_constant]] struct
{
    uint count;
} constants;

[numthreads(256, 1, 1)]
void main(uint3 thread_id : SV_DISPATCHTHREADID)
{
    uint i = thread_id.x;
    result[i] = buffer0[i] + buffer1[i];
}
