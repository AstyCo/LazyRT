#ifndef SPLITTED_STRING_HPP
#define SPLITTED_STRING_HPP

#include "extensions/help_functions.hpp"

#include <sstream>
#include <iterator>
#include <string>
#include <list>

template <typename THashedString>
class SplittedString
{
public:
    typedef THashedString HashedType;
    typedef std::list<THashedString> SplittedType;

public:
    SplittedString(const std::string &joint_ = std::string(), const std::string &separator_ = std::string("/"))
        : _joint(joint_), _separator(separator_)
    {
        init();
    }

    SplittedString(const SplittedType &splitted_)
        : _splitted(splitted_)
    {
        init();
    }

    void prepend(const THashedString &s)
    {
        if (_isJointValid)
            clearJoint();

        if (!_isSplittedValid)
            _isSplittedValid = true;

        _splitted.push_front(s);
    }

    void append(const THashedString &s)
    {
        if (!_isSplittedValid)
            split();
        clearJoint();

        if (_splitted.size() == 1 && _splitted.front().empty())
            _splitted.clear();
        _splitted.push_back(s);
    }

    bool empty() const
    {
        return !_isSplittedValid && !_isJointValid;
    }

    const std::list<THashedString> &splitted() const
    {
        if (!_isSplittedValid)
            split();
        return _splitted;
    }
    const std::string &joint() const
    {
        if (!_isJointValid)
            join();
        return _joint;
    }

    const char *c_str() const { return joint().c_str();}

    const THashedString &last() const
    {
        if (!_isSplittedValid)
            split();
        if (_splitted.empty())
            return emptyString();
        return _splitted.back();
    }

    SplittedString operator+(const SplittedString &extra_path) const
    {
        std::string joint_concat;
        if (joint().empty())
            joint_concat = extra_path.joint();
        else if (extra_path.joint().empty())
            joint_concat = _joint;
        else
            joint_concat = _joint + _separator + extra_path._joint;
        return SplittedString<THashedString>(joint_concat, _separator);
    }
    bool operator<(const SplittedString<THashedString> &other) const
    {
        return joint() < other.joint();
    }

    void setSeparator(const std::string &sep)
    {
        _separator = sep;
        _separatorSize = _separator.size();
        if (_isJointValid && !_isSplittedValid)
            split();
        clearJoint();
    }


    static const std::string &namespaceSep()
    {
        static std::string sep("::");
        return sep;
    }

    static const std::string &osSep()
    {
        static const char osSep = osSeparator();
        static std::string sep(std::string(&osSep, 1));
        return sep;
    }

    static const std::string &unixSep()
    {
        static std::string sep("/");
        return sep;
    }


    void setNamespaceSeparator() { setSeparator(namespaceSep());}
    void setOsSeparator() { setSeparator(osSep());}
    void setUnixSeparator() { setSeparator(unixSep());}

    void clear()
    {
        clearJoint();
        clearSplitted();
    }

private:
    const THashedString &emptyString() const
    {
        static const THashedString _e = THashedString(std::string());
        return _e;
    }

    void clearJoint()
    {
        if (_isJointValid) {
            _isJointValid = false;
            _joint.clear();
        }
    }

    void join() const
    {
        MY_ASSERT(!_isJointValid);
        size_t totalSize = 0;
        for (THashedString &sw : _splitted)
            totalSize += sw.size() + _separatorSize;

        if (!_splitted.empty())
            totalSize -= _separatorSize;

        _joint.reserve(totalSize);
        bool first = true;
        for (THashedString &sw : _splitted) {
            if (!first)
                _joint += _separator;
            else
                first = false;
            _joint += sw;
        }

        _isJointValid = true;
    }

    void clearSplitted()
    {
        if (_isSplittedValid) {
            _isSplittedValid = false;
            _splitted.clear();
        }
    }

    void split() const
    {
        MY_ASSERT(!_isSplittedValid);
        _isSplittedValid = true;
        if (_joint.empty())
            return;
        size_t pos = 0, sepPos;
        while ((sepPos = _joint.find(_separator, pos)) != (size_t)-1) {
            _splitted.push_back(_joint.substr(pos, sepPos - pos));

            pos = sepPos + _separatorSize;
        }
        _splitted.push_back(_joint.substr(pos));
    }

    void init()
    {
        if (_separator.empty()) {
            _separator = std::string("/");
        }
        _separatorSize = _separator.size();

        _isJointValid = !_joint.empty();
        _isSplittedValid = !_splitted.empty();
    }

public:
    mutable bool _isJointValid;
    mutable bool _isSplittedValid;

    mutable bool _isEmpty;
protected:
    mutable std::string _joint;
    mutable SplittedType _splitted;

    std::string _separator;
    size_t _separatorSize;
};

class HashedString: public std::string
{
public:
    typedef MurmurHashType HashType;
public:
    HashedString(const std::string &str);

    MurmurHashType hash() const;

    bool operator==(const HashedString &other) const { return hash() == other.hash();}
    bool operator!=(const HashedString &other) const { return !(*this == other);}
protected:
    mutable MurmurHashType _hash;
};

class HashedFileName : public HashedString
{
public:
    HashedFileName(const std::string &str);

    bool isDot() const { return hash() == _hashDot;}
    bool isDotDot() const { return hash() == _hashDotDot;}
    bool isEmpty() const { return length() == 0;}

    static const MurmurHashType _hashDot;
    static const MurmurHashType _hashDotDot;
};

typedef SplittedString<HashedFileName> ScopedName;
typedef SplittedString<HashedFileName> SplittedPath;

#endif // SPLITTED_STRING_HPP

