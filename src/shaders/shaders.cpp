#include "shaders.h"

#include "compiled/shader_includes.h"

FR_NAMESPACE_BEGIN

static ShaderBlob shader_blobs[] =
{
#include "compiled/shader_names.h"
};

ShaderBlob get_shader_blob(ShaderID shader_id)
{
    return shader_blobs[static_cast<size_t>(shader_id)];
}

FR_NAMESPACE_END
