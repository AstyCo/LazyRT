#include "dependency_analyzer.hpp"


HashedStringNode::HashedStringNode(const HashedString &hs_)
    : hs(hs_), parent(nullptr)
{

}

HashedStringNode::~HashedStringNode()
{
    // memory free
    for (auto const &m: childs)
        delete m.second;
}

HashedStringNode *HashedStringNode::findOrNew(const HashedString &key)
{
    if (HashedStringNode *node = find(key))
        return node;

    HashedStringNode *node = new HashedStringNode(key);
    node->parent = this;
    childs.insert(std::make_pair(key.hash(), node));
    return node;
}

HashedStringNode *HashedStringNode::find(const HashedString &key) const
{
    auto it = childs.find(key.hash());
    if (it == childs.end())
        return nullptr;
    return it->second;
}

HashedStringNode *HashedStringNode::findSplitted(const HashedStringNode::TSplittedString &splittedString)
{
    HashedStringNode *current_node = this;
    for (const auto &s: splittedString.splitted()) {
//        if (current_node) {
//            std::cout << current_node->hs << std::string(" search for ") << s << std::endl;
//            std::cout << current_node->hs.hash() << ' ' << s.hash() << std::endl;
//            for (const auto &x: current_node->childs) {
//                std::cout << std::string("---") << x.first << ' ' << '('
//                          << x.second->hs << ',' << x.second->hs.hash() << ')' << std::endl;
//            }
//        }
        current_node = current_node->find(s);
//        std::cout << (current_node ? std::string("found") : std::string("not found")) << std::endl;
        if (current_node == nullptr)
            return nullptr;
    }
    return current_node;
}

HashedStringNode::TSplittedString HashedStringNode::fullname() const
{
    TSplittedString result;
    result.setSeparator("::");

    const HashedStringNode *n = this;
    while (n) {
        result.prepend(n->hs);
        n = n->parent;
    }
    return result;
}

void HashedStringNode::print(int indent)
{
    std::string strIndents = makeIndents(indent, 0);

    std::cout << strIndents << hs << std::endl;
    for (const auto &fn: data)
        std::cout << strIndents << "-- " << fn->record()._path.joint() << std::endl;

    for (const auto &x: childs)
        x.second->print(indent + 1);
}

void HashedStringNode::insert(const HashedStringNode::TSplittedString &splittedString, FileNodePtr fnode)
{
    HashedStringNode *current_node = this;
    for (const auto &s: splittedString.splitted())
        current_node = current_node->findOrNew(s);

    current_node->data.push_front(fnode);
}

DependencyAnalyzer::DependencyAnalyzer()
    : _rootClassDecls(std::string("CLASSES")), _rootFuncDecls(std::string("GLOBAL FUNCTIONS"))
{

}

void DependencyAnalyzer::setRoot(FileNode *fnode)
{
    if (!fnode)
        return;

    readDecls(fnode);
    analyzeImpls(fnode);
}

void DependencyAnalyzer::print()
{
    /// DEBUG
    _rootClassDecls.print();
    _rootFuncDecls.print();
}

void DependencyAnalyzer::analyzeImpl(const ScopedName &impl, FileNode *fnode)
{
    HashedStringNode *associatedDecl;

    if (associatedDecl = _rootFuncDecls.findSplitted(impl)) {
        // global function implementation
        addFunctionImpl(impl, fnode, associatedDecl);
        return;
    }
    if (associatedDecl = findClassForMethod(impl, fnode, &_rootClassDecls)) {
        // no namespace
        addClassImpl(impl, fnode, associatedDecl);
        return;
    }
    // check for method implementation
    // according to using namespaces
    for (auto &ns: fnode->record()._listUsingNamespace) {
        if (associatedDecl = findClassForMethod(impl, fnode, _rootClassDecls.findSplitted(ns))) {
            addClassImpl(impl, fnode, associatedDecl);
            return;
        }
    }
}

HashedStringNode *DependencyAnalyzer::findClassForMethod(const ScopedName &impl, FileNode *fnode, HashedStringNode *hsnode)
{
    if (nullptr == hsnode)
        return nullptr;
    int size = impl.splitted().size();
    int i = 0;
    for (const auto &s: impl.splitted()) {
        if (++i == size) // last
            break; // omit function/member name
        hsnode = hsnode->find(s);
        if (hsnode == nullptr)
            return nullptr;
    }

    return hsnode;
}

void DependencyAnalyzer::addFunctionImpl(const ScopedName &impl, FileNode *implNode, HashedStringNode *hsnode)
{
    auto &listNodes = hsnode->data;
    for (auto &node: listNodes)
        implNode->record()._setFuncImpl.insert(node->record()._path.joint());
}

void DependencyAnalyzer::addClassImpl(const ScopedName &impl, FileNode *implNode, HashedStringNode *hsnode)
{
    auto &listNodes = hsnode->data;
    for (auto &node: listNodes)
        implNode->record()._setClassImpl.insert(node->record()._path.joint());
}

void DependencyAnalyzer::readDecls(FileNode *fnode)
{
    auto &listClassDecls = fnode->record()._listClassDecl;
    auto &listFuncDecls = fnode->record()._listFuncDecl;

    for (const auto &hs: listClassDecls)
        _rootClassDecls.insert(hs, fnode);
    for (const auto &hs: listFuncDecls)
        _rootFuncDecls.insert(hs, fnode);

    for (const auto &chnode: fnode->childs())
        readDecls(chnode);
}

void DependencyAnalyzer::analyzeImpls(FileNode *fnode)
{
//    auto &listClassImpls = fnode->record()._listClassImpl;
//    auto &listFuncImpls = fnode->record()._listFuncImpl;

    auto &listImpls = fnode->record()._listImpl;

    for (const auto &impl: listImpls)
        analyzeImpl(impl, fnode);

    for (const auto &chnode: fnode->childs())
        analyzeImpls(chnode);
}
