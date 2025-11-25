#include <iostream>
#include <htmlparser.hpp>

int main() {
    parser::HTMLElement document = parser::ParseHTML("<h1>Hello</h1>");
    std::cout << document.children.at(0).tag << "\n";
    return 0;
}