#if 0
; SPIR-V
; Version: 1.5
; Generator: Google spiregg; 0
; Bound: 45
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %src %dst %constants %gl_GlobalInvocationID
               OpExecutionMode %main LocalSize 32 32 1
               OpSource HLSL 600
               OpName %type_2d_image "type.2d.image"
               OpName %src "src"
               OpName %type_2d_image_0 "type.2d.image"
               OpName %dst "dst"
               OpName %type_PushConstant_ "type.PushConstant."
               OpMemberName %type_PushConstant_ 0 "res"
               OpName %constants "constants"
               OpName %main "main"
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
               OpDecorate %src DescriptorSet 0
               OpDecorate %src Binding 0
               OpDecorate %dst DescriptorSet 0
               OpDecorate %dst Binding 1
               OpMemberDecorate %type_PushConstant_ 0 Offset 0
               OpDecorate %type_PushConstant_ Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
      %float = OpTypeFloat 32
    %float_2 = OpConstant %float 2
    %float_1 = OpConstant %float 1
    %v4float = OpTypeVector %float 4
         %17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_2d_image_0 = OpTypeImage %float 2D 2 0 0 2 Rgba32f
%_ptr_UniformConstant_type_2d_image_0 = OpTypePointer UniformConstant %type_2d_image_0
     %v2uint = OpTypeVector %uint 2
%type_PushConstant_ = OpTypeStruct %v2uint
%_ptr_PushConstant_type_PushConstant_ = OpTypePointer PushConstant %type_PushConstant_
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
       %void = OpTypeVoid
         %25 = OpTypeFunction %void
%_ptr_PushConstant_v2uint = OpTypePointer PushConstant %v2uint
       %bool = OpTypeBool
     %v2bool = OpTypeVector %bool 2
        %src = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
        %dst = OpVariable %_ptr_UniformConstant_type_2d_image_0 UniformConstant
  %constants = OpVariable %_ptr_PushConstant_type_PushConstant_ PushConstant
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
       %main = OpFunction %void None %25
         %29 = OpLabel
         %30 = OpLoad %v3uint %gl_GlobalInvocationID
               OpSelectionMerge %31 None
               OpSwitch %uint_0 %32
         %32 = OpLabel
         %33 = OpVectorShuffle %v2uint %30 %30 0 1
         %34 = OpAccessChain %_ptr_PushConstant_v2uint %constants %int_0
         %35 = OpLoad %v2uint %34
         %36 = OpUGreaterThanEqual %v2bool %33 %35
         %37 = OpAny %bool %36
               OpSelectionMerge %38 None
               OpBranchConditional %37 %39 %38
         %39 = OpLabel
               OpBranch %31
         %38 = OpLabel
         %40 = OpLoad %type_2d_image %src
         %41 = OpImageFetch %v4float %40 %33 Lod %uint_0
         %42 = OpVectorTimesScalar %v4float %41 %float_2
         %43 = OpFAdd %v4float %42 %17
         %44 = OpLoad %type_2d_image_0 %dst
               OpImageWrite %44 %33 %43 None
               OpBranch %31
         %31 = OpLabel
               OpReturn
               OpFunctionEnd

#endif

const unsigned char test_image_cs[] = {
  0x03, 0x02, 0x23, 0x07, 0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0x0e, 0x00,
  0x2d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x09, 0x00, 0x05, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x6d, 0x61, 0x69, 0x6e, 0x00, 0x00, 0x00, 0x00,
  0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x05, 0x00, 0x00, 0x00, 0x10, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x11, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x05, 0x00, 0x00, 0x00,
  0x58, 0x02, 0x00, 0x00, 0x05, 0x00, 0x06, 0x00, 0x06, 0x00, 0x00, 0x00,
  0x74, 0x79, 0x70, 0x65, 0x2e, 0x32, 0x64, 0x2e, 0x69, 0x6d, 0x61, 0x67,
  0x65, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x02, 0x00, 0x00, 0x00,
  0x73, 0x72, 0x63, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 0x00, 0x00, 0x00,
  0x74, 0x79, 0x70, 0x65, 0x2e, 0x32, 0x64, 0x2e, 0x69, 0x6d, 0x61, 0x67,
  0x65, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00,
  0x64, 0x73, 0x74, 0x00, 0x05, 0x00, 0x07, 0x00, 0x08, 0x00, 0x00, 0x00,
  0x74, 0x79, 0x70, 0x65, 0x2e, 0x50, 0x75, 0x73, 0x68, 0x43, 0x6f, 0x6e,
  0x73, 0x74, 0x61, 0x6e, 0x74, 0x2e, 0x00, 0x00, 0x06, 0x00, 0x04, 0x00,
  0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x72, 0x65, 0x73, 0x00,
  0x05, 0x00, 0x05, 0x00, 0x04, 0x00, 0x00, 0x00, 0x63, 0x6f, 0x6e, 0x73,
  0x74, 0x61, 0x6e, 0x74, 0x73, 0x00, 0x00, 0x00, 0x05, 0x00, 0x04, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x6d, 0x61, 0x69, 0x6e, 0x00, 0x00, 0x00, 0x00,
  0x47, 0x00, 0x04, 0x00, 0x05, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
  0x1c, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00, 0x02, 0x00, 0x00, 0x00,
  0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00,
  0x02, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x47, 0x00, 0x04, 0x00, 0x03, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00, 0x03, 0x00, 0x00, 0x00,
  0x21, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x48, 0x00, 0x05, 0x00,
  0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x03, 0x00, 0x08, 0x00, 0x00, 0x00,
  0x02, 0x00, 0x00, 0x00, 0x15, 0x00, 0x04, 0x00, 0x09, 0x00, 0x00, 0x00,
  0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x04, 0x00,
  0x09, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x15, 0x00, 0x04, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x04, 0x00, 0x0b, 0x00, 0x00, 0x00,
  0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x00, 0x03, 0x00,
  0x0d, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x04, 0x00,
  0x0d, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
  0x2b, 0x00, 0x04, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x80, 0x3f, 0x17, 0x00, 0x04, 0x00, 0x10, 0x00, 0x00, 0x00,
  0x0d, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x07, 0x00,
  0x10, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x0f, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x19, 0x00, 0x09, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x20, 0x00, 0x04, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x06, 0x00, 0x00, 0x00, 0x19, 0x00, 0x09, 0x00, 0x07, 0x00, 0x00, 0x00,
  0x0d, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00, 0x13, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x17, 0x00, 0x04, 0x00,
  0x14, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
  0x1e, 0x00, 0x03, 0x00, 0x08, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00,
  0x20, 0x00, 0x04, 0x00, 0x15, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
  0x08, 0x00, 0x00, 0x00, 0x17, 0x00, 0x04, 0x00, 0x16, 0x00, 0x00, 0x00,
  0x0b, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00,
  0x17, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00,
  0x13, 0x00, 0x02, 0x00, 0x18, 0x00, 0x00, 0x00, 0x21, 0x00, 0x03, 0x00,
  0x19, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00,
  0x1a, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00,
  0x14, 0x00, 0x02, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x17, 0x00, 0x04, 0x00,
  0x1c, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
  0x3b, 0x00, 0x04, 0x00, 0x12, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x04, 0x00, 0x13, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x04, 0x00,
  0x15, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
  0x3b, 0x00, 0x04, 0x00, 0x17, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x36, 0x00, 0x05, 0x00, 0x18, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00,
  0xf8, 0x00, 0x02, 0x00, 0x1d, 0x00, 0x00, 0x00, 0x3d, 0x00, 0x04, 0x00,
  0x16, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
  0xf7, 0x00, 0x03, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xfb, 0x00, 0x03, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
  0xf8, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x00, 0x4f, 0x00, 0x07, 0x00,
  0x14, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
  0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x41, 0x00, 0x05, 0x00, 0x1a, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x3d, 0x00, 0x04, 0x00,
  0x14, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00,
  0xae, 0x00, 0x05, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
  0x21, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00, 0x9a, 0x00, 0x04, 0x00,
  0x1b, 0x00, 0x00, 0x00, 0x25, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
  0xf7, 0x00, 0x03, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xfa, 0x00, 0x04, 0x00, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0x00,
  0x26, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x02, 0x00, 0x27, 0x00, 0x00, 0x00,
  0xf9, 0x00, 0x02, 0x00, 0x1f, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x02, 0x00,
  0x26, 0x00, 0x00, 0x00, 0x3d, 0x00, 0x04, 0x00, 0x06, 0x00, 0x00, 0x00,
  0x28, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x07, 0x00,
  0x10, 0x00, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
  0x21, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00,
  0x8e, 0x00, 0x05, 0x00, 0x10, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00,
  0x29, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x81, 0x00, 0x05, 0x00,
  0x10, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00,
  0x11, 0x00, 0x00, 0x00, 0x3d, 0x00, 0x04, 0x00, 0x07, 0x00, 0x00, 0x00,
  0x2c, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x63, 0x00, 0x05, 0x00,
  0x2c, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xf9, 0x00, 0x02, 0x00, 0x1f, 0x00, 0x00, 0x00,
  0xf8, 0x00, 0x02, 0x00, 0x1f, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x01, 0x00,
  0x38, 0x00, 0x01, 0x00
};
