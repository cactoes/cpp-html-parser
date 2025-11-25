## HTML Parser
A simple html parser written in c++

## including this project
```cmake
add_subdirectory(PATH_TO_THIS_LIBRARY)
# ... add_executable or simmilair
target_link_libraries(${PROJECT_NAME} PRIVATE cpp-html-parser)
```

## example
```cpp
#include <htmlparser.hpp>

int main () {
    using namespace parser;

    // parse the html wstring
    HTMLElement document = parseHTML("<h1>Hello</h1>");

    // print the h1 tag
    std::cout << document.children.at(0)->tag << "\n";

    return 0;
}
```

## functions
### constructor
only needs a tagname to create the element
```cpp
HTMLElement::HTMLElement(const std::string& tag);
```
### GetElementById
returns an optional pointer to the element with the specified id
```cpp
std::optional<HTMLElement*> HTMLElement::GetElementById(const std::string& idName);
```

### GetElementsByClassName
returns a list of element pointers which include the specified class name
```cpp
std::vector<HTMLElement*> HTMLElement::GetElementsByClassName(const std::string& className);
```

### GetElementsByTagName
returns a list of element pointers which include the specified tag name
```cpp
std::vector<HTMLElement*> HTMLElement::GetElementsByTagName(const std::string& tagName);
```

## contributors
* [@cactoes](https://www.github.com/cactoes)
* [@MeloenCoding](https://www.github.com/MeloenCoding)
* [@Altair-sh](https://github.com/Altair-sh)

## license
[MIT](LICENSE)
