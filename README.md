# imflamegraphs
Flame Graphs Widget based on ImGui.

![flame_graph_02](https://github.com/HuCoco/imflamegraphs/blob/main/images/image01.png?raw=true)

![flame_graph_01](https://github.com/HuCoco/imflamegraphs/blob/main/images/image02.png?raw=true)

---

## Features

- WIP

---

## Example

```c++

class Data
{
    const char* name;
    float value;
}

template<>
class ImFlameGraphItemType<Data> : public ImFlameGraphItem
{
public:
    ...

    virtual const char* Label() const
    {
        return data.name;
    }

    virtual float Percentage() const
    {
        return data.value;
    }

private:
    Data* data;
}

int main()
{
    Data d1;
    Data d2;
    ...
    Data dn;

...
    ImFlameGraphData FlameGraphData;
    ImFlameGraphItem* Root = FlameGraphData.GetRootItem();
    ImFlameGraphItem* Child = Root.AddChild(d1);
    Child = Child.AddChild(d2);
    Child = Child.AddNext(d3);
    ...
...
    ImGui::Begin("Flame Graphs");   
    ImFlameGraphs::FlameGraph("FlameGraphs", &FlameGraphData, ImGui::GetContentRegionAvail().x, 16);
    ImGui::End();
...
}
```

