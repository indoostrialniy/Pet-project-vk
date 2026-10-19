#include "fhRender.h"

#include "macros.h"











VkShaderModule Graphics::createShaderModule(const std::vector<char>& code)	// создать из присланного байт-кода шейдерный модуль
{
	VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();										// количество байт кода
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());		// сам байт-код

	VkShaderModule shaderModule;	// дескриптор шейдерного модуля, пока пустой
	
	VkResult result = vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule);
	if ( result != VK_SUCCESS) 
	{
		checkResult(result);
		throw std::runtime_error("failed to create shader module!");
	}
	
	// The buffer with the code can be freed immediately after creating the shader module

	return shaderModule;			// вернем готовый шейдерный модуль
}



