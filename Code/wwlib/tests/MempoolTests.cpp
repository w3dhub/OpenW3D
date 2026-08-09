#include "mempool.h"

#include <cstdint>
#include <iostream>
#include <vector>

namespace {

struct alignas(32) PooledValue
{
    std::uintptr_t first = 0;
    std::uintptr_t second = 0;
};

struct alignas(32) AutoPooledValue : public AutoPoolClass<AutoPooledValue, 3>
{
    std::uintptr_t value = 0;
};

} // namespace

int main()
{
    {
        ObjectPoolClass<PooledValue, 3> pool;
        std::vector<PooledValue *> values;

        for (std::uintptr_t index = 0; index < 10; ++index) {
            PooledValue *value = pool.Allocate_Object();
            if ((reinterpret_cast<std::uintptr_t>(value) % alignof(PooledValue)) != 0) {
                std::cerr << "Object pool returned a misaligned value.\n";
                return 1;
            }
            value->first = index;
            value->second = index + 100;
            values.push_back(value);
        }

        for (std::uintptr_t index = 0; index < values.size(); ++index) {
            if (values[index]->first != index || values[index]->second != index + 100) {
                std::cerr << "Object pool values overlapped or were corrupted.\n";
                return 1;
            }
        }

        for (PooledValue *value : values) {
            pool.Free_Object(value);
        }
    }

    std::vector<AutoPooledValue *> automatic_values;
    for (std::uintptr_t index = 0; index < 10; ++index) {
        AutoPooledValue *value = new AutoPooledValue;
        if ((reinterpret_cast<std::uintptr_t>(value) % alignof(AutoPooledValue)) != 0) {
            std::cerr << "Automatic object pool returned a misaligned value.\n";
            return 1;
        }
        value->value = index;
        automatic_values.push_back(value);
    }

    for (std::uintptr_t index = 0; index < automatic_values.size(); ++index) {
        if (automatic_values[index]->value != index) {
            std::cerr << "Automatic object pool values were corrupted.\n";
            return 1;
        }
    }

    for (AutoPooledValue *value : automatic_values) {
        delete value;
    }

    return 0;
}
