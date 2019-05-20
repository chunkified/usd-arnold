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
#include "readProcedural.h"

#include <usdKatana/attrMap.h>

#include <usdKatana/readXformable.h>
#include <usdKatana/usdInPrivateData.h>

#include <pxr/usd/usdAi/aiProcedural.h>
#include <pxr/usd/usdAi/aiProceduralNode.h>

#include "arnoldHelpers.h"

PXR_NAMESPACE_OPEN_SCOPE

void readAiProcedural(
    const UsdAiProcedural& procedural, const PxrUsdKatanaUsdInPrivateData& data,
    PxrUsdKatanaAttrMap& attrs) {
    // Read in general attributes for a transformable prim.
    PxrUsdKatanaReadXformable(procedural, data, attrs);

    const UsdPrim& prim = procedural.GetPrim();
    const double currentTime = data.GetUsdInArgs()->GetCurrentTime();

    attrs.set("type", FnKat::StringAttribute("renderer procedural"));

    SdfAssetPath filepath;
    if (auto fileAttr = procedural.GetFilepathAttr()) {
        if (fileAttr.HasValue()) { fileAttr.Get(&filepath, currentTime); }
    }
    attrs.set(
        "rendererProcedural.procedural",
        FnKat::StringAttribute(filepath.GetResolvedPath()));

    // This plugin is registered for both AiProcedural and AiProceduralNode,
    // and we need to handle them slightly differently for KtoA.
    if (auto proceduralNode = UsdAiProceduralNode(prim)) {
        TfToken nodeType;
        proceduralNode.GetNodeTypeAttr().Get(&nodeType);
        attrs.set(
            "rendererProcedural.node",
            FnKat::StringAttribute(nodeType.GetString()));
    } else {
        attrs.set(
            "rendererProcedural.node", FnKat::StringAttribute("procedural"));
    }

    FnKat::GroupBuilder argsBuilder;
    if (applyProceduralArgsAttrs(
            procedural.GetPrim(), argsBuilder, currentTime)) {
        argsBuilder.set(
            "__outputStyle", FnKat::StringAttribute("typedArguments"));
        attrs.set("rendererProcedural.args", argsBuilder.build());
    }
}

PXR_NAMESPACE_CLOSE_SCOPE
