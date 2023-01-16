#include "shaders.h"

#include "test_cs.h"

FR_NAMESPACE_BEGIN

static ShaderBlob shader_blobs[] =
{
    test_cs,
};

ShaderBlob get_shader_blob(ShaderID shader_id)
{
    return shader_blobs[static_cast<size_t>(shader_id)];
}

FR_NAMESPACE_END
