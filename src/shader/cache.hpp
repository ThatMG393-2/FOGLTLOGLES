#pragma once

#include "common/env.hpp"
#include "common/log.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <mutex>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <type_traits>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ShaderConverter::Cache {
    // Bump whenever converter output changes for the same input.
    inline constexpr uint32_t CONVERTER_REVISION = 2;

    inline uint64_t hashBytes(const void* data, size_t size, uint64_t seed = 0) {
        constexpr uint64_t k1 = 0x9E3779B97F4A7C15ull;
        constexpr uint64_t k2 = 0xC2B2AE3D27D4EB4Full;

        const auto* p = static_cast<const uint8_t*>(data);
        uint64_t h = seed ^ (size * k1);

        while (size >= 8) {
            uint64_t v;
            std::memcpy(&v, p, 8);
            v *= k2;
            v ^= v >> 29;
            h = (h ^ v) * k1;
            h ^= h >> 32;
            p += 8;
            size -= 8;
        }

        uint64_t tail = 0;
        std::memcpy(&tail, p, size);
        h = (h ^ (tail * k2)) * k1;
        h ^= h >> 31;
        h *= k2;
        h ^= h >> 29;

        return h;
    }

    inline uint64_t hashCombine(uint64_t seed, uint64_t value) {
        return hashBytes(&value, sizeof(value), seed);
    }

    template<typename T>
    inline size_t getHash(const T& key) {
        if (!ENABLE_SHADER_CACHE) return 0;

        if constexpr (std::is_convertible_v<const T&, std::string_view>) {
            std::string_view view = key;
            return hashBytes(view.data(), view.size());
        } else {
            static_assert(std::is_trivially_copyable_v<T>);
            return hashBytes(&key, sizeof(T));
        }
    }

    class Store {
    public:
        explicit Store(const char* name) : name(name) {}

        void init(const std::string& root, time_t retentionSeconds) {
            std::lock_guard lock(mutex);

            directory = root + "/" + name;
            keys.clear();

            std::error_code ec;
            std::filesystem::create_directories(directory, ec);
            if (ec) {
                LOGW("Failed to create cache directory '%s': %s", directory.c_str(), ec.message().c_str());
                directory.clear();
                return;
            }

            const time_t now = time(nullptr);
            size_t removed = 0;

            for (std::filesystem::directory_iterator it(directory, ec), end; !ec && it != end; it.increment(ec)) {
                const std::string path = it->path().string();
                const std::string file = it->path().filename().string();

                struct stat st;
                bool stale = stat(path.c_str(), &st) != 0
                    || file.size() != 16
                    || now - std::max(st.st_atime, st.st_mtime) > retentionSeconds;

                char* parseEnd = nullptr;
                const uint64_t key = stale ? 0 : std::strtoull(file.c_str(), &parseEnd, 16);
                if (!stale && (parseEnd == nullptr || *parseEnd != '\0')) stale = true;

                if (stale) {
                    std::filesystem::remove(it->path(), ec);
                    removed++;
                    ec.clear();
                    continue;
                }

                keys.emplace(key, false);
            }

            LOGI("Cache '%s': %zu entries, %zu removed", name, keys.size(), removed);
        }

        bool contains(uint64_t key) const {
            std::lock_guard lock(mutex);
            return !directory.empty() && keys.find(key) != keys.end();
        }

        bool read(uint64_t key, std::string& out) {
            std::string path;
            {
                std::lock_guard lock(mutex);
                auto it = keys.find(key);
                if (directory.empty() || it == keys.end()) return false;

                path = pathFor(key);
                if (!it->second) {
                    it->second = true;
                    utimes(path.c_str(), nullptr);
                }
            }

            FILE* file = std::fopen(path.c_str(), "rb");
            if (!file) {
                erase(key);
                return false;
            }

            Header header;
            bool ok = std::fread(&header, sizeof(header), 1, file) == 1
                && header.magic == MAGIC
                && header.size < (1ull << 30);

            if (ok) {
                out.resize(header.size);
                ok = std::fread(out.data(), 1, header.size, file) == header.size
                    && hashBytes(out.data(), out.size()) == header.checksum;
            }

            std::fclose(file);

            if (!ok) {
                erase(key);
                out.clear();
            }

            return ok;
        }

        bool write(uint64_t key, const void* data, size_t size) {
            std::string path;
            {
                std::lock_guard lock(mutex);
                if (directory.empty()) return false;
                path = pathFor(key);
            }

            const std::string tmp = path + "." + std::to_string(getpid()) + ".tmp";
            FILE* file = std::fopen(tmp.c_str(), "wb");
            if (!file) return false;

            const Header header { MAGIC, 0, size, hashBytes(data, size) };
            bool ok = std::fwrite(&header, sizeof(header), 1, file) == 1
                && (size == 0 || std::fwrite(data, 1, size, file) == size);
            ok = (std::fclose(file) == 0) && ok;

            if (!ok || std::rename(tmp.c_str(), path.c_str()) != 0) {
                std::remove(tmp.c_str());
                return false;
            }

            std::lock_guard lock(mutex);
            keys[key] = true;
            return true;
        }

        bool erase(uint64_t key) {
            std::string path;
            {
                std::lock_guard lock(mutex);
                if (keys.erase(key) == 0) return false;
                path = pathFor(key);
            }

            return std::remove(path.c_str()) == 0;
        }

    private:
        static constexpr uint32_t MAGIC = 0x4C474F46; // "FOGL"

        struct Header {
            uint32_t magic;
            uint32_t reserved;
            uint64_t size;
            uint64_t checksum;
        };

        std::string pathFor(uint64_t key) const {
            char name[17];
            std::snprintf(name, sizeof(name), "%016llx", static_cast<unsigned long long>(key));
            return directory + "/" + name;
        }

        const char* name;
        std::string directory;
        std::unordered_map<uint64_t, bool> keys; // value: mtime refreshed this session
        mutable std::mutex mutex;
    };

    inline Store convertedStore("converted");
    inline Store binaryStore("binaries");

    using UBOBinding = std::pair<std::string, uint32_t>;

    inline bool isShaderInCache(size_t key) {
        return ENABLE_SHADER_CACHE && key && convertedStore.contains(key);
    }

    inline void putShaderInCache(size_t key, const std::string& source) {
        if (!ENABLE_SHADER_CACHE || !key) return;
        convertedStore.write(key, source.data(), source.size());
    }

    inline std::string getCachedShaderSource(size_t key) {
        std::string source;
        if (ENABLE_SHADER_CACHE && key) convertedStore.read(key, source);
        return source;
    }

    inline bool invalidateShaderCache(size_t key) {
        return ENABLE_SHADER_CACHE && key && convertedStore.erase(key);
    }

    // Converted GLSL entries also carry the UBO bindings the converter assigned,
    // so a cache hit can restore the name -> binding table.
    inline void putConverted(size_t key, const std::string& source, const std::vector<UBOBinding>& ubos) {
        if (!ENABLE_SHADER_CACHE || !key) return;

        std::string blob;
        auto append32 = [&](uint32_t v) { blob.append(reinterpret_cast<const char*>(&v), sizeof(v)); };

        append32(static_cast<uint32_t>(ubos.size()));
        for (const auto& [name, binding] : ubos) {
            append32(binding);
            append32(static_cast<uint32_t>(name.size()));
            blob += name;
        }
        blob += source;

        convertedStore.write(key, blob.data(), blob.size());
    }

    inline bool getConverted(size_t key, std::string& source, std::vector<UBOBinding>& ubos) {
        std::string blob;
        if (!ENABLE_SHADER_CACHE || !key || !convertedStore.read(key, blob)) return false;

        size_t pos = 0;
        auto read32 = [&](uint32_t& v) {
            if (blob.size() - pos < sizeof(v)) return false;
            std::memcpy(&v, blob.data() + pos, sizeof(v));
            pos += sizeof(v);
            return true;
        };

        uint32_t count = 0;
        bool ok = read32(count) && count < 4096;
        ubos.clear();

        for (uint32_t i = 0; ok && i < count; i++) {
            uint32_t binding = 0, len = 0;
            ok = read32(binding) && read32(len) && blob.size() - pos >= len;
            if (!ok) break;

            ubos.emplace_back(blob.substr(pos, len), binding);
            pos += len;
        }

        if (!ok) {
            convertedStore.erase(key);
            return false;
        }

        source.assign(blob, pos, std::string::npos);
        return !source.empty();
    }

    namespace Binary {
        inline bool get(size_t key, std::string& out) {
            return ENABLE_BINARY_CACHE && binaryStore.read(key, out);
        }

        inline void put(size_t key, const void* data, size_t size) {
            if (ENABLE_BINARY_CACHE) binaryStore.write(key, data, size);
        }

        inline void invalidate(size_t key) {
            if (ENABLE_BINARY_CACHE) binaryStore.erase(key);
        }
    }

    inline void init() {
        if (!ENABLE_SHADER_CACHE || SHADER_CACHE_DIR.empty()) {
            LOGI("Shader cache is disabled!");
            ENABLE_SHADER_CACHE = false;
            ENABLE_BINARY_CACHE = false;
            return;
        }

        const time_t retention = static_cast<time_t>(SHADER_RETENTION_DAYS) * 24 * 60 * 60;

        convertedStore.init(SHADER_CACHE_DIR, retention);
        if (ENABLE_BINARY_CACHE) binaryStore.init(SHADER_CACHE_DIR, retention);
    }
}
