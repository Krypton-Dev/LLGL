/*
 * MTShaderProgram.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTShaderProgram.h"
#include "MTShader.h"
#include "../../CheckedCast.h"


namespace LLGL
{


MTShaderProgram::MTShaderProgram(id<MTLDevice> device, const ShaderProgramDescriptor& desc) :
    device_ { device }
{
    Attach(desc.vertexShader);
    Attach(desc.tessControlShader);
    Attach(desc.tessEvaluationShader);
    Attach(desc.geometryShader);
    Attach(desc.fragmentShader);
    Attach(desc.computeShader);

    /* Grab vertex descriptor and increment reference counter */
    if (auto vs = desc.vertexShader)
    {
        auto vsMT = LLGL_CAST(MTShader*, vs);
        if (auto vertexDesc = vsMT->GetMTLVertexDesc())
        {
            [vertexDesc retain];
            vertexDesc_ = vertexDesc;
        }
    }
}

MTShaderProgram::~MTShaderProgram()
{
    if (vertexDesc_)
        [vertexDesc_ release];
}

bool MTShaderProgram::HasErrors() const
{
    return (shaderWithError_ != nullptr);
}

std::string MTShaderProgram::GetReport() const
{
    if (shaderWithError_ != nullptr)
        return shaderWithError_->GetReport();
    else
        return "";
}

bool MTShaderProgram::Reflect(ShaderReflection& reflection) const
{
    ShaderProgram::ClearShaderReflection(reflection);

    if (kernelFunc_ != nil)
        ReflectComputePipeline(reflection);
    else
        ReflectRenderPipeline(reflection);

    ShaderProgram::FinalizeShaderReflection(reflection);
    return true;
}

UniformLocation MTShaderProgram::FindUniformLocation(const char* /*name*/) const
{
    return -1; // dummy
}

bool MTShaderProgram::SetWorkGroupSize(const Extent3D& workGroupSize)
{
    if (kernelFunc_ != nil && workGroupSize.width > 0 && workGroupSize.height > 0 && workGroupSize.depth > 0)
    {
        numThreadsPerGroup_.width  = workGroupSize.width;
        numThreadsPerGroup_.height = workGroupSize.height;
        numThreadsPerGroup_.depth  = workGroupSize.depth;
        return true;
    }
    return false;
}

bool MTShaderProgram::GetWorkGroupSize(Extent3D& workGroupSize) const
{
    if (kernelFunc_ != nil)
    {
        workGroupSize.width  = static_cast<std::uint32_t>(numThreadsPerGroup_.width);
        workGroupSize.height = static_cast<std::uint32_t>(numThreadsPerGroup_.height);
        workGroupSize.depth  = static_cast<std::uint32_t>(numThreadsPerGroup_.depth);
        return true;
    }
    return false;
}


/*
 * ======= Private: =======
 */

void MTShaderProgram::Attach(Shader* shader)
{
    if (shader != nullptr)
    {
        /* Get native shader function */
        auto* shaderMT = LLGL_CAST(MTShader*, shader);
        id<MTLFunction> shaderFunc = shaderMT->GetNative();

        /* Check for errors */
        if (shaderMT->HasError())
        {
            /* Store reference to shader with error (for later info log queries) */
            shaderWithError_ = shaderMT;
        }
        else
        {
            /* Store reference to shader function */
            switch (shaderMT->GetType())
            {
                case ShaderType::Vertex:
                    vertexFunc_ = shaderFunc;
                    break;
                case ShaderType::TessControl:
                    //???
                    break;
                case ShaderType::TessEvaluation:
                    //???
                    break;
                case ShaderType::Geometry:
                    //???
                    break;
                case ShaderType::Fragment:
                    fragmentFunc_ = shaderFunc;
                    break;
                case ShaderType::Compute:
                    kernelFunc_ = shaderFunc;
                    break;
                default:
                    break;
            }
        }
    }
}

static ResourceType ToResourceType(MTLArgumentType type)
{
    switch (type)
    {
        case MTLArgumentTypeBuffer:     return ResourceType::Buffer;
        case MTLArgumentTypeTexture:    return ResourceType::Texture;
        case MTLArgumentTypeSampler:    return ResourceType::Sampler;
        default:                        return ResourceType::Undefined;
    }
}

static void ReflectShaderArgument(MTLArgument* arg, ShaderReflection& reflection)
{
    auto resourceType = ToResourceType(arg.type);
    if (resourceType != ResourceType::Undefined)
    {
        ShaderResource resource;
        {
            resource.binding.name       = [arg.name UTF8String];
            resource.binding.type       = resourceType;
            resource.binding.slot       = static_cast<std::uint32_t>(arg.index);
            resource.binding.arraySize  = static_cast<std::uint32_t>(arg.arrayLength);
            if (resourceType == ResourceType::Buffer)
                resource.constantBufferSize = static_cast<std::uint32_t>(arg.bufferDataSize);
        }
        reflection.resources.push_back(resource);
    }
}

void MTShaderProgram::ReflectRenderPipeline(ShaderReflection& reflection) const
{
    //TODO: get pixel formats from render target or render context
    /* Create render pipeline state */
    MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    {
        pipelineDesc.vertexDescriptor                   = GetMTLVertexDesc();
        pipelineDesc.alphaToCoverageEnabled             = NO;
        pipelineDesc.alphaToOneEnabled                  = NO;
        pipelineDesc.fragmentFunction                   = GetFragmentMTLFunction();
        pipelineDesc.vertexFunction                     = GetVertexMTLFunction();
        pipelineDesc.inputPrimitiveTopology             = MTLPrimitiveTopologyClassTriangle;
        pipelineDesc.colorAttachments[0].pixelFormat    = MTLPixelFormatBGRA8Unorm;
        pipelineDesc.depthAttachmentPixelFormat         = MTLPixelFormatDepth32Float_Stencil8;
        pipelineDesc.stencilAttachmentPixelFormat       = MTLPixelFormatDepth32Float_Stencil8;
        pipelineDesc.rasterizationEnabled               = (GetFragmentMTLFunction() != nil);
        pipelineDesc.sampleCount                        = 1;
    }

    MTLRenderPipelineReflection* psoReflect = nullptr;
    MTLPipelineOption opt = (MTLPipelineOptionArgumentInfo | MTLPipelineOptionBufferTypeInfo);
    NSError* error = nullptr;

    id<MTLRenderPipelineState> pso = [device_
        newRenderPipelineStateWithDescriptor:   pipelineDesc
        options:                                opt
        reflection:                             &psoReflect
        error:                                  &error
    ];

    [pipelineDesc release];

    for (MTLArgument* arg in psoReflect.vertexArguments)
        ReflectShaderArgument(arg, reflection);
    for (MTLArgument* arg in psoReflect.fragmentArguments)
        ReflectShaderArgument(arg, reflection);

    [pso release];
}

void MTShaderProgram::ReflectComputePipeline(ShaderReflection& reflection) const
{
    //TODO
}


} // /namespace LLGL



// ================================================================================
