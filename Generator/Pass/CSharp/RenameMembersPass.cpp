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

#include <cppast/cpp_template.hpp>
#include <fmt/format.h>
#include "GeneratorContext.h"
#include "RenameMembersPass.h"


namespace Urho3D
{

bool RenameMembersPass::Visit(MetaEntity* entity, cppast::visitor_info info)
{
    switch (entity->kind_)
    {
    case cppast::cpp_entity_kind::member_variable_t:
    case cppast::cpp_entity_kind::variable_t:
        if (IsConstantName(entity->name_))
            // Constants are used in default parameter expressions. Since these expressions come in as raw strings it is
            // rather complicate to rename these entities.
            return true;
        entity->name_ = str::join(SplitName(entity->name_), "");
//        if (info.access != cppast::cpp_public)
//            entity->name_[0] = static_cast<char>(tolower(entity->name_[0]));
        break;
    case cppast::cpp_entity_kind::member_function_t:
    case cppast::cpp_entity_kind::function_t:
    // case cppast::cpp_entity_kind::enum_t:
    // case cppast::cpp_entity_kind::enum_value_t:
    // Enums appear in default parameters as well. Reason for skipping them is same as constants above.
        entity->name_ = str::join(SplitName(entity->name_), "");
    default:
        break;
    }
    return true;
}

std::vector<std::string> RenameMembersPass::SplitName(const std::string& name)
{
    std::vector<std::string> result;

    std::string fragment;
    for (int last = 0, first = 0; first < name.length();)
    {
        bool isUnderscore = name[last] == '_';
        if (isUnderscore || (isupper(name[last]) && islower(name[std::max(last - 1, 0)])) || last == name.length())
        {
            fragment = name.substr(first, last - first);
            if (isUnderscore)
                last++;
            first = last;
            if (!fragment.empty())
            {
                std::transform(fragment.begin(), fragment.end(), fragment.begin(), ::tolower);
                fragment[0] = static_cast<char>(::toupper(fragment[0]));
                result.emplace_back(fragment);
            }
        }
        last++;
    }

    return result;
}

bool RenameMembersPass::IsConstantName(const std::string& name)
{
    for (char c : name)
    {
        if (!isupper(c) && c != '_')
            return false;
    }
    return true;
}

}
