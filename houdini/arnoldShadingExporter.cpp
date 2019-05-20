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
#include "arnoldShadingExporter.h"

#include <gusd/primWrapper.h>

#include <VOP/VOP_Node.h>
#include <SHOP/SHOP_Node.h>

#include <pxr/usd/usdShade/connectableAPI.h>
#include <pxr/usd/usdShade/input.h>
#include <pxr/usd/usdShade/output.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>

#include <pxr/usd/usdAi/aiMaterialAPI.h>
#include <pxr/usd/usdAi/aiNodeAPI.h>
#include <pxr/usd/usdAi/aiShader.h>
#include <pxr/usd/usdAi/tokens.h>

#include <ai.h>

PXR_NAMESPACE_OPEN_SCOPE

namespace {

SdfPath inline
getPathNoFirstSlashFromOp(OP_Node* op) {
    const auto* cStr = op->getFullPath().c_str();
    if (cStr == nullptr || cStr[0] != '/') { return SdfPath(); }
    std::string pathStr(cStr + 1);
    
    // We need to remove SHOP from the beginning.
    // Alternatively we could look for the segment after the last /
    return (pathStr.substr(0, 5) == "shop/") ? SdfPath(pathStr.substr(5)) : SdfPath(pathStr);
}

template <typename T> inline
T getSingleValue(const PRM_Parm* parm) {
    auto v = T{0};
    parm->getValue(0.0f, v, 0, SYSgetSTID());
    return v;
}

template <typename T> inline
T getTupleValue(const PRM_Parm* parm) {
    T v;
    parm->getValues(0, v.data(), SYSgetSTID());
    return v;
}

template <typename T, typename H> inline
VtValue readSingleValue(const PRM_Parm* parm) {
    return VtValue(static_cast<H>(getSingleValue<T>(parm)));
};

template <> inline
VtValue readSingleValue<int32, bool>(const PRM_Parm* parm) {
    return VtValue(getSingleValue<int32>(parm) != 0);
};

// All these doubles and we know the GfVecxd layouts, so
// we just use the internal structures. Also, no traits...
template <typename T, typename H> inline
VtValue readTupleValue(const PRM_Parm* parm) {
    return VtValue(H(getTupleValue<T>(parm)));
};

template <typename T, typename H> inline
VtValue readSingleValues(const PRM_Parm* parm) {
    VtArray<H> ret;
    const auto count = parm->getMultiParmCount();
    ret.resize(static_cast<unsigned int>(count));
    for (auto i = decltype(count){0}; i < count; ++i) {
        ret[i] = H(getSingleValue<T>(parm->getMultiParm(i)));
    }
    return VtValue(ret);
};

template <typename T, typename H> inline
VtValue readTupleValues(const PRM_Parm* parm) {
    VtArray<H> ret;
    const auto count = parm->getMultiParmCount();
    ret.resize(static_cast<unsigned int>(count));
    for (auto i = decltype(count){0}; i < count; ++i) {
        ret[i] = H(getTupleValue<T>(parm->getMultiParm(i)));
    }
    return VtValue(ret);
};

SdfPath
exportNode(const UsdStagePtr& stage, const UsdStagePtr& descStage, const SdfPath& looksPath, VOP_Node* vop) {
    auto getShaderDesc = [&descStage] (const std::string& typeName) -> UsdPrim {
        return descStage->GetPrimAtPath(SdfPath("/" + typeName));
    };

    const auto vopTypeName = vop->getOperator()->getName();
    const auto vopShaderPath = getPathNoFirstSlashFromOp(vop);
    if (vopShaderPath.IsEmpty()) { return SdfPath(); }
    const auto outShaderPath = looksPath.AppendPath(vopShaderPath);
    // We already exported the shader.
    if (stage->GetPrimAtPath(outShaderPath).IsValid()) { return outShaderPath; }
    static constexpr auto aiShaderPrefix = "arnold::";
    static constexpr auto aiShaderPrefixLength = strlen(aiShaderPrefix);
    // We only export arnold nodes, and they should start with arnold::
    if (!vopTypeName.startsWith(aiShaderPrefix)) { return SdfPath(); }
    // Yes! I know I should use substring, but that api on UT_String is so ugly.
    const std::string aiTypeName(vopTypeName.c_str() + aiShaderPrefixLength);
    auto aiShader = UsdAiShader::Define(stage, outShaderPath);
    aiShader.CreateIdAttr().Set(TfToken(aiTypeName));

    auto desc = getShaderDesc(aiTypeName);
    if (!desc.IsValid()) { return SdfPath(); }
    UsdAiNodeAPI descAPI(desc);

    auto getParamDesc = [&desc] (const char* paramName) -> UsdAttribute {
        return desc.GetAttribute(TfToken(paramName));
    };

    auto getTypeMetadata = [] (const UsdAttribute& attr, const TfToken& meta) -> int {
        TfToken token;
        if (!attr.GetMetadata(meta, &token)) {
            return AI_TYPE_NONE;
        } else {
            return UsdAiNodeAPI::GetParamTypeFromToken(token);
        }
    };

    struct ParmConversion {
        SdfValueTypeName type;
        std::function<VtValue(const PRM_Parm*)> fn;

        ParmConversion(const SdfValueTypeName& _type, decltype(fn) _fn) :
            type(_type), fn(std::move(_fn)) { }
    };

    auto readStringValue = [] (const PRM_Parm* parm) -> VtValue {
        UT_String v;
        parm->getValue(0, v, 0, true, SYSgetSTID());
        return VtValue(std::string(v.c_str()));
    };

    auto readStringValues = [] (const PRM_Parm* parm) -> VtValue {
        const auto count = parm->getMultiParmCount();
        VtStringArray ret(static_cast<unsigned int>(count));
        for (auto i = decltype(count){0}; i < count; ++i) {
            UT_String v;
            parm->getMultiParm(i)->getValue(0, v, 0, true, SYSgetSTID());
            ret[i] = v.c_str();
        }

        return VtValue(ret);
    };

    static const std::unordered_map<int, ParmConversion> parmConversions = {
        {AI_TYPE_BYTE, {SdfValueTypeNames->UChar, readSingleValue<int32, uint8_t>}},
        {AI_TYPE_INT, {SdfValueTypeNames->Int, readSingleValue<int32, int32_t>}},
        {AI_TYPE_UINT, {SdfValueTypeNames->UInt, readSingleValue<int32, uint32_t>}},
        {AI_TYPE_BOOLEAN, {SdfValueTypeNames->Bool, readSingleValue<int32, bool>}},
        {AI_TYPE_FLOAT, {SdfValueTypeNames->Float, readSingleValue<fpreal, float>}},
        {AI_TYPE_RGB, {SdfValueTypeNames->Color3f, readTupleValue<GfVec3d, GfVec3f>}},
        {AI_TYPE_RGBA, {SdfValueTypeNames->Color4f, readTupleValue<GfVec4d, GfVec4f>}},
        {AI_TYPE_VECTOR, {SdfValueTypeNames->Vector3f, readTupleValue<GfVec3d, GfVec3f>}},
        {AI_TYPE_VECTOR2, {SdfValueTypeNames->Float2, readTupleValue<GfVec2d, GfVec2f>}},
        {AI_TYPE_STRING, {SdfValueTypeNames->String, readStringValue}},
        {AI_TYPE_POINTER, {SdfValueTypeNames->String, nullptr}},
        {AI_TYPE_MATRIX, {SdfValueTypeNames->Matrix4d, [] (const PRM_Parm* parm) -> VtValue {
            GfMatrix4d v;
            parm->getValues(0.0f, v.GetArray(), SYSgetSTID());
            return VtValue(v);
        }}},
        {AI_TYPE_ENUM, {SdfValueTypeNames->String, readStringValue}},
        {AI_TYPE_CLOSURE, {SdfValueTypeNames->String, nullptr}},
        {AI_TYPE_USHORT, {SdfValueTypeNames->UInt, readSingleValue<int32, uint16_t>}},
        {AI_TYPE_HALF, {SdfValueTypeNames->Half, readSingleValue<fpreal, GfHalf>}},
    };

    static const std::unordered_map<int, ParmConversion> arrayParmConversions = {
        {AI_TYPE_BYTE, {SdfValueTypeNames->UCharArray, readSingleValues<int32, uint8_t>}},
        {AI_TYPE_INT, {SdfValueTypeNames->IntArray, readSingleValues<int32, int32_t>}},
        {AI_TYPE_UINT, {SdfValueTypeNames->UIntArray, readSingleValues<int32, uint32_t>}},
        {AI_TYPE_BOOLEAN, {SdfValueTypeNames->BoolArray, readSingleValues<int32, bool>}},
        {AI_TYPE_FLOAT, {SdfValueTypeNames->FloatArray, readSingleValues<fpreal, float>}},
        {AI_TYPE_RGB, {SdfValueTypeNames->Color3fArray, readTupleValues<GfVec3d, GfVec3f>}},
        {AI_TYPE_RGBA, {SdfValueTypeNames->Color4fArray, readTupleValues<GfVec4d, GfVec4f>}},
        {AI_TYPE_VECTOR, {SdfValueTypeNames->Vector3fArray, readTupleValues<GfVec3d, GfVec3f>}},
        {AI_TYPE_VECTOR2, {SdfValueTypeNames->Float2Array, readTupleValues<GfVec2d, GfVec2f>}},
        {AI_TYPE_STRING, {SdfValueTypeNames->String, readStringValues}},
        {AI_TYPE_POINTER, {SdfValueTypeNames->String, nullptr}},
        {AI_TYPE_MATRIX, {SdfValueTypeNames->Matrix4d, [] (const PRM_Parm* parm) -> VtValue {
            const auto count = parm->getMultiParmCount();
            VtMatrix4dArray ret(static_cast<unsigned int>(count));
            for (auto i = decltype(count){0}; i < count; ++i) {
                parm->getMultiParm(i)->getValues(0.0f, ret[i].GetArray(), SYSgetSTID());
            }
            return VtValue(ret);
        }}},
        {AI_TYPE_ENUM, {SdfValueTypeNames->StringArray, readStringValues}},
        {AI_TYPE_CLOSURE, {SdfValueTypeNames->StringArray, nullptr}},
        {AI_TYPE_USHORT, {SdfValueTypeNames->UIntArray, readSingleValues<int32, uint16_t>}},
        {AI_TYPE_HALF, {SdfValueTypeNames->HalfArray, readSingleValues<fpreal, GfHalf>}},
    };

    auto getParmConversion = [] (const int type, decltype(parmConversions)& conversions) -> const ParmConversion* {
        const auto it = conversions.find(type);
        if (it == conversions.end()) {
            return nullptr;
        }
        return &it->second;
    };

    auto isBlacklisted = [](const std::vector<UsdAttribute>& metas) -> bool {
        for (const auto& meta: metas) {
            const static TfToken blacklist("houdini.blacklist");
            auto v = false;
            if (UsdAiNodeAPI::GetMetadataNameFromAttr(meta) == blacklist
                && meta.Get(&v) && v) {
                return true;
            }
        }
        return false;
    };

    auto isLinkable = [](const std::vector<UsdAttribute>& metas) -> bool {
        for (const auto& meta: metas) {
            const static TfToken linkable("linkable");
            auto v = true;
            if (UsdAiNodeAPI::GetMetadataNameFromAttr(meta) == linkable
                && meta.Get(&v)) {
                return v;
            }
        }
        return true;
    };

    static const TfToken positionToken("position");
    static const TfToken valueToken("value");
    static const TfToken colorToken("color");
    static const TfToken interpolationToken("interpolation");

    // These two are special cases.
    if (aiTypeName == "ramp_rgb") {
        auto* parm = vop->getParmPtr("ramp");
        if (parm != nullptr) {
            const auto numPoints = parm->getMultiParmCount() / 3;
            VtArray<float> positions(static_cast<unsigned int>(numPoints));
            VtArray<GfVec3f> colors(static_cast<unsigned int>(numPoints));
            VtArray<int> interpolations(static_cast<unsigned int>(numPoints));
            for (auto i = decltype(numPoints){0}; i < numPoints; ++i) {
                positions[i] = static_cast<float>(getSingleValue<double>(parm->getMultiParm(i * 3)));
                colors[i] = GfVec3f(getTupleValue<GfVec3d>(parm->getMultiParm(i * 3 + 1)));
                interpolations[i] = getSingleValue<int32>(parm->getMultiParm(i * 3 + 2));
            }
            aiShader.CreateInput(positionToken, SdfValueTypeNames->FloatArray).Set(positions);
            aiShader.CreateInput(colorToken, SdfValueTypeNames->Color3fArray).Set(colors);
            aiShader.CreateInput(interpolationToken, SdfValueTypeNames->IntArray).Set(interpolations);
        }
    } else if (aiTypeName == "ramp_float") {
        auto* parm = vop->getParmPtr("ramp");
        if (parm != nullptr) {
            const auto numPoints = parm->getMultiParmCount() / 3;
            VtArray<float> positions(static_cast<unsigned int>(numPoints));
            VtArray<float> values(static_cast<unsigned int>(numPoints));
            VtArray<int> interpolations(static_cast<unsigned int>(numPoints));
            for (auto i = decltype(numPoints){0}; i < numPoints; ++i) {
                positions[i] = static_cast<float>(getSingleValue<double>(parm->getMultiParm(i * 3)));
                values[i] = static_cast<float>(getSingleValue<double>(parm->getMultiParm(i * 3 + 1)));
                interpolations[i] = getSingleValue<int32>(parm->getMultiParm(i * 3 + 2));
            }
            aiShader.CreateInput(positionToken, SdfValueTypeNames->FloatArray).Set(positions);
            aiShader.CreateInput(valueToken, SdfValueTypeNames->FloatArray).Set(values);
            aiShader.CreateInput(interpolationToken, SdfValueTypeNames->IntArray).Set(interpolations);
        }
    }

    auto calculateNotDefault = [](const PRM_Parm* parm) {
        if (!parm->isTrueFactoryDefault()) { return true; }
        if (parm->isMultiParm()) {
            const auto count = parm->getMultiParmCount();
            for (auto i = decltype(count){0}; i < count; ++i) {
                auto* nparm = parm->getMultiParm(i);
                if (nparm != nullptr && !nparm->isTrueFactoryDefault()) {
                    return true;
                }
            }
        }
        return false;
    };

    const auto* parms = vop->getParmList();
    const auto parmCount = parms->getEntries();
    for (auto pid = decltype(parmCount){0}; pid < parmCount; ++pid) {
        const auto* parm = parms->getParmPtr(pid);
        if (parm == nullptr) { continue; }
        auto paramDesc = getParamDesc(parm->getToken());
        if (!paramDesc.IsValid()) { continue; }
        auto metadatas = descAPI.GetMetadataForAttribute(paramDesc);
        if (isBlacklisted(metadatas)) { continue; }

        const auto paramIdx = vop->getInputFromName(parm->getToken());
        const auto isConnected = paramIdx >= 0 && vop->isConnected(paramIdx, true);
        const auto notDefault = calculateNotDefault(parm);
        if (isConnected || notDefault) {
            // Checking if the param is blacklisted
            const auto paramType = getTypeMetadata(paramDesc, UsdAiTokens->paramType);
            const auto isArray = paramType == AI_TYPE_ARRAY;
            const auto conversionType = isArray ? getTypeMetadata(paramDesc, UsdAiTokens->elemType) : paramType;
            const auto* conversion = getParmConversion(conversionType, isArray ? arrayParmConversions : parmConversions);

            if (conversion == nullptr) { continue; }
            auto outAttr = aiShader.CreateInput(TfToken(parm->getToken()), conversion->type);
            if (conversion->fn != nullptr && notDefault) {
                outAttr.Set(conversion->fn(parm));
            }

            if (isLinkable(metadatas) && isConnected) {
                VOP_Node* inputVop = vop->findSimpleInput(paramIdx);
                if (inputVop == nullptr) { continue; }
                const auto inParamIdx = inputVop->whichOutputIs(vop, paramIdx);
                auto inShaderPath = exportNode(stage, descStage, looksPath, inputVop);
                if (inShaderPath.IsEmpty()) { continue; }
                UT_String inputName;
                inputVop->getOutputName(inputName, inParamIdx);
                // Parameters in houdini named the same as the arnold type.
                // Same goes for components, so we can significantly simplify the code compared
                // to AiShaderExport.
                static const std::unordered_map<std::string, SdfValueTypeName> inputTypes = {
                    {"rgb", SdfValueTypeNames->Color3f},
                    {"rgba", SdfValueTypeNames->Color4f},
                    {"vector", SdfValueTypeNames->Vector3f},
                    {"vector2", SdfValueTypeNames->Float2},
                    {"string", SdfValueTypeNames->String},
                    {"float", SdfValueTypeNames->Float},
                    {"int", SdfValueTypeNames->Int},
                };

                static const std::unordered_set<std::string> componentNames = {
                    "r", "g", "b", "a", "x", "y", "z"
                };

                UsdShadeConnectableAPI inAPI(stage->GetPrimAtPath(inShaderPath));
                if (!inAPI) { continue; }
                const auto inputType = inputTypes.find(inputName.c_str());
                TfToken inputNameToken(inputName.c_str());
                if (inputType != inputTypes.end()) {
                    auto in = inAPI.GetOutput(inputNameToken);
                    if (!in) {
                        in = inAPI.CreateOutput(inputNameToken, inputType->second);
                    }
                    UsdShadeConnectableAPI::ConnectToSource(outAttr, in);
                // All the components are float in arnold.
                } else if (componentNames.find(inputName.c_str()) != componentNames.end()) {
                    auto in = inAPI.GetOutput(inputNameToken);
                    if (!in) {
                        in = inAPI.CreateOutput(inputNameToken, SdfValueTypeNames->Float);
                    }
                    UsdShadeConnectableAPI::ConnectToSource(outAttr, in);
                }
            }
        }
    }
    return outShaderPath;
}

OP_Node*
findFirstChildrenOfType(OP_Node* op, const char* type) {
    const auto nchildren = op->getNchildren();
    for (auto cid = decltype(nchildren){0}; cid < nchildren; ++cid) {
        auto* ch = op->getChild(cid);
        if (ch->getOperator()->getName() == type) {
            return ch;
        }
    }
    return nullptr;
}

// We need the description of all the arnold parameters. We want to avoid
// issues with the arnold universe and multiple threads accessing / creating
// Arnold universes so we cache the description of the shaders into a simplified
// USD tree.
UsdStageRefPtr getArnoldShaderDesc() {
    static UsdStageRefPtr shaderDescCache = nullptr;
    if (shaderDescCache == nullptr) {
        std::stringstream command; command << "usdAiShaderInfo --cout";
        auto* HTOA_PATH = getenv("HTOA_PATH");
        if (HTOA_PATH != nullptr) {
             command << " --meta " << HTOA_PATH << "/arnold/metadata";
        }
        FILE* pipe = popen(command.str().c_str(), "r");
        if (pipe != nullptr) {
            // TODO: optimize!
            std::stringstream result;
            std::array<char, 128> buffer;
            while (!feof(pipe)) {
                if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                    result << buffer.data();
                }
            }
            pclose(pipe);
            shaderDescCache = UsdStage::CreateInMemory(".usda");
            shaderDescCache->GetRootLayer()->ImportFromString(result.str());
        }
    }
    return shaderDescCache;
}

}

void exportArnoldShaders(
    OP_Node* opNode,
    const UsdStagePtr& stage,
    const SdfPath& looksPath,
    const GusdShadingModeRegistry::HouMaterialMap& houMaterialMap,
    const std::string& shaderOutDir) {
    auto shaderDesc = getArnoldShaderDesc();
    if (shaderDesc == nullptr) { return; }

    for (const auto& assignment: houMaterialMap) {
        // Initially we only care about assigned shops.
        auto* shop = opNode->findSHOPNode(assignment.first.c_str());
        // We only support arnold_vopnets.
        if (shop->getOperator()->getName() != "arnold_vopnet") { continue; }

        const auto shopPath = getPathNoFirstSlashFromOp(shop);
        if (shopPath.IsEmpty()) { continue; }
        const SdfPath materialPath = looksPath.AppendPath(shopPath);

        auto alreadyExported = false;
        if (stage->GetPrimAtPath(materialPath).IsValid()) {
            alreadyExported = true;
        }
        auto shadeMaterial = UsdShadeMaterial::Define(stage, materialPath);

        // We are using the new material binding API, the old is deprecated.
        for (const auto& primToBind: assignment.second) {
            UsdShadeMaterialBindingAPI(stage->GetPrimAtPath(primToBind)).Bind(shadeMaterial);
        }

        // We skip export if it's already exported by somebody else.
        if (alreadyExported) { continue; }

        // We have to find the output node, HtoA simply looks for the first
        // vop node with the type of arnold_material.
        static constexpr auto arnoldMaterialTypeName = "arnold_material";
        auto* possibleArnoldMaterial = findFirstChildrenOfType(shop, arnoldMaterialTypeName);
        auto* vop = possibleArnoldMaterial == nullptr ? nullptr : possibleArnoldMaterial->castToVOPNode();

        if (vop == nullptr) { continue; }

        UsdAiMaterialAPI aiMaterialAPI(shadeMaterial.GetPrim());
        static const std::vector<std::pair<const char*,
            decltype(&UsdAiMaterialAPI::CreateSurfaceRel)>> materialParams = {
            {"surface", &UsdAiMaterialAPI::CreateSurfaceRel},
            {"displacement", &UsdAiMaterialAPI::CreateDisplacementRel},
            {"volume", &UsdAiMaterialAPI::CreateVolumeRel}
        };
        for (const auto& materialParam: materialParams) {
            const auto idx = vop->getInputFromName(materialParam.first);
            if (!vop->isConnected(idx, true)) { continue; }
            auto* inputVOP = vop->findSimpleInput(idx);
            auto inputPath = exportNode(stage, shaderDesc, looksPath, inputVOP);
            if (!inputPath.IsEmpty()) {
                ((aiMaterialAPI).*(materialParam.second))().AddTarget(inputPath);
            }
        }
    }
}

PXR_NAMESPACE_CLOSE_SCOPE
