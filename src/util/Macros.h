#pragma once

#define CW_IS_GEODE_THEME(var)                                      \
    auto gode = geode::Loader::get()->getLoadedMod("geode.loader"); \
    var = gode->getSettingValue<std::string>("used-theme") != "Geometry Dash"