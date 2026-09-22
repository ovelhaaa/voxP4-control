#include "ParameterRegistry.h"
#include "ParameterRegistryData.inl"
#include <cctype>
#include <cmath>
#include <cstring>

static void normalizeName(const char* src, char* dst, size_t dstSize) {
    size_t d = 0;
    for (size_t s = 0; src[s] != '\0' && d + 1 < dstSize; ++s) {
        char c = src[s];
        if (c == '.' || c == '_' || c == '-' || c == ' ') continue;
        dst[d++] = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    dst[d] = '\0';
}

const ParamDescriptor* ParameterRegistry::getByIndex(size_t index) {
    if (index >= kParamCount) return nullptr;
    return &kParamDescriptors[index];
}

const ParamDescriptor* ParameterRegistry::getByWireId(uint16_t wireId) {
    for (size_t i = 0; i < kParamCount; ++i) {
        if (kParamDescriptors[i].wireId == wireId) {
            return &kParamDescriptors[i];
        }
    }
    return nullptr;
}

const ParamDescriptor* ParameterRegistry::getByName(const char* name) {
    if (!name || name[0] == '\0') return nullptr;

    char targetNorm[64];
    normalizeName(name, targetNorm, sizeof(targetNorm));

    // Common musical aliases
    if (std::strcmp(targetNorm, "reverbmix") == 0) {
        return getByWireId(1025u); // reverb.wet
    }
    if (std::strcmp(targetNorm, "delaymix") == 0) {
        return getByWireId(772u); // delay.wet
    }
    if (std::strcmp(targetNorm, "modulationmix") == 0) {
        return getByWireId(1538u); // chorus.mix
    }
    if (std::strcmp(targetNorm, "modulationmode") == 0) {
        return getByWireId(1537u); // chorus.mode
    }
    if (std::strcmp(targetNorm, "modulationenable") == 0) {
        return getByWireId(1536u); // chorus.enable
    }

    char candNorm[64];
    for (size_t i = 0; i < kParamCount; ++i) {
        const ParamDescriptor& d = kParamDescriptors[i];

        normalizeName(d.semanticName, candNorm, sizeof(candNorm));
        if (std::strcmp(targetNorm, candNorm) == 0) return &d;

        normalizeName(d.key, candNorm, sizeof(candNorm));
        if (std::strcmp(targetNorm, candNorm) == 0) return &d;

        if (d.dspBinding && d.dspBinding[0] != '\0') {
            normalizeName(d.dspBinding, candNorm, sizeof(candNorm));
            if (std::strcmp(targetNorm, candNorm) == 0) return &d;
        }
    }

    return nullptr;
}

int ParameterRegistry::wireIdToDenseIndex(uint16_t wireId) {
    for (size_t i = 0; i < kParamCount; ++i) {
        if (kParamDescriptors[i].wireId == wireId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

uint16_t ParameterRegistry::denseIndexToWireId(size_t index) {
    if (index >= kParamCount) return 0;
    return kParamDescriptors[index].wireId;
}

ParameterValue ParameterRegistry::getDefaultValue(size_t index) {
    if (index >= kParamCount) return ParameterValue();
    const ParamDescriptor& d = kParamDescriptors[index];
    switch (d.type) {
        case ParamType::Bool:
            return ParameterValue::makeBool(d.defaultValue > 0.5f);
        case ParamType::Int:
            return ParameterValue::makeInt(static_cast<int32_t>(d.defaultValue));
        case ParamType::Enum:
            return ParameterValue::makeEnum(static_cast<int32_t>(d.defaultValue));
        case ParamType::Float:
            return ParameterValue::makeFloat(d.defaultValue);
    }
    return ParameterValue();
}

bool ParameterRegistry::validateAndClamp(const ParamDescriptor& desc, ParameterValue& inOut) {
    switch (desc.type) {
        case ParamType::Bool: {
            bool b = inOut.asBool();
            inOut = ParameterValue::makeBool(b);
            return true;
        }
        case ParamType::Int: {
            int32_t v = inOut.asInt();
            if (v < static_cast<int32_t>(desc.minValue)) v = static_cast<int32_t>(desc.minValue);
            if (v > static_cast<int32_t>(desc.maxValue)) v = static_cast<int32_t>(desc.maxValue);
            inOut = ParameterValue::makeInt(v);
            return true;
        }
        case ParamType::Enum: {
            int32_t v = inOut.asInt();
            if (v < static_cast<int32_t>(desc.minValue)) v = static_cast<int32_t>(desc.minValue);
            if (v > static_cast<int32_t>(desc.maxValue)) v = static_cast<int32_t>(desc.maxValue);
            inOut = ParameterValue::makeEnum(v);
            return true;
        }
        case ParamType::Float: {
            float f = inOut.asFloat();
            if (std::isnan(f)) return false;
            if (f < desc.minValue) f = desc.minValue;
            if (f > desc.maxValue) f = desc.maxValue;
            inOut = ParameterValue::makeFloat(f);
            return true;
        }
    }
    return false;
}
