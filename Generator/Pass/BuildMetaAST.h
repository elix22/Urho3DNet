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
#include "CppPass.h"
#include "Utilities.h"
#include "GeneratorContext.h"

namespace Urho3D
{

/// Walk AST and build API tree which later can be altered and used for generating a wrapper.
class BuildMetaAST : public CppAstPass
{
    URHO3D_OBJECT(BuildMetaAST, CppAstPass);
public:
    explicit BuildMetaAST(Context* context) : CppAstPass(context) { };

    void Start() override
    {
        symbolChecker_.Load(generator->rules_->GetRoot().Get("symbols"));
        stack_.Push(generator->apiRoot_.Get());
    }

    bool Visit(const cppast::cpp_entity& e, cppast::visitor_info info) override
    {
        switch (e.kind())
        {
        case cppast::cpp_entity_kind::file_t:
        case cppast::cpp_entity_kind::include_directive_t:
        case cppast::cpp_entity_kind::language_linkage_t:
        case cppast::cpp_entity_kind::namespace_alias_t:
        case cppast::cpp_entity_kind::using_directive_t:
        case cppast::cpp_entity_kind::using_declaration_t:
        case cppast::cpp_entity_kind::type_alias_t:
            return true;
        default:
            break;
        }

        if (!e.name().empty() /* anonymous */)
        {
            auto symbolName = GetUniqueName(e);
            if (!symbolChecker_.IsIncluded(symbolName))
                return info.event != cppast::visitor_info::container_entity_enter;
        }

        // Skip children of private entities
        if (info.access == cppast::cpp_private)
            return info.event != cppast::visitor_info::container_entity_enter;

        if (e.kind() == cppast::cpp_entity_kind::class_t)
        {
            // Ignore forward-declarations
            if (!cppast::is_definition(e))
                return info.event != cppast::visitor_info::container_entity_enter;
        }

        if (info.event == cppast::visitor_info::container_entity_exit)
        {
            stack_.Pop();
            return true;
        }
        else
        {
            auto* entity = new MetaEntity(e, info.access);
            stack_.Back()->Add(entity);
            if (info.event == cppast::visitor_info::container_entity_enter)
                stack_.Push(entity);

            if (e.kind() == cppast::cpp_entity_kind::enum_value_t)
            {
                // Cache enum values. They will be used when inserting default arguments.
                // TODO: Assertion is likely to cause issues if two enums have values with identical names.
                assert(!generator->enumValues_.Contains(entity->name_));
                generator->enumValues_[entity->name_] = entity;
            }

            if (info.event != cppast::visitor_info::container_entity_exit)
            {
                // Workaround for cppast function parameters being not visited.
                if (e.kind() == cppast::cpp_entity_kind::function_t)
                {
                    for (const auto& param : entity->Ast<cppast::cpp_function>().parameters())
                        entity->Add(new MetaEntity(param, cppast::cpp_public));
                }
                else if (e.kind() == cppast::cpp_entity_kind::member_function_t)
                {
                    for (const auto& param : entity->Ast<cppast::cpp_member_function>().parameters())
                        entity->Add(new MetaEntity(param, cppast::cpp_public));
                }
                else if (e.kind() == cppast::cpp_entity_kind::constructor_t)
                {
                    for (const auto& param : entity->Ast<cppast::cpp_constructor>().parameters())
                        entity->Add(new MetaEntity(param, cppast::cpp_public));
                }
            }
        }

        return true;
    }

protected:
    IncludedChecker symbolChecker_;
    Vector<MetaEntity*> stack_;
};

}
