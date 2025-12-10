#pragma once
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>

#include "../BaseClasses/Base.h"   // already defines Load
#include "Allstructs.h"

class DrawLayer {
public:
    // priority → list of objects
    std::map<int, std::vector<Load*>> drawcalls;

    // object → current priority
    std::unordered_map<Load*, int> objectPriority;

    void AddDrawCall(Load* load, int priority);
    void RemoveDrawCall(Load* load);
    void ChangePriority(Load* load, int newPriority);
};
class DrawingPipeline
{
private:
    // layers keyed by string id
    std::map<std::string, DrawLayer*> layers;

public:
    void AddLayer(DrawLayer* layer, const std::string& id);
    void RemoveLayer(const std::string& id);

    void DrawAll();
};