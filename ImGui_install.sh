#!/bin/bash

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

INCLUDE_DIR="$ROOT_DIR/include"

mkdir -p "$INCLUDE_DIR"

IMGUI_DIR="$ROOT_DIR/imgui_clone"

cp "$IMGUI_DIR"/imconfig.h "$INCLUDE_DIR"
cp "$IMGUI_DIR"/imgui*.h "$INCLUDE_DIR"
cp "$IMGUI_DIR"/imgui*.cpp "$INCLUDE_DIR"
cp "$IMGUI_DIR"/imgui.h "$INCLUDE_DIR"
cp "$IMGUI_DIR"/imgui.cpp "$INCLUDE_DIR"
cp "$IMGUI_DIR"/backends/imgui_impl_glfw.h "$INCLUDE_DIR"
cp "$IMGUI_DIR"/backends/imgui_impl_glfw.cpp "$INCLUDE_DIR"
cp "$IMGUI_DIR"/backends/imgui_impl_opengl3.h "$INCLUDE_DIR"
cp "$IMGUI_DIR"/backends/imgui_impl_opengl3.cpp "$INCLUDE_DIR"
cp "$IMGUI_DIR"/backends/imgui_impl_opengl3_loader.h "$INCLUDE_DIR"
cp "$IMGUI_DIR"/imstb_textedit.h "$INCLUDE_DIR"
cp "$IMGUI_DIR"/imstb_rectpack.h "$INCLUDE_DIR"
cp "$IMGUI_DIR"/imstb_truetype.h "$INCLUDE_DIR"

echo "ImGui headers and source files have been installed in the 'include' directory."
