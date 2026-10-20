#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ShaderConverter::GLSLPreprocessor {
    namespace detail {
        inline bool isWord(char c) {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
        }

        inline bool isBlank(char c) {
            return c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f';
        }

        inline size_t skipBlank(std::string_view s, size_t i) {
            while (i < s.size() && isBlank(s[i])) i++;
            return i;
        }

        inline size_t wordEnd(std::string_view s, size_t i) {
            while (i < s.size() && isWord(s[i])) i++;
            return i;
        }

        template<typename Fn>
        inline void forEachLine(std::string_view s, Fn&& fn) {
            size_t pos = 0;
            while (pos < s.size()) {
                size_t nl = s.find('\n', pos);
                size_t end = nl == std::string_view::npos ? s.size() : nl;
                fn(s.substr(pos, end - pos), nl != std::string_view::npos);
                pos = end + 1;
            }
        }

        struct UniformInit {
            size_t begin, nameEnd, end;
            std::string_view name, init;
        };

        // uniform mat4 <name> = <init>;
        inline bool matchUniformInit(std::string_view line, UniformInit& m) {
            for (size_t p = line.find("uniform"); p != std::string_view::npos; p = line.find("uniform", p + 1)) {
                if (p > 0 && isWord(line[p - 1])) continue;

                size_t i = skipBlank(line, p + 7);
                if (i == p + 7 || line.compare(i, 4, "mat4") != 0) continue;

                size_t j = skipBlank(line, i + 4);
                if (j == i + 4) continue;

                size_t nameEnd = wordEnd(line, j);
                if (nameEnd == j) continue;

                size_t k = skipBlank(line, nameEnd);
                if (k >= line.size() || line[k] != '=') continue;

                size_t initBegin = skipBlank(line, k + 1);
                size_t semi = line.find(';', initBegin);
                if (semi == std::string_view::npos || semi == initBegin) continue;

                m = { p, nameEnd, semi + 1, line.substr(j, nameEnd - j), line.substr(initBegin, semi - initBegin) };
                return true;
            }

            return false;
        }

        template<typename Map>
        inline void replaceWords(std::string_view line, const Map& map, std::string& out) {
            size_t i = 0;
            while (i < line.size()) {
                if (!isWord(line[i])) {
                    out += line[i++];
                    continue;
                }

                size_t end = wordEnd(line, i);
                std::string_view word = line.substr(i, end - i);
                auto it = map.find(word);

                if (it != map.end()) out += it->second;
                else out += word;

                i = end;
            }
        }
    }

    // Initializers on uniforms are illegal in the target; hoist them into locals in main().
    inline void moveVariableInitialization(std::string& source) {
        using namespace detail;

        if (source.find("mat4") == std::string::npos) return;

        std::vector<std::pair<std::string, std::string>> uniforms;
        std::string stripped;
        stripped.reserve(source.size());

        forEachLine(source, [&](std::string_view line, bool hasNewline) {
            UniformInit m;
            if (matchUniformInit(line, m)) {
                uniforms.emplace_back(m.name, m.init);
                stripped.append(line.substr(0, m.nameEnd));
                stripped += ';';
                stripped.append(line.substr(m.end));
            } else {
                stripped.append(line);
            }

            if (hasNewline) stripped += '\n';
        });

        if (uniforms.empty()) return;

        std::unordered_map<std::string_view, std::string> replacements;
        for (size_t i = 0; i < uniforms.size(); i++) {
            replacements[uniforms[i].first] = "customMat4Var" + std::to_string(i + 1) + "Fix";
        }

        std::string output;
        output.reserve(stripped.size() + uniforms.size() * 64);

        bool inMain = false, inserted = false;
        forEachLine(stripped, [&](std::string_view line, bool hasNewline) {
            if (!inMain && line.find("void main()") != std::string_view::npos) inMain = true;

            if (inMain && !inserted && line.find('{') != std::string_view::npos) {
                output.append(line);
                output += '\n';

                for (const auto& [name, init] : uniforms) {
                    output += "mat4 ";
                    output += replacements[name];
                    output += " = ";
                    output += init;
                    output += ";\n";
                }

                inserted = true;
                return;
            }

            if (inserted) replaceWords(line, replacements, output);
            else output.append(line);

            if (hasNewline) output += '\n';
        });

        source = std::move(output);
    }

    inline void removeLineDirectives(std::string& source) {
        using namespace detail;

        if (source.find("#line") == std::string::npos) return;

        std::string output;
        output.reserve(source.size());

        forEachLine(source, [&](std::string_view line, bool hasNewline) {
            size_t i = skipBlank(line, 0);
            bool directive = line.compare(i, 5, "#line") == 0 && (i + 5 >= line.size() || !isWord(line[i + 5]));

            if (!directive) output.append(line);
            if (hasNewline) output += '\n';
        });

        source = std::move(output);
    }

    inline void fixDeprecatedTextureFunction(std::string& source) {
        using namespace detail;

        if (source.find("texture2D") == std::string::npos) return;

        std::string output;
        output.reserve(source.size());

        size_t i = 0;
        while (i < source.size()) {
            if (!isWord(source[i])) {
                output += source[i++];
                continue;
            }

            size_t end = wordEnd(source, i);
            std::string_view word(source.data() + i, end - i);

            size_t paren = skipBlank(source, end);
            if (word == "texture2D" && paren < source.size() && source[paren] == '(') {
                output += "texture";
                i = paren;
            } else {
                output.append(word);
                i = end;
            }
        }

        source = std::move(output);
    }

    inline void run(std::string& source, int version) {
        moveVariableInitialization(source);
        removeLineDirectives(source);

        if (version <= 120) fixDeprecatedTextureFunction(source);
    }
}
