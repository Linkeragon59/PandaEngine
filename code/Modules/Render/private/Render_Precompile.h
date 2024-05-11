#pragma once

#include "Core_Defines.h"
#include "Core_Assert.h"
#include "Core_glm.h"
#include "Core_Log.h"
#include "Core_Utils.h"

#include "vulkan/vulkan.h"
#define VMA_ASSERT(expr) Verify((expr)) // workaround for Release builds
#include "vk_mem_alloc.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include "Render_Helpers.h"
#include "Render_RenderCore.h"
