/*
 * DbgShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgShaderProgram.h"
#include "DbgShader.h"
#include "DbgCore.h"
#include "../CheckedCast.h"
#include <LLGL/Strings.h>


namespace LLGL
{


DbgShaderProgram::DbgShaderProgram(
    ShaderProgram&                  instance,
    RenderingDebugger*              debugger,
    const ShaderProgramDescriptor&  desc,
    const RenderingCapabilities&    caps)
:   instance  { instance },
    debugger_ { debugger }
{
    /* Debug all attachments and shader composition */
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateShaderAttachment(desc.vertexShader, ShaderType::Vertex);
        ValidateShaderAttachment(desc.tessControlShader, ShaderType::TessControl);
        ValidateShaderAttachment(desc.tessEvaluationShader, ShaderType::TessEvaluation);
        ValidateShaderAttachment(desc.geometryShader, ShaderType::Geometry);
        ValidateShaderAttachment(desc.fragmentShader, ShaderType::Fragment);
        ValidateShaderAttachment(desc.computeShader, ShaderType::Compute);
        ValidateShaderComposition();
        QueryInstanceAndVertexIDs(caps);
    }

    /* Store all attributes of vertex layout */
    if (auto shader = desc.vertexShader)
    {
        auto shaderDbg = LLGL_CAST(DbgShader*, desc.vertexShader);
        vertexLayout_.attributes    = shaderDbg->desc.vertex.inputAttribs;
        vertexLayout_.bound         = true;
    }

    /* Store information whether this shader program contains a fragment shader */
    hasFragmentShader_ = (desc.fragmentShader != nullptr && desc.fragmentShader->GetType() == ShaderType::Fragment);
}

bool DbgShaderProgram::HasErrors() const
{
    return instance.HasErrors();
}

std::string DbgShaderProgram::GetReport() const
{
    return instance.GetReport();
}

bool DbgShaderProgram::Reflect(ShaderReflection& reflection) const
{
    return instance.Reflect(reflection);
}

UniformLocation DbgShaderProgram::FindUniformLocation(const char* name) const
{
    return instance.FindUniformLocation(name);
}

bool DbgShaderProgram::SetWorkGroupSize(const Extent3D& workGroupSize)
{
    return instance.SetWorkGroupSize(workGroupSize);
}

bool DbgShaderProgram::GetWorkGroupSize(Extent3D& workGroupSize) const
{
    return instance.GetWorkGroupSize(workGroupSize);
}

const char* DbgShaderProgram::GetVertexID() const
{
    return (vertexID_.empty() ? nullptr : vertexID_.c_str());
}

const char* DbgShaderProgram::GetInstanceID() const
{
    return (instanceID_.empty() ? nullptr : instanceID_.c_str());
}


/*
 * ======= Private: =======
 */

#define LLGL_SHADERTYPE_MASK(TYPE)  (1 << static_cast<int>(TYPE))
#define LLGL_VS_MASK                LLGL_SHADERTYPE_MASK(ShaderType::Vertex)
#define LLGL_PS_MASK                LLGL_SHADERTYPE_MASK(ShaderType::Fragment)
#define LLGL_HS_MASK                LLGL_SHADERTYPE_MASK(ShaderType::TessControl)
#define LLGL_DS_MASK                LLGL_SHADERTYPE_MASK(ShaderType::TessEvaluation)
#define LLGL_GS_MASK                LLGL_SHADERTYPE_MASK(ShaderType::Geometry)
#define LLGL_CS_MASK                LLGL_SHADERTYPE_MASK(ShaderType::Compute)

void DbgShaderProgram::ValidateShaderAttachment(Shader* shader, const ShaderType type)
{
    if (shader != nullptr)
    {
        auto& shaderDbg = LLGL_CAST(DbgShader&, *shader);

        /* Check compilation state */
        if (!shaderDbg.IsCompiled())
            LLGL_DBG_ERROR(ErrorType::InvalidState, "attempt to attach uncompiled shader to shader program");

        /* Check if shader type already has been attached */
        if (shaderDbg.GetType() != type)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "mismatch between shader type (" + std::string(ToString(shaderDbg.GetType())) +
                ") and shader program attachment (" + std::string(ToString(type)) + ")"
            );
        }

        /* Add shader type to list */
        shaderTypes_.push_back(shaderDbg.GetType());

        /* Update shader attachment mask */
        shaderAttachmentMask_ |= LLGL_SHADERTYPE_MASK(shaderDbg.GetType());
    }
}

void DbgShaderProgram::ValidateShaderComposition()
{
    /* Validate shader composition by shader attachment bit mask */
    switch (shaderAttachmentMask_)
    {
        case ( LLGL_VS_MASK                                                             ):
        case ( LLGL_VS_MASK |                               LLGL_GS_MASK                ):
        case ( LLGL_VS_MASK | LLGL_HS_MASK | LLGL_DS_MASK                               ):
        case ( LLGL_VS_MASK | LLGL_HS_MASK | LLGL_DS_MASK | LLGL_GS_MASK                ):
        case ( LLGL_VS_MASK |                                              LLGL_PS_MASK ):
        case ( LLGL_VS_MASK |                               LLGL_GS_MASK | LLGL_PS_MASK ):
        case ( LLGL_VS_MASK | LLGL_HS_MASK | LLGL_DS_MASK |                LLGL_PS_MASK ):
        case ( LLGL_VS_MASK | LLGL_HS_MASK | LLGL_DS_MASK | LLGL_GS_MASK | LLGL_PS_MASK ):
        case ( LLGL_CS_MASK ):
            break;
        default:
            LLGL_DBG_ERROR(ErrorType::InvalidState, "invalid shader composition");
            break;
    }
}

void DbgShaderProgram::QueryInstanceAndVertexIDs(const RenderingCapabilities& caps)
{
    ShaderReflection reflect;
    if (instance.Reflect(reflect))
    {
        for (const auto& attr : reflect.vertex.inputAttribs)
        {
            if (vertexID_.empty())
            {
                if (attr.systemValue == SystemValue::VertexID)
                    vertexID_ = attr.name;
            }
            if (instanceID_.empty())
            {
                if (attr.systemValue == SystemValue::InstanceID)
                    instanceID_ = attr.name;
            }
            if (!vertexID_.empty() && !instanceID_.empty())
                break;
        }
    }
}

#undef LLGL_SHADERTYPE_MASK
#undef LLGL_VS_MASK
#undef LLGL_PS_MASK
#undef LLGL_HS_MASK
#undef LLGL_DS_MASK
#undef LLGL_GS_MASK
#undef LLGL_CS_MASK


} // /namespace LLGL



// ================================================================================
