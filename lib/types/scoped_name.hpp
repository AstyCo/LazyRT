#ifndef SCOPED_NAME_HPP
#define SCOPED_NAME_HPP

#include <string>
#include <list>

struct ScopedName
{
    std::list<std::string> data;

    void clear() { data.clear();}
    const std::string &name() const;
    bool hasNamespace() const;
    void pushScopeOrName(const std::string &word) { data.push_back(word);}
    bool isEmpty() const { return data.empty();}
    std::string fullname() const;

    static const std::string emptyName;
};


#endif // SCOPED_NAME_HPP
