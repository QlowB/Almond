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
        //!
        //! \brief Arena-allocator for a generic type
        //!
        //! The arena allocator provides an allocate function to allocate
        //! and construct objects of type T. All allocated objects live as
        //! long as the Arena lives and are destructed in the inverse order.
        //! 
        //! \tparam T the type for the Arena to allocate
        //! \tparam chunkSize the Arena allocates objects in chunks of this size
        //!
        template <typename T, int chunkSize = 64>
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

            Chunk& lastChunk(void) { return *chunks[chunks.size() - 1]; }
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

            //!
            //! \brief construct one object whose lifetime is managed by
            //!        the arena.
            //!
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
