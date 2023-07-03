#include  "job.h" 

DynamicVar JobAdditionalInfo::get(string name, DynamicVar defaultValue)
{
    for (auto &c: data)
        if (std::get<0>(c) == name)
            return std::get<1>(c);

    return defaultValue;
}

vector<DynamicVar> JobAdditionalInfo::getMultiple(string name, DynamicVar defaultValue)
{
    vector<DynamicVar> result;
    for (auto &c: data)
        if (std::get<0>(c) == name)
            result.push_back(std::get<1>(c));

    if(result.size() == 0)
        result.push_back(defaultValue);

    return result;
}

void JobAdditionalInfo::add(string name, DynamicVar value)
{
    this->data.push_back({name, value});
}

size_t JobAdditionalInfo::size()
{
    return data.size();
}

DynamicVar JobAdditionalInfo::operator[] (int index)
{
    if (index >= 0 && index < size())
        return std::get<1>(data[index]);
}

DynamicVar JobAdditionalInfo::operator[] (string name)
{
    return get(name, "");
}