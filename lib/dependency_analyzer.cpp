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
        current_node = current_node->find(s);
        if (current_node == nullptr)
            return nullptr;
    }
    return current_node;
}

HashedStringNode::TSplittedString HashedStringNode::fullname() const
{
    TSplittedString result;
    result.setNamespaceSeparator();

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
        std::cout << strIndents << "-- " << fn->name() << std::endl;

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
    analyze(fnode);
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
        addFunctionImpl(fnode, associatedDecl);
        return;
    }
    if (associatedDecl = findClassForMethod(impl, fnode, &_rootClassDecls)) {
        // no namespace
        addClassImpl(fnode, associatedDecl);
        return;
    }
    // check for method implementation
    // according to using namespaces
    for (auto &ns: fnode->record()._listUsingNamespace) {
        if (associatedDecl = findClassForMethod(impl, fnode, _rootClassDecls.findSplitted(ns))) {
            addClassImpl(fnode, associatedDecl);
            return;
        }
    }
}

void DependencyAnalyzer::analyzeInheritance(const ScopedName &baseClass, FileNode *fnode)
{
    HashedStringNode *associatedDecl;

    if (associatedDecl = findClass(baseClass, fnode, &_rootClassDecls)) {
        // no namespace
        addClassInheritance(fnode, associatedDecl);
        return;
    }
    // check for method implementation
    // according to using namespaces
    for (auto &ns: fnode->record()._listUsingNamespace) {
        if (associatedDecl = findClass(baseClass, fnode, _rootClassDecls.findSplitted(ns))) {
            addClassInheritance(fnode, associatedDecl);
            return;
        }
    }
}

HashedStringNode *DependencyAnalyzer::findClassForMethod(const ScopedName &impl, FileNode *fnode, HashedStringNode *hsnode)
{
    findScopedPrivate(impl, fnode, hsnode, SearchMethod);
}

HashedStringNode *DependencyAnalyzer::findClass(const ScopedName &impl, FileNode *fnode, HashedStringNode *hsnode)
{
    findScopedPrivate(impl, fnode, hsnode, SearchClass);
}

HashedStringNode *DependencyAnalyzer::findScopedPrivate(const ScopedName &impl, FileNode *fnode, HashedStringNode *hsnode, DependencyAnalyzer::SearchType st)
{
    if (nullptr == hsnode)
        return nullptr;
    const auto &splitted = impl.splitted();

    int i = 0;
    int size;
    switch (st) {
    case SearchClass:
        size = splitted.size();
        break;
    case SearchMethod:
        size = splitted.size() - 1;
        break;
    default:
        MY_ASSERT(false);
        return nullptr;
    }

    auto it = splitted.cbegin();
    for (int i  = 0; i < size; ++i, ++it) {
        hsnode = hsnode->find(*it);
        if (hsnode == nullptr)
            return nullptr;
    }

    return hsnode;
}

void DependencyAnalyzer::addFunctionImpl(FileNode *implNode, HashedStringNode *hsnode)
{
    auto &listNodes = hsnode->data;
    for (auto &node: listNodes) {
        implNode->record()._setImplementFiles.insert(node->path());
        implNode->record()._setFuncImplFiles.insert(node->path());
    }
}

void DependencyAnalyzer::addClassImpl(FileNode *implNode, HashedStringNode *hsnode)
{
    auto &listNodes = hsnode->data;
    for (auto &node: listNodes) {
        implNode->record()._setImplementFiles.insert(node->path());
        implNode->record()._setClassImplFiles.insert(node->path());
    }
}

void DependencyAnalyzer::addClassInheritance(FileNode *implNode, HashedStringNode *hsnode)
{
    auto &listNodes = hsnode->data;
    for (auto &node: listNodes)
        implNode->record()._setBaseClassFiles.insert(node->path());
}

void DependencyAnalyzer::readDecls(FileNode *fnode)
{
    auto &classDecls = fnode->record()._setClassDecl;
    auto &functionDecls = fnode->record()._setFuncDecl;

    for (const auto &hs: classDecls)
        _rootClassDecls.insert(hs, fnode);
    for (const auto &hs: functionDecls)
        _rootFuncDecls.insert(hs, fnode);

    for (const auto &chnode: fnode->childs())
        readDecls(chnode);
}

void DependencyAnalyzer::analyze(FileNode *fnode)
{
    auto &impls = fnode->record()._setImplements;
    auto &inheritances = fnode->record()._setInheritances;

    for (const auto &impl: impls)
        analyzeImpl(impl, fnode);
    for (const auto &inh: inheritances)
        analyzeInheritance(inh, fnode);

    for (const auto &chnode: fnode->childs())
        analyze(chnode);
}
