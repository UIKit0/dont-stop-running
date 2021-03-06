#pragma once

#include <imgui.h>

#include "../engine/input/input.h"

#include "context.h"

namespace dsr {

    void uiUpdate(EditorContext &ctx) {

        imguiBeginFrame((int32_t) input::mouseX(), (int32_t) input::mouseY(), (uint8_t) (mouseButtonDown(input::MouseButton::LEFT) ? IMGUI_MBUT_LEFT : 0), (U32) input::wheelX(), ctx.viewportWidth, ctx.viewportHeight, 0, 0);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(5.0f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 1));

        ImGui::SetNextWindowPos(ImVec2(ctx.viewportWidth - 200, 0), ImGuiSetCondition_Always);
        ImGui::Begin("Editor", &ctx.ui.editorHeaderVisible, ImVec2(200, ctx.viewportHeight), 0.3, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

        // ==============================================================
        //  Action type
        // ==============================================================
        static const char *items[] = {"select", "spawn", "draw shape"};
        if (ImGui::Combo("mode", &ctx.action, items, 3)) {
            if (ctx.fnActionChanged) {
                ctx.fnActionChanged();
            }
        }

        // ==============================================================
        //  Spawn Object
        // ==============================================================
        static const char *objects[] = {"grass_tile", "dirt_tile", "saw"};
        if (ImGui::Combo("item", &ctx.selectedSpawnObject, objects, 3)) {
            if (ctx.fnSpawnObjectSelected) {
                ctx.fnSpawnObjectSelected();
            }
        }

        ImGui::Checkbox("snap to grid", &ctx.snapToGrid);
        ImGui::Checkbox("show grid", &ctx.gridVisible);

        float scale[2];
        float rotation;

        // ==============================================================
        //  Selection
        // ==============================================================
        if (ctx.selectedEntity() != nullptr) {
            if (ImGui::CollapsingHeader("Selection")) {
                ImGui::InputFloat2("Pos", (float *) &ctx.selectedEntity()->position, 1);
                ImGui::InputFloat2("Scale", scale, 1);
                ImGui::InputFloat("Rot", &rotation, 1);
            }
        }

        // ==============================================================
        //  Actions
        // ==============================================================
        if (ImGui::CollapsingHeader("Actions")) {
            if (ImGui::Button("Save", ImVec2(55, 20), true)) {
                if (ctx.fnActionSave) {
                    if (ctx.saveFilename == "") {
                        ctx.saveFilename = saveFileDialog();
                    }

                    if (ctx.saveFilename != "") {
                        ctx.fnActionSave(ctx.saveFilename);
                    }
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Save As", ImVec2(55, 20), true)) {
                if (ctx.fnActionSave) {
                    ctx.saveFilename = saveFileDialog();

                    if (ctx.saveFilename != "") {
                        ctx.fnActionSave(ctx.saveFilename);
                    }
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Load", ImVec2(55, 20), true)) {
                if (ctx.fnActionLoad) {
                    auto filename = ctx.fnActionLoad();
                    if (filename != "") {
                        ctx.saveFilename = filename;
                    }
                }
            }
        }

        glm::vec2 statusBarPosition = glm::vec2(0, ctx.viewportHeight - 30);
        glm::vec2 statusBarSize = glm::vec2(ctx.viewportWidth - 200, 30);


        std::stringstream ss;
        ss << "mouse: " << input::mouseX() << ", " << input::mouseY();

        nvgFillColor(nvgCtx(), {0, 0, 0, 0.3});
        nvgBeginPath(nvgCtx());
        nvgRect(nvgCtx(), statusBarPosition.x, statusBarPosition.y, statusBarSize.x, statusBarSize.y);
        nvgClosePath(nvgCtx());
        nvgFill(nvgCtx());

        nvgFillColor(nvgCtx(), {0.8, 0.8, 0.8, 1.0});
        nvgText(nvgCtx(), statusBarPosition.x + 10, statusBarPosition.y + 20, ss.str().c_str(), nullptr);

        if (ctx.levelMap) {
            ss.str("");
            ss << "objects: " << ctx.levelMap->entities.size();
            nvgText(nvgCtx(), statusBarPosition.x + 150, statusBarPosition.y + 20, ss.str().c_str(), nullptr);
        }

        ImGui::End();
        imguiEndFrame();
    }
}