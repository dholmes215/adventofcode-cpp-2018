// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include <cstdint>
#include <iostream>
#include <vector>

std::uint8_t ReadNumber(std::istream & stream)
{
    std::size_t number = 0;
    stream >> number;
    return number;
}

class TreeNode
{
public:
    TreeNode(std::istream & stream);
    void Print(std::ostream & stream) const;
    std::uint32_t MetadataEntrySum() const;
    std::uint32_t TreeMetadataEntrySum() const;
    std::uint32_t Value() const;

private:
    void PrintLevel(std::ostream & stream, std::uint8_t level) const;
    std::vector<TreeNode> childNodes;
    std::vector<std::uint8_t> metadataEntries;
};

TreeNode::TreeNode(std::istream & stream) : childNodes(), metadataEntries()
{
    const int childNodeCount = ReadNumber(stream);
    const int metadataEntryCount = ReadNumber(stream);
    for (int i = 0; i < childNodeCount; i++) {
        childNodes.emplace_back(stream);
    }
    for (int i = 0; i < metadataEntryCount; i++) {
        metadataEntries.push_back(ReadNumber(stream));
    }
}

void TreeNode::Print(std::ostream & stream) const { PrintLevel(stream, 0); }

void TreeNode::PrintLevel(std::ostream & stream, std::uint8_t level) const
{

    for (std::uint8_t l = 0; l < level; l++) {
        stream << "  ";
    }
    for (auto e : metadataEntries) {
        stream << static_cast<int>(e) << ' ';
    }
    std::cout << '\n';
    for (auto & c : childNodes) {
        c.PrintLevel(stream, level + 1);
    }
}

std::uint32_t TreeNode::MetadataEntrySum() const
{
    std::uint32_t sum = 0;
    for (auto e : metadataEntries) {
        sum += e;
    }
    return sum;
}

std::uint32_t TreeNode::TreeMetadataEntrySum() const
{
    std::uint32_t sum = MetadataEntrySum();
    for (auto & c : childNodes) {
        sum += c.TreeMetadataEntrySum();
    }
    return sum;
}

std::uint32_t TreeNode::Value() const
{
    if (childNodes.empty()) {
        return MetadataEntrySum();
    }

    std::uint32_t sum = 0;
    for (auto e : metadataEntries) {
        std::size_t childIndex = e - 1;
        if (childIndex < childNodes.size()) {
            sum += childNodes.at(e - 1).Value();
        }
    }
    return sum;
}

int main(int /*argc*/, char ** /*argv*/)
{
    const TreeNode license(std::cin);
    license.Print(std::cout);
    std::cout << "Entry sum: " << license.TreeMetadataEntrySum() << '\n';
    std::cout << "Value: " << license.Value() << '\n';
    return 0;
}
