#include "ui/params/UiParamState.h"

#include <cmath>

namespace {

float clamp_to_descriptor(const ParamDescriptor* d, float value) {
    if (d == nullptr) return value;
    if (!std::isfinite(value)) return d->defaultValue;

    ParameterValue v;
    switch (d->type) {
        case ParamType::Bool:
            v = ParameterValue::makeBool(value > 0.5f);
            break;
        case ParamType::Int:
            v = ParameterValue::makeInt(static_cast<int32_t>(std::lround(value)));
            break;
        case ParamType::Enum:
            v = ParameterValue::makeEnum(static_cast<int32_t>(std::lround(value)));
            break;
        case ParamType::Float:
        default:
            v = ParameterValue::makeFloat(value);
            break;
    }
    ParameterRegistry::validateAndClamp(*d, v);
    return v.asFloat();
}

} // namespace

int UiParamState::dense(uint16_t wireId) {
    return ParameterRegistry::wireIdToDenseIndex(wireId);
}

void UiParamState::resetToDefaults() {
    for (size_t i = 0; i < kCount; ++i) {
        const float def = ParameterRegistry::getDefaultValue(i).asFloat();
        values_[i] = def;
        authoritative_[i] = def;
        valid_[i] = true;
        authority_[i] = UiValueAuthority::LocalDefault;
    }
}

float UiParamState::get(uint16_t wireId) const {
    const int idx = dense(wireId);
    if (idx < 0) return 0.0f;
    return values_[idx];
}

float UiParamState::authoritative(uint16_t wireId) const {
    const int idx = dense(wireId);
    if (idx < 0) return 0.0f;
    return authoritative_[idx];
}

bool UiParamState::isValid(uint16_t wireId) const {
    const int idx = dense(wireId);
    if (idx < 0) return false;
    return valid_[idx];
}

UiValueAuthority UiParamState::authority(uint16_t wireId) const {
    const int idx = dense(wireId);
    if (idx < 0) return UiValueAuthority::LocalDefault;
    return authority_[idx];
}

void UiParamState::setLocal(uint16_t wireId, float value) {
    const int idx = dense(wireId);
    if (idx < 0) return;
    const ParamDescriptor* d = ParameterRegistry::getByWireId(wireId);
    values_[idx] = clamp_to_descriptor(d, value);
    valid_[idx] = true;
    authority_[idx] = UiValueAuthority::LocalPending;
}

void UiParamState::applyAuthoritative(uint16_t wireId, float value) {
    const int idx = dense(wireId);
    if (idx < 0) return;
    const ParamDescriptor* d = ParameterRegistry::getByWireId(wireId);
    const float clamped = clamp_to_descriptor(d, value);
    values_[idx] = clamped;
    authoritative_[idx] = clamped;
    valid_[idx] = true;
    authority_[idx] = UiValueAuthority::Authoritative;
}

void UiParamState::revert(uint16_t wireId) {
    const int idx = dense(wireId);
    if (idx < 0) return;
    values_[idx] = authoritative_[idx];
    authority_[idx] = UiValueAuthority::Authoritative;
}
