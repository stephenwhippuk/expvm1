#include "assembler/lexer/lexer.h"
#include "assembler/parser/parser.h"
#include "assembler/semantic/semantic_analyzer.h"
#include "assembler/semantic/instruction_rewriter.h"
#include "assembler/ir/code_graph_builder.h"
#include "assembler/codegen/address_resolver.h"
#include "assembler/codegen/binary_writer.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

using namespace lvm;
using namespace lvm::assembler;

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <input.asm> [-o <output.bin>] [-v]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -o <file>    Output binary file (default: out.bin)" << std::endl;
    std::cout << "  -v           Verbose output" << std::endl;
    std::cout << "  -h, --help   Show this help message" << std::endl;
}

std::string read_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    
    return content;
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::string input_file;
    std::string output_file = "out.bin";
    bool verbose = false;
    
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                std::cerr << "Error: -o requires an argument" << std::endl;
                return 1;
            }
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else if (input_file.empty()) {
            input_file = argv[i];
        } else {
            std::cerr << "Error: Unexpected argument: " << argv[i] << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (input_file.empty()) {
        std::cerr << "Error: No input file specified" << std::endl;
        print_usage(argv[0]);
        return 1;
    }
    
    try {
        if (verbose) {
            std::cout << "Assembling: " << input_file << std::endl;
        }
        
        // Read source file
        std::string source = read_file(input_file);
        
        // Pass 1: Lexer + Parser
        if (verbose) std::cout << "Pass 1: Lexing and parsing..." << std::endl;
        Lexer lexer(source);
        Parser parser(lexer);
        auto ast = parser.parse();
        
        if (parser.has_errors()) {
            std::cerr << "Parse errors:" << std::endl;
            for (const auto& error : parser.errors()) {
                std::cerr << "  " << error << std::endl;
            }
            return 1;
        }
        
        // Pass 1.5: Rewrite syntactic sugar
        if (verbose) std::cout << "Pass 1.5: Rewriting syntactic sugar..." << std::endl;
        InstructionRewriter rewriter;
        rewriter.rewrite(*ast);
        
        // Pass 2: Semantic analysis
        if (verbose) std::cout << "Pass 2: Semantic analysis..." << std::endl;
        SymbolTable symbol_table;
        SemanticAnalyzer analyzer(symbol_table);
        if (!analyzer.analyze(*ast)) {
            std::cerr << "Semantic errors:" << std::endl;
            for (const auto& error : analyzer.errors()) {
                std::cerr << "  " << error.to_string() << std::endl;
            }
            return 1;
        }
        
        // Pass 3: Build code graph
        if (verbose) std::cout << "Pass 3: Building code graph..." << std::endl;
        CodeGraphBuilder builder(symbol_table, &analyzer);
        auto graph = builder.build(*ast);
        
        if (builder.has_errors()) {
            std::cerr << "Code graph errors:" << std::endl;
            for (const auto& error : builder.errors()) {
                std::cerr << "  " << error.to_string() << std::endl;
            }
            return 1;
        }
        
        // Pass 4: Resolve addresses
        if (verbose) std::cout << "Pass 4: Resolving addresses..." << std::endl;
        AddressResolver resolver(symbol_table, *graph);
        if (!resolver.resolve()) {
            std::cerr << "Address resolution errors:" << std::endl;
            for (const auto& error : resolver.errors()) {
                std::cerr << "  " << error << std::endl;
            }
            return 1;
        }
        
        // Pass 5: Generate binary
        if (verbose) std::cout << "Pass 5: Generating binary..." << std::endl;
        BinaryWriter writer;
        
        // Extract program name from input filename
        std::string program_name = input_file;
        size_t last_slash = program_name.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            program_name = program_name.substr(last_slash + 1);
        }
        size_t last_dot = program_name.find_last_of('.');
        if (last_dot != std::string::npos) {
            program_name = program_name.substr(0, last_dot);
        }
        
        writer.write_binary(*graph, output_file, program_name);
        
        if (verbose) {
            std::cout << "Successfully assembled to: " << output_file << std::endl;
            std::cout << "Data segment: " << graph->data_segment_size() << " bytes" << std::endl;
            std::cout << "Code segment: " << graph->code_segment_size() << " bytes" << std::endl;
        } else {
            std::cout << "Assembly successful: " << output_file << std::endl;
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
