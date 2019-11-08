//
// Created by Yuerer on 2019/11/7.
//

#ifndef YY_ALLOCATOR_H
#define YY_ALLOCATOR_H

namespace YY {

    static const int ALIGN = 8;
    static const int MAX_BYTES = 128;
    static const int FREE_LIST_NUMS = MAX_BYTES / ALIGN; // 16

    class Alloc {
    public:
        static void *allocate(size_t n) {
            obj **my_free_list;
            obj *result;
            if (n > (size_t)MAX_BYTES) {
                return ::operator new(n);
            }
            my_free_list = free_list + FREE_LIST_INDEX(n);
            result = *my_free_list;
            if (result == nullptr) {
                void *r = refill(ROUND_UP(n));
                return r;
            }
            *my_free_list = result->next;
            return result;
        }
        static void deallocate(void *p, size_t n) {
            obj *q = (obj *)p;
            obj **my_free_list;
            if (n > (size_t)MAX_BYTES) {
                return ::operator delete(p);
            }
            my_free_list = free_list + FREE_LIST_INDEX(n);
            q->next = *my_free_list;
            *my_free_list = q;
        }
    private:
        // 单向链表
        union obj {
            union obj *next;
        };
    private:
        // 内存池
        static char *start_free;
        static char *end_free;
        static size_t heap_size;
    private:
        static obj *free_list[FREE_LIST_NUMS];
        static void *refill(size_t n);
        static char *chunk_alloc(size_t n, int &blocks);
    private:
        // 从 1 起步
        static size_t FREE_LIST_INDEX(size_t bytes) {
            return ((bytes + ALIGN - 1) / (ALIGN - 1));
        }
        static size_t ROUND_UP(size_t bytes) {
            return ((bytes + ALIGN - 1) & ~(ALIGN - 1));
        }
    };




    Alloc::obj *Alloc::free_list[FREE_LIST_NUMS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char *Alloc::start_free = nullptr;
    char *Alloc::end_free = nullptr;
    size_t Alloc::heap_size = 0;

    void *Alloc::refill(size_t n) {
        int blocks = 20;
        char *chunk = chunk_alloc(n, blocks);

        obj **my_free_list;
        obj *result;
        obj *next_obj, *current_obj;
        if (blocks == 1) {
            return chunk;
        }
        my_free_list = free_list + FREE_LIST_INDEX(n);
        result = (obj*)chunk;
        *my_free_list = next_obj = (obj*)(chunk + n);
        for (int i = 1; ; ++ i) {
            current_obj = next_obj;
            next_obj = (obj*)((char*)next_obj + n);
            if (blocks - 1 == i) {
                current_obj->next = nullptr;
                break;
            } else {
                current_obj->next = next_obj;
            }
        }
        return result;
    }

    char *Alloc::chunk_alloc(size_t n, int &blocks) {
        size_t bytes_left = end_free - start_free;
        size_t total_bytes = n * blocks;
        char *result;
        if (bytes_left >= total_bytes) {
            result = start_free;
            start_free += total_bytes;
            return result;
        } else if (bytes_left >= n) {
            blocks = bytes_left / n;
            total_bytes = n * blocks;
            result = start_free;
            start_free += total_bytes;
            return result;
        } else {
            size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
            if (bytes_left > 0) {
                obj **my_free_list = free_list + FREE_LIST_INDEX(bytes_left);
                ((obj*)start_free)->next = *my_free_list;
                *my_free_list = (obj*)start_free;
            }
            start_free = (char*)malloc(bytes_to_get);
            if (start_free == nullptr) {
                obj **my_free_list;
                obj *p;
                for (int i = n + ALIGN; i <= MAX_BYTES; i += ALIGN) {
                    my_free_list = free_list + FREE_LIST_INDEX(i);
                    p = *my_free_list;
                    if (p != nullptr) {
                        *my_free_list = p->next;
                        start_free = (char *) p;
                        end_free = (char *) p + i;
                        return chunk_alloc(n, blocks);
                    }
                }
                end_free = nullptr;
                start_free = (char*)::operator new(bytes_to_get);
            }
            heap_size += bytes_to_get;
            end_free = start_free + bytes_to_get;
            return chunk_alloc(n, blocks);
        }
    }

    template <typename T>
    class Allocator {
    public:
        typedef T value_type;
        typedef T* pointer;
        typedef const T* const_pointer;
        typedef T& reference;
        typedef const T& const_reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        static T* allocate(size_t n) {
            return (n == 0) ? 0 : (T*)Alloc::allocate(n * sizeof(T));
        }
        static T* allocate() {
            return (T*)Alloc::allocate(sizeof(T));
        }
        static void deallocate(T *p, size_t n) {
            if (n != 0) {
                Alloc::deallocate(p, n * sizeof(T));
            }
        }
        static void deallocate(T *p) {
            Alloc::deallocate(p, sizeof(T));
        }
    };
}

#endif //YY_ALLOCATOR_H
