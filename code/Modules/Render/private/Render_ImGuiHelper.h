#pragma once

// Define the ImGUIContext as a thread local variable, so that we can safely use several contexts from multiple threads.
struct ImGuiContext;
struct ImPlotContext;

extern thread_local ImGuiContext* MyImGuiTLS;
#define GImGui MyImGuiTLS

extern thread_local ImPlotContext* MyImPlotTLS;
#define GImPlot MyImPlotTLS

#include "imgui_helpers.h"
