#pragma once

#include <vector>
#include <functional>
#include <unordered_map>
#include "Config.h"

class ConfigPanel {
public:
    explicit ConfigPanel(Config &config) : config(config) {}
    ~ConfigPanel() { destroy(); };
private:
    void destroy();

private:
  Config &config;
};
