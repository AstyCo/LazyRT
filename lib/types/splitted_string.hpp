#ifndef SPLITTED_STRING_HPP
#define SPLITTED_STRING_HPP

#include "extensions/help_functions.hpp"

#include <sstream>
#include <iterator>
#include <string>
#include <list>

const char *get_home_dir();

class HashedString : public std::string
{
public:
    using HashType = MurmurHashType;

public:
    HashedString(const std::string &str);

    MurmurHashType hash() const;

    bool operator==(const HashedString &other) const
    {
        return hash() == other.hash();
    }
    bool operator!=(const HashedString &other) const
    {
        return !(*this == other);
    }

protected:
    mutable MurmurHashType _hash;
};

class HashedFileName : public HashedString
{
public:
    HashedFileName(const std::string &str);

    bool isDot() const { return hash() == _hashDot; }
    bool isDotDot() const { return hash() == _hashDotDot; }

    static const MurmurHashType _hashDot;
    static const MurmurHashType _hashDotDot;
};

template < typename THashedString >
class SplittedString
{
public:
    using Type = SplittedString< THashedString >;
    using HashedType = THashedString;
    using SplittedType = std::vector< THashedString >;

public:
    SplittedString();
    explicit SplittedString(const std::string &joint_,
                            const std::string &separator_);
    SplittedString(const SplittedType &splitted_);

    void prepend(const THashedString &s);

    void append(const THashedString &s);
    void appendPath(const SplittedString< THashedString > &extra_path);

    bool empty() const;
    bool isRelative() const;

    const std::string &separator() const { return _separator; }
    void setSeparator(const std::string &sep);

    const std::string &joint() const;
    const std::vector< THashedString > &splitted() const;

    const THashedString &last() const;

    void clear();
    void removeLast();
    void removeFirst();

    // Operators overloading
    SplittedString operator+(const SplittedString &extra_path) const;
    SplittedString operator+(const THashedString &str);
    bool operator==(const SplittedString< THashedString > &other) const;
    bool operator!=(const SplittedString< THashedString > &other) const;
    bool operator<(const SplittedString< THashedString > &other) const;

public:
    // Common methods
    std::string jointSep(const std::string &separator) const;
    std::string jointUnix() const { return jointSep(unixSep()); }
    std::string jointOs() const { return jointSep(osSep()); }

    void setNamespaceSeparator() { setSeparator(namespaceSep()); }
    void setOsSeparator() { setSeparator(osSep()); }
    void setUnixSeparator() { setSeparator(unixSep()); }

    const char *c_str() const { return joint().c_str(); }

public:
    // Static methods
    static const std::string &namespaceSep();
    static const std::string &osSep();
    static const std::string &unixSep();
    static const THashedString &emptyString();

private:
    void init();
    void join() const;
    void split() const;
    void clearJoint();
    void clearSplitted();
    void normalize();
    void removeHomeDirSign();
    void removeTrailingSeparators();

private:
    mutable bool _isJointValid;
    mutable bool _isSplittedValid;
    mutable bool _isEmpty;

protected:
    mutable std::string _joint;
    mutable SplittedType _splitted;

    std::string _separator;
    size_t _separatorSize;
};

using ScopedName = SplittedString< HashedFileName >;
using SplittedPath = SplittedString< HashedFileName >;

// result.separator() is base.separator()
SplittedPath relative_path(const SplittedPath &path_to_file,
                           const SplittedPath &base, bool *error = nullptr);
bool is_relative(const SplittedPath &path);
SplittedPath absolute_path(const SplittedPath &path, const SplittedPath &base);

// Explicit template specialization
template class SplittedString< HashedFileName >;

#endif // SPLITTED_STRING_HPP
