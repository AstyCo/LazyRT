#include "types/scoped_name.hpp"

const std::string ScopedName::emptyName = std::string();

const std::string &ScopedName::name() const
{
    auto listSize = data.size();
    if (listSize > 0)
        return data.back();

    return emptyName;
}

bool ScopedName::hasNamespace() const
{
    return data.size() > 1;
}

std::string ScopedName::fullname() const
{
    std::string result;
    bool first = true;
    for (auto &word : data) {
        if (first)
            first = false;
        else
            result.append(std::string("::"));
        result.append(word);
    }

    return result;
}
