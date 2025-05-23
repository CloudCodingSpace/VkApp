#include "Renderer.h"

#include "Global.h"

#include "Utils.h"

#include <glm/gtc/matrix_transform.hpp>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <iostream>

Renderer::Renderer(Window& window)
	: m_Window{ window }
{
	m_CurrentFrameIdx = 0;

	// The Ctx
	VkCtxHandler::SetCrntCtx(m_Ctx);
	VkCtxHandler::InitCtx(window, false);

	// RndrPass
	m_Pass = VulkanRenderpass::Create(VULKAN_RP_FLAG_CLEAR_COLOR);

	// Sync Objs
	CreateSyncObjs();

	// Framebuff
	{
		VulkanFramebufferInputData inputData{};
		inputData.width = m_Ctx.scExtent.width;
		inputData.height = m_Ctx.scExtent.height;
		inputData.pass = m_Pass.GetHandle();

		int i = 0;
		m_Framebuffs.resize(m_Ctx.scImgViews.size());
		for (const auto& imgView : m_Ctx.scImgViews)
		{
			inputData.views = {
				imgView
			};

			m_Framebuffs[i] = VulkanFramebuffer::Create(inputData);

			i++;
		}
	}
	// Cmd pool and buff
	{
		VulkanCmdPoolInputData poolData{};
		poolData.queueFamily = m_Ctx.queueProps.graphicsQueueIdx;
		poolData.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		m_CmdPool = VulkanCommandPool::Create(poolData);
		poolData.flags = 0;
		m_ImGuiCmdPool = VulkanCommandPool::Create(poolData);

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VulkanCmdBufferInputData buffData{};
			buffData.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			buffData.pool = m_CmdPool;
			m_CmdBuffs[i] = VulkanCmdBuffer::Allocate(buffData);
			m_ViewportCmdBuffs[i] = VulkanCmdBuffer::Allocate(buffData);
		}
	}
	// Descriptor pool
	{
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } 
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.maxSets = 1000 * ARR_SIZE(poolSizes);
		poolInfo.poolSizeCount = ARR_SIZE(poolSizes);
		poolInfo.pPoolSizes = poolSizes;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		
		VK_CHECK(vkCreateDescriptorPool(m_Ctx.device, &poolInfo, nullptr, &m_DescPool));
		VK_CHECK(vkCreateDescriptorPool(m_Ctx.device, &poolInfo, nullptr, &m_ImGuiDescPool));
	}
	// Descriptor Layout
	{
		VkDescriptorSetLayoutBinding bindings[1] = {};
		bindings[0].binding = 0;
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = sizeof(bindings)/sizeof(bindings[0]);
		info.pBindings = bindings;

		VK_CHECK(vkCreateDescriptorSetLayout(m_Ctx.device, &info, nullptr, &m_SetLayout));
	}
	// Descriptor Sets
	{
		VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			layouts[i] = m_SetLayout;

		VkDescriptorSetAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		info.descriptorPool = m_DescPool;
		info.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
		info.pSetLayouts = layouts;
		
		VK_CHECK(vkAllocateDescriptorSets(m_Ctx.device, &info, m_DescSets));
	}
	// Viewport renderpass
	{
		{
			VkCtx* ctx = VkCtxHandler::GetCrntCtx();

			VkAttachmentDescription colorAttach{};
			colorAttach.format = ctx->scFormat.format;
			colorAttach.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttach.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkAttachmentReference colorRef{};
			colorRef.attachment = 0;
			colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorRef;

			VkSubpassDependency spDependency{};
			spDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			spDependency.dstSubpass = 0;
			spDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			spDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			spDependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			spDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			info.attachmentCount = 1;
			info.pAttachments = &colorAttach;
			info.subpassCount = 1;
			info.pSubpasses = &subpass;
			info.dependencyCount = 1;
			info.pDependencies = &spDependency;

			VK_CHECK(vkCreateRenderPass(ctx->device, &info, nullptr, &m_ViewportPass));
		}
	}
	// Pipeline
	{
		std::vector<VkVertexInputBindingDescription> bindingDescs;
		std::vector<VkVertexInputAttributeDescription> attribDescs;

		bindingDescs.push_back(Vertex::GetBindingDesc());

		attribDescs.push_back(Vertex::GetPosAttribDesc());
		attribDescs.push_back(Vertex::GetColAttribDesc());

		VkPushConstantRange range{};
		range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		range.offset = 0;
		range.size = sizeof(PushConstData);

		VulkanPipelineInfo info;
		info.type = VulkanPipelineType::VULKAN_PIPELINE_TYPE_GRAPHICS;
		info.extent = m_Ctx.scExtent;
		info.vertPath = "assets/shaders/default.vert.spv";
		info.fragPath = "assets/shaders/default.frag.spv";
		info.renderPass = m_ViewportPass;
		info.vertBindingCount = bindingDescs.size();
		info.vertBindings = bindingDescs.data();
		info.vertAttribCount = attribDescs.size();
		info.vertAttribs = attribDescs.data();
		info.pushConstRangeCount = 1;
		info.pushConstRanges = &range;
		info.layoutCount = 1;
		info.layouts = &m_SetLayout;

		m_Pipeline = VulkanPipeline::Create(info);
	}
	// Vertex & Index Buffer
	{
		Vertex vertices[3];
		vertices[0].pos = glm::vec3(0.0f, 0.5f, 0.0f);
		vertices[1].pos = glm::vec3(0.5f, -0.5f, 0.0f);
		vertices[2].pos = glm::vec3(-0.5f, -0.5f, 0.0f);

		vertices[0].col = glm::vec3(1.0f, 0.0f, 0.0f);
		vertices[1].col = glm::vec3(0.0f, 1.0f, 0.0f);
		vertices[2].col = glm::vec3(0.0f, 0.0f, 1.0f);

		m_VertCount = 3;

		VulkanBufferInputData inputData{};
		inputData.data = vertices;
		inputData.pool = m_CmdPool;
		inputData.size = sizeof(vertices[0]) * 3;

		m_VertBuffer = VulkanBuffer::Create(VulkanBufferType::VULKAN_BUFFER_TYPE_VERTEX, inputData);

		uint32_t indices[3] = { 0, 1, 2 };

		inputData.data = indices;
		inputData.size = sizeof(indices[0]) * 3;
		m_IndexBuffer = VulkanBuffer::Create(VulkanBufferType::VULKAN_BUFFER_TYPE_INDEX, inputData);
	}
	// Texture Image
	{
		uint32_t data = 0xffff00ff;
		m_Image = VulkanImage::Create(1, 1, (unsigned char*)&data, m_CmdPool, true);
	}
	// Updating descriptors
	{
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkDescriptorImageInfo imgInfo{};
			imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imgInfo.imageView = m_Image.GetViewHandle();
			imgInfo.sampler = m_Image.GetSamplerHandle();

			VkWriteDescriptorSet writes[1] = {};
			writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[0].descriptorCount = 1;
			writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writes[0].dstArrayElement = 0;
			writes[0].dstBinding = 0;
			writes[0].dstSet = m_DescSets[i];
			writes[0].pImageInfo = &imgInfo;

			vkUpdateDescriptorSets(m_Ctx.device, 1, writes, 0, nullptr);
		}
	}
	// ImGui
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowPadding = ImVec2(0, 0);

		ImGui_ImplGlfw_InitForVulkan(m_Window.GetInternHandle(), true);

		ImGui_ImplVulkan_InitInfo info{};
		info.Allocator = nullptr;
		info.ApiVersion = VK_API_VERSION_1_0;
		info.DescriptorPool = m_ImGuiDescPool;
		info.Device = m_Ctx.device;
		info.ImageCount = MAX_FRAMES_IN_FLIGHT;
		info.Instance = m_Ctx.instance;
		info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
		info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		info.PhysicalDevice = m_Ctx.physicalDevice;
		info.Subpass = 0;
		info.RenderPass = m_Pass.GetHandle();
		info.Queue = m_Ctx.gQueue;
		info.QueueFamily = m_Ctx.queueProps.graphicsQueueIdx;

		ImGui_ImplVulkan_Init(&info);

		VulkanCmdBufferInputData data{};
		data.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		data.pool = m_CmdPool;
		VulkanCmdBuffer cmd = VulkanCmdBuffer::Allocate(data);
		ImGui_ImplVulkan_CreateFontsTexture();
		cmd.Free();
	}
	// Viewport stuff
	{
		m_ViewportSize.x = m_Ctx.scExtent.width;
		m_ViewportSize.y = m_Ctx.scExtent.height;

		m_ViewportImgsDesc.resize(m_Ctx.scImgs.size());
		m_ViewportImages.resize(m_Ctx.scImgs.size());
		m_ViewportFramebuffers.resize(m_Ctx.scImgs.size());
		for (auto& img : m_ViewportImages)
			img = VulkanImage::Create(m_Ctx.scExtent.width, m_Ctx.scExtent.height, nullptr, m_CmdPool, false, true);

		for (uint32_t i = 0; i < m_ViewportImages.size(); i++)
			m_ViewportImgsDesc[i] = ImGui_ImplVulkan_AddTexture(m_ViewportImages[i].GetSamplerHandle(), m_ViewportImages[i].GetViewHandle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	
		for (uint32_t i = 0; i < m_ViewportFramebuffers.size(); i++)
		{
			std::vector<VkImageView> view;
			view.push_back(m_ViewportImages[i].GetViewHandle());

			VulkanFramebufferInputData data{};
			data.width = m_ViewportImages[i].GetWidth();
			data.height = m_ViewportImages[i].GetHeight();
			data.pass = m_ViewportPass;
			data.views = view;

			m_ViewportFramebuffers[i] = VulkanFramebuffer::Create(data);
		}
	}
}

Renderer::~Renderer()
{
	VkCtxHandler::WaitDeviceIdle();

	// Viewport
	{
		for (auto& img : m_ViewportImages)
			img.Destroy();

		for (auto& fb : m_ViewportFramebuffers)
			fb.Destroy();
	}

	// ImGui
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	m_Pipeline.Destroy();

	DestroySyncObjs();

	vkDestroyDescriptorSetLayout(m_Ctx.device, m_SetLayout, nullptr);
	vkDestroyDescriptorPool(m_Ctx.device, m_DescPool, nullptr);
	vkDestroyDescriptorPool(m_Ctx.device, m_ImGuiDescPool, nullptr);

	m_Image.Destroy();

	m_IndexBuffer.Destroy();
	m_VertBuffer.Destroy();

	m_ImGuiCmdPool.Destroy();
	m_CmdPool.Destroy();

	for (auto& buff : m_Framebuffs)
		buff.Destroy();

	vkDestroyRenderPass(m_Ctx.device, m_ViewportPass, nullptr);
	m_Pass.Destroy();
	VkCtxHandler::DestroyCtx();
}

void Renderer::Render()
{
	BeginFrame();

	RecordFrame();
	
	EndFrame();
}

void Renderer::RenderUI()
{
	ImGui::Begin("Scene");

	m_ViewportSize = ImGui::GetContentRegionAvail();

	ImGui::Image((ImTextureID)m_ViewportImgsDesc[m_CrntImgIdx], ImVec2(m_ViewportFramebuffers[m_CrntImgIdx].GetWidth(), m_ViewportFramebuffers[m_CrntImgIdx].GetHeight()));

	ImGui::End();

	ImGui::Begin("Performance");

	ImGui::Text("FPS :- %0.0f", ImGui::GetIO().Framerate);

	ImGui::End();
}

void Renderer::Update()
{
	if (m_Ctx.scExtent.width != m_Window.GetWidth() || m_Ctx.scExtent.height != m_Window.GetHeight())
	{
		OnResize(m_Window.GetWidth(), m_Window.GetHeight());
	}
}

void Renderer::SetClearColor(float r, float g, float b)
{
	m_ClearVal = {{{ r, g, b, 1.0f }}};
}

void Renderer::BeginFrame()
{
	VK_CHECK(vkWaitForFences(m_Ctx.device, 1, &m_InFlightFences[m_CurrentFrameIdx], VK_TRUE, MAX_UINT64))
	VK_CHECK(vkWaitForFences(m_Ctx.device, 1, &m_ViewportInFlightFences[m_CurrentFrameIdx], VK_TRUE, MAX_UINT64))
	VK_CHECK(vkResetFences(m_Ctx.device, 1, &m_InFlightFences[m_CurrentFrameIdx]))
	VK_CHECK(vkResetFences(m_Ctx.device, 1, &m_ViewportInFlightFences[m_CurrentFrameIdx]))

	VkResult result = vkAcquireNextImageKHR(m_Ctx.device, m_Ctx.swapchain, MAX_UINT64, m_ImgAvailableSemas[m_CurrentFrameIdx], nullptr, &m_CrntImgIdx);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		OnResize(m_Window.GetWidth(), m_Window.GetHeight());
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		std::string resultStr = string_VkResult(result);
		std::string msg = "VkResult is " + resultStr;
		FATAL(msg);
	}

	// Checking if framebuffer needs to be resized
	if (m_ViewportFramebuffers[m_CrntImgIdx].GetWidth() != m_ViewportSize.x || m_ViewportFramebuffers[m_CrntImgIdx].GetHeight() != m_ViewportSize.y)
	{
		VkCtxHandler::WaitDeviceIdle();

		for (auto& img : m_ViewportImages)
			img.Destroy();

		for (auto& fb : m_ViewportFramebuffers)
			fb.Destroy();

		for (auto& desc : m_ViewportImgsDesc)
			ImGui_ImplVulkan_RemoveTexture(desc);

		m_ViewportImgsDesc.resize(m_Ctx.scImgs.size());
		m_ViewportImages.resize(m_Ctx.scImgs.size());
		m_ViewportFramebuffers.resize(m_Ctx.scImgs.size());
		for (auto& img : m_ViewportImages)
			img = VulkanImage::Create(m_ViewportSize.x, m_ViewportSize.y, nullptr, m_CmdPool, false, true);

		for (uint32_t i = 0; i < m_ViewportImages.size(); i++)
			m_ViewportImgsDesc[i] = ImGui_ImplVulkan_AddTexture(m_ViewportImages[i].GetSamplerHandle(), m_ViewportImages[i].GetViewHandle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		for (uint32_t i = 0; i < m_ViewportFramebuffers.size(); i++)
		{
			std::vector<VkImageView> view;
			view.push_back(m_ViewportImages[i].GetViewHandle());

			VulkanFramebufferInputData data{};
			data.width = m_ViewportImages[i].GetWidth();
			data.height = m_ViewportImages[i].GetHeight();
			data.pass = m_ViewportPass;
			data.views = view;

			m_ViewportFramebuffers[i] = VulkanFramebuffer::Create(data);
		}
	}

	m_ViewportCmdBuffs[m_CurrentFrameIdx].Reset();
	m_ViewportCmdBuffs[m_CurrentFrameIdx].Begin();

	auto& frameBuff = m_ViewportFramebuffers[m_CrntImgIdx];

	VkClearValue clearVal = { {{ 0.1f, 0.1f, 0.1f, 1.0f }} };

	VkRenderPassBeginInfo rpBeginInfo{};
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.renderPass = m_ViewportPass;
	rpBeginInfo.framebuffer = frameBuff.GetHandle();
	rpBeginInfo.renderArea.offset = { 0, 0 };
	rpBeginInfo.renderArea.extent = { frameBuff.GetWidth(), frameBuff.GetHeight() };
	rpBeginInfo.clearValueCount = 1;
	rpBeginInfo.pClearValues = &m_ClearVal;

	m_Pass.Begin(m_ViewportCmdBuffs[m_CurrentFrameIdx].GetHandle(), rpBeginInfo);
}

void Renderer::EndFrame()
{
	vkCmdEndRenderPass(m_ViewportCmdBuffs[m_CurrentFrameIdx].GetHandle());
	m_ViewportCmdBuffs[m_CurrentFrameIdx].End();

	// Submit
	{
		auto cmdBuffHandle = m_ViewportCmdBuffs[m_CurrentFrameIdx].GetHandle();

		VkSemaphore waitSemas[] = {
			m_ImgAvailableSemas[m_CurrentFrameIdx]
		};

		VkSemaphore signalSemas[] = {
			m_RndrFinishedSemas[m_CurrentFrameIdx]
		};

		VkPipelineStageFlags waitDstFlags[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffHandle;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemas;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemas;
		submitInfo.pWaitDstStageMask = waitDstFlags;

		VK_CHECK(vkQueueSubmit(m_Ctx.gQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrameIdx]))
	}

	m_CmdBuffs[m_CurrentFrameIdx].Reset();
	m_CmdBuffs[m_CurrentFrameIdx].Begin();

	auto& frameBuff = m_Framebuffs[m_CrntImgIdx];

	VkClearValue clearVal = { {{ 0.1f, 0.1f, 0.1f, 1.0f }} };

	VkRenderPassBeginInfo rpBeginInfo{};
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.renderPass = m_Pass.GetHandle();
	rpBeginInfo.framebuffer = frameBuff.GetHandle();
	rpBeginInfo.renderArea.offset = { 0, 0 };
	rpBeginInfo.renderArea.extent = m_Ctx.scExtent;
	rpBeginInfo.clearValueCount = 1;
	rpBeginInfo.pClearValues = &m_ClearVal;

	m_Pass.Begin(m_CmdBuffs[m_CurrentFrameIdx].GetHandle(), rpBeginInfo);

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::DockSpaceOverViewport(ImGui::GetID("Dockspace"), ImGui::GetMainViewport());

	RenderUI();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_CmdBuffs[m_CurrentFrameIdx].GetHandle(), nullptr);

	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();

	m_Pass.End(m_CmdBuffs[m_CurrentFrameIdx].GetHandle());
	m_CmdBuffs[m_CurrentFrameIdx].End();

	// Submit
	{
		auto cmdBuffHandle = m_CmdBuffs[m_CurrentFrameIdx].GetHandle();

		VkSemaphore waitSemas[] = {
			m_RndrFinishedSemas[m_CurrentFrameIdx]
		};

		VkSemaphore signalSemas[] = {
			m_RndrFinishedSemas[m_CurrentFrameIdx]
		};

		VkPipelineStageFlags waitDstFlags[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffHandle;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemas;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemas;
		submitInfo.pWaitDstStageMask = waitDstFlags;

		VK_CHECK(vkQueueSubmit(m_Ctx.gQueue, 1, &submitInfo, m_ViewportInFlightFences[m_CurrentFrameIdx]))
	}
	// Present
	{
		VkSemaphore waitSemas[] = {
			m_RndrFinishedSemas[m_CurrentFrameIdx]
		};

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Ctx.swapchain;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = waitSemas;
		presentInfo.pImageIndices = &m_CrntImgIdx;

		VkResult result = vkQueuePresentKHR(m_Ctx.pQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			OnResize(m_Window.GetWidth(), m_Window.GetHeight());
			return;
		}
		else
		{
			VK_CHECK(result)
		}
	}

	m_CurrentFrameIdx = (m_CurrentFrameIdx + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::RecordFrame()
{
	m_Pipeline.Bind(m_ViewportCmdBuffs[m_CurrentFrameIdx].GetHandle());

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = (float)m_ViewportFramebuffers[m_CrntImgIdx].GetHeight();
	viewport.width = (float)m_ViewportFramebuffers[m_CrntImgIdx].GetWidth();
	viewport.height = -(float)m_ViewportFramebuffers[m_CrntImgIdx].GetHeight();
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_ViewportCmdBuffs[m_CurrentFrameIdx].GetHandle(), 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_Ctx.scExtent;
	vkCmdSetScissor(m_ViewportCmdBuffs[m_CurrentFrameIdx].GetHandle(), 0, 1, &scissor);

	{
		PushConstData data{};
		data.ViewProj = glm::perspectiveFov(glm::radians(60.0f), (float)viewport.width, -(float)viewport.height, 0.1f, 100.0f)
			* glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 2.0f)));
		data.Transform = glm::rotate(glm::mat4(1.0f), glm::radians((float)(glfwGetTime() * 5)), glm::vec3(1.0f, 0.0f, 1.0f));

		vkCmdPushConstants(m_ViewportCmdBuffs[m_CurrentFrameIdx].GetHandle(), m_Pipeline.GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstData), &data);
	}

	{
		VkBuffer vertBuff[] = { m_VertBuffer.GetHandle() };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindDescriptorSets(m_ViewportCmdBuffs[m_CurrentFrameIdx].GetHandle(), m_Pipeline.GetBindPoint(), m_Pipeline.GetLayout(), 0, 1, &m_DescSets[m_CurrentFrameIdx], 0, nullptr);

		vkCmdBindVertexBuffers(m_ViewportCmdBuffs[m_CurrentFrameIdx].GetHandle(), 0, 1, vertBuff, offsets);
		vkCmdBindIndexBuffer(m_ViewportCmdBuffs[m_CurrentFrameIdx].GetHandle(), m_IndexBuffer.GetHandle(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(m_ViewportCmdBuffs[m_CurrentFrameIdx].GetHandle(), m_VertCount, 1, 0, 0, 0);
	}
}

void Renderer::CreateSyncObjs()
{
	VkSemaphoreCreateInfo semaInfo{};
	semaInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VK_CHECK(vkCreateFence(m_Ctx.device, &fenceInfo, nullptr, &m_InFlightFences[i]))
		VK_CHECK(vkCreateFence(m_Ctx.device, &fenceInfo, nullptr, &m_ViewportInFlightFences[i]))
		VK_CHECK(vkCreateSemaphore(m_Ctx.device, &semaInfo, nullptr, &m_ImgAvailableSemas[i]))
		VK_CHECK(vkCreateSemaphore(m_Ctx.device, &semaInfo, nullptr, &m_RndrFinishedSemas[i]))
	}
}

void Renderer::DestroySyncObjs()
{
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyFence(m_Ctx.device, m_ViewportInFlightFences[i], nullptr);
		vkDestroyFence(m_Ctx.device, m_InFlightFences[i], nullptr);
		vkDestroySemaphore(m_Ctx.device, m_ImgAvailableSemas[i], nullptr);
		vkDestroySemaphore(m_Ctx.device, m_RndrFinishedSemas[i], nullptr);
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	while (m_Window.GetWidth() == 0 || m_Window.GetHeight() == 0) {
		glfwWaitEvents();
	}

	VkCtxHandler::WaitDeviceIdle();

	for (auto& buff : m_Framebuffs)
		buff.Destroy();

	VkCtxHandler::OnResize(m_Window, width, height);

	VulkanFramebufferInputData inputData{};
	inputData.width = m_Ctx.scExtent.width;
	inputData.height = m_Ctx.scExtent.height;
	inputData.pass = m_Pass.GetHandle();

	int i = 0;
	m_Framebuffs.resize(m_Ctx.scImgViews.size());
	for (const auto& imgView : m_Ctx.scImgViews)
	{
		inputData.views = {
			imgView
		};

		m_Framebuffs[i] = VulkanFramebuffer::Create(inputData);

		i++;
	}
}