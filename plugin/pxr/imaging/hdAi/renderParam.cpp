// Copyright 2019 Luma Pictures
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "pxr/imaging/hdAi/renderParam.h"

#include <ai.h>

PXR_NAMESPACE_OPEN_SCOPE

bool HdAiRenderParam::Render() {
    const auto status = AiRenderGetStatus();
    if (status == AI_RENDER_STATUS_NOT_STARTED) {
        AiRenderBegin();
        return false;
    }
    if (status == AI_RENDER_STATUS_PAUSED) {
        AiRenderRestart();
        return false;
    }
    if (status == AI_RENDER_STATUS_FINISHED) { return true; }
    if (status == AI_RENDER_STATUS_RESTARTING) { return false; }
    AiRenderBegin();
    return false;
}

void HdAiRenderParam::Restart() {
    const auto status = AiRenderGetStatus();
    if (status != AI_RENDER_STATUS_NOT_STARTED) {
        if (status == AI_RENDER_STATUS_RENDERING) {
            AiRenderInterrupt(AI_BLOCKING);
        } else if (status == AI_RENDER_STATUS_FINISHED) {
            AiRenderRestart();
        }
    }
}

void HdAiRenderParam::End() {
    const auto status = AiRenderGetStatus();
    if (status != AI_RENDER_STATUS_NOT_STARTED) {
        if (status == AI_RENDER_STATUS_RENDERING ||
            status == AI_RENDER_STATUS_RESTARTING) {
            AiRenderAbort(AI_BLOCKING);
        }
        AiRenderEnd();
    }
}

PXR_NAMESPACE_CLOSE_SCOPE
