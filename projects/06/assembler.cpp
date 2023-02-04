#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <string>

using std::string;
using std::unordered_map;

// INVALID: invalid instruction
// A_INSTRUCTION: address instruction
// C_INSTRUCTION: computation instruction
// L_INSTRUCTION: label declaration instruction
enum InstructionType { INVALID, A_INSTRUCTION, C_INSTRUCTION, L_INSTRUCTION };

char blanks[] = { ' ', '\t', '\n', '\r', '\v' };

inline bool is_blank(char x) {
    for (char ch: blanks) {
        if (x == ch) return true;
    }
    return false;
}

class Parser {
private:
    FILE* fp;
    string instruction;

public:
    Parser(FILE* _fp = nullptr): fp(_fp) {}

    bool hasMoreLines() {
        return !feof(fp);
    }

    // Skips over white space and comments, if necessary
    // Reads the next instruction from the input, and make it the current instruction
    void advance() {
        int ch;
        instruction = "";
        while ((ch = fgetc(fp)) != EOF) {
            if (is_blank(ch))
                continue;
            if (ch == '/') {
                ch = fgetc(fp);
                if (ch != '/') {
                    instruction += '/';
                    if (ch != EOF && !is_blank(ch))
                        instruction += ch;
                    break;
                }
                // If it is comment, skip the rest of current line
                while ((ch = fgetc(fp)) != EOF && ch != '\n');
                continue;
            }
            instruction += ch;
            break;
        }

        while ((ch = fgetc(fp)) != EOF) {
            if (ch == '/') {
                ch = fgetc(fp);
                if (ch == '/') {
                    while ((ch = fgetc(fp)) != EOF && ch != '\n');
                    break;
                }
                instruction += '/';
                if (ch == EOF)
                    break;
            }
            if (ch == '\n')
                break;
            if (is_blank(ch))
                continue;
            instruction += ch;
        }
    }

    // Reset the file pointer to start position
    void reset() {
        rewind(fp);
    }

    InstructionType instructionType() {
        if (instruction.empty())
            return INVALID;
        char ch = instruction[0];
        if (ch == '@')
            return A_INSTRUCTION;
        if (ch == '(')
            return L_INSTRUCTION;
        return C_INSTRUCTION;
    }

    string symbol() {
        InstructionType type = instructionType();
        if (type == A_INSTRUCTION) {
            return instruction.substr(1);
        }
        if (type == L_INSTRUCTION) {
            return instruction.substr(1, instruction.length() - 2);
        }
        return "";
    }

    string dest() {
        size_t i = instruction.find('=');
        return i == string::npos ? "": instruction.substr(0, i);
    }

    string comp() {
        size_t first = instruction.find('=');
        size_t second = instruction.find(';');
        size_t count = (second == string::npos ? second : (second - first - 1));
        return instruction.substr(first + 1, count);
    }

    string jump() {
        size_t i = instruction.find(';');
        return i == string::npos ? "" : instruction.substr(i + 1);
    }
};

class Code {
private:
    unordered_map<string, string> comp_table;
    unordered_map<string, string> jmp_table;

public:
    Code() {
        comp_table["0"]   = "0101010";
        comp_table["1"]   = "0111111";
        comp_table["-1"]  = "0111010";
        comp_table["D"]   = "0001100";
        comp_table["A"]   = "0110000";
        comp_table["M"]   = "1110000";
        comp_table["!D"]  = "0001101";
        comp_table["!A"]  = "0110001";
        comp_table["!M"]  = "1110001";
        comp_table["-D"]  = "0001111";
        comp_table["-A"]  = "0110011";
        comp_table["-M"]  = "1110011";
        comp_table["D+1"] = "0011111";
        comp_table["A+1"] = "0110111";
        comp_table["M+1"] = "1110111";
        comp_table["D-1"] = "0001110";
        comp_table["A-1"] = "0110010";
        comp_table["M-1"] = "1110010";
        comp_table["D+A"] = "0000010";
        comp_table["D+M"] = "1000010";
        comp_table["D-A"] = "0010011";
        comp_table["D-M"] = "1010011";
        comp_table["A-D"] = "0000111";
        comp_table["M-D"] = "1000111";
        comp_table["D&A"] = "0000000";
        comp_table["D&M"] = "1000000";
        comp_table["D|A"] = "0010101";
        comp_table["D|M"] = "1010101";

        jmp_table[""]    = "000";
        jmp_table["JGT"] = "001";
        jmp_table["JEQ"] = "010";
        jmp_table["JGE"] = "011";
        jmp_table["JLT"] = "100";
        jmp_table["JNE"] = "101";
        jmp_table["JLE"] = "110";
        jmp_table["JMP"] = "111";
    }

    string dest(string s) {
        string bin(3, '0');
        if (s.find('A') != string::npos)
            bin[0] = '1';
        if (s.find('D') != string::npos)
            bin[1] = '1';
        if (s.find('M') != string::npos)
            bin[2] = '1';
        return bin;
    }

    string comp(string s) {
        return comp_table[s];
    }

    string jump(string s) {
        return jmp_table[s];
    }
};

class Hack {
private:
    Parser parser;
    Code code;
    unordered_map<string, int> symbol_table;

public:
    // Start to translate the assembly file
    void run(const char* path) {
        FILE *input, *output;
        if (!path || strcmp(path, "") == 0) {
            input = stdin;
            output = stdout;
        } else {
            input = fopen(path, "r");

            char name[128]{};
            int len = strlen(path);
            memcpy(name, path, len);
            int i = len - 1;
            while (i >= 0 && name[i] != '.')
                --i;
            memcpy(name + i, ".hack", 6);
            output = fopen(name, "w");
        }

        parser = Parser(input);

        // Add all pre-defined symbols
        initSymbolTable();

        // Scan the assembly file, collect all the symbol labels
        collectLabels();

        // Start to translate the assembly code to binary code
        translate(output);

        fclose(input);
        fclose(output);
    }

    void initSymbolTable() {
        symbol_table.clear();
        for (int i = 0; i < 16; ++i) {
            symbol_table["R" + std::to_string(i)] = i;
        }
        symbol_table["SP"] = 0;
        symbol_table["LCL"] = 1;
        symbol_table["ARG"] = 2;
        symbol_table["THIS"] = 3;
        symbol_table["THAT"] = 4;
        symbol_table["SCREEN"] = 0x4000;
        symbol_table["KBD"] = 0x6000;
    }

    void collectLabels() {
        int num = 0;
        while (parser.hasMoreLines()) {
            parser.advance();
            InstructionType type = parser.instructionType();
            if (type == L_INSTRUCTION) {
                symbol_table[parser.symbol()] = num;
            } else if (type == A_INSTRUCTION || type == C_INSTRUCTION) {
                num += 1;
            }
        }
        parser.reset();
    }

    void translate(FILE* output) {
        int addr = 16;
        while (parser.hasMoreLines()) {
            parser.advance();
            InstructionType type = parser.instructionType();
            if (type == A_INSTRUCTION) {
                string s = parser.symbol();
                int num;
                if (isdigit(s[0])) {
                    num = stoi(s);
                } else {
                    if (!symbol_table.count(s))
                        symbol_table[s] = addr++;
                    num = symbol_table[s];
                }
                string bin;
                while (num) {
                    bin += '0' + (num & 1);
                    num >>= 1;
                }
                bin.resize(16, '0');
                reverse(bin.begin(), bin.end());
                fprintf(output, "%s\n", bin.c_str());
            } else if (type == C_INSTRUCTION) {
                string bin = "111";
                bin += code.comp(parser.comp());
                bin += code.dest(parser.dest());
                bin += code.jump(parser.jump());
                fprintf(output, "%s\n", bin.c_str());
            }
        }
    }
};

int main(int argc, char* argv[]) {
    Hack hack;

    // If no arguments, handle assembly code from standard input
    if (argc <= 1) {
        hack.run(nullptr);
        return 0;
    }

    // Receive multiple arguments
    for (int i = 1; i < argc; ++i) {
        hack.run((char *)argv[i]);
    }
    return 0;
}
