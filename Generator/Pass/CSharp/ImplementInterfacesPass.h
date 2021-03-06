//
// Copyright (c) 2008-2018 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once


#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Container/HashMap.h>
#include <unordered_map>
#include <Urho3D/Core/Object.h>
#include "Pass/CppPass.h"


namespace Urho3D
{

/// Walk AST and mark suitable classes as interfaces. This has to be done beforehand so entity order does not cause
/// some interface methods be not implemented.
class DiscoverInterfacesPass : public CppApiPass
{
    URHO3D_OBJECT(DiscoverInterfacesPass, CppApiPass);
public:
    explicit DiscoverInterfacesPass(Context* context) : CppApiPass(context) { };
    bool Visit(MetaEntity* entity, cppast::visitor_info info) override;

    /// Map name of symbol to list of classes that multiple-inherit that symbol.
    HashMap<Urho3D::WeakPtr<MetaEntity>, std::vector<Urho3D::WeakPtr<MetaEntity>>> inheritedBy_;
};

/// Copy methods into classes that implement interfaces but do not have these methods defined.
class ImplementInterfacesPass : public CppApiPass
{
    URHO3D_OBJECT(ImplementInterfacesPass, CppApiPass);
public:
    explicit ImplementInterfacesPass(Context* context) : CppApiPass(context) { };
    bool Visit(MetaEntity* entity, cppast::visitor_info info) override;

};

}
