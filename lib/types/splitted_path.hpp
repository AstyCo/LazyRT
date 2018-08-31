#ifndef SPLITTED_PATH_HPP
#define SPLITTED_PATH_HPP

#include "extensions/help_functions.hpp"

#include <string>
#include <list>

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

class SplittedPath
{
public:
    SplittedPath(const std::string &path = std::string());

    const std::list<HashedFileName> &splittedPath() const;
    const std::string &string() const { return _string;}
    const char *c_str() const { return _string.c_str();}

    void calculateSplittedPath() const;

    SplittedPath operator+(const SplittedPath &extra_path) const;

    const HashedFileName &filename() const { return splittedPath().back(); }

//    ListFileNames::iterator find(const SplittedPath &root) const;
private:
    std::string _string;

    typedef std::list<HashedFileName> ListFileNames;

    mutable bool _isCalculatedSplittedPath;
    mutable ListFileNames _splittedPath;
};


#endif // SPLITTED_PATH_HPP

