// ====================
// Copyright (c) 2024 cactoes. All rights reserved.
// MIT License
// ====================

#ifndef __HTMLPARSER_HPP__
#define __HTMLPARSER_HPP__

#include <string>
#include <vector>
#include <map>
#include <optional>

namespace parser {
    class HTMLElement {
    public:
        HTMLElement(const std::string& _tag) : tag(_tag) {};

        std::optional<HTMLElement*> GetElementById(const std::string& idName);
        std::vector<HTMLElement*> GetElementsByClassName(const std::string& className);
        std::vector<HTMLElement*> GetElementsByTagName(const std::string& tagName);

    public:
        std::string tag;
        std::string inner;
        std::map<std::string, std::string> attributes;
        std::vector<HTMLElement> children;
        std::vector<std::string> classList;
        std::string id;
        std::optional<HTMLElement*> parent;
    };

    HTMLElement ParseHTML(const std::string& htmlString);
}; // parser

#endif // __HTMLPARSER_HPP__