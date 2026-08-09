/*
** Command & Conquer Renegade(tm)
** Copyright 2025 Electronic Arts Inc.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
*/

#include "AudibleSound.h"
#include "agg_def.h"
#include "soundrobj.h"

#include <cstdio>
#include <memory>
#include <type_traits>
#include <utility>

static_assert(std::is_same_v<
    decltype(std::declval<const AggregateDefClass &>().Get_Base_Model_Name()),
    const char *>);
static_assert(std::is_same_v<
    decltype(std::declval<const SoundRenderObjDefClass &>().Peek_Sound_Definition()),
    const AudibleSoundDefinitionClass *>);
static_assert(std::is_same_v<
    decltype(std::declval<const AudibleSoundDefinitionClass &>().Get_Priority()),
    float>);
static_assert(std::is_same_v<
    decltype(std::declval<const AudibleSoundDefinitionClass &>().Get_Loop_Count()),
    int>);
static_assert(std::is_same_v<
    decltype(std::declval<const AudibleSoundDefinitionClass &>().Is_3D()),
    bool>);
static_assert(std::is_same_v<
    decltype(std::declval<const AudibleSoundDefinitionClass &>().Get_Type()),
    int>);

namespace
{
bool Check(bool condition, const char *message)
{
    if (!condition) {
        std::fprintf(stderr, "%s\n", message);
    }
    return condition;
}

struct ReleaseSoundDefinition
{
    void operator()(SoundRenderObjDefClass *definition) const
    {
        if (definition != nullptr) {
            definition->Release_Ref();
        }
    }
};
}

int main()
{
    AggregateDefClass aggregate;
    const char *base_model_name = aggregate.Get_Base_Model_Name();
    if (!Check(base_model_name != nullptr && base_model_name[0] == '\0',
               "default aggregate base-model name was not an empty string")) {
        return 1;
    }

    std::unique_ptr<SoundRenderObjDefClass, ReleaseSoundDefinition> render_definition(
        new SoundRenderObjDefClass);
    const AudibleSoundDefinitionClass *definition =
        render_definition->Peek_Sound_Definition();
    if (!Check(definition != nullptr,
               "sound render-object definition did not expose its embedded definition")) {
        return 2;
    }

    if (!Check(definition->Get_Priority() == 0.5F,
               "default sound priority was not exposed") ||
        !Check(definition->Get_Loop_Count() == 1,
               "default sound loop count was not exposed") ||
        !Check(definition->Is_3D(),
               "default sound 3D state was not exposed") ||
        !Check(definition->Get_Type() == AudibleSoundClass::TYPE_SOUND_EFFECT,
               "default sound type was not exposed")) {
        return 3;
    }

    return 0;
}
