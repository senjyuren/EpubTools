#ifndef EPUB_KERNEL_HPP
#define EPUB_KERNEL_HPP

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

void *operator new(size_t v)
{
    auto *obj = malloc(v);
    if (obj != nullptr)
        memset(obj, 0, v);
    return obj;
}

void operator delete(void *v) noexcept
{
    if (v != nullptr)
        free(v);
}

void *operator new[](size_t v)
{
    auto *obj = malloc(v);
    if (obj != nullptr)
        memset(obj, 0, v);
    return obj;
}

void operator delete[](void *v) noexcept
{
    if (v != nullptr)
        free(v);
}

template<typename T>
using Type = T;

#endif //EPUB_KERNEL_HPP
