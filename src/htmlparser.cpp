// ====================
// Copyright (c) 2024 cactoes. All rights reserved.
// MIT License
// ====================

#include "include/HTMLParser.hpp"

#include <ranges>
#include <regex>

std::vector<std::wstring> CreateTokenArray(const std::wstring& htmlString) {
    std::vector<std::wstring> tokenArray;
    std::wstring buffer;

    for (const auto& character : htmlString) {
        switch (character) {
            case '<':
                if (buffer.empty())
                    break;
                tokenArray.push_back(buffer);
                buffer.clear();
                break;
            case '>':
                buffer += character;
                tokenArray.push_back(buffer);
                buffer.clear();
                continue;
            default:
                break;
        }

        buffer += character;
    }

    return tokenArray; 
}

enum class TokenType {
    OPEN = 0,
    CLOSE,
    NO_CLOSING,
    NO_TAG
};

// http://xahlee.info/js/html5_non-closing_tag.html
bool IsVoidElement(const std::wstring& token) {
    if (token.starts_with(L"<area")) return true;
    if (token.starts_with(L"<base")) return true;
    if (token.starts_with(L"<br")) return true;
    if (token.starts_with(L"<col")) return true;
    if (token.starts_with(L"<embed")) return true;
    if (token.starts_with(L"<hr")) return true;
    if (token.starts_with(L"<img")) return true;
    if (token.starts_with(L"<input")) return true;
    if (token.starts_with(L"<link")) return true;
    if (token.starts_with(L"<meta")) return true;
    if (token.starts_with(L"<param")) return true;
    if (token.starts_with(L"<source")) return true;
    if (token.starts_with(L"<track")) return true;
    if (token.starts_with(L"<wbr")) return true;
    if (token.starts_with(L"<!")) return true;
    return false;
}

TokenType ParseToTokenType(const std::wstring& token) {
    if (token.starts_with(L"</"))
        return TokenType::CLOSE;

    if (token.starts_with(L"<") && token.ends_with(L">")) {
        if (IsVoidElement(token))
            return TokenType::NO_CLOSING;
        return TokenType::OPEN;
    }

    return TokenType::NO_TAG;
}

std::vector<std::wstring> Split(const std::wstring& s, const std::wstring& delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::wstring token;
    std::vector<std::wstring> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::wstring::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr(pos_start));
    return res;
}

std::vector<std::wstring> SplitRegex(const std::wstring& s, const std::wregex& sep_regex) {
    std::wsregex_token_iterator iter(s.begin(), s.end(), sep_regex, -1);
    std::wsregex_token_iterator end;
    return { iter, end };
}

std::vector<std::wstring> MatchAllRegex(const std::wstring& input, const std::wregex& pattern) {
    std::vector<std::wstring> matches;
    std::wsregex_iterator iter(input.begin(), input.end(), pattern);
    std::wsregex_iterator end;

    while (iter != end) {
        matches.push_back(iter->str());
        ++iter;
    }

    return matches;
}

void ReplaceAll(std::wstring& str, const std::wstring& value, const std::wstring& replaceValue = L"") {
    size_t pos;
    while ((pos = str.find(value)) != std::wstring::npos)
        str = str.replace(pos, value.size(), replaceValue);
}

std::pair<std::wstring, std::wstring> ParseAttributeValue(const std::wstring& attrib) {
    std::vector<std::wstring> attribParts = Split(attrib, L"=");

    if (attribParts.size() < 2ull)
        return { attribParts.at(0), L"" };

    ReplaceAll(attribParts.at(1), L"\"");
    
    return { attribParts.at(0), attribParts.at(1) };
}

std::vector<std::wstring> GetClassList(const std::map<std::wstring, std::wstring>& attributes) {
    std::vector<std::wstring> classList;

    if (attributes.contains(L"class"))
        classList = Split(attributes.at(L"class"), L" ");

    return classList;
}

std::wstring GetId(const std::map<std::wstring, std::wstring>& attributes) {
    if (attributes.contains(L"id"))
        return attributes.at(L"id");

    return L"";
}

parser::HTMLElement ParseAttributes(const std::wstring& token) {
    std::wstring tokenCpy = token;
    if (auto pos = tokenCpy.find(L"</"); pos != std::wstring::npos)
        tokenCpy = tokenCpy.replace(pos, 2, L"");
    else if (pos = tokenCpy.find(L"<"); pos != std::wstring::npos)
        tokenCpy = tokenCpy.replace(pos, 1, L"");

    if (auto pos = tokenCpy.find(L"/>"); pos != std::wstring::npos)
        tokenCpy = tokenCpy.replace(pos, 2, L"");
    else if (pos = tokenCpy.find(L">"); pos != std::wstring::npos)
        tokenCpy = tokenCpy.replace(pos, 1, L"");


    // https://regex101.com/
    // test strings:
    // 1. !DOCTYPE HTML
    // 2. tagname class="cl1 cl2 cl3-a cl3_b" id="test" asd zzxc d-ata-tag="2"
    auto attributes = MatchAllRegex(tokenCpy, std::wregex{ L"[^ ]+=\"[^\"]*\"|[^ ]+" });
    // auto attributes = Split(tokenCpy, " ");

    parser::HTMLElement newElement(L"");

    if (attributes.size() > 0) {
        newElement.tag = attributes.at(0);

        for (size_t i = 1; i < attributes.size(); i++) {
            const auto& attrib = attributes.at(i);
            if (attrib != L" " && !attrib.empty())
                newElement.attributes.emplace(ParseAttributeValue(attrib));
        }
    }

    newElement.classList = GetClassList(newElement.attributes);
    newElement.id = GetId(newElement.attributes);

    return newElement;
}

parser::HTMLElement parser::ParseHTML(const std::wstring& htmlString) {
    parser::HTMLElement document(L"document");
    parser::HTMLElement* current = &document;

    for (const std::wstring& token : CreateTokenArray(htmlString)) {
        switch (ParseToTokenType(token)) {
            case TokenType::OPEN: {
                parser::HTMLElement newElement = ParseAttributes(token);
                newElement.parent = current;
                current->children.push_back(newElement);
                current = &current->children.back();
                break;
            }
            case TokenType::CLOSE:
                current = current->parent.value();
                break;
            case TokenType::NO_CLOSING: {
                parser::HTMLElement newElement = ParseAttributes(token);
                newElement.parent = current;
                current->children.push_back(newElement);
                break;
            }
            case TokenType::NO_TAG: {
                parser::HTMLElement newElement(L"");
                newElement.inner = token;
                // quick fix to replace &#39; for its actual value
                ReplaceAll(newElement.inner, L"&#39;", L"'");
                newElement.parent = current;
                current->children.push_back(newElement);
                break;
            }
            default:
                break;
        }
    }

    return document;
}

std::vector<parser::HTMLElement*> RecursiveGetElementsByClassName(parser::HTMLElement* element, const std::wstring& className) {
    std::vector<parser::HTMLElement*> elements;

    for (auto& child : element->children) {
        if (std::ranges::find(child.classList, className) != child.classList.end())
            elements.push_back(&child);

        auto possibleNestedElemets = RecursiveGetElementsByClassName(&child, className);
        if (!possibleNestedElemets.empty())
            elements.insert(elements.end(), possibleNestedElemets.begin(), possibleNestedElemets.end());
    }

    return elements;
}

parser::HTMLElement* RecursiveGetElementById(parser::HTMLElement* element, const std::wstring& id) {
    for (auto& child : element->children) {
        if (child.id == id)
            return &child;

        if (auto e = RecursiveGetElementById(&child, id))
            return e;
    }

    return nullptr;
}

std::optional<parser::HTMLElement*> parser::HTMLElement::GetElementById(const std::wstring& idName) {
    parser::HTMLElement* element = RecursiveGetElementById(this, idName);

    if (!element)
        return std::nullopt;

    return element;
}

std::vector<parser::HTMLElement*> parser::HTMLElement::GetElementsByClassName(const std::wstring& className) {
    return RecursiveGetElementsByClassName(this, className);
}

std::vector<parser::HTMLElement*> RecursiveGetElementsByTagName(parser::HTMLElement* element, const std::wstring& tagName) {
    std::vector<parser::HTMLElement*> elements;

    for (auto& child : element->children) {
        if (child.tag == tagName)
            elements.push_back(&child);

        auto possibleNestedElemets = RecursiveGetElementsByTagName(&child, tagName);
        if (!possibleNestedElemets.empty())
            elements.insert(elements.end(), possibleNestedElemets.begin(), possibleNestedElemets.end());
    }

    return elements;
}

std::vector<parser::HTMLElement*> parser::HTMLElement::GetElementsByTagName(const std::wstring& tagName) {
    return RecursiveGetElementsByTagName(this, tagName);
}