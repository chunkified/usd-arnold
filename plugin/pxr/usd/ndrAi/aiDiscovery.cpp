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
#include "pxr/usd/ndrAi/aiDiscovery.h"

#include <pxr/base/tf/getenv.h>
#include <pxr/base/tf/staticTokens.h>
#include <pxr/base/tf/stringUtils.h>

#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>

#include "pxr/usd/usdAi/tokens.h"

#include "pxr/usd/ndrAi/utils.h"

#include <iostream>

#include <ai.h>

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PRIVATE_TOKENS(_tokens, (shader)(arnold));

NDR_REGISTER_DISCOVERY_PLUGIN(NdrAiDiscoveryPlugin);

NdrAiDiscoveryPlugin::NdrAiDiscoveryPlugin() {}

NdrAiDiscoveryPlugin::~NdrAiDiscoveryPlugin() {}

NdrNodeDiscoveryResultVec NdrAiDiscoveryPlugin::DiscoverNodes(
    const Context& context) {
    NdrNodeDiscoveryResultVec ret;
    auto shaderDefs = NdrAiGetShaderDefs();
    for (const UsdPrim& prim : shaderDefs->Traverse()) {
        const auto shaderName = prim.GetName();
        TfToken filename("<built-in>");
        prim.GetMetadata(UsdAiTokens->filename, &filename);
        ret.emplace_back(
            NdrIdentifier(
                TfStringPrintf("ai:%s", shaderName.GetText())),    // identifier
            NdrVersion(AI_VERSION_ARCH_NUM, AI_VERSION_MAJOR_NUM), // version
            shaderName,                                            // name
            _tokens->shader,                                       // family
            _tokens->arnold, // discoveryType
            _tokens->arnold, // sourceType
            filename,        // uri
            filename         // resolvedUri
        );
    }
    return ret;
}

const NdrStringVec& NdrAiDiscoveryPlugin::GetSearchURIs() const {
    static const auto result = []() -> NdrStringVec {
        NdrStringVec ret = TfStringSplit(TfGetenv("ARNOLD_PLUGIN_PATH"), ":");
        ret.push_back("<built-in>");
        return ret;
    }();
    return result;
}

PXR_NAMESPACE_CLOSE_SCOPE
