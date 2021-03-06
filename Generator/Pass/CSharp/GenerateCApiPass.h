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


#include <Urho3D/Core/Object.h>
#include "GeneratorContext.h"
#include "Pass/CppPass.h"
#include "Printer/CSharpPrinter.h"

namespace Urho3D
{

/// Walk AST and gather known defined classes. Exclude protected/private members from generation.
class GenerateCApiPass : public CppApiPass
{
    URHO3D_OBJECT(GenerateCApiPass, CppApiPass);
public:
    explicit GenerateCApiPass(Context* context) : CppApiPass(context) { };

    void Start() override;
    bool Visit(MetaEntity* entity, cppast::visitor_info info) override;
    void Stop() override;

protected:
    std::string GetUniqueName(const std::string& baseName);
    std::string MapToCNoCopy(const std::string& type, const std::string& expression);
    std::string MapToCpp(const cppast::cpp_type& type, const std::string& expression);
    std::string MapToC(const cppast::cpp_type& type, const std::string& expression);
    std::string ToCType(const cppast::cpp_type& type, bool disallowReferences=false);
    void PrintParameterHandlingCodePre(const std::vector<SharedPtr<MetaEntity>>& parameters);
    void PrintParameterHandlingCodePost(const std::vector<SharedPtr<MetaEntity>>& parameters);

    CSharpPrinter printer_;
    StringVector usedNames_;
};

}
