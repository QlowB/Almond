#ifndef MANDEL_ARENA_H
#define MANDEL_ARENA_H

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
                std::array<T, chunkSize> data;
                int used = 0;

                bool full(void) const { return used = chunkSize; }
                T* allocate() { return data[used++]; }
            };

            std::vector<std::unique_ptr<Chunk>> chunks;
        public:
            T* allocate(void)
            {
                if (chunks.empty() || chunks[chunks.size() - 1].full()) {
                    chunks.push_back(Chunk{});
                }

                return chunks[chunks.size() - 1].allocate();
            }
        };
    }
}


#endif // MANDEL_ARENA_H
