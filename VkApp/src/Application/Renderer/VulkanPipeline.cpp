#include "VulkanPipeline.h"

#include "../Utils.h"

static VkPipelineLayout CreateLayout(VulkanPipelineInfo& info)
{
	VkCtx* ctx = VkCtxHandler::GetCrntCtx();

	VkPipelineLayout layout;

	VkPipelineLayoutCreateInfo plInfo{};
	plInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	plInfo.setLayoutCount = info.layoutCount;
	plInfo.pSetLayouts = info.layouts;
	plInfo.pushConstantRangeCount = info.pushConstRangeCount;
	plInfo.pPushConstantRanges = info.pushConstRanges;

	VK_CHECK(vkCreatePipelineLayout(ctx->device, &plInfo, nullptr, &layout))

	return layout;
}

static VkPipeline CreateGraphicsPipeline(VulkanPipelineInfo& info, VkPipelineLayout layout)
{
	VkPipeline pipeline = nullptr;
	VkCtx* ctx = VkCtxHandler::GetCrntCtx();

	VkShaderModule vertMod = nullptr, fragMod = nullptr;
	// Shader modules
	{
		std::vector<char> vertCode, fragCode;
		vertCode = ReadFile(info.vertPath);
		fragCode = ReadFile(info.fragPath);

		VkShaderModuleCreateInfo vertInfo{};
		vertInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vertInfo.codeSize = vertCode.size();
		vertInfo.pCode = reinterpret_cast<const uint32_t*>(vertCode.data());
		VK_CHECK(vkCreateShaderModule(ctx->device, &vertInfo, nullptr, &vertMod))

		VkShaderModuleCreateInfo fragInfo{};
		fragInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		fragInfo.codeSize = fragCode.size();
		fragInfo.pCode = reinterpret_cast<const uint32_t*>(fragCode.data());
		VK_CHECK(vkCreateShaderModule(ctx->device, &fragInfo, nullptr, &fragMod))
	}
	
	VkPipelineRasterizationStateCreateInfo rInfo{};
	rInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rInfo.cullMode = info.enableCulling;
	rInfo.frontFace = info.frontFace;
	rInfo.lineWidth = info.lineWidth;
	rInfo.polygonMode = info.polygonMode;
	rInfo.depthClampEnable = info.enableDepthClamp;
	rInfo.depthBiasEnable = VK_FALSE;
	rInfo.depthBiasConstantFactor = 0.0f;
	rInfo.depthBiasClamp = 0.0f;
	rInfo.depthBiasSlopeFactor = 0.0f;
	rInfo.rasterizerDiscardEnable = info.enableRasterizationDiscard;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colBlendInfo{};
	colBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colBlendInfo.attachmentCount = 1;
	colBlendInfo.pAttachments = &colorBlendAttachment;
	colBlendInfo.logicOpEnable = VK_FALSE;
	colBlendInfo.logicOp = VK_LOGIC_OP_COPY;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)info.extent.width;
	viewport.height = (float)info.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = info.extent;

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = info.vertBindingCount;
	vertexInputInfo.pVertexBindingDescriptions = info.vertBindings;
	vertexInputInfo.vertexAttributeDescriptionCount = info.vertAttribCount;
	vertexInputInfo.pVertexAttributeDescriptions = info.vertAttribs;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = info.topology;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineShaderStageCreateInfo stages[2] = { {}, {} };
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].module = vertMod;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].pName = "main";

	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].module = fragMod;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].pName = "main";

	VkGraphicsPipelineCreateInfo gpInfo{};
	gpInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gpInfo.layout = layout;
	gpInfo.subpass = 0;
	gpInfo.renderPass = info.renderPass;
	gpInfo.basePipelineHandle = VK_NULL_HANDLE;
	gpInfo.pRasterizationState = &rInfo;
	gpInfo.pColorBlendState = &colBlendInfo;
	gpInfo.pDynamicState = &dynamicState;
	gpInfo.pViewportState = &viewportState;
	gpInfo.pVertexInputState = &vertexInputInfo;
	gpInfo.pMultisampleState = &multisampling;
	gpInfo.pInputAssemblyState = &inputAssembly;
	gpInfo.stageCount = 2;
	gpInfo.pStages = stages;

	VK_CHECK(vkCreateGraphicsPipelines(ctx->device, VK_NULL_HANDLE, 1, &gpInfo, nullptr, &pipeline))

	vkDestroyShaderModule(ctx->device, vertMod, nullptr);
	vkDestroyShaderModule(ctx->device, fragMod, nullptr);

	return pipeline;
}

static VkPipeline CreateComputePipeline(VulkanPipelineInfo& info)
{
	// TODO
	VkPipeline pipeline = nullptr;

	return pipeline;
}

VulkanPipeline VulkanPipeline::Create(VulkanPipelineInfo& info)
{
	VulkanPipeline pipeline;
	pipeline.m_Info = info;
	pipeline.m_Ctx = VkCtxHandler::GetCrntCtx();

	pipeline.m_Layout = CreateLayout(info);

	if (info.type == VulkanPipelineType::VULKAN_PIPELINE_TYPE_GRAPHICS)
	{
		pipeline.m_BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		pipeline.m_Handle = CreateGraphicsPipeline(info, pipeline.m_Layout);
	}
	else if (info.type == VulkanPipelineType::VULKAN_PIPELINE_TYPE_COMPUTE)
	{
		pipeline.m_BindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
		pipeline.m_Handle = CreateComputePipeline(info);
	}

	return pipeline;
}

void VulkanPipeline::Destroy()
{
	vkDestroyPipelineLayout(m_Ctx->device, m_Layout, nullptr);
	vkDestroyPipeline(m_Ctx->device, m_Handle, nullptr);
}

void VulkanPipeline::Bind(VkCommandBuffer buffer)
{
	vkCmdBindPipeline(buffer, m_BindPoint, m_Handle);
}