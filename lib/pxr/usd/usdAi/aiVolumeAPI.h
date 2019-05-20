//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#ifndef USDAI_GENERATED_AIVOLUMEAPI_H
#define USDAI_GENERATED_AIVOLUMEAPI_H

/// \file usdAi/aiVolumeAPI.h

#include "pxr/pxr.h"
#include "pxr/usd/usdAi/api.h"
#include "pxr/usd/usd/apiSchemaBase.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdAi/tokens.h"

#include "pxr/base/vt/value.h"

#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4d.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// AIVOLUMEAPI                                                                //
// -------------------------------------------------------------------------- //

/// \class UsdAiVolumeAPI
///
/// API for interacting with common Arnold volume node parameters on prims.
///
class UsdAiVolumeAPI : public UsdAPISchemaBase
{
public:
    /// Compile time constant representing what kind of schema this class is.
    ///
    /// \sa UsdSchemaType
    static const UsdSchemaType schemaType = UsdSchemaType::SingleApplyAPI;

    /// Construct a UsdAiVolumeAPI on UsdPrim \p prim .
    /// Equivalent to UsdAiVolumeAPI::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit UsdAiVolumeAPI(const UsdPrim& prim=UsdPrim())
        : UsdAPISchemaBase(prim)
    {
    }

    /// Construct a UsdAiVolumeAPI on the prim held by \p schemaObj .
    /// Should be preferred over UsdAiVolumeAPI(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit UsdAiVolumeAPI(const UsdSchemaBase& schemaObj)
        : UsdAPISchemaBase(schemaObj)
    {
    }

    /// Destructor.
    USDAI_API
    virtual ~UsdAiVolumeAPI();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    USDAI_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a UsdAiVolumeAPI holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// UsdAiVolumeAPI(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    USDAI_API
    static UsdAiVolumeAPI
    Get(const UsdStagePtr &stage, const SdfPath &path);


    /// Applies this <b>single-apply</b> API schema to the given \p prim.
    /// This information is stored by adding "AiVolumeAPI" to the 
    /// token-valued, listOp metadata \em apiSchemas on the prim.
    /// 
    /// \return A valid UsdAiVolumeAPI object is returned upon success. 
    /// An invalid (or empty) UsdAiVolumeAPI object is returned upon 
    /// failure. See \ref UsdAPISchemaBase::_ApplyAPISchema() for conditions 
    /// resulting in failure. 
    /// 
    /// \sa UsdPrim::GetAppliedSchemas()
    /// \sa UsdPrim::HasAPI()
    ///
    USDAI_API
    static UsdAiVolumeAPI 
    Apply(const UsdPrim &prim);

protected:
    /// Returns the type of schema this class belongs to.
    ///
    /// \sa UsdSchemaType
    USDAI_API
    virtual UsdSchemaType _GetSchemaType() const;

private:
    // needs to invoke _GetStaticTfType.
    friend class UsdSchemaRegistry;
    USDAI_API
    static const TfType &_GetStaticTfType();

    static bool _IsTypedSchema();

    // override SchemaBase virtuals.
    USDAI_API
    virtual const TfType &_GetTfType() const;

public:
    // --------------------------------------------------------------------- //
    // AIVOLUMEPADDING 
    // --------------------------------------------------------------------- //
    /// Amount to enlarge the volume bounds by.
    ///
    /// \n  C++ Type: float
    /// \n  Usd Type: SdfValueTypeNames->Float
    /// \n  Variability: SdfVariabilityVarying
    /// \n  Fallback Value: 0.0
    USDAI_API
    UsdAttribute GetAiVolumePaddingAttr() const;

    /// See GetAiVolumePaddingAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDAI_API
    UsdAttribute CreateAiVolumePaddingAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // AISTEPSIZE 
    // --------------------------------------------------------------------- //
    /// The step size for sampling inside the volume. If this is set to
    /// 0, a step size will be automatically calculated based on the
    /// volume's grid information.
    ///
    /// \n  C++ Type: float
    /// \n  Usd Type: SdfValueTypeNames->Float
    /// \n  Variability: SdfVariabilityVarying
    /// \n  Fallback Value: 0.0
    USDAI_API
    UsdAttribute GetAiStepSizeAttr() const;

    /// See GetAiStepSizeAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDAI_API
    UsdAttribute CreateAiStepSizeAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // AISTEPSCALE 
    // --------------------------------------------------------------------- //
    /// A scaling factor applied to the step size, mostly useful when 
    /// `step_size` is set to 0 to modulate the automatic value.
    ///
    /// \n  C++ Type: float
    /// \n  Usd Type: SdfValueTypeNames->Float
    /// \n  Variability: SdfVariabilityVarying
    /// \n  Fallback Value: 1.0
    USDAI_API
    UsdAttribute GetAiStepScaleAttr() const;

    /// See GetAiStepScaleAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDAI_API
    UsdAttribute CreateAiStepScaleAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // AICOMPRESS 
    // --------------------------------------------------------------------- //
    /// Whether to compress grids to reduce memory usage. This is a
    /// lossy compression, however in most cases renders will be almost
    /// the same.
    ///
    /// \n  C++ Type: bool
    /// \n  Usd Type: SdfValueTypeNames->Bool
    /// \n  Variability: SdfVariabilityUniform
    /// \n  Fallback Value: True
    USDAI_API
    UsdAttribute GetAiCompressAttr() const;

    /// See GetAiCompressAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDAI_API
    UsdAttribute CreateAiCompressAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // AIVELOCITYSCALE 
    // --------------------------------------------------------------------- //
    /// A scale factor for the velocity field. A value of 0 disables 
    /// motion blur.
    ///
    /// \n  C++ Type: float
    /// \n  Usd Type: SdfValueTypeNames->Float
    /// \n  Variability: SdfVariabilityVarying
    /// \n  Fallback Value: 1.0
    USDAI_API
    UsdAttribute GetAiVelocityScaleAttr() const;

    /// See GetAiVelocityScaleAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDAI_API
    UsdAttribute CreateAiVelocityScaleAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // AIVELOCITYFPS 
    // --------------------------------------------------------------------- //
    /// The frame rate of the velocity value.
    ///
    /// \n  C++ Type: float
    /// \n  Usd Type: SdfValueTypeNames->Float
    /// \n  Variability: SdfVariabilityVarying
    /// \n  Fallback Value: 24.0
    USDAI_API
    UsdAttribute GetAiVelocityFPSAttr() const;

    /// See GetAiVelocityFPSAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDAI_API
    UsdAttribute CreateAiVelocityFPSAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // AIVELOCITYOUTLIERTHRESHOLD 
    // --------------------------------------------------------------------- //
    /// Controls the filtering of noisy velocities, which can result in
    /// faster rendering of motion blur from physics simulations. The
    /// default value of 0.001 should have little to no visual impact.
    /// Setting this to zero disables filtering entirely.
    ///
    /// \n  C++ Type: float
    /// \n  Usd Type: SdfValueTypeNames->Float
    /// \n  Variability: SdfVariabilityVarying
    /// \n  Fallback Value: 0.0010000000475
    USDAI_API
    UsdAttribute GetAiVelocityOutlierThresholdAttr() const;

    /// See GetAiVelocityOutlierThresholdAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDAI_API
    UsdAttribute CreateAiVelocityOutlierThresholdAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // ===================================================================== //
    // Feel free to add custom code below this line, it will be preserved by 
    // the code generator. 
    //
    // Just remember to: 
    //  - Close the class declaration with }; 
    //  - Close the namespace with PXR_NAMESPACE_CLOSE_SCOPE
    //  - Close the include guard with #endif
    // ===================================================================== //
    // --(BEGIN CUSTOM CODE)--
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
