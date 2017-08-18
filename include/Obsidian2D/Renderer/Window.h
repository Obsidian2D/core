//
// Created by luizorv on 7/22/17.
//

#ifndef OBSIDIAN2D_CORE_WINDOW2_H
#define OBSIDIAN2D_CORE_WINDOW2_H

#include "Camera.h"
#define APP_NAME "Obsidian2D"



namespace Obsidian2D
{
	namespace Renderer
	{
		class Window : public Camera {

		public:
			VkInstance 								instance;
			std::vector<VkPhysicalDevice> 			gpu_vector;
			u_int32_t							 	queue_family_count;
			std::vector<VkQueueFamilyProperties> 	queue_family_props;
			VkPipeline 								vkPipeline;
			VkPipelineCache 						pPipelineCache;
			VkDescriptorPool 						desc_pool;

			~Window()
			{
				vkDestroyInstance(instance, NULL);
			}
		private:
			VkPhysicalDeviceProperties 				gpu_props;
			VkSemaphore         					imageAcquiredSemaphore;
			VkSemaphore         					renderSemaphore;
			VkFence             					drawFence;
			VkSwapchainKHR swap_chain;

		protected:

			void createInstance()
			{
				std::vector<const char *> _layer_names = this->getLayerNames();
				std::vector<const char *> _instance_extension_names;

				_instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
				//@TODO extension for win32 VK_KHR_WIN32_SURFACE_EXTENSION_NAME
				_instance_extension_names.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);

				VkApplicationInfo _app_info = {};
				_app_info.sType 				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
				_app_info.pNext 				= NULL;
				_app_info.pApplicationName 		= APP_NAME;
				_app_info.applicationVersion 	= 1;
				_app_info.pEngineName 			= APP_NAME;
				_app_info.engineVersion 		= 1;
				_app_info.apiVersion 			= VK_API_VERSION_1_0;

				VkInstanceCreateInfo _inst_info = {};
				_inst_info.sType 					= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				_inst_info.pNext 					= NULL;
				_inst_info.flags 					= 0;
				_inst_info.pApplicationInfo 		= &_app_info;
				_inst_info.enabledLayerCount 		= (uint32_t) _layer_names.size();
				_inst_info.ppEnabledLayerNames 		= _layer_names.size() ? _layer_names.data() : NULL;
				_inst_info.enabledExtensionCount 	= (uint32_t) _instance_extension_names.size();
				_inst_info.ppEnabledExtensionNames 	= _instance_extension_names.data();

				VkResult res = vkCreateInstance(&_inst_info, NULL, &instance);
				assert(res == VK_SUCCESS);
			}

			void createLogicalDeviceAndCommandBuffer()
			{

				VkResult res = vkEnumeratePhysicalDevices(instance, &queue_family_count, NULL);
				assert(res == VK_SUCCESS && queue_family_count);
				gpu_vector.resize(queue_family_count);
				res = vkEnumeratePhysicalDevices(instance, &queue_family_count, gpu_vector.data());
				assert(res == VK_SUCCESS);

				vkGetPhysicalDeviceQueueFamilyProperties(gpu_vector[0], &queue_family_count, NULL);
				assert(queue_family_count >= 1);

				queue_family_props.resize(queue_family_count);
				vkGetPhysicalDeviceQueueFamilyProperties(gpu_vector[0], &queue_family_count, queue_family_props.data());
				assert(queue_family_count >= 1);

				vkGetPhysicalDeviceMemoryProperties(gpu_vector[0], &memory_properties);
				vkGetPhysicalDeviceProperties(gpu_vector[0], &gpu_props);

				VkDeviceQueueCreateInfo queue_info = {};

				bool found = false;
				for (unsigned int i = 0; i < queue_family_count; i++) {
					if (queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
						queue_info.queueFamilyIndex = i;
						found = true;
						break;
					}
				}
				assert(found);

				float queue_priorities[1] = {0.0};
				queue_info.sType 			= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queue_info.pNext 			= NULL;
				queue_info.queueCount 		= 1;
				queue_info.pQueuePriorities = queue_priorities;

				std::vector<const char *> device_extension_names;
				device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

				VkDeviceCreateInfo device_info = {};
				device_info.sType 					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				device_info.pNext 					= NULL;
				device_info.queueCreateInfoCount 	= 1;
				device_info.pQueueCreateInfos 		= &queue_info;
				device_info.enabledExtensionCount 	= (uint32_t)device_extension_names.size();
				device_info.ppEnabledExtensionNames = device_info.enabledExtensionCount ? device_extension_names.data() : NULL;
				device_info.enabledLayerCount 		= 0;
				device_info.ppEnabledLayerNames 	= NULL;
				device_info.pEnabledFeatures 		= NULL;

				res = vkCreateDevice(gpu_vector[0], &device_info, NULL, &device);
				assert(res == VK_SUCCESS);

				VkCommandPoolCreateInfo cmd_pool_info = {};
				cmd_pool_info.sType 			= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				cmd_pool_info.pNext 			= NULL;
				cmd_pool_info.queueFamilyIndex  = queue_info.queueFamilyIndex;
				cmd_pool_info.flags 			= 0;

				VkCommandPool _command_pool;
				res = vkCreateCommandPool(device, &cmd_pool_info, NULL, &_command_pool);
				assert(res == VK_SUCCESS);

				/* Create the command buffer from the command pool */
				VkCommandBufferAllocateInfo cmd = {};
				cmd.sType 				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				cmd.pNext 			 	= NULL;
				cmd.commandPool 	 	= _command_pool;
				cmd.level 			 	= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				cmd.commandBufferCount  = 1;

				res = vkAllocateCommandBuffers(device, &cmd, &command_buffer);
				assert(res == VK_SUCCESS);
			}

			void initBuffers (
					uint32_t graphics_queue_family_index,
					uint32_t present_queue_family_index,
					VkSurfaceKHR surface,
					const bool depthPresent,
					const void *vertexData,
					uint32_t dataSize,
					uint32_t dataStride,
					VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
			){
				/* DEPENDS on init_command_buffer() */
				VkResult U_ASSERT_ONLY res;

				VkCommandBufferBeginInfo cmd_buf_info = {};
				cmd_buf_info.sType 							= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				cmd_buf_info.pNext 							= NULL;
				cmd_buf_info.flags 							= 0;
				cmd_buf_info.pInheritanceInfo 				= NULL;

				res = vkBeginCommandBuffer(command_buffer, &cmd_buf_info);
				assert(res == VK_SUCCESS);

				vkGetDeviceQueue(device, graphics_queue_family_index, 0, &graphics_queue);
				if (graphics_queue_family_index == present_queue_family_index) {
					present_queue = graphics_queue;
				} else {
					vkGetDeviceQueue(device, present_queue_family_index, 0, &present_queue);
				}


				/* DEPENDS on info.cmd and info.queue initialized */

				VkSurfaceCapabilitiesKHR surfCapabilities;

				res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu_vector[0], surface, &surfCapabilities);
				assert(res == VK_SUCCESS);

				uint32_t presentModeCount;
				res = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu_vector[0], surface, &presentModeCount, NULL);
				assert(res == VK_SUCCESS);
				VkPresentModeKHR *presentModes = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
				assert(presentModes);
				res = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu_vector[0], surface, &presentModeCount, presentModes);
				assert(res == VK_SUCCESS);

				VkExtent2D swapchainExtent;
				// width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
				if (surfCapabilities.currentExtent.width == 0xFFFFFFFF) {
					// If the surface size is undefined, the size is set to
					// the size of the images requested.
					swapchainExtent.width =  (uint32_t)width;
					swapchainExtent.height = (uint32_t)height;
					if (swapchainExtent.width < surfCapabilities.minImageExtent.width) {
						swapchainExtent.width = surfCapabilities.minImageExtent.width;
					} else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width) {
						swapchainExtent.width = surfCapabilities.maxImageExtent.width;
					}

					if (swapchainExtent.height < surfCapabilities.minImageExtent.height) {
						swapchainExtent.height = surfCapabilities.minImageExtent.height;
					} else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height) {
						swapchainExtent.height = surfCapabilities.maxImageExtent.height;
					}
				} else {
					// If the surface size is defined, the swap chain size must match
					swapchainExtent = surfCapabilities.currentExtent;
				}

				// The FIFO present mode is guaranteed by the spec to be supported
				// Also note that current Android driver only supports FIFO
				VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

				// Determine the number of VkImage's to use in the swap chain.
				// We need to acquire only 1 presentable image at at time.
				// Asking for minImageCount images ensures that we can acquire
				// 1 presentable image as long as we present it before attempting
				// to acquire another.
				uint32_t desiredNumberOfSwapChainImages = surfCapabilities.minImageCount;

				VkSurfaceTransformFlagBitsKHR preTransform;
				if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
					preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
				} else {
					preTransform = surfCapabilities.currentTransform;
				}

				// Find a supported composite alpha mode - one of these is guaranteed to be set
				VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
						VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
						VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
						VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
						VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
				};
				for (uint32_t i = 0; i < sizeof(compositeAlphaFlags); i++) {
					if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i]) {
						compositeAlpha = compositeAlphaFlags[i];
						break;
					}
				}

				VkSwapchainCreateInfoKHR swapchain_ci = {};
				swapchain_ci.sType 					= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				swapchain_ci.pNext 					= NULL;
				swapchain_ci.surface 				= surface;
				swapchain_ci.minImageCount 			= desiredNumberOfSwapChainImages;
				swapchain_ci.imageFormat 			= format;
				swapchain_ci.imageExtent.width 		= swapchainExtent.width;
				swapchain_ci.imageExtent.height 	= swapchainExtent.height;
				swapchain_ci.preTransform 			= preTransform;
				swapchain_ci.compositeAlpha 		= compositeAlpha;
				swapchain_ci.imageArrayLayers 		= 1;
				swapchain_ci.presentMode 			= swapchainPresentMode;
				swapchain_ci.oldSwapchain 			= VK_NULL_HANDLE;
#ifndef __ANDROID__
				swapchain_ci.clipped 				= (VkBool32)true;
#else
				swapchain_ci.clipped 				= false;
#endif
				swapchain_ci.imageColorSpace 		= VK_COLORSPACE_SRGB_NONLINEAR_KHR;
				swapchain_ci.imageUsage 			= usageFlags;
				swapchain_ci.imageSharingMode 		= VK_SHARING_MODE_EXCLUSIVE;
				swapchain_ci.queueFamilyIndexCount  = 0;
				swapchain_ci.pQueueFamilyIndices 	= NULL;

				uint32_t queueFamilyIndices[2] = {(uint32_t)graphics_queue_family_index, (uint32_t)present_queue_family_index};
				if (graphics_queue_family_index != present_queue_family_index) {
					// If the graphics and present queues are from different queue families,
					// we either have to explicitly transfer ownership of images between the
					// queues, or we have to create the swapchain with imageSharingMode
					// as VK_SHARING_MODE_CONCURRENT
					swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
					swapchain_ci.queueFamilyIndexCount = 2;
					swapchain_ci.pQueueFamilyIndices = queueFamilyIndices;
				}


				res = vkCreateSwapchainKHR(device, &swapchain_ci, NULL, &swap_chain);
				assert(res == VK_SUCCESS);

				uint32_t swapchainImageCount;
				res = vkGetSwapchainImagesKHR(device, swap_chain, &swapchainImageCount, NULL);
				assert(res == VK_SUCCESS);

				VkImage *swapchainImages = (VkImage *)malloc(swapchainImageCount * sizeof(VkImage));
				assert(swapchainImages);
				res = vkGetSwapchainImagesKHR(device, swap_chain,
											  &swapchainImageCount, swapchainImages);
				assert(res == VK_SUCCESS);

				for (uint32_t i = 0; i < swapchainImageCount; i++) {
					swap_chain_buffer sc_buffer;

					VkImageViewCreateInfo color_image_view = {};
					color_image_view.sType 								= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
					color_image_view.pNext 								= NULL;
					color_image_view.format								= format;
					color_image_view.components.r 						= VK_COMPONENT_SWIZZLE_R;
					color_image_view.components.g 						= VK_COMPONENT_SWIZZLE_G;
					color_image_view.components.b 						= VK_COMPONENT_SWIZZLE_B;
					color_image_view.components.a 						= VK_COMPONENT_SWIZZLE_A;
					color_image_view.subresourceRange.aspectMask 		= VK_IMAGE_ASPECT_COLOR_BIT;
					color_image_view.subresourceRange.baseMipLevel		= 0;
					color_image_view.subresourceRange.levelCount 		= 1;
					color_image_view.subresourceRange.baseArrayLayer 	= 0;
					color_image_view.subresourceRange.layerCount 		= 1;
					color_image_view.viewType 							= VK_IMAGE_VIEW_TYPE_2D;
					color_image_view.flags 								= 0;

					sc_buffer.image = swapchainImages[0];
					color_image_view.image = sc_buffer.image;

					res = vkCreateImageView(device, &color_image_view, NULL, &sc_buffer.view);
					buffers.push_back(sc_buffer);
					assert(res == VK_SUCCESS);
				}

				free(swapchainImages);
				current_buffer = 0;

				if (NULL != presentModes) {
					free(presentModes);
				}

				struct {
					VkFormat format = VK_FORMAT_UNDEFINED;
					VkImage image;
					VkDeviceMemory mem;
					VkImageView view;
				} depth;

				bool U_ASSERT_ONLY pass;
				VkImageCreateInfo image_info = {};

				/* allow custom depth formats */
				if (depth.format == VK_FORMAT_UNDEFINED) depth.format = VK_FORMAT_D16_UNORM;

#ifdef __ANDROID__
				// Depth format needs to be VK_FORMAT_D24_UNORM_S8_UINT on Android.
    const VkFormat depth_format = VK_FORMAT_D24_UNORM_S8_UINT;
#else
				const VkFormat depth_format = depth.format;
#endif
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(gpu_vector[0], depth_format, &props);
				if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
					image_info.tiling = VK_IMAGE_TILING_LINEAR;
				} else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
					image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
				} else {
					/* Try other depth formats? */
					std::cout << "depth_format " << depth_format << " Unsupported.\n";
					exit(-1);
				}

				image_info.sType 								= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				image_info.pNext 								= NULL;
				image_info.imageType 							= VK_IMAGE_TYPE_2D;
				image_info.format 								= depth_format;
				image_info.extent.width 						= (uint32_t)width;
				image_info.extent.height 						= (uint32_t)height;
				image_info.extent.depth 						= 1;
				image_info.mipLevels 							= 1;
				image_info.arrayLayers 							= 1;
				image_info.samples 								= VK_SAMPLE_COUNT_1_BIT;
				image_info.initialLayout 						= VK_IMAGE_LAYOUT_UNDEFINED;
				image_info.queueFamilyIndexCount 				= 0;
				image_info.pQueueFamilyIndices 					= NULL;
				image_info.sharingMode 							= VK_SHARING_MODE_EXCLUSIVE;
				image_info.usage 								= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				image_info.flags 								= 0;

				VkMemoryAllocateInfo mem_alloc = {};
				mem_alloc.sType									= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				mem_alloc.pNext									= NULL;
				mem_alloc.allocationSize 						= 0;
				mem_alloc.memoryTypeIndex 						= 0;

				VkImageViewCreateInfo view_info = {};
				view_info.sType 								= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				view_info.pNext 								= NULL;
				view_info.image 								= VK_NULL_HANDLE;
				view_info.format 								= depth_format;
				view_info.components.r 							= VK_COMPONENT_SWIZZLE_R;
				view_info.components.g 							= VK_COMPONENT_SWIZZLE_G;
				view_info.components.b 							= VK_COMPONENT_SWIZZLE_B;
				view_info.components.a 							= VK_COMPONENT_SWIZZLE_A;
				view_info.subresourceRange.aspectMask 			= VK_IMAGE_ASPECT_DEPTH_BIT;
				view_info.subresourceRange.baseMipLevel 		= 0;
				view_info.subresourceRange.levelCount 			= 1;
				view_info.subresourceRange.baseArrayLayer 		= 0;
				view_info.subresourceRange.layerCount 			= 1;
				view_info.viewType 								= VK_IMAGE_VIEW_TYPE_2D;
				view_info.flags 								= 0;

				if (depth_format == VK_FORMAT_D16_UNORM_S8_UINT || depth_format == VK_FORMAT_D24_UNORM_S8_UINT ||
					depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
					view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				}

				/* Create image */
				res = vkCreateImage(device, &image_info, NULL, &depth.image);
				assert(res == VK_SUCCESS);

				VkMemoryRequirements mem_reqs;
				vkGetImageMemoryRequirements(device, depth.image, &mem_reqs);

				mem_alloc.allocationSize = mem_reqs.size;
				/* Use the memory properties to determine the type of memory required */
				pass = this->memory_type_from_properties(
						memory_properties,
						mem_reqs.memoryTypeBits,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
						&mem_alloc.memoryTypeIndex
				);
				assert(pass);

				/* Allocate memory */
				res = vkAllocateMemory(device, &mem_alloc, NULL, &depth.mem);
				assert(res == VK_SUCCESS);

				/* Bind memory */
				res = vkBindImageMemory(device, depth.image, depth.mem, 0);
				assert(res == VK_SUCCESS);

				/* Create image view */
				view_info.image = depth.image;
				res = vkCreateImageView(device, &view_info, NULL, &depth.view);
				assert(res == VK_SUCCESS);



				this->initCamera(width, height);


				struct {
					VkBuffer buf;
					VkDeviceMemory mem;
					VkDescriptorBufferInfo buffer_info;
				} uniform_data;

				glm::mat4 MVP;
				uint8_t *pData;
				VkBufferCreateInfo buf_info = {};

				buf_info.sType							= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				buf_info.pNext							= NULL;
				buf_info.usage							= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
				buf_info.size 							= sizeof(MVP);
				buf_info.queueFamilyIndexCount			= 0;
				buf_info.pQueueFamilyIndices 			= NULL;
				buf_info.sharingMode 					= VK_SHARING_MODE_EXCLUSIVE;
				buf_info.flags 							= 0;

				res = vkCreateBuffer(device, &buf_info, NULL, &uniform_data.buf);
				assert(res == VK_SUCCESS);

				vkGetBufferMemoryRequirements(device, uniform_data.buf, &mem_reqs);

				VkMemoryAllocateInfo alloc_info = {};
				alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				alloc_info.pNext = NULL;
				alloc_info.memoryTypeIndex = 0;

				alloc_info.allocationSize = mem_reqs.size;
				pass = memory_type_from_properties(
						memory_properties,
						mem_reqs.memoryTypeBits,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						&alloc_info.memoryTypeIndex
				);
				assert(pass && "No mappable, coherent memory");

				res = vkAllocateMemory(device, &alloc_info, NULL, &(uniform_data.mem));
				assert(res == VK_SUCCESS);

				res = vkMapMemory(device, uniform_data.mem, 0, mem_reqs.size, 0, (void **)&pData);
				assert(res == VK_SUCCESS);

				memcpy(pData, &MVP, sizeof(MVP));
				this->setCameraBufferAddress(pData);

				vkUnmapMemory(device, uniform_data.mem);

				res = vkBindBufferMemory(device, uniform_data.buf, uniform_data.mem, 0);
				assert(res == VK_SUCCESS);

				uniform_data.buffer_info.buffer = uniform_data.buf;
				uniform_data.buffer_info.offset = 0;
				uniform_data.buffer_info.range = sizeof(MVP);


				bool use_texture = false;
				VkDescriptorSetLayoutBinding layout_bindings[2];
				layout_bindings[0].binding 								= 0;
				layout_bindings[0].descriptorType 						= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				layout_bindings[0].descriptorCount 						= 1;
				layout_bindings[0].stageFlags 							= VK_SHADER_STAGE_VERTEX_BIT;
				layout_bindings[0].pImmutableSamplers					= NULL;

				if (use_texture) {
					layout_bindings[1].binding 							= 1;
					layout_bindings[1].descriptorType 					= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					layout_bindings[1].descriptorCount 					= 1;
					layout_bindings[1].stageFlags 						= VK_SHADER_STAGE_FRAGMENT_BIT;
					layout_bindings[1].pImmutableSamplers 				= NULL;
				}

				/* Next take layout bindings and use them to create a descriptor set layout
				 */
				VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
				descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				descriptor_layout.pNext = NULL;
				descriptor_layout.bindingCount = use_texture ? 2 : 1;
				descriptor_layout.pBindings = layout_bindings;

				std::vector<VkDescriptorSetLayout> desc_layout;

				desc_layout.resize(1);
				res = vkCreateDescriptorSetLayout(device, &descriptor_layout, NULL, desc_layout.data());
				assert(res == VK_SUCCESS);

				/* Now use the descriptor layout to create a pipeline layout */
				VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
				pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				pPipelineLayoutCreateInfo.pNext = NULL;
				pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
				pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
				pPipelineLayoutCreateInfo.setLayoutCount = 1;
				pPipelineLayoutCreateInfo.pSetLayouts = desc_layout.data();

				VkPipelineLayout pipeline_layout;
				res = vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, NULL, &pipeline_layout);
				assert(res == VK_SUCCESS);



				/* DEPENDS on init_swap_chain() and init_depth_buffer() */
				bool clear = true;
				VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				/* Need attachments for render target and depth buffer */
				VkAttachmentDescription attachments[2];
				attachments[0].format 						= format;
				attachments[0].samples						= VK_SAMPLE_COUNT_1_BIT;
				attachments[0].loadOp 						= clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachments[0].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
				attachments[0].stencilLoadOp 				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachments[0].stencilStoreOp 				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachments[0].initialLayout 				= VK_IMAGE_LAYOUT_UNDEFINED;
				attachments[0].finalLayout 					= finalLayout;
				attachments[0].flags 						= 0;

				if (depthPresent) {
					attachments[1].format 					= depth.format;
					attachments[1].samples 					= VK_SAMPLE_COUNT_1_BIT;
					attachments[1].loadOp 					= clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					attachments[1].storeOp 					= VK_ATTACHMENT_STORE_OP_STORE;
					attachments[1].stencilLoadOp 			= VK_ATTACHMENT_LOAD_OP_LOAD;
					attachments[1].stencilStoreOp 			= VK_ATTACHMENT_STORE_OP_STORE;
					attachments[1].initialLayout 			= VK_IMAGE_LAYOUT_UNDEFINED;
					attachments[1].finalLayout 				= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					attachments[1].flags 					= 0;
				}

				VkAttachmentReference color_reference = {};
				color_reference.attachment 					= 0;
				color_reference.layout 						= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				VkAttachmentReference depth_reference = {};
				depth_reference.attachment 					= 1;
				depth_reference.layout 						= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				VkSubpassDescription subpass = {};
				subpass.pipelineBindPoint 					= VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass.flags 								= 0;
				subpass.inputAttachmentCount 				= 0;
				subpass.pInputAttachments 					= NULL;
				subpass.colorAttachmentCount 				= 1;
				subpass.pColorAttachments 					= &color_reference;
				subpass.pResolveAttachments 				= NULL;
				subpass.pDepthStencilAttachment 			= depthPresent ? &depth_reference : NULL;
				subpass.preserveAttachmentCount 			= 0;
				subpass.pPreserveAttachments 				= NULL;

				VkRenderPassCreateInfo rp_info = {};
				rp_info.sType 								= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				rp_info.pNext 								= NULL;
				rp_info.attachmentCount 					= depthPresent ? 2 : 1;
				rp_info.pAttachments 						= attachments;
				rp_info.subpassCount 						= 1;
				rp_info.pSubpasses 							= &subpass;
				rp_info.dependencyCount 					= 0;
				rp_info.pDependencies 						= NULL;

				VkRenderPass render_pass;
				res = vkCreateRenderPass(device, &rp_info, NULL, &render_pass);
				assert(res == VK_SUCCESS);




				bool U_ASSERT_ONLY retVal;

				// If no shaders were submitted, just return
				if (!(this->initialVertShaderText || this->initialFragShaderText)) return;

				init_glslang();
				VkShaderModuleCreateInfo moduleCreateInfo;
				VkPipelineShaderStageCreateInfo shaderStages[2];

				if (this->initialVertShaderText) {
					std::vector<unsigned int> vtx_spv;
					shaderStages[0].sType 								= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
					shaderStages[0].pNext 								= NULL;
					shaderStages[0].pSpecializationInfo 				= NULL;
					shaderStages[0].flags 								= 0;
					shaderStages[0].stage 								= VK_SHADER_STAGE_VERTEX_BIT;
					shaderStages[0].pName 								= "main";

					retVal = GLSLtoSPV(VK_SHADER_STAGE_VERTEX_BIT, this->initialVertShaderText, vtx_spv);
					assert(retVal);

					moduleCreateInfo.sType 								= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
					moduleCreateInfo.pNext 								= NULL;
					moduleCreateInfo.flags 								= 0;
					moduleCreateInfo.codeSize							= vtx_spv.size() * sizeof(unsigned int);
					moduleCreateInfo.pCode 								= vtx_spv.data();
					res = vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderStages[0].module);
					assert(res == VK_SUCCESS);
				}

				if (this->initialFragShaderText) {
					std::vector<unsigned int> frag_spv;
					shaderStages[1].sType 								= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
					shaderStages[1].pNext 								= NULL;
					shaderStages[1].pSpecializationInfo 				= NULL;
					shaderStages[1].flags 								= 0;
					shaderStages[1].stage 								= VK_SHADER_STAGE_FRAGMENT_BIT;
					shaderStages[1].pName 								= "main";

					retVal = GLSLtoSPV(VK_SHADER_STAGE_FRAGMENT_BIT, this->initialFragShaderText, frag_spv);
					assert(retVal);

					moduleCreateInfo.sType 								= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
					moduleCreateInfo.pNext 								= NULL;
					moduleCreateInfo.flags 								= 0;
					moduleCreateInfo.codeSize 							= frag_spv.size() * sizeof(unsigned int);
					moduleCreateInfo.pCode 								= frag_spv.data();

					res = vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderStages[1].module);
					assert(res == VK_SUCCESS);
				}

				finalize_glslang();


				VkImageView img_attachments[2];
				img_attachments[1] = depth.view;

				VkFramebufferCreateInfo fb_info = {};
				fb_info.sType 											= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				fb_info.pNext 											= NULL;
				fb_info.renderPass 										= render_pass;
				fb_info.attachmentCount 								= depthPresent ? 2 : 1;
				fb_info.pAttachments 									= img_attachments;
				fb_info.width 											= (uint32_t)width;
				fb_info.height 											= (uint32_t)height;
				fb_info.layers 											= 1;

				uint32_t i;

				VkFramebuffer *framebuffers = (VkFramebuffer *)malloc(swapchainImageCount * sizeof(VkFramebuffer));

				for (i = 0; i < swapchainImageCount; i++) {
					img_attachments[0] = buffers[i].view;
					res = vkCreateFramebuffer(device, &fb_info, NULL, &framebuffers[i]);
					assert(res == VK_SUCCESS);
				}



				VkBufferCreateInfo vertex_buf_info = {};
				vertex_buf_info.sType 									= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				vertex_buf_info.pNext 									= NULL;
				vertex_buf_info.usage 									= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				vertex_buf_info.size 									= dataSize;
				vertex_buf_info.queueFamilyIndexCount 					= 0;
				vertex_buf_info.pQueueFamilyIndices 					= NULL;
				vertex_buf_info.sharingMode 							= VK_SHARING_MODE_EXCLUSIVE;
				vertex_buf_info.flags 									= 0;

				struct {
					VkBuffer buf;
					VkDeviceMemory mem;
					VkDescriptorBufferInfo buffer_info;
				} vertex_buffer;

				res = vkCreateBuffer(device, &vertex_buf_info, NULL, &vertex_buffer.buf);
				assert(res == VK_SUCCESS);

				vkGetBufferMemoryRequirements(device, vertex_buffer.buf, &mem_reqs);

				VkMemoryAllocateInfo m_alloc_info = {};
				m_alloc_info.sType 										= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				m_alloc_info.pNext 										= NULL;
				m_alloc_info.memoryTypeIndex 							= 0;
				m_alloc_info.allocationSize 							= mem_reqs.size;
				pass = memory_type_from_properties (
						memory_properties,
						mem_reqs.memoryTypeBits,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						&m_alloc_info.memoryTypeIndex
				);
				assert(pass && "No mappable, coherent memory");

				res = vkAllocateMemory(device, &alloc_info, NULL, &(vertex_buffer.mem));
				assert(res == VK_SUCCESS);
				vertex_buffer.buffer_info.range 						= mem_reqs.size;
				vertex_buffer.buffer_info.offset 						= 0;

				res = vkMapMemory(device, vertex_buffer.mem, 0, mem_reqs.size, 0, (void **)&pData);
				assert(res == VK_SUCCESS);

				memcpy(pData, vertexData, dataSize);

				vkUnmapMemory(device, vertex_buffer.mem);

				res = vkBindBufferMemory(device, vertex_buffer.buf, vertex_buffer.mem, 0);
				assert(res == VK_SUCCESS);

				VkVertexInputBindingDescription vi_binding;
				vi_binding.binding 										= 0;
				vi_binding.inputRate 									= VK_VERTEX_INPUT_RATE_VERTEX;
				vi_binding.stride 										= dataStride;

				VkVertexInputAttributeDescription vi_attribs[2];
				vi_attribs[0].binding 									= 0;
				vi_attribs[0].location 									= 0;
				vi_attribs[0].format 									= VK_FORMAT_R32G32B32A32_SFLOAT;
				vi_attribs[0].offset 									= 0;
				vi_attribs[1].binding 									= 0;
				vi_attribs[1].location 									= 1;
				vi_attribs[1].format 									= use_texture ? VK_FORMAT_R32G32_SFLOAT : VK_FORMAT_R32G32B32A32_SFLOAT;
				vi_attribs[1].offset 									= 16;


				VkDescriptorPoolSize type_count[2];
				type_count[0].type 												= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				type_count[0].descriptorCount 									= 1;
				if (use_texture) {
					type_count[1].type 											= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					type_count[1].descriptorCount 								= 1;
				}

				VkDescriptorPoolCreateInfo descriptor_pool = {};
				descriptor_pool.sType 											= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				descriptor_pool.pNext 											= NULL;
				descriptor_pool.maxSets 										= 1;
				descriptor_pool.poolSizeCount 									= use_texture ? 2 : 1;
				descriptor_pool.pPoolSizes 										= type_count;


				std::vector<VkDescriptorSet> desc_set;

				res = vkCreateDescriptorPool(device, &descriptor_pool, NULL, &desc_pool);
				assert(res == VK_SUCCESS);

				VkDescriptorSetAllocateInfo _alloc_info[1];
				_alloc_info[0].sType 											= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				_alloc_info[0].pNext 											= NULL;
				_alloc_info[0].descriptorPool 									= desc_pool;
				_alloc_info[0].descriptorSetCount 								= 1;
				_alloc_info[0].pSetLayouts 										= desc_layout.data();

				desc_set.resize(1);
				res = vkAllocateDescriptorSets(device, _alloc_info, desc_set.data());
				assert(res == VK_SUCCESS);

				VkWriteDescriptorSet writes[2];

				struct {
					VkDescriptorImageInfo image_info;
				} texture_data;

				writes[0] = {};
				writes[0].sType 												= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writes[0].pNext 												= NULL;
				writes[0].dstSet 												= desc_set[0];
				writes[0].descriptorCount 										= 1;
				writes[0].descriptorType 										= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writes[0].pBufferInfo 											= &uniform_data.buffer_info;
				writes[0].dstArrayElement 										= 0;
				writes[0].dstBinding 											= 0;

				if (use_texture) {
					writes[1] = {};
					writes[1].sType 											= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writes[1].dstSet 											= desc_set[0];
					writes[1].dstBinding 										= 1;
					writes[1].descriptorCount 									= 1;
					writes[1].descriptorType 									= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					writes[1].pImageInfo 										= &texture_data.image_info;
					writes[1].dstArrayElement 									= 0;
				}

				vkUpdateDescriptorSets(device, use_texture ? 2 : 1, writes, 0, NULL);




				VkPipelineCacheCreateInfo pipelineCache;
				pipelineCache.sType 											= VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
				pipelineCache.pNext 											= NULL;
				pipelineCache.initialDataSize 									= 0;
				pipelineCache.pInitialData 										= NULL;
				pipelineCache.flags 											= 0;
				res = vkCreatePipelineCache(device, &pipelineCache, NULL, &pPipelineCache);
				assert(res == VK_SUCCESS);


				VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
				VkPipelineDynamicStateCreateInfo dynamicState = {};
				memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);

				dynamicState.sType 												= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
				dynamicState.pNext 												= NULL;
				dynamicState.pDynamicStates 									= dynamicStateEnables;
				dynamicState.dynamicStateCount 									= 0;

				bool include_vi = true;

				VkPipelineVertexInputStateCreateInfo vi;
				memset(&vi, 0, sizeof(vi));
				vi.sType 														= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
				if (include_vi) {
					vi.pNext 													= NULL;
					vi.flags 													= 0;
					vi.vertexBindingDescriptionCount 							= 1;
					vi.pVertexBindingDescriptions 								= &vi_binding;
					vi.vertexAttributeDescriptionCount 							= 2;
					vi.pVertexAttributeDescriptions 							= vi_attribs;
				}
				VkPipelineInputAssemblyStateCreateInfo ia;
				ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
				ia.pNext = NULL;
				ia.flags = 0;
				ia.primitiveRestartEnable = VK_FALSE;
				ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

				VkPipelineRasterizationStateCreateInfo rs;
				rs.sType 														= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
				rs.pNext 														= NULL;
				rs.flags 														= 0;
				rs.polygonMode 													= VK_POLYGON_MODE_FILL;
				rs.cullMode 													= VK_CULL_MODE_BACK_BIT;
				rs.frontFace 													= VK_FRONT_FACE_CLOCKWISE;
				rs.depthClampEnable 											= VK_FALSE;
				rs.rasterizerDiscardEnable 										= VK_FALSE;
				rs.depthBiasEnable 												= VK_FALSE;
				rs.depthBiasConstantFactor 										= 0;
				rs.depthBiasClamp 												= 0;
				rs.depthBiasSlopeFactor 										= 0;
				rs.lineWidth 													= 1.0f;

				VkPipelineColorBlendStateCreateInfo cb;
				cb.sType 														= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
				cb.flags 														= 0;
				cb.pNext 														= NULL;

				VkPipelineColorBlendAttachmentState att_state[1];
				att_state[0].colorWriteMask 									= 0xf;
				att_state[0].blendEnable 										= VK_FALSE;
				att_state[0].alphaBlendOp 										= VK_BLEND_OP_ADD;
				att_state[0].colorBlendOp 										= VK_BLEND_OP_ADD;
				att_state[0].srcColorBlendFactor 								= VK_BLEND_FACTOR_ZERO;
				att_state[0].dstColorBlendFactor 								= VK_BLEND_FACTOR_ZERO;
				att_state[0].srcAlphaBlendFactor 								= VK_BLEND_FACTOR_ZERO;
				att_state[0].dstAlphaBlendFactor 								= VK_BLEND_FACTOR_ZERO;
				cb.attachmentCount 												= 1;
				cb.pAttachments 												= att_state;
				cb.logicOpEnable 												= VK_FALSE;
				cb.logicOp 														= VK_LOGIC_OP_NO_OP;
				cb.blendConstants[0] 											= 1.0f;
				cb.blendConstants[1] 											= 1.0f;
				cb.blendConstants[2] 											= 1.0f;
				cb.blendConstants[3] 											= 1.0f;

				VkPipelineViewportStateCreateInfo vp = {};
				vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
				vp.pNext = NULL;
				vp.flags = 0;
#ifndef __ANDROID__
				vp.viewportCount 												= 1;
				dynamicStateEnables[dynamicState.dynamicStateCount++] 			= VK_DYNAMIC_STATE_VIEWPORT;
				vp.scissorCount 												= 1;
				dynamicStateEnables[dynamicState.dynamicStateCount++] 			= VK_DYNAMIC_STATE_SCISSOR;
				vp.pScissors 													= NULL;
				vp.pViewports 													= NULL;
#else
				// Temporary disabling dynamic viewport on Android because some of drivers doesn't
    // support the feature.
    VkViewport viewports;
    viewports.minDepth = 0.0f;
    viewports.maxDepth = 1.0f;
    viewports.x = 0;
    viewports.y = 0;
    viewports.width = info.width;
    viewports.height = info.height;
    VkRect2D scissor;
    scissor.extent.width = info.width;
    scissor.extent.height = info.height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vp.viewportCount = NUM_VIEWPORTS;
    vp.scissorCount = NUM_SCISSORS;
    vp.pScissors = &scissor;
    vp.pViewports = &viewports;
#endif
				VkPipelineDepthStencilStateCreateInfo ds;
				ds.sType 														= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
				ds.pNext 														= NULL;
				ds.flags 														= 0;
				ds.depthTestEnable 												= (VkBool32) depthPresent;
				ds.depthWriteEnable 											= (VkBool32) depthPresent;
				ds.depthCompareOp 												= VK_COMPARE_OP_LESS_OR_EQUAL;
				ds.depthBoundsTestEnable 										= VK_FALSE;
				ds.stencilTestEnable 											= VK_FALSE;
				ds.back.failOp 													= VK_STENCIL_OP_KEEP;
				ds.back.passOp 													= VK_STENCIL_OP_KEEP;
				ds.back.compareOp 												= VK_COMPARE_OP_ALWAYS;
				ds.back.compareMask 											= 0;
				ds.back.reference 												= 0;
				ds.back.depthFailOp 											= VK_STENCIL_OP_KEEP;
				ds.back.writeMask 												= 0;
				ds.minDepthBounds 												= 0;
				ds.maxDepthBounds 												= 0;
				ds.stencilTestEnable 											= VK_FALSE;
				ds.front 														= ds.back;

				VkPipelineMultisampleStateCreateInfo ms;
				ms.sType 														= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				ms.pNext 														= NULL;
				ms.flags 														= 0;
				ms.pSampleMask 													= NULL;
				ms.rasterizationSamples 										= VK_SAMPLE_COUNT_1_BIT;
				ms.sampleShadingEnable 											= VK_FALSE;
				ms.alphaToCoverageEnable 										= VK_FALSE;
				ms.alphaToOneEnable 											= VK_FALSE;
				ms.minSampleShading 											= 0.0;

				VkGraphicsPipelineCreateInfo pipeline;
				pipeline.sType 													= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
				pipeline.pNext 													= NULL;
				pipeline.layout 												= pipeline_layout;
				pipeline.basePipelineHandle 									= VK_NULL_HANDLE;
				pipeline.basePipelineIndex 										= 0;
				pipeline.flags 													= 0;
				pipeline.pVertexInputState 										= &vi;
				pipeline.pInputAssemblyState 									= &ia;
				pipeline.pRasterizationState 									= &rs;
				pipeline.pColorBlendState 										= &cb;
				pipeline.pTessellationState 									= NULL;
				pipeline.pMultisampleState 										= &ms;
				pipeline.pDynamicState 											= &dynamicState;
				pipeline.pViewportState 										= &vp;
				pipeline.pDepthStencilState 									= &ds;
				pipeline.pStages 												= shaderStages;
				pipeline.stageCount 											= 2;
				pipeline.renderPass 											= render_pass;
				pipeline.subpass 												= 0;

				res = vkCreateGraphicsPipelines(device, pPipelineCache, 1, &pipeline, NULL, &vkPipeline);
				assert(res == VK_SUCCESS);




				VkClearValue clear_values[2];
				clear_values[0].color.float32[0] = 0.2f;
				clear_values[0].color.float32[1] = 0.2f;
				clear_values[0].color.float32[2] = 0.2f;
				clear_values[0].color.float32[3] = 0.2f;
				clear_values[1].depthStencil.depth = 1.0f;
				clear_values[1].depthStencil.stencil = 0;

				VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
				imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
				imageAcquiredSemaphoreCreateInfo.pNext = NULL;
				imageAcquiredSemaphoreCreateInfo.flags = 0;

				res = vkCreateSemaphore(device, &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);
				assert(res == VK_SUCCESS);

				res = vkCreateSemaphore(device, &imageAcquiredSemaphoreCreateInfo, NULL, &renderSemaphore);
				assert(res == VK_SUCCESS);


				VkRenderPassBeginInfo rp_begin;
				rp_begin.sType 											= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				rp_begin.pNext 											= NULL;
				rp_begin.renderPass 									= render_pass;
				rp_begin.framebuffer 									= framebuffers[current_buffer];
				rp_begin.renderArea.offset.x 							= 0;
				rp_begin.renderArea.offset.y 							= 0;
				rp_begin.renderArea.extent.width 						= (uint32_t)width;
				rp_begin.renderArea.extent.height 						= (uint32_t)height;
				rp_begin.clearValueCount 								= 2;
				rp_begin.pClearValues 									= clear_values;

				vkCmdBeginRenderPass(command_buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);
				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
										desc_set.data(), 0, NULL);

				const VkDeviceSize offsets[1] = {0};
				vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer.buf, offsets);

				this->init_viewports();
				this->init_scissors();

				vkCmdDraw(command_buffer, 12 * 3, 1, 0, 0);
				vkCmdEndRenderPass(command_buffer);
				vkEndCommandBuffer(command_buffer);
				VkFenceCreateInfo fenceInfo;

				fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceInfo.pNext = NULL;
				fenceInfo.flags = 0;
				vkCreateFence(device, &fenceInfo, NULL, &drawFence);
			}

			void draw()
			{
				VkResult res;

				res = vkAcquireNextImageKHR(device, swap_chain, UINT64_MAX,
											imageAcquiredSemaphore, VK_NULL_HANDLE, &current_buffer);
				assert(res == VK_SUCCESS);

				VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				const VkCommandBuffer cmd_bufs[] = {command_buffer};

				VkSubmitInfo submit_info;
				submit_info.pNext                   = NULL;
				submit_info.sType                   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submit_info.waitSemaphoreCount      = 1;
				submit_info.pWaitSemaphores         = &imageAcquiredSemaphore;
				submit_info.pWaitDstStageMask       = &pipe_stage_flags;
				submit_info.commandBufferCount      = 1;
				submit_info.pCommandBuffers         = cmd_bufs;
				submit_info.signalSemaphoreCount    = 1;
				submit_info.pSignalSemaphores       = &renderSemaphore;

				res = vkQueueSubmit(graphics_queue, 1, &submit_info, drawFence);
				assert(res == VK_SUCCESS);

				do {
					res = vkWaitForFences(device, 1, &drawFence, VK_TRUE, VK_SAMPLE_COUNT_1_BIT);
				} while (res == VK_TIMEOUT);
				assert(res == VK_SUCCESS);
				vkResetFences(device, 1, &drawFence);


				VkPresentInfoKHR present;
				present.sType 				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
				present.pNext 				= NULL;
				present.swapchainCount 		= 1;
				present.pSwapchains 		= &swap_chain;
				present.pImageIndices 		= &current_buffer;
				present.pWaitSemaphores 	= NULL;
				present.waitSemaphoreCount 	= 0;
				present.pResults            = NULL;

				if (renderSemaphore != VK_NULL_HANDLE)
				{
					present.pWaitSemaphores = &renderSemaphore;
					present.waitSemaphoreCount = 1;
				}

				res = vkQueuePresentKHR(present_queue, &present);
				assert(res == VK_SUCCESS);
			}
		};
	}
}
#endif //OBSIDIAN2D_CORE_WINDOW2_H