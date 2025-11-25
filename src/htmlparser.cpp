// ====================
// Copyright (c) 2024 cactoes. All rights reserved.
// MIT License
// ====================

#include "include/htmlparser.hpp"
#include <ranges>
#include <sstream>

std::vector<std::string> CreateTokenArray(const std::string& htmlString) {
    std::vector<std::string> tokenArray;
    size_t token_begin = 0;
    size_t i = 0;

    for (char character : htmlString) {
        if(character == '<') {
            if (i - token_begin > 0){
                tokenArray.push_back(htmlString.substr(token_begin, i - token_begin));
                token_begin = i;
            }
        }
        else if(character == '>') {
            tokenArray.push_back(htmlString.substr(token_begin, i - token_begin + 1));
            token_begin = i + 1;
        }

        i++;
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
bool IsVoidElement(const std::string& token) {
    if (token.starts_with("<area")) return true;
    if (token.starts_with("<base")) return true;
    if (token.starts_with("<br")) return true;
    if (token.starts_with("<co")) return true;
    if (token.starts_with("<embed")) return true;
    if (token.starts_with("<hr")) return true;
    if (token.starts_with("<img")) return true;
    if (token.starts_with("<input")) return true;
    if (token.starts_with("<link")) return true;
    if (token.starts_with("<meta")) return true;
    if (token.starts_with("<param")) return true;
    if (token.starts_with("<source")) return true;
    if (token.starts_with("<track")) return true;
    if (token.starts_with("<wbr")) return true;
    if (token.starts_with("<!")) return true;
    if (token.ends_with("/>")) return true;
    return false;
}

TokenType ParseToTokenType(const std::string& token) {
    if (token.starts_with("</"))
        return TokenType::CLOSE;

    if (token.starts_with("<") && token.ends_with(">")) {
        if (IsVoidElement(token))
            return TokenType::NO_CLOSING;
        return TokenType::OPEN;
    }

    return TokenType::NO_TAG;
}

std::vector<std::string> Split(const std::string& s, const std::string& delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        res.push_back(s.substr(pos_start, pos_end - pos_start));
        pos_start = pos_end + delim_len;
    }

    res.push_back(s.substr(pos_start));
    return res;
}

void ReplaceAll(std::string& str, const std::string& value, const std::string& replacement = ""){
    size_t sub_begin = str.find(value);
    if(sub_begin == std::string::npos)
        return;
    
    const size_t value_len = value.size();
    size_t other_content_begin = 0;
    std::stringstream ss;
    do {
        ss<<std::string_view(str.data() + other_content_begin, sub_begin - other_content_begin); // add content before value
        ss<<replacement;
        other_content_begin = sub_begin + value_len;
        sub_begin = str.find(value, sub_begin + value_len); // find next value
    } while(sub_begin != std::string::npos);
    
    ss<<std::string_view(str.data() + other_content_begin);
    str = ss.str();
}

std::pair<std::string, std::string> ParseAttributeValue(const std::string& attrib) {
    std::vector<std::string> attribParts = Split(attrib, "=");

    if (attribParts.size() < 2ull)
        return { attribParts.at(0), "" };

    ReplaceAll(attribParts.at(1), "\"");
    
    return { attribParts.at(0), attribParts.at(1) };
}

std::vector<std::string> GetClassList(const std::map<std::string, std::string>& attributes) {
    std::vector<std::string> classList;

    if (attributes.contains("class"))
        classList = Split(attributes.at("class"), " ");

    return classList;
}

std::string GetId(const std::map<std::string, std::string>& attributes) {
    if (attributes.contains("id"))
        return attributes.at("id");

    return "";
}

std::vector<std::string> SplitStrWithBrackets(const std::string& str){
    std::vector<std::string> parts;
    bool bracketOpen = false;
    char bracket = 0;
    size_t part_begin = 0;
    size_t i = 0;
    for(char c : str){
        if(bracketOpen){
            if(c == bracket){
                bracketOpen = false;
                // push back substring including brackets
                parts.push_back(str.substr(part_begin, i + 1 - part_begin));
                part_begin = i+1;
            }
        }
        else switch(c){
            case '"': case '\'': 
                bracketOpen = true;
                bracket = c;
                break;
            case ' ': case '\t':
            case '\r': case '\n':
                // push back substring before space if it isn't previous space
                size_t part_length = i - part_begin;
                if(part_length > 0)
                    parts.push_back(str.substr(part_begin, part_length));
                part_begin = i+1;
                break;
        }
        i++;
    }

    // push back the remaining characters
    if(part_begin < str.size())
        parts.push_back(str.substr(part_begin));

    return parts;
}

parser::HTMLElement ParseAttributes(const std::string& token) {
    std::string tokenCpy = token;
    if (auto pos = tokenCpy.find("</"); pos != std::string::npos)
        tokenCpy = tokenCpy.replace(pos, 2, "");
    else if (pos = tokenCpy.find("<"); pos != std::string::npos)
        tokenCpy = tokenCpy.replace(pos, 1, "");

    if (auto pos = tokenCpy.find("/>"); pos != std::string::npos)
        tokenCpy = tokenCpy.replace(pos, 2, "");
    else if (pos = tokenCpy.find(">"); pos != std::string::npos)
        tokenCpy = tokenCpy.replace(pos, 1, "");


    // regex: [^ ]+=\"[^\"]*\"|[^ ]+
    // https://regex101.com/
    // test strings:
    // 1. !DOCTYPE HTML
    // 2. tagname class="cl1 cl2 cl3-a cl3_b" id="test" asd zzxc d-ata-tag="2"
    auto attributes = SplitStrWithBrackets(tokenCpy);

    parser::HTMLElement newElement("");

    if (attributes.size() > 0) {
        newElement.tag = attributes.at(0);

        for (size_t i = 1; i < attributes.size(); i++) {
            const auto& attrib = attributes.at(i);
            if (attrib != " " && !attrib.empty())
                newElement.attributes.emplace(ParseAttributeValue(attrib));
        }
    }

    newElement.classList = GetClassList(newElement.attributes);
    newElement.id = GetId(newElement.attributes);

    return newElement;
}

parser::HTMLElement parser::ParseHTML(const std::string& htmlString) {
    parser::HTMLElement document("document");
    parser::HTMLElement* current = &document;

    for (const std::string& token : CreateTokenArray(htmlString)) {
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
                parser::HTMLElement newElement("");
                newElement.inner = token;
                // quick fix to replace &#39; for its actual value
                ReplaceAll(newElement.inner, "&#39;", "'");
                // quick fix to replace &quot; for its actual value
                ReplaceAll(newElement.inner, "&quot;", "\"");
                // quick fix to replace &amp; for its actual value
                ReplaceAll(newElement.inner, "&amp;", "&");
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

std::vector<parser::HTMLElement*> RecursiveGetElementsByClassName(parser::HTMLElement* element, const std::string& className) {
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

parser::HTMLElement* RecursiveGetElementById(parser::HTMLElement* element, const std::string& id) {
    for (auto& child : element->children) {
        if (child.id == id)
            return &child;

        if (auto e = RecursiveGetElementById(&child, id))
            return e;
    }

    return nullptr;
}

std::optional<parser::HTMLElement*> parser::HTMLElement::GetElementById(const std::string& idName) {
    parser::HTMLElement* element = RecursiveGetElementById(this, idName);

    if (!element)
        return std::nullopt;

    return element;
}

std::vector<parser::HTMLElement*> parser::HTMLElement::GetElementsByClassName(const std::string& className) {
    return RecursiveGetElementsByClassName(this, className);
}

std::vector<parser::HTMLElement*> RecursiveGetElementsByTagName(parser::HTMLElement* element, const std::string& tagName) {
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

std::vector<parser::HTMLElement*> parser::HTMLElement::GetElementsByTagName(const std::string& tagName) {
    return RecursiveGetElementsByTagName(this, tagName);
}
