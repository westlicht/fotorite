#include "shaders.h"

#include "test_buffer_cs.h"
#include "test_image_cs.h"

FR_NAMESPACE_BEGIN

static ShaderBlob shader_blobs[] =
{
    test_buffer_cs,
    test_image_cs,
};

ShaderBlob get_shader_blob(ShaderID shader_id)
{
    return shader_blobs[static_cast<size_t>(shader_id)];
}

FR_NAMESPACE_END
