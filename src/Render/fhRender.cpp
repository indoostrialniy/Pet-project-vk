#include "fhRender.h"

#include "macros.h"

#include "Scena.h" // MicroWorld, Camera
#include "Camera.h"
#include "Laboratory.h"

#include "Entity.h"
#include "Mesh.h"
#include "MovingPartsController.h"
#include "DynamicLightSource.h"

#include <glm/glm.hpp>
#include "Weapon.h"

//~ Game::Game()
//~ {
	
//~ }


void Graphics::run() 
{
	initVulkan();
	
	
}

	

void Graphics::initVulkan() 
{
	// fhRender_1.cpp
	createInstance();
	createSurface();				// Our engine needs to be able to draw to screen, so we create "Surface" BEFORE picking physical device, 
									// because physical device must be able to draw to this window via this "Surface".
	pickPhysicalDevice();			// No need to manually destroy!
	createLogicalDevice();
			
			
	// fhRender_2.cpp	
	createSwapChain();				// create Swapchain
	getSwapchainImages();			// get VkImage handles
	createSwapchainImageViews();	// wrap this handles in VkImageView. This will be Color attachment for Framebuffer
	// Создание VkImageView позволяет начать воспринимать изображения VkImage из Swapchain как текстуры, но их еще нельзя использовать как RenderTarget.
	
	
	// fhRender_4.cpp
	createCommandPool();
	createCommandBuffer();
	createSyncObjects();
	
	
	Meshes.init(*this);

	VkTextures.init(*this);

	Materials.init(*this);
	
	// vulkan inited!
	
	createGpass();

	createLpassDynamicLightSourceInfoBuffer();
	
	
	//~ printMessage("Vulkan inited")
}



void Graphics::spawns()	// calls from main.cpp
{
	// XXX тупо создали экземпляры классов
	//~ g_Camera = std::make_shared<Camera>("Camera");
	//~ g_SceneMicromir = std::make_shared<Scena>("Scena");	// create a level in heap
	
	g_Lab = std::make_shared<Laboratory>("Oasis", *this); // здесь грузятся акторы и сущности, которым нужен G-renderpass!
	
	if( CHECK_PTR(g_Lab) )
	{
		g_Camera = std::dynamic_pointer_cast<Camera>(g_Lab->getActor("Camera"));
		//g_SceneMicromir = std::dynamic_pointer_cast<Scena>(g_Lab->getActor("Scena"));

	}
	
	//~ printMessage("Spawns ok")
	
	//~ if( CHECK_PTR(g_SceneMicromir) )
	//~ {
		//~ printMessage("\n\nFrom Render:\n")
		
		//1 entity
		//~ FhEntityInitializationInfo ent1;
			//~ ent1.initSysname = "SR3";
			//~ ent1.textureName = "sr3m_1_albedo.jpg";
			//~ ent1.initMeshName = MODEL_PATH+"SR3.obj";
			//~ ent1.initPos = glm::vec3(1,0,0);
			//~ ent1.initRot = glm::quat(0.707,0,0,0.707);
			
		//~ g_SceneMicromir->addEntity( ent1 );	//g_SceneMicromir->addEntity("SR3", "sr3m_1_albedo.jpg", MODEL_PATH+"SR3.obj");
		
		
		// 2 entity	
		////g_SceneMicromir->addEntity("room", "bricks-mortar-albedo.png", MODEL_PATH+"room.obj");
		//~ FhEntityInitializationInfo ent2;
			//~ ent2.initSysname = "room";
			//~ ent2.textureName = "bricks-mortar-albedo.png";
			//~ ent2.initMeshName = MODEL_PATH+"room.obj";
			//~ ent2.initPos = glm::vec3(0,0,-2);
			//~ ent2.initRot = glm::quat(1,0,0,0);
			
		//~ g_SceneMicromir->addEntity( ent2 );
				
	//~ }
	
}


void Graphics::createDeferredPipeline()
{
	
	

	
	createFullscreenQuad();
	
	createLpass();
	
	createFpass();
	
	
	
	// create lights uniform buffers
	//createLightsUniformBuffers(); // XXX
}

bool flagG = false;
bool flagR = false;

void Graphics::mainLoop() 
{
	while (!glfwWindowShouldClose(window)) 
	{
		glfwPollEvents();
		
		
		//updateLightsUniformBuffer();
		
		drawFrame();
		
		//~ printMessage("mainLoop")
		//~ system("pause");
		
		if( g_Key[GLFW_KEY_ESCAPE] ) {break;}
		
		if( g_Key[GLFW_KEY_E] && flagE)
		{
			flagE = false;
			printMessage("E Pressed!")
			
			if(CHECK_PTR(g_Lab))
			{
				//~ auto ScenaActor = g_Lab->getActor("Scena");
				if(CHECK_PTR(g_Lab->getActor("SR3")))
				{
					std::shared_ptr<Weapon> DC = std::dynamic_pointer_cast<Weapon>(g_Lab->getActor("SR3"));
					if( CHECK_PTR(DC) )
					{
						//DC->wpnShoot();		//moving();
					}
				}
			}
						
			UPD = true;			
		}
		if( !g_Key[GLFW_KEY_E] && !flagE)
		{
			flagE = true;
		}
		
		//-------------
		if( g_Key[GLFW_KEY_G] && flagG)
		{
			flagG = false;
			printMessage("G Pressed!")
			
			if(CHECK_PTR(g_Lab))
			{
				//~ auto ScenaActor = g_Lab->getActor("Scena");
				if(CHECK_PTR(g_Lab->getActor("SR3")))
				{
					std::shared_ptr<Weapon> DC = std::dynamic_pointer_cast<Weapon>(g_Lab->getActor("SR3"));
					if( CHECK_PTR(DC) )
					{
						//DC->wpnChangePriklad();	
					}
				}
			}
			
			UPD = true;			
		}
		if( !g_Key[GLFW_KEY_G] && !flagG)
		{
			flagG = true;
		}
		
		//-------------
		if( g_Key[GLFW_KEY_R] && flagR)
		{
			flagR = false;
			printMessage("R Pressed!")
			
			if(CHECK_PTR(g_Lab))
			{
				//~ auto ScenaActor = g_Lab->getActor("Scena");
				if(CHECK_PTR(g_Lab->getActor("SR3")))
				{
					std::shared_ptr<Weapon> DC = std::dynamic_pointer_cast<Weapon>(g_Lab->getActor("SR3"));
					if( CHECK_PTR(DC) )
					{
						//DC->wpnReload();	
					}
				}
			}
			
			UPD = true;			
		}
		if( !g_Key[GLFW_KEY_R] && !flagR)
		{
			flagR = true;
		}
		
		
		//~ system("pause");
		
	}
	vkDeviceWaitIdle(logicalDevice);
}


void Graphics::cleanup() 
{
	
	Meshes.clear();
	VkTextures.clear();
	Materials.clear();
	
	
	//~ std::shared_ptr<VkTexture> probka;
	//for(int i=0; i<2; i++)
	//{
		// вручную удаляю текстуры
		//~ if( CHECK_PTR(gameObjects[i]) ) gameObjects[i]->TEXTURE.reset();// = probka;
		//~ if(CHECK_PTR(g_SceneMicromir))
		//~ {
			//~ for(auto ent : g_SceneMicromir->components)
			//~ {
				//~ ent.second->TEXTURE.reset();
			//~ }
		//~ }
	//}
	destroyFpass();
	
	destroyLpass();
	
	destroyFullscreenQuad();
	destroyGpass();
	
	//destroyLightsUniformBuffers();
	
	//~ printMessage("\n\nCLEANUP APP 1")
	
	// -- Vulkan
	destroySyncObjects();
	
	//~ printMessage("\n\nCLEANUP APP 2")
	
	cleanupSwapChain();
	
	//~ if(CHECK_PTR(g_SceneMicromir))
	//~ {
		//~ for(auto ent : g_SceneMicromir->components)
		//~ {
			// ручное удаление ресурсов, хранящихся в Entity
			//~ Graphics::destroyVertexBuffer( ent.second->vertexBuffer, ent.second->vertexBufferMemory);	
		//~ }
	//~ }
	//~ if(CHECK_PTR(gameObjects[1])) destroyVertexBuffer(gameObjects[1]->vertexBuffer, gameObjects[1]->vertexBufferMemory);
	//~ if(CHECK_PTR(gameObjects[0])) destroyVertexBuffer(gameObjects[0]->vertexBuffer, gameObjects[0]->vertexBufferMemory);
	
	//~ printMessage("\n\nCLEANUP APP 3")
	
	//~ destroyDescriptorPool();
	
	
	//delete g_Lab?
	g_Lab.reset();
	
	//~ if(CHECK_PTR(g_SceneMicromir))
	//~ {
		//int i=0;
		//~ for(auto ent : g_SceneMicromir->components)
		//~ {
			//destroyUniformBuffers(ent.first); // перенесенно в ~Entity
			//i++;
		//~ }
	//~ }
	//~ if(CHECK_PTR(gameObjects[1])) destroyUniformBuffers(1);
	//~ if(CHECK_PTR(gameObjects[0])) destroyUniformBuffers(0);
	
	//~ destroyFramebuffers();
		//~ destroyGraphicsPipeline();
		//~ destroyGraphicsPipelineLayout();
		//~ destroyDescriptorSetLayout();
		//~ destroyRenderPass();
	//~ destroySwapchainImageViews();
	//~ destroySwapChain();
	//~ 
	
	//~ destroyTextureSampler( gameObjects[1].textureSampler );
	//~ destroyTextureImageView( gameObjects[1].textureImageView );
	//~ destroyTextureImage( gameObjects[1].textureImage, gameObjects[1].textureImageMemory);
	
	//~ destroyTextureSampler( gameObjects[0]->textureSampler );
	//~ destroyTextureImageView( gameObjects[0]->textureImageView );
	//~ destroyTextureImage( gameObjects[0]->textureImage, gameObjects[0]->textureImageMemory);
	//~ destroyTextureSampler( textureSampler );
	//~ destroyTextureImageView( textureImageView );
	//~ destroyTextureImage( textureImage, textureImageMemory);
	
	destroyCommandPool();
	
	destroyLogicalDevice(); // неявно самостоятельно удаляет и дескрипторы VkQueue graphicsQueue, presentQueue
	
	destroySurface();
	
	destroyInstance();
	
	
}




void Graphics::checkResult( VkResult result )
{
	switch(result)
	{
		case VK_NOT_READY:
			printVariable(VK_NOT_READY)
			break;
		case VK_TIMEOUT:
			printVariable(VK_TIMEOUT)
			break;
		case VK_INCOMPLETE:
			printVariable(VK_INCOMPLETE)
			break;
			
		case VK_ERROR_DEVICE_LOST:
			printVariable(VK_ERROR_DEVICE_LOST)
			break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			printVariable(VK_ERROR_OUT_OF_HOST_MEMORY)
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			printVariable(VK_ERROR_OUT_OF_DEVICE_MEMORY)
			break;
		case VK_ERROR_INITIALIZATION_FAILED:
			printVariable(VK_ERROR_INITIALIZATION_FAILED)
			break;
						
		case VK_ERROR_LAYER_NOT_PRESENT:
			printVariable(VK_ERROR_LAYER_NOT_PRESENT)
			break;
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			printVariable(VK_ERROR_EXTENSION_NOT_PRESENT)
			break;
		case VK_ERROR_FEATURE_NOT_PRESENT:
			printVariable(VK_ERROR_FEATURE_NOT_PRESENT)
			break;
		
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			printVariable(VK_ERROR_INCOMPATIBLE_DRIVER)
			break;
		case VK_ERROR_TOO_MANY_OBJECTS:
			printVariable(VK_ERROR_TOO_MANY_OBJECTS)
			break;
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			printVariable(VK_ERROR_FORMAT_NOT_SUPPORTED)
			break;
		case VK_PIPELINE_COMPILE_REQUIRED:
			printVariable(VK_PIPELINE_COMPILE_REQUIRED)
			break;
		case VK_ERROR_FRAGMENTATION:
			printVariable(VK_ERROR_FRAGMENTATION)
			break;
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			printVariable(VK_ERROR_OUT_OF_POOL_MEMORY)
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			printVariable(VK_ERROR_OUT_OF_DATE_KHR)
			break;	
		case VK_SUBOPTIMAL_KHR:
			//~ printMessage(VK_SUBOPTIMAL_KHR)
			printVariable(VK_SUBOPTIMAL_KHR)
			break;	
			
		
		default:
			break;
	}

}

	
	
	
void Graphics::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) 
{
	

}



void Graphics::drawFrame() 
{
	GLFW::updateInputs();	// update states of almost all buttons on keyboard
	
	if( CHECK_PTR(g_Camera) ) {g_Camera->updateCamera();}
	
	updateGpassGeneralInfoBuffer();
	
	updateLpassDynamicLightSourceInfoBuffer();
	
	// UPDATE ENTITIES!
	if(CHECK_PTR(g_Lab))
	{
		//~ if( CHECK_PTR(g_Lab->getActor("SR3")) )
		//~ {
			//~ float rotSpeed = 180.0f * Stopwatch::timeSinceLastFrame; // 180 degrees per second
			//~ g_Lab->getActor("SR3")->addActorWorldRotation(glm::angleAxis( glm::radians(rotSpeed), glm::vec3(0,0,1) ) );
		//~ }
		
		//printMessage("drawFrame: update all entities") 
		// вместо целевых updateUniformBuffer(currentFrame, "SR3"); // обязательно до начала обработки кадра!
		// и updateUniformBuffer(currentFrame, "room");
		for(auto act : g_Lab->actors)
		{
			//std::cout << "Actor: " << act.first << "\n";
			for(auto ent : act.second->components)
			{
				//std::cout << "\tEntity: " << ent.first << "\n";
				//ent.second->updateEntityUniformBuffer(currentFrame);	// XXX update PVM for every Entity
				//ent.second->updateEntityMaterialInfoBuffer(currentFrame);
			}
		}
		
	}
	
	
	vkWaitForFences(logicalDevice, 1, &_frames[currentFrame].inFlightFence, VK_TRUE, UINT64_MAX); // подождали разрешения - 1 fence

	// 1 - обработка команд рендеринга
	// 1.1 - запросили индекс кадра из цепочки, который доступен для взятия в работу
	uint32_t imageIndex;
	//vkAcquireNextImageKHR( device,   swapchain,  timeout,  semaphore, fence,  pImageIndex[] );
	VkResult acquire = vkAcquireNextImageKHR(logicalDevice, swapChain, UINT64_MAX, _frames[currentFrame].imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);	// запросили следующее изображение для рендера
	
	if( framebufferResized ) //acquire == VK_ERROR_OUT_OF_DATE_KHR) // || acquire == VK_SUBOPTIMAL_KHR || framebufferResized) 
	{
		framebufferResized = false;
		printMessage("Rebuild swapchain")
		recreateSwapChain();
		return;
	}
	else if (acquire != VK_SUCCESS && acquire != VK_SUBOPTIMAL_KHR) 
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	
	vkResetFences(logicalDevice, 1, &_frames[currentFrame].inFlightFence); // device, count, pFences[]
	
	// 1.2 - сбросили to make sure it is able to be recorded 
	vkResetCommandBuffer(_frames[currentFrame].commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);	//  0 - т.е. ничего специального не делаем, флаг пустой
	
	
	// 1.2.1 - перезапишем буфер команд
	// recordCommandBuffer(_frames[currentFrame].commandBuffer, imageIndex);
	{
		
		// We always begin recording a command buffer by calling vkBeginCommandBuffer
		// with a small VkCommandBufferBeginInfo structure as argument that specifies
		// some details about the usage of this specific command buffer.
		
		VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0; // Optional
			//beginInfo.pInheritanceInfo = nullptr; // Optional

		// 1 - BEGIN COMMAND BUFFER: Initialize the command buffer
		if (vkBeginCommandBuffer(_frames[currentFrame].commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		// If the command buffer was already recorded once, then a call to vkBeginCommandBuffer
		// will implicitly reset it. It’s not possible to append commands to a buffer at a
		// later time.
		
		//~ printVarComment("Frame: ", currentFrame)
		
		// G-pass
		{
			// 2 - рендерпас
			VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = renderPass_G;							// the render pass itself and the attachments to bind			
				renderPassInfo.framebuffer = frameBuffer_G;	// привязали к рендерпасу один из ранее созданных framebuffer кадров из цепочки, в который хотим рендерить
				renderPassInfo.renderArea.offset = {0, 0};
				renderPassInfo.renderArea.extent = swapChainExtent;
				
			std::array<VkClearValue, gPassAttachmentsCount> clearValues{};
				clearValues[G_PASS_COLOR_ATTACHMENT].color = {{0.96, 0.46, 0.039, 1.0}};	// цвет фона - огненный, непрозрачность 1 (100%)
				clearValues[G_PASS_NORMAL_ATTACHMENT].color = {{0.46, 0.96, 0.039, 1.0}};	// цвет фона - салатовый, непрозрачность 1 (100%)
				clearValues[G_PASS_POSITION_ATTACHMENT].color = {{0.46, 0.039, 0.96, 1.0}};	// цвет фона - "нормальный", непрозрачность 1 (100%)
				clearValues[G_PASS_METAL_ROUGHNESS_PBR_ATTACHMENT].color = {{0.46, 0.039, 0.96, 1.0}};	// цвет фона - "нормальный", непрозрачность 1 (100%)
				clearValues[G_PASS_EMISSION_ATTACHMENT].color = {{0.0, 0.0, 0.0, 1.0}};	// цвет фона для излучения - абсолютно черный, непрозрачность 1 (100%)
				clearValues[G_PASS_DEPTH_ATTACHMENT].depthStencil = {1.0f, 0};
				
				
				renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
				renderPassInfo.pClearValues = clearValues.data();
				

			// 3 - привязка рендерпаса Start a new renderpass with the image index from swapchain as target to render onto
			vkCmdBeginRenderPass(_frames[currentFrame].commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
				// 3.1 - привязка графического (!!!) конвейера
				vkCmdBindPipeline(_frames[currentFrame].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_G); // Bind the shaders and configuration used to render the object

				// 3.2 ВЬЮПОРТ: т.к. ранее выбрали динамическое состояние для viewport и scissors, прописываем их...
				VkViewport viewport{};
					viewport.x = 0.0f;
					viewport.y = 0.0f;
					viewport.width = (float) swapChainExtent.width;
					viewport.height = (float) swapChainExtent.height;
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;
				vkCmdSetViewport(_frames[currentFrame].commandBuffer, 0, 1, &viewport);		// ... и передаем в шейдер

				VkRect2D scissor{ };
					scissor.offset = {0, 0};
					scissor.extent = swapChainExtent;
				vkCmdSetScissor(_frames[currentFrame].commandBuffer, 0, 1, &scissor);			// ... и передаем в шейдер

				// Внутри render pass'а может идти серия операций:
				// for (auto object : objects) {}

				// отрисовка 1 объекта
				//~ updateUniformBuffer(currentFrame, 0); // обязательно до начала обработки кадра!
				// 3.3 привязываем VBO
				// Параметр массива VkBuffer в команде vkCmdBindVertexBuffers() нужен для поддержки сложных моделей вершинных данных, когда каждая такая часть хранится в отдельном буфере: VkBuffer vertexBuffers[] = { vertexBuffer, colorBuffer, uvBuffer };
				//VkBuffer vertexBuffers[] = { gameObjects[0]->vertexBuffer /*colorBuffer, uvBuffer*/ };
				VkDeviceSize offsets[] = {0 /*,0,0*/ }; // массив отступов по соответствующим буферам: vertex, color, uv etc
				//#define offsets 0
				
				FhGpassPushConstantsInfo constants;			// PUSH CONSTANTS
					//constants.data = glm::vec4( 1,1,0,1);
				glm::mat4 modelMatrix = glm::mat4(1.0f);	// матрица модели сущности
				
				// DRAW ENTITIES
				if( CHECK_PTR(g_Lab) )//g_SceneMicromir)
				{
					for(auto act : g_Lab->actors)
					{
						//int i=0;
						for(auto ent : act.second->components)	//int i=0; i<2; i++) g_SceneMicromir
						{
							
							//~ printVarComment("recordCommandBuffer ", ent.first)
							if( CHECK_PTR(ent.second->entityMeshPtr) )
							{
								vkCmdBindVertexBuffers(_frames[currentFrame].commandBuffer, 0, 1, &ent.second->entityMeshPtr->getMeshVertexBuffer(), offsets);	
								//~ printMessage("recordCommandBuffer Flag 0")
					
								vkCmdBindDescriptorSets(_frames[currentFrame].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_G, 0, 1, &ent.second->GpassDescriptorSet, 0, nullptr);	// привязали дексриптор, где хранятся ссылки на наши матрицы PVM и текстуры
								
								//~ printMessage("recordCommandBuffer Flag 1")
								
								//~ if(i==0) g_SceneMicromir->configureModelMatrix(modelMatrix, glm::vec3(2,0,0), glm::quat(1,0,0,0) );
								//~ if(i==1) g_SceneMicromir->configureModelMatrix(modelMatrix, glm::vec3(0,0,0), glm::quat(1,0,0,0) );
								
								ent.second->configureModelMatrix( modelMatrix );
									
								constants.model = ent.second->entityMatrix;	//modelMatrix;
								
								//constants.data[0] = 
								
								//~ if( CHECK_PTR(ent.second->entityMaterialPtr) )
								//{
									//constants.data[1] = 1.0f;	//ent.second->entityMaterialPtr->getMatInfo()[2][0];// UV scale
								//}
								
								//~ if( ent.second->entityType == 1 ) // TRANSPARENT
								//~ {
									constants.type = ent.second->entityType;
									constants.reserve1 = 0;
									constants.reserve2 = 0.0f;
									constants.reserve3 = 0.0f;
								//~ }
								//~ printMessage("recordCommandBuffer Flag 2")
								//~ printMat4("entity ", constants.model ) 
									
								vkCmdPushConstants(_frames[currentFrame].commandBuffer, pipelineLayout_G, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(FhGpassPushConstantsInfo), &constants);	//upload the matrix to the GPU via push constants
								vkCmdDraw(_frames[currentFrame].commandBuffer, static_cast<uint32_t>(ent.second->entityMeshPtr->getVerticesCount()), 1, 0, 0);	//vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertexOffset, firstInstanceOffset);
								
								//~ printVarComment("recordCommandBuffer end ", ent.first)
								//i++;
							}   
							
						}
						
					}
				}
				
				

			// 3.4 - Finalize the render pass and command buffer
			vkCmdEndRenderPass(_frames[currentFrame].commandBuffer);
		}

		//~ printMessage("G_pass OK")

		// Shadow-pass
		{
			// get all dynamic light source actors and call their func
			
			std::shared_ptr<DynamicLight> spot = dynamic_pointer_cast<DynamicLight>(g_Lab->getActor("LabSpot"));
			if( spot != nullptr )
			{
				spot->recordShadowpassCommand( _frames[currentFrame].commandBuffer );
			}
			
			std::shared_ptr<DynamicLight> helispot = dynamic_pointer_cast<DynamicLight>(g_Lab->getActor("HeliSpot"));
			if( helispot != nullptr )
			{
				helispot->recordShadowpassCommand( _frames[currentFrame].commandBuffer );
			}
			
		}
		//~ printMessage("Shadow_pass OK")


		//#ifdef L_PASS
		// L-pass
		{
			// 2 - рендерпас
			VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = renderPass_L;							// the render pass itself and the attachments to bind			
				renderPassInfo.framebuffer = frameBuffer_L;	// привязали к рендерпасу один из ранее созданных framebuffer кадров из цепочки, в который хотим рендерить
				renderPassInfo.renderArea.offset = {0, 0};
				renderPassInfo.renderArea.extent = swapChainExtent;
				
			std::array<VkClearValue, gPassAttachmentsCount-1> clearValues{};
				for(int i=0; i<gPassAttachmentsCount-1; i++)
				{
					clearValues[i].color = {{0.96, 0.46, 0.96, 1.0}};	// цвет фона - , непрозрачность 1 (100%)
				}
			
				renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
				renderPassInfo.pClearValues = clearValues.data();
				

			// 3 - привязка рендерпаса Start a new renderpass with the image index from swapchain as target to render onto
			vkCmdBeginRenderPass(_frames[currentFrame].commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
				// 3.1 - привязка графического (!!!) конвейера
				vkCmdBindPipeline(_frames[currentFrame].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_L); // Bind the shaders and configuration used to render the object

				// 3.2 ВЬЮПОРТ: т.к. ранее выбрали динамическое состояние для viewport и scissors, прописываем их...
				VkViewport viewport{};
					viewport.x = 0.0f;
					viewport.y = 0.0f;
					viewport.width = (float) swapChainExtent.width;
					viewport.height = (float) swapChainExtent.height;
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;
				vkCmdSetViewport(_frames[currentFrame].commandBuffer, 0, 1, &viewport);		// ... и передаем в шейдер

				VkRect2D scissor{ };
					scissor.offset = {0, 0};
					scissor.extent = swapChainExtent;
				vkCmdSetScissor(_frames[currentFrame].commandBuffer, 0, 1, &scissor);			// ... и передаем в шейдер

				// Внутри render pass'а может идти серия операций:
				// for (auto object : objects) {}

				// отрисовка 1 объекта
				//~ updateUniformBuffer(currentFrame, 0); // обязательно до начала обработки кадра!
				// 3.3 привязываем VBO
				// Параметр массива VkBuffer в команде vkCmdBindVertexBuffers() нужен для поддержки сложных моделей вершинных данных, когда каждая такая часть хранится в отдельном буфере: VkBuffer vertexBuffers[] = { vertexBuffer, colorBuffer, uvBuffer };
				//VkBuffer vertexBuffers[] = { gameObjects[0]->vertexBuffer /*colorBuffer, uvBuffer*/ };
				VkDeviceSize offsets[] = {0 /*,0,0*/ }; // массив отступов по соответствующим буферам: vertex, color, uv etc
				//#define offsets 0
				
				//FhGpassPushConstantsInfo constants;			// PUSH CONSTANTS
					//constants.data = glm::vec4( 1,1,0,1);
				//glm::mat4 modelMatrix = glm::mat4(1.0f);	// матрица модели сущности
				
				
				vkCmdBindVertexBuffers(_frames[currentFrame].commandBuffer, 0, 1, &fullscreenQuadVertexBuffer, offsets);	

				vkCmdBindDescriptorSets(_frames[currentFrame].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_L, 0, 1, &descriptorSet_L, 0, nullptr);	// привязали дексриптор
					
				//vkCmdPushConstants(_frames[currentFrame].commandBuffer, pipelineLayout_G, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(FhGpassPushConstantsInfo), &constants);	//upload the matrix to the GPU via push constants
				int vertexCount = fullscreenQuadVertexDataVector.size();
				vkCmdDraw(_frames[currentFrame].commandBuffer, vertexCount, 1, 0, 0);	//vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertexOffset, firstInstanceOffset);
		  
			// 3.4 - Finalize the render pass and command buffer
			vkCmdEndRenderPass(_frames[currentFrame].commandBuffer);
		}
		//#endif
	
		//~ printMessage("L_pass OK")

		// F-pass
		{
			// 2 - рендерпас
			VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = renderPass_F;							// the render pass itself and the attachments to bind			
				renderPassInfo.framebuffer = frameBuffer_F[imageIndex];	// привязали к рендерпасу один из ранее созданных framebuffer кадров из цепочки, в который хотим рендерить
				renderPassInfo.renderArea.offset = {0, 0};
				renderPassInfo.renderArea.extent = swapChainExtent;
				
			std::array<VkClearValue, 1> clearValues{};
				clearValues[0].color = {{0.96, 0.46, 0.96, 1.0}};	// цвет фона - , непрозрачность 1 (100%)
				
				renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
				renderPassInfo.pClearValues = clearValues.data();
				

			// 3 - привязка рендерпаса Start a new renderpass with the image index from swapchain as target to render onto
			vkCmdBeginRenderPass(_frames[currentFrame].commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
				// 3.1 - привязка графического (!!!) конвейера
				vkCmdBindPipeline(_frames[currentFrame].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_F); // Bind the shaders and configuration used to render the object

				// 3.2 ВЬЮПОРТ: т.к. ранее выбрали динамическое состояние для viewport и scissors, прописываем их...
				VkViewport viewport{};
					viewport.x = 0.0f;
					viewport.y = 0.0f;
					viewport.width = (float) swapChainExtent.width;
					viewport.height = (float) swapChainExtent.height;
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;
				vkCmdSetViewport(_frames[currentFrame].commandBuffer, 0, 1, &viewport);		// ... и передаем в шейдер

				VkRect2D scissor{ };
					scissor.offset = {0, 0};
					scissor.extent = swapChainExtent;
				vkCmdSetScissor(_frames[currentFrame].commandBuffer, 0, 1, &scissor);			// ... и передаем в шейдер

				// Внутри render pass'а может идти серия операций:
				// for (auto object : objects) {}

				// отрисовка 1 объекта
				//~ updateUniformBuffer(currentFrame, 0); // обязательно до начала обработки кадра!
				// 3.3 привязываем VBO
				// Параметр массива VkBuffer в команде vkCmdBindVertexBuffers() нужен для поддержки сложных моделей вершинных данных, когда каждая такая часть хранится в отдельном буфере: VkBuffer vertexBuffers[] = { vertexBuffer, colorBuffer, uvBuffer };
				//VkBuffer vertexBuffers[] = { gameObjects[0]->vertexBuffer /*colorBuffer, uvBuffer*/ };
				VkDeviceSize offsets[] = {0 /*,0,0*/ }; // массив отступов по соответствующим буферам: vertex, color, uv etc
				//#define offsets 0
				
				//FhGpassPushConstantsInfo constants;			// PUSH CONSTANTS
					//constants.data = glm::vec4( 1,1,0,1);
				//glm::mat4 modelMatrix = glm::mat4(1.0f);	// матрица модели сущности
				
				
				vkCmdBindVertexBuffers(_frames[currentFrame].commandBuffer, 0, 1, &fullscreenQuadVertexBuffer, offsets);	

				vkCmdBindDescriptorSets(_frames[currentFrame].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_F, 0, 1, &descriptorSet_F[currentFrame], 0, nullptr);	// привязали дексриптор
					
				//vkCmdPushConstants(_frames[currentFrame].commandBuffer, pipelineLayout_G, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(FhGpassPushConstantsInfo), &constants);	//upload the matrix to the GPU via push constants
				int vertexCount = fullscreenQuadVertexDataVector.size();
				vkCmdDraw(_frames[currentFrame].commandBuffer, vertexCount, 1, 0, 0);	//vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertexOffset, firstInstanceOffset);
		  
			// 3.4 - Finalize the render pass and command buffer
			vkCmdEndRenderPass(_frames[currentFrame].commandBuffer);
		}

		//~ printMessage("F_pass OK")

		// 4 - END COMMAND BUFFER:
		if (vkEndCommandBuffer(_frames[currentFrame].commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
	
	

	
	// 1.3 - семафоров может быть множество, поэтому представляем их как массивы
	VkSemaphore waitSemaphores[] = {_frames[currentFrame].imageAvailableSemaphore};
	VkSemaphore signalSemaphores[] = {_frames[currentFrame].renderFinishedSemaphore};
	
	// 1.4 - указание этапов графического конвейера, 
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; // ждать синхронизации на этапе записи цветового прикрепления
	
	// 1.5 - информация о передаваемом на исполнение буфере
	VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;							// ожидающих семафоров
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;					//
		// Each entry in the waitStages array corresponds to the semaphore with the same index in pWaitSemaphores.
		submitInfo.signalSemaphoreCount = 1;						// сигнализирующих семафоров
		submitInfo.pSignalSemaphores = signalSemaphores;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &_frames[currentFrame].commandBuffer;
		
	// все вызовы vkQueueSubmit() делают возврат значения немедленно (хотя исполнение на GPU занимает время),
	// а ожидание происходит непосредственно на GPU - CPU продолжает крутиться дальше.
	// XXX это как бендикс от стартера? в одну сторону крутится, а обратно нет? типа если CPU предоставит новые данные до того, как карта отрисует,
	// то следующим кадром пойдут уже новые данные? То, что GPU не успел отрисовать в этот промежуток, уже удалится? Ведь оно уже и не актуально!
			
	// 1.6 - отправляем буфер команд в конкретную очередь на исполнение с указанием выставить флаг inFlightFence по завершении
	VkResult resSubmit = vkQueueSubmit(graphicsQueue, 1, &submitInfo, _frames[currentFrame].inFlightFence);
	if ( resSubmit != VK_SUCCESS) 
	{
		checkResult(resSubmit);
		DEBUG_INFO
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	// 2 - демонстрация
	// 2.1 - цепочек тоже может быть много (допускается же мультирендеринг), поэтому представляем их как массивы
	VkSwapchainKHR swapChains[] = {swapChain};
	
	// 2.2 - информация о способе отображения
	VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;							// ожидающих семафоров
		presentInfo.pWaitSemaphores = signalSemaphores;		// signalSemaphores выставится после исполнения буфера команд. Только после этого можно отправлять на демонстрацию
		presentInfo.swapchainCount = 1;								// цепочек
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

	// 2.3 - передача на отображение. Display the image we just rendered on the screen. presentInfo has semaphores, which make sure the image isn't presented until `cmd` is finished executing
	vkQueuePresentKHR(presentQueue, &presentInfo); 
	
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}



