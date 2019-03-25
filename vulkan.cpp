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
#include <set>

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
	void pickphysicdevice();
	void createlogicdevice();
	int32_t queryQueueFamily(VkPhysicalDevice device);
	int32_t querypresendfamily(VkPhysicalDevice device);
	void createsurface();
	bool checkdeviceextensionsupport(VkPhysicalDevice device);
private:
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT callback;
	std::vector<std::string> layerNames;
	std::vector<const char*> playerNames;
	std::vector<std::string> externNames;
	std::vector<const char*> pexternNames;
	VkPhysicalDevice physicdevice;
	VkDevice logicdevice;
	VkQueue logicgraphicsQueue;
	VkQueue logicpresendQueue;
	float logicqueuePriority;
	VkSurfaceKHR surface;
};

HelloTriangleApp::HelloTriangleApp()
{
	window = NULL;
	logicqueuePriority = 1.0f;
	surface = VK_NULL_HANDLE;
	logicpresendQueue = VK_NULL_HANDLE;
	logicgraphicsQueue = VK_NULL_HANDLE;
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
	pickphysicdevice();
	createsurface();
	createlogicdevice();
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
	vkDestroyDevice(logicdevice, nullptr);

	vkDestroySurfaceKHR(instance, surface, NULL);

	vkDestroyInstance(instance, NULL);

	glfwDestroyWindow(window);

	glfwTerminate();
}

void HelloTriangleApp::pickphysicdevice()
{
	uint32_t devicecount = 0;
	vkEnumeratePhysicalDevices(instance, &devicecount, NULL);

	std::vector<VkPhysicalDevice> devices(devicecount);
	vkEnumeratePhysicalDevices(instance, &devicecount, devices.data());

	std::cout << "physic device: " << std::endl;
	int rate = 0;
	VkPhysicalDevice device = VK_NULL_HANDLE;

	for (auto& item : devices)
	{
		int score = 0;
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(item, &deviceProperties);
		
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(item, &features);

		std::cout << deviceProperties.deviceName << " " << deviceProperties.vendorID << std::endl;
		if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}

		score += deviceProperties.limits.maxImageDimension2D;
		
		if (!features.geometryShader)
		{
			continue;
		}

		if (queryQueueFamily(item) == -1)
		{
			continue;
		}

		if (score > rate)
		{
			rate = score;
			device = item;
		}

		std::cout << "rate: " << rate << std::endl;
		physicdevice = device;
		assert(device != VK_NULL_HANDLE);
	}
}

int32_t HelloTriangleApp::queryQueueFamily(VkPhysicalDevice device)
{
	uint32_t familycount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &familycount, NULL);

	std::vector<VkQueueFamilyProperties> properties(familycount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &familycount, properties.data());

	int32_t ret = -1;
	int32_t index = -1;
	for (auto& item : properties)
	{
		++index;
		if (item.queueCount > 0 && (item.queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			ret = index;
			break;
		}
	}

	return ret;
}

int32_t HelloTriangleApp::querypresendfamily(VkPhysicalDevice device)
{
	uint32_t familycount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &familycount, NULL);

	std::vector<VkQueueFamilyProperties> properties(familycount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &familycount, properties.data());

	int32_t ret = -1;
	int32_t index = -1;
	for (auto& item : properties)
	{
		++index;
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &presentSupport);
		if (item.queueCount > 0 && presentSupport)
		{
			ret = index;
			break;
		}
	}

	return ret ;
}

void HelloTriangleApp::createlogicdevice()
{
	assert(physicdevice != VK_NULL_HANDLE);

	std::vector<VkDeviceQueueCreateInfo> queuecreateinfos;
	VkDeviceQueueCreateInfo queuecreateinfo = {};
	queuecreateinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queuecreateinfo.queueFamilyIndex = queryQueueFamily(physicdevice);
	queuecreateinfo.pQueuePriorities = &logicqueuePriority;
	queuecreateinfo.queueCount = 1;
	queuecreateinfos.push_back(queuecreateinfo);

	queuecreateinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queuecreateinfo.queueFamilyIndex = querypresendfamily(physicdevice);
	queuecreateinfo.pQueuePriorities = &logicqueuePriority;
	queuecreateinfo.queueCount = 1;
	queuecreateinfos.push_back(queuecreateinfo);


	VkPhysicalDeviceFeatures features = {};

	std::vector<const char*> validextensionnames;
	validextensionnames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	VkDeviceCreateInfo createinfo = {};
	createinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createinfo.pQueueCreateInfos = queuecreateinfos.data();
	createinfo.queueCreateInfoCount = queuecreateinfos.size();
	createinfo.pEnabledFeatures = &features;
	createinfo.enabledExtensionCount = 0;
	std::vector<const char*> layers;
	playerNames.push_back("VK_LAYER_LUNARG_standard_validation");
	createinfo.enabledLayerCount = layers.size();
	createinfo.ppEnabledLayerNames = layers.data();

	VkResult ret = vkCreateDevice(physicdevice, &createinfo, NULL, &logicdevice);
	if (ret != VK_SUCCESS)
	{
		std::cout << "create logic device failed. " << ret << std::endl;
		return;
	}

	vkGetDeviceQueue(logicdevice, queryQueueFamily(physicdevice), 0, &logicgraphicsQueue);
	vkGetDeviceQueue(logicdevice, querypresendfamily(physicdevice), 0, &logicpresendQueue);
}

void HelloTriangleApp::createsurface()
{
	VkResult ret = glfwCreateWindowSurface(instance, window, nullptr, &surface);
	if (ret != VK_SUCCESS)
	{
		std::cout << "create surface failed. " << ret << std::endl;
	}
}

bool HelloTriangleApp::checkdeviceextensionsupport(VkPhysicalDevice device)
{
	std::vector<const char*> validextensionnames;
	validextensionnames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	uint32_t extensioncount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensioncount, nullptr);

	std::vector< VkExtensionProperties> properties(extensioncount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensioncount, properties.data());

	for (auto s : validextensionnames)
	{
		bool r = false;
		for (auto item : properties)
		{
			if (strcmp(item.extensionName, s) == 0)
			{
				r = true;
				break;
			}
		}

		if (!r)
		{
			return false;
		}
	}

	return true;
}

int main(int argc, char **argv)
{
	HelloTriangleApp app;
	app.run();
	return 0;
}