#ifndef MANDEL_ARENA_H
#define MANDEL_ARENA_H

#include <vector>
#include <array>
#include <utility>
#include <memory>

namespace mnd
{
    namespace util
    {
        template <typename T, int chunkSize = 32>
        class Arena
        {
            struct Chunk
            {
                char data[sizeof(T) * chunkSize];
                int used = 0;

                bool full(void) const { return used == chunkSize; }

                template<typename... Args>
                T* allocate(Args&&... args)
                {
                    return new(reinterpret_cast<T*>(&data[(used++) * sizeof(T)])) T(std::forward<Args>(args)...);
                }

                ~Chunk(void)
                {
                    for (int i = used - 1; i >= 0; i--) {
                        reinterpret_cast<T*>(&data[i * sizeof(T)])->~T();
                    }
                }
            };

            std::vector<std::unique_ptr<Chunk>> chunks;
        public:

            Arena(void) = default;
            Arena(const Arena&) = delete;
            Arena(Arena&&) = default;
            ~Arena(void)
            {
                for (auto it = chunks.rbegin(); it != chunks.rend(); ++it) {
                    *it = nullptr;
                }
            }

            Arena& operator=(const Arena&) = delete;
            Arena& operator=(Arena&&) = default;

            Chunk& lastChunk(void) { return *chunks[chunks.size() - 1]; }

            template<typename... Args>
            T* allocate(Args&&... args)
            {
                if (chunks.empty() || lastChunk().full()) {
                    chunks.push_back(std::make_unique<Chunk>());
                }

                return lastChunk().allocate(std::forward<Args>(args)...);
            }
        };
    }
}


#endif // MANDEL_ARENA_H
