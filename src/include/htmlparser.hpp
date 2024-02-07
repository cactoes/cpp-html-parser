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
        HTMLElement(const std::wstring& tag);

        std::optional<HTMLElement*> GetElementById(const std::wstring& idName);
        std::vector<HTMLElement*> GetElementsByClassName(const std::wstring& className);
        std::vector<HTMLElement*> GetElementsByTagName(const std::wstring& tagName);

    public:
        std::wstring tag;
        std::wstring inner;
        std::map<std::wstring, std::wstring> attributes;
        std::vector<HTMLElement> children;
        std::vector<std::wstring> classList;
        std::wstring id;
        std::optional<HTMLElement*> parent;
    };

    HTMLElement ParseHTML(const std::wstring& htmlString);
}; // parser

#endif // __HTMLPARSER_HPP__