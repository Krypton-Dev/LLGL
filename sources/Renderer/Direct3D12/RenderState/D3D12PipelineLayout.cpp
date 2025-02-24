/*
 * D3D12PipelineLayout.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12PipelineLayout.h"
#include "../Shader/D3D12RootSignature.h"
#include "../D3DX12/d3dx12.h"
#include "../D3D12ObjectUtils.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


D3D12PipelineLayout::D3D12PipelineLayout(ID3D12Device* device, const PipelineLayoutDescriptor& desc)
{
    bindFlags_.reserve(desc.bindings.size());
    CreateRootSignature(device, desc);
}

void D3D12PipelineLayout::SetName(const char* name)
{
    D3D12SetObjectName(rootSignature_.Get(), name);
}

static D3D12_ROOT_SIGNATURE_FLAGS GetRootSignatureFlags(const PipelineLayoutDescriptor& layoutDesc)
{
    D3D12_ROOT_SIGNATURE_FLAGS signatureFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

    /* Always allow acces to the input assembly */
    signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;

    /* Determine which shader stages are not used for any binding points */
    long stageFlags = 0;
    for (const auto& binding : layoutDesc.bindings)
        stageFlags |= binding.stageFlags;

    /* Deny access to root signature for shader stages that are not affected by any binding point */
    if ((stageFlags & StageFlags::VertexStage        ) == 0) { signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;   }
    if ((stageFlags & StageFlags::TessControlStage   ) == 0) { signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;     }
    if ((stageFlags & StageFlags::TessEvaluationStage) == 0) { signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;   }
    if ((stageFlags & StageFlags::GeometryStage      ) == 0) { signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; }
    if ((stageFlags & StageFlags::FragmentStage      ) == 0) { signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;    }

    return signatureFlags;
}

void D3D12PipelineLayout::CreateRootSignature(ID3D12Device* device, const PipelineLayoutDescriptor& desc)
{
    D3D12RootSignature rootSignature;
    rootSignature.Reset(static_cast<UINT>(desc.bindings.size()), 0);

    /* Build root parameter for each descriptor range type */
    BuildRootParameter(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_CBV,     desc, ResourceType::Buffer,  BindFlags::ConstantBuffer );
    BuildRootParameter(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,     desc, ResourceType::Buffer,  BindFlags::Sampled        );
    BuildRootParameter(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,     desc, ResourceType::Texture, BindFlags::Sampled        );
    BuildRootParameter(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_UAV,     desc, ResourceType::Buffer,  BindFlags::Storage        );
    BuildRootParameter(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_UAV,     desc, ResourceType::Texture, BindFlags::Storage        );
    BuildRootParameter(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, desc, ResourceType::Sampler, 0                         );

    /* Build final root signature descriptor */
    rootSignature_ = rootSignature.Finalize(device, GetRootSignatureFlags(desc));
}

void D3D12PipelineLayout::ReleaseRootSignature()
{
    rootSignature_.Reset();
}

long D3D12PipelineLayout::GetBindFlagsByIndex(std::size_t idx) const
{
    return (idx < bindFlags_.size() ? bindFlags_[idx] : 0);
}


/*
 * ======= Private: =======
 */

void D3D12PipelineLayout::BuildRootParameter(
    D3D12RootSignature&             rootSignature,
    D3D12_DESCRIPTOR_RANGE_TYPE     descRangeType,
    const PipelineLayoutDescriptor& layoutDesc,
    const ResourceType              resourceType,
    long                            bindFlags)
{
    for (const auto& binding : layoutDesc.bindings)
    {
        if (binding.type == resourceType && (bindFlags == 0 || (binding.bindFlags & bindFlags) != 0))
        {
            if (auto rootParam = rootSignature.FindCompatibleRootParameter(descRangeType))
            {
                /* Append descriptor range to previous root parameter */
                rootParam->AppendDescriptorTableRange(descRangeType, binding.slot, binding.arraySize);
            }
            else
            {
                /* Create new root parameter and append descriptor range */
                rootParam = rootSignature.AppendRootParameter();
                rootParam->InitAsDescriptorTable(static_cast<UINT>(layoutDesc.bindings.size()));
                rootParam->AppendDescriptorTableRange(descRangeType, binding.slot, binding.arraySize);
            }

            /* Cache binding flags in the same order root parameters are build */
            bindFlags_.push_back(binding.bindFlags);
        }
    }
}


} // /namespace LLGL



// ================================================================================
