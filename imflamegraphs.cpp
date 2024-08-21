#include "imflamegraphs.h"

#include "imgui.h"
#include "imgui_internal.h"

#define USE_CSTDINT 1

#if defined(USE_CSTDINT)
#include <cstdint>
#else
// to define int types.
#endif

#include <algorithm>


ImFlameGraphItem::ImFlameGraphItem()
{

}

ImFlameGraphItem::~ImFlameGraphItem()
{

}

void ImFlameGraphItem::Build()
{
    if (parent == nullptr)
    {
        start = 0.0f;
        end = 1.0f;
    }
    else
    {
        start = prev_item == nullptr ? parent->start : prev_item->end;
        end = start + parent->PercentageWidth() * PercentageToParent();
    }
}

bool ImFlameGraphItem::Check()
{
    if (parent)
    {
        if (Percentage() > parent->Percentage())
        {
            return false;
        }
    }
    else
    {
        if (Percentage() != 1.0)
        {
            return false;
        }
    }

    if (child_head_item != nullptr)
    {
        float all_childitems_percentage = 0.0f;
        ImFlameGraphItem* item = child_head_item;
        while (item)
        {
            all_childitems_percentage += item->Percentage();
            item = item->Next();
        }

        if (all_childitems_percentage > Percentage())
        {
            return false;
        }
    }

    return true;
}

ImFlameGraphItem* ImFlameGraphItem::AddNextInternal(ImFlameGraphItem* item)
{
    IM_ASSERT(next_item == nullptr);
    next_item = item;
    next_item->index = index + 1;
    next_item->depth = depth;
    next_item->parent = parent;
    next_item->prev_item = this;
    return item;
}

ImFlameGraphItem* ImFlameGraphItem::AddChildInternal(ImFlameGraphItem* item)
{
    if (child_head_item == nullptr)
    {
        child_head_item = item;
        item->parent = this;
        item->depth = depth + 1;
        item->index = 0;
        return item;
    }

    ImFlameGraphItem* curr_item = child_head_item;
    while (curr_item->next_item != nullptr)
    {
        curr_item = curr_item->next_item;
    }
    ImFlameGraphItem* new_item = curr_item->AddNextInternal(item);
    new_item->parent = this;
    IM_ASSERT(new_item->index == (curr_item->index + 1));
    IM_ASSERT(new_item->depth == curr_item->depth);
    IM_ASSERT(new_item->depth == (depth + 1));
    return new_item;
}

float ImFlameGraphItem::PercentageToParent()
{
    if (parent == nullptr)
    {
        return 1.0f;
    }

    return this->Percentage() / parent->Percentage();
}

float ImFlameGraphItem::PercentageWidth()
{
    return end - start;
}

class ImFlameGraphRootItem : public ImFlameGraphItem
{
public:
    ImFlameGraphRootItem();
    virtual ~ImFlameGraphRootItem();

    virtual const char* Label() const;
    virtual float Percentage() const;
    virtual ImGuiID Id() const;
};

ImFlameGraphRootItem::ImFlameGraphRootItem()
    : ImFlameGraphItem()
{
    start = 0.0f;
    end = 1.0f;
}

ImFlameGraphRootItem::~ImFlameGraphRootItem()
{

}

const char* ImFlameGraphRootItem::Label() const
{
    return "All";
}

float ImFlameGraphRootItem::Percentage() const
{
    return 1.0f;
}

ImGuiID ImFlameGraphRootItem::Id() const
{
    return (ImGuiID)reinterpret_cast<intptr_t>(this);
}

ImFlameGraphData::ImFlameGraphData()
    : root(new ImFlameGraphRootItem())
    , focus_item(root)
{

}

ImFlameGraphData::~ImFlameGraphData()
{

}

void ImFlameGraphData::Build()
{
    if (items_dirty)
    {
        max_depth = 0;

        Foreach(
            [this](ImFlameGraphItem* item, bool focus)
            {
                (void)focus;

                item->Check();
                item->Build();
                max_depth = std::max(item->Depth(), max_depth);
            });
        items_dirty = false;
    }
}

void ImFlameGraphData::SetDirty()
{
    items_dirty = true;
}

namespace ImFlameGraphs
{
    ImU32 GetFlameGraphColor(int index)
    {
        const int max_color_index = 16;
        const int max_color_index_half = max_color_index / 2;
        index = (index * 11) % max_color_index;

        float g = 0.25f + float(index) / float(max_color_index) * 0.5f;
        return ImColor(1.0f, g, 0.0f, 1.0f);
    }

    void FlameGraphEx(ImFlameGraphData* data, const ImVec2& size)
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        IM_ASSERT(window);
        if (window->SkipItems)
            return;

        const auto block_height = ImGui::GetTextLineHeight() + (g.Style.FramePadding.y * 2);
        const float max_width = size.x - window->ScrollbarSizes.x;

        data->Build();

        int32_t maxDepth = data->MaxDepth();
        for (int i = maxDepth; i >= 0 ; --i)
        {
            ImGui::ItemSize(ImVec2(max_width, block_height), 0);
        }

        ImVec2 origin = ImVec2(
            window->DC.CursorPos.x - window->WindowPadding.x,
            std::max(window->DC.CursorPos.y, window->Pos.y + size.y - g.Style.FramePadding.y));

        ImFlameGraphItem* base_item = data->GetFocusItem();
        float base_start = base_item->Start();
        float base_end = base_item->End();

        data->Foreach(
            [base_start, base_end, block_height, max_width, origin, window, data](ImFlameGraphItem* item, bool focus)
            {
                const ImVec2 padding = ImVec2(1.0f, 1.0f);
                const ImU32 col_base = GetFlameGraphColor(item->Depth() + item->Index()) & 0xAFFFFFFF;
                const ImU32 col_hovered = GetFlameGraphColor(item->Depth() + item->Index()) & 0xFFFFFFFF;
                const ImU32 col_nofocus = GetFlameGraphColor(item->Depth() + item->Index()) & 0x4FFFFFFF;
                const ImU32 col_outline_base = IM_COL32_BLACK & 0xAFFFFFFF;
                const ImU32 col_outline_hovered = IM_COL32_BLACK & 0xAFFFFFFF;

                float width = max_width;
                float height = block_height * item->Depth();
                float min_percentage = 8.0f / max_width;

                ImVec2 origin_offset = origin;
                origin_offset.y -= height;

                float scale = 1.0f / (base_end - base_start);
                float offset = base_start;

                float start = std::max(0.0f, (item->Start() - offset) * scale);
                float end = std::min(1.0f, (item->End() - offset) * scale);

                if ((end - start) < min_percentage)
                {
                    return;
                }

                auto pos0 = origin_offset + ImVec2(width * start,   -block_height   ) + padding;
                auto pos1 = origin_offset + ImVec2(width * end,     0.0f            ) - padding;

                ImRect bb(pos0, pos1);
                if (!ImGui::ItemAdd(bb, 0))
                    return;

                bool hovered = false;
                bool press = ImGui::ButtonBehavior(bb, item->Id(), &hovered, nullptr, ImGuiButtonFlags_PressedOnClick);
                window->DrawList->AddRectFilled(pos0, pos1, focus ? col_base : col_nofocus);
                if (hovered)
                {
                    window->DrawList->AddRect(pos0, pos1, col_outline_hovered);
                    if (ImGui::BeginTooltip())
                    {
                        ImGui::Text(item->Label());
                        ImGui::End();
                    }
                }
                if (press)
                {
                    data->SetFocusItem(item);
                }

                auto textSize = ImGui::CalcTextSize(item->Label());
                auto boxSize = (pos1 - pos0);
                auto textOffset = ImVec2(0.0f, 0.0f);
                if (textSize.x < boxSize.x)
                {
                    textOffset = ImVec2(0.5f, 0.5f) * (boxSize - textSize);
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_BLACK);
                    ImGui::RenderText(pos0 + textOffset, item->Label());
                    ImGui::PopStyleColor();
                }
            }
        );
    }

    void FlameGraph(const char* label, ImFlameGraphData* data, float width /*= 0*/, int depth /*= 0*/)
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        IM_ASSERT(window);
        IM_ASSERT(data);

        data->Build();

        const auto block_height = ImGui::GetTextLineHeight() + (g.Style.FramePadding.y * 2);
        const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

        if (width == 0.0) width = ImGui::GetContentRegionAvail().x;
        if (depth == 0) depth = data->MaxDepth();

        ImVec2 graph_size = ImVec2(width, depth * block_height);

        ImGuiID id = window->GetID(label);
        ImGui::BeginChildEx(label, id, graph_size, ImGuiChildFlags_Border, ImGuiWindowFlags_None);

        FlameGraphEx(data, graph_size);

        ImGui::EndChild();
    }
}
