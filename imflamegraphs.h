// The MIT License(MIT)
//
// Copyright(c) 2024 Hu Ke
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#include "imgui.h"

template<typename T>
class ImFlameGraphItemType;

class ImFlameGraphItem
{
public:
    friend class ImFlameGraphData;

public:
    ImFlameGraphItem();
    virtual ~ImFlameGraphItem();

    template<typename T>
    ImFlameGraphItem* AddNext(T* data)
    {
        return ImFlameGraphItem::AddNextInternal(new ImFlameGraphItemType<T>(data));
    }

    template<typename T>
    ImFlameGraphItem* AddChild(T* data)
    {
        return ImFlameGraphItem::AddChildInternal(new ImFlameGraphItemType<T>(data));
    }

    virtual void Build();
    virtual bool Check();

    inline int Depth() const { return depth; }
    inline int Index() const { return index; }
    inline const ImFlameGraphItem* Next() const { return next_item; }
    inline const ImFlameGraphItem* ChildHead() const { return child_head_item; }
    inline const ImFlameGraphItem* Parent() const { return parent; }

    inline ImFlameGraphItem* Next() { return next_item; }
    inline ImFlameGraphItem* Prev() { return next_item; }
    inline ImFlameGraphItem* ChildHead() { return child_head_item; }
    inline ImFlameGraphItem* Parent() { return parent; }

    virtual const char* Label() const = 0;
    virtual float Percentage() const = 0;
    virtual ImGuiID Id() const = 0;

    inline float Start() const { return start; }
    inline float End() const { return end; }
protected:
    ImFlameGraphItem* AddNextInternal(ImFlameGraphItem* item);
    ImFlameGraphItem* AddChildInternal(ImFlameGraphItem* item);
    float PercentageToParent();
    float PercentageWidth();

protected:
    ImFlameGraphItem* next_item{ nullptr };
    ImFlameGraphItem* prev_item{ nullptr };
    ImFlameGraphItem* child_head_item{ nullptr };
    ImFlameGraphItem* parent{ nullptr };
    int depth{ 0 };
    int index{ 0 };

    float start{ 0.0f };
    float end{ 0.0f };
};

class ImFlameGraphData
{
public:
    ImFlameGraphData();
    virtual ~ImFlameGraphData();

    void Build();
    void SetDirty();
    template<typename ItemFn>
    void Foreach(ItemFn fn)
    {
        if (focus_item == nullptr)
        {
            focus_item = root;
        }
        ImFlameGraphItem* current_focus_item = focus_item;
        ImFlameGraphItem* item = nullptr;
        ImFlameGraphItem* foreach_parent_item = nullptr;
        bool foreach_parents = false;

        if (current_focus_item->ChildHead())
        {
            item = current_focus_item;
        }
        else
        {
            fn(current_focus_item, true);
            foreach_parent_item = current_focus_item;
            foreach_parents = true;
        }

        while (item)
        {
            fn(item, true);

            if (item->ChildHead())
            {
                item = item->ChildHead();
                continue;
            }
            else if (item->Next())
            {
                item = item->Next();
                continue;
            }
            else
            {
                item = item->Parent();
                while (item)
                {
                    if (item == current_focus_item)
                    {
                        foreach_parent_item = item;
                        foreach_parents = true;
                        item = nullptr;
                        break;
                    }

                    if (item->Next())
                    {
                        item = item->Next();
                        break;
                    }
                    else
                    {
                        item = item->Parent();
                    }
                }

                if (item)
                {
                    continue;
                }
            }
            break;
        }

        if (foreach_parent_item && foreach_parents)
        {
            foreach_parent_item = foreach_parent_item->Parent();
            while (foreach_parent_item)
            {
                fn(foreach_parent_item, false);
                foreach_parent_item = foreach_parent_item->Parent();
            }
        }
    }

    inline int MaxDepth() { return max_depth; }
    ImFlameGraphItem* GetRootItem() { return root; }
    void SetFocusItem(ImFlameGraphItem* item) { focus_item = item; }
    ImFlameGraphItem* GetFocusItem() { return focus_item; }
protected:
    ImFlameGraphItem* root;
    ImFlameGraphItem* focus_item;
    int max_depth{ 0 };
    bool items_dirty{ true };
};

namespace ImFlameGraphs
{
    //    Example:
    // 
    //    template<>
    //    class ImFlameGraphItemType<void> : public ImFlameGraphItem
    //    {
    //    public:
    //        ImFlameGraphItemType(void* data)
    //            : item_data(data)
    //        {
    //
    //        }
    //
    //        virtual ~ImFlameGraphItemType()
    //        {
    //
    //        }
    //
    //        virtual const char* Label() const
    //        {
    //            return "Void";
    //        }
    //
    //        virtual float Percentage() const
    //        {
    //            return 1.0f;
    //        }
    //
    //    private:
    //        void* item_data;
    //    };

    IMGUI_API void FlameGraph(const char* label, ImFlameGraphData* data, float width = 0, int depth = 0);
}

