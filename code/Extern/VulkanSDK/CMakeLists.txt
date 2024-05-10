cmake_minimum_required(VERSION 3.16)

add_library(GLM INTERFACE)

target_include_directories(GLM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/1.3.275.0/include/glm)

#------------------------------------------------------------------------------------------------

add_library(VulkanInclude INTERFACE)
add_library(VulkanLib INTERFACE)

target_include_directories(VulkanInclude INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/1.3.275.0/include)

target_link_directories(VulkanLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/1.3.275.0/lib/win64)
target_link_libraries(VulkanLib INTERFACE vulkan-1)

#------------------------------------------------------------------------------------------------

add_library(VulkanMemoryAllocator INTERFACE)

target_include_directories(VulkanMemoryAllocator INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/1.3.275.0/include/vma)
