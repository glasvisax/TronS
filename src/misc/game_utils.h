#include <unordered_map>
#include <cstdint>

template <typename T1, typename T2> 
class LUT
{
public:
    LUT(T1 _min, T1 _max)
    {
        min = _min;
        max = _max;
        for (T1 i = min; i <= max; ++i)
        {
            table[i] = static_cast<T2>(i);
        }
    }

    T2 get( T1 value) const 
    {
        return table.at(value);
    }

private:
    T1 min;
    T2 max;
    std::unordered_map<T1, T2> table; 
};

extern LUT<uint8_t, float> u8_f;
extern LUT<float, uint8_t> f_u8;