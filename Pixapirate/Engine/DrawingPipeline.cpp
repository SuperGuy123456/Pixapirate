#include "DrawingPipeline.h"

void DrawLayer::AddDrawCall(Load* load, int priority)
{
    drawcalls[priority].push_back(load);
    objectPriority[load] = priority;
}

void DrawLayer::RemoveDrawCall(Load* load)
{
    auto it = objectPriority.find(load);
    if (it != objectPriority.end())
    {
        int priority = it->second;
        auto& vec = drawcalls[priority];
        vec.erase(std::remove(vec.begin(), vec.end(), load), vec.end());
        if (vec.empty()) {
            drawcalls.erase(priority);
        }
        objectPriority.erase(it);
    }
}

void DrawLayer::ChangePriority(Load* load, int newPriority)
{
    RemoveDrawCall(load);
    AddDrawCall(load, newPriority);
}
// ------------------- DrawingPipeline -------------------
void DrawingPipeline::AddLayer(DrawLayer* layer, const std::string& id)
{
    layers[id] = layer;
}

void DrawingPipeline::RemoveLayer(const std::string& id)
{
    layers.erase(id);
}

void DrawingPipeline::DrawAll()
{
    for (auto& layerpair : layers)
    {
        DrawLayer* layer = layerpair.second;

        // iterate priorities in ascending order
        for (auto& [priority, loads] : layer->drawcalls)
        {
            for (Load* drawcall : loads)
            {
                drawcall->Draw();
            }
        }
    }
}