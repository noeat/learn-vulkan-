// vulkan.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

int main1(int argc, char** argv)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkanwindow", nullptr, nullptr);

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr,
		&extensionCount, nullptr);

	std::cout << extensionCount << " extensions supported" <<
		std::endl;

	glm::mat4 matrix;
	glm::vec4 vec;
	auto test = matrix * vec;

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件





#include "vulkan/vulkan.h"
#include <functional>
#include <thread>
#include <stdexcept>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>

class HelloTriangleApp
{
public:
	HelloTriangleApp();
	void run();

private:
	void initwindow();
	void initvulkan();
	void mainloop();
	void cleanup();
	void createvulkaninstance();
	void initrequiredexterns();
	void setdebugcallback();
private:
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT callback;
	std::vector<std::string> layerNames;
	std::vector<const char*> playerNames;
	std::vector<std::string> externNames;
	std::vector<const char*> pexternNames;
};

HelloTriangleApp::HelloTriangleApp()
{
	window = NULL;
}

void HelloTriangleApp::run()
{
	initwindow();
	initvulkan();
	mainloop();
	cleanup();
}

void HelloTriangleApp::initwindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
}

void HelloTriangleApp::initvulkan()
{
	createvulkaninstance();
}

void HelloTriangleApp::initrequiredexterns()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExterns = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (uint32_t i = 0; i < glfwExtensionCount; ++i)
	{
		externNames.push_back(glfwExterns[i]);
	}

	for (auto& item : externNames)
	{
		pexternNames.push_back(item.c_str());
	}

	pexternNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage
		<< std::endl;

	return VK_FALSE;
}

void HelloTriangleApp::setdebugcallback()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = (void*)this; // Optional

	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance,
			"vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr) 
	{
		func(instance, &createInfo, nullptr, &callback);
	}
	else 
	{
		std::cerr << "vkCreateDebugUtilsMessengerEXT null" << std::endl;
	}
	
}

void HelloTriangleApp::createvulkaninstance()
{
	VkApplicationInfo app = {};
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pApplicationName = "VulkanApp";
	app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app.pEngineName = "None";
	app.engineVersion = VK_MAKE_VERSION(0, 0, 0);
	app.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createinfo = {};
	createinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createinfo.pApplicationInfo = &app;

	uint32_t glfwExternCount = 0;
	const char** glExterns = NULL;

	initrequiredexterns();
	createinfo.enabledExtensionCount = pexternNames.size();
	createinfo.ppEnabledExtensionNames = pexternNames.data();

	std::cout << "glfwExterns: " << std::endl;
	for (uint32_t i = 0; i < glfwExternCount; ++i)
	{
		std::cout << glExterns[i] << std::endl;
	}

	uint32_t vkExnterCount = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &vkExnterCount, NULL);

	std::vector<VkExtensionProperties> properties(vkExnterCount);
	vkEnumerateInstanceExtensionProperties(NULL, &vkExnterCount, properties.data());
	
	std::cout << "vkextern:" << std::endl;
	for (auto& item : properties)
	{
		std::cout << item.extensionName << std::endl;
	}

	createinfo.enabledLayerCount = 0;

	std::vector<VkLayerProperties> layers;
	vkEnumerateInstanceLayerProperties(&createinfo.enabledLayerCount, NULL);
	layers.resize(createinfo.enabledLayerCount);
	vkEnumerateInstanceLayerProperties(&createinfo.enabledLayerCount, layers.data());
	for (auto& item : layers)
	{
		layerNames.push_back(std::string(item.layerName));
	}

	/*for (auto& ii : layerNames)
	{
		playerNames.push_back(ii.c_str());
	}*/
	playerNames.push_back("VK_LAYER_LUNARG_standard_validation");
	createinfo.enabledLayerCount = playerNames.size();
	createinfo.ppEnabledLayerNames = playerNames.data();
	std::cout << "vklayername:" << std::endl;
	for (auto& item : layerNames)
	{
		std::cout << item << std::endl;
	}

	VkResult result = vkCreateInstance(&createinfo, NULL, &instance);
	if (result != VK_SUCCESS)
	{
		std::cout << "create instance failed, " << result << std::endl;
	}

	setdebugcallback();
}

void HelloTriangleApp::mainloop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
}

void HelloTriangleApp::cleanup()
{
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance,
			"vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr) 
	{
		func(instance, callback, nullptr);
	}

	vkDestroyInstance(instance, NULL);

	glfwDestroyWindow(window);

	glfwTerminate();
}

int main(int argc, char **argv)
{
	HelloTriangleApp app;
	app.run();
	return 0;
}