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
#include <ai.h>

AI_SHADER_NODE_EXPORT_METHODS(uvTextureMtd);

namespace {
enum {
    p_file = 0,
    p_st,
    p_wrapS,
    p_wrapT,
    p_fallback,
    p_scale,
    p_bias,
};

const AtString empty;
const AtString black("black");
const AtString clamp("black");
const AtString repeat("repeat");
const AtString mirror("mirror");
const AtString useMetadata("useMetadata");
} // namespace

node_parameters {
    AiParameterStr("file", "");
    AiParameterVec2("st", 0.0f, 0.0f);
    AiParameterStr("wrapS", "useMetadata");
    AiParameterStr("wrapT", "useMetadata");
    AiParameterRGBA("fallback", 0.0f, 0.0f, 0.0f, 1.0f);
    AiParameterRGBA("scale", 1.0f, 1.0f, 1.0f, 1.0f);
    AiParameterRGBA("bias", 0.0f, 0.0f, 0.0f, 0.0f);

    AiMetaDataSetBool(nentry, "", "ndrai_dont_discover", true);
}

node_initialize {}

node_update {}

node_finish {}

shader_evaluate {
    const auto file = AiShaderEvalParamStr(p_file);
    if (file.empty()) {
        sg->out.RGBA() = AiShaderEvalParamRGBA(p_fallback);
        return;
    }

    AtTextureParams params = {};
    AiTextureParamsSetDefaults(params);

    auto getWrapMode = [](const AtString& mode) -> uint8_t {
        if (mode == black) {
            return AI_WRAP_BLACK;
        } else if (mode == clamp) {
            return AI_WRAP_CLAMP;
        } else if (mode == mirror) {
            return AI_WRAP_MIRROR;
        } else if (mode == repeat) {
            return AI_WRAP_PERIODIC;
        } else {
            return AI_WRAP_FILE;
        }
    };

    params.wrap_s = getWrapMode(AiShaderEvalParamStr(p_wrapS));
    params.wrap_t = getWrapMode(AiShaderEvalParamStr(p_wrapT));

    auto success = false;
    sg->out.RGBA() = AiTextureAccess(sg, file, empty, params, &success);
    if (!success) {
        sg->out.RGBA() = AiShaderEvalParamRGBA(p_fallback);
        return;
    }
    sg->out.RGBA() *= AiShaderEvalParamRGBA(p_scale);
    sg->out.RGBA() += AiShaderEvalParamRGBA(p_bias);
}
