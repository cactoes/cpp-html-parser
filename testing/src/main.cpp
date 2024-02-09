#include <iostream>
#include <htmlparser.hpp>

int main() {
    parser::HTMLElement document = parser::ParseHTML(L"<h1>Hello</h1>");
    std::wcout << document.children.at(0).tag << "\n";
    return 0;
}