#ifndef PARAMETER_VALUE_H
#define PARAMETER_VALUE_H

#include <cstdint>
#include <cstring>
#include <string>

enum class ParamType : uint8_t {
    Bool = 1,
    Int = 2,
    Float = 3,
    Enum = 4
};

struct ParameterValue {
    ParamType type;

    union {
        bool boolVal;
        int32_t intVal;
        float floatVal;
    } val;

    ParameterValue() : type(ParamType::Float) {
        val.floatVal = 0.0f;
    }

    static ParameterValue makeBool(bool b) {
        ParameterValue p;
        p.type = ParamType::Bool;
        p.val.boolVal = b;
        return p;
    }

    static ParameterValue makeInt(int32_t i) {
        ParameterValue p;
        p.type = ParamType::Int;
        p.val.intVal = i;
        return p;
    }

    static ParameterValue makeEnum(int32_t e) {
        ParameterValue p;
        p.type = ParamType::Enum;
        p.val.intVal = e;
        return p;
    }

    static ParameterValue makeFloat(float f) {
        ParameterValue p;
        p.type = ParamType::Float;
        p.val.floatVal = f;
        return p;
    }

    bool asBool() const {
        if (type == ParamType::Bool) return val.boolVal;
        if (type == ParamType::Int || type == ParamType::Enum) return val.intVal != 0;
        return val.floatVal > 0.5f;
    }

    int32_t asInt() const {
        if (type == ParamType::Int || type == ParamType::Enum) return val.intVal;
        if (type == ParamType::Bool) return val.boolVal ? 1 : 0;
        return static_cast<int32_t>(val.floatVal >= 0.0f ? val.floatVal + 0.5f : val.floatVal - 0.5f);
    }

    float asFloat() const {
        if (type == ParamType::Float) return val.floatVal;
        if (type == ParamType::Bool) return val.boolVal ? 1.0f : 0.0f;
        return static_cast<float>(val.intVal);
    }

    bool operator==(const ParameterValue& other) const {
        if (type != other.type) {
            // Compare as float if types differ
            return asFloat() == other.asFloat();
        }
        switch (type) {
            case ParamType::Bool:
                return val.boolVal == other.val.boolVal;
            case ParamType::Int:
            case ParamType::Enum:
                return val.intVal == other.val.intVal;
            case ParamType::Float:
                return val.floatVal == other.val.floatVal;
        }
        return false;
    }

    bool operator!=(const ParameterValue& other) const {
        return !(*this == other);
    }
};

#endif // PARAMETER_VALUE_H
