cmake_minimum_required(VERSION 3.16)

# Interface Libraries

add_subdirectory(rapidjson)
add_subdirectory(tinygltf)
add_subdirectory(VulkanSDK)

# Dependency Libraries

add_subdirectory(glfw)
set_target_properties(GLFW PROPERTIES FOLDER "Extern")
add_subdirectory(imgui)
set_target_properties(ImGui PROPERTIES FOLDER "Extern")
add_subdirectory(neat)
set_target_properties(NEAT PROPERTIES FOLDER "Extern")
