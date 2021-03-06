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

#include <fstream>
#include <thread>
#include <cppast/libclang_parser.hpp>
#include <CLI11/CLI11.hpp>
#include <Urho3D/Urho3DAll.h>
#include "Pass/BuildMetaAST.h"
#include "Pass/UnknownTypesPass.h"
#include "Pass/CSharp/Urho3DTypeMaps.h"
#include "Pass/CSharp/MoveGlobalsPass.h"
#include "Pass/CSharp/ConvertToPropertiesPass.h"
#include "Pass/CSharp/ImplementInterfacesPass.h"
#include "Pass/CSharp/Urho3DCustomPass.h"
#include "Pass/CSharp/GenerateClassWrappers.h"
#include "Pass/CSharp/GenerateCApiPass.h"
#include "Pass/CSharp/RenameMembersPass.h"
#include "Pass/CSharp/GeneratePInvokePass.h"
#include "Pass/CSharp/GenerateCSApiPass.h"

namespace Urho3D
{

GeneratorContext* generator = nullptr;

}

int main(int argc, char* argv[])
{
    String rulesFile;
    String sourceDir;
    String outputDirCpp;
    String outputDirCs;
    std::vector<std::string> includes;
    std::vector<std::string> defines;
    std::vector<std::string> options;
    SharedPtr<Context> context;

    CLI::App app{"CSharp bindings generator"};

    app.add_option("-I", includes, "Target include paths.");
    app.add_option("-D", defines, "Target preprocessor definitions.");
    app.add_option("-O", options, "Target compiler options.");

    app.add_option("rules", rulesFile, "Path to rules json file")->required();//->check(CLI::ExistingFile);
    app.add_option("source", sourceDir, "Path to source directory")->required();//->check(CLI::ExistingDirectory);
    app.add_option("output", outputDirCpp, "Path to output directory")->required();

    std::vector<std::string> cmdLines;
    do
    {
        if (argc != 2)
            break;

        std::string parametersPath;
        CLI::App app{"CSharp bindings generator"};
        app.add_option("parameters", parametersPath)->required()->check(CLI::ExistingFile);;

        try
        {
            app.parse(argc, argv);
        }
        catch(const CLI::ParseError &e)
        {
            // First parameter is not file path with parameters. Could be a --help flag for example.
            break;
        }

        cmdLines.emplace_back(argv[0]);
        std::ifstream infile(parametersPath);
        std::string line;
        while (std::getline(infile, line))
        {
            str::rtrim(line);
            if (!line.empty())
                cmdLines.push_back(line);
        }

        char** newArgv = argv = new char*[cmdLines.size()];
        for (const auto& ln : cmdLines)
            *newArgv++ = (char*)ln.c_str();
        argc = (int)cmdLines.size();
    } while (false);

    CLI11_PARSE(app, argc, argv);

    // Remove preceding _ from `options` because if they start with - then CLI11 treats them as options and not as
    // values to -O flag.
    for (auto& opt : options)
    {
        if (opt.find("_-") == 0)
            opt = opt.substr(1);
    }

    sourceDir = AddTrailingSlash(sourceDir);
    outputDirCs = AddTrailingSlash(outputDirCpp) + "CSharp/";
    outputDirCpp = AddTrailingSlash(outputDirCpp) + "Native/";

    context = new Context();
    context->RegisterSubsystem(new FileSystem(context));
    context->RegisterSubsystem(new Log(context));
    context->RegisterSubsystem(new WorkQueue(context));
#if URHO3D_LOGGING
    context->GetLog()->SetLevel(LOG_DEBUG);
#endif
#if URHO3D_THREADING
    context->GetWorkQueue()->CreateThreads(std::thread::hardware_concurrency());
#endif
    context->GetFileSystem()->CreateDirsRecursive(outputDirCpp);
    context->GetFileSystem()->CreateDirsRecursive(outputDirCs);

    // Generate bindings
    generator = new GeneratorContext(context);
    context->RegisterSubsystem(generator);

    generator->LoadCompileConfig(includes, defines, options);
#if _WIN32
    generator->config_.set_flags(cppast::cpp_standard::cpp_14, {
        cppast::compile_flag::ms_compatibility | cppast::compile_flag::ms_extensions
    });
#else
    generator->config_.set_flags(cppast::cpp_standard::cpp_11, {cppast::compile_flag::gnu_extensions});
#endif

    generator->LoadRules(rulesFile);
    generator->ParseFiles(sourceDir);

    generator->AddCppPass<BuildMetaAST>();
    generator->AddApiPass<Urho3DTypeMaps>();
    generator->AddApiPass<UnknownTypesPass>();
    generator->AddApiPass<DiscoverInterfacesPass>();
    generator->AddApiPass<ImplementInterfacesPass>();
    generator->AddApiPass<GenerateClassWrappers>();
    generator->AddApiPass<Urho3DCustomPass>();
    generator->AddApiPass<MoveGlobalsPass>();
    generator->AddApiPass<GenerateCApiPass>();
    generator->AddApiPass<RenameMembersPass>();
    generator->AddApiPass<GeneratePInvokePass>();
    generator->AddApiPass<ConvertToPropertiesPass>();
    generator->AddApiPass<GenerateCSApiPass>();

    generator->Generate(outputDirCpp, outputDirCs);
}
