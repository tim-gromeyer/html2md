# html2md

## Table of Contents

- [What does it do](#what-does-it-do)
- [Usage Example](#usage-example)
- [Supported Tags](#supported-tags)
- [License](#license)


## What does it do

html2md is a rudimentary solution for HTML to Markdown conversion, implemented as a C++ single header.  

Also the project is a good example for efficient abstraction of logical decision trees via strategy pattern.


### Usage Example

```c++
#include <html2md.hpp>

//...

std::cout << html2md::Convert(html);
```

See the included [example.cpp](example.cpp) 


## Supported Tags

The following table lists the HTML tags supported by html2md:


| Tag | Description | Comment |
| - | - | - |
| `a` | Anchor or link | Supports the `href` and `name` attributes. |
| `b` | Bold | Same as `strong` |
| `blockquote` | Indented paragraph | |
| `br` | Line break | |
| `cite` | Inline citation | Same as `i`. |
| `code` | Code | Same as `tt`. |
| `dd` | Definition data | |
| `del` | Strikethrough | |
| `dfn` | Definition | Same as `i`. |
| `div` | Document division | |
| `em` | Emphasized | Same as `i`. |
| `h1` | Level 1 heading | |
| `h2` | Level 2 heading | |
| `h3` | Level 3 heading | |
| `h4` | Level 4 heading | |
| `h5` | Level 5 heading | |
| `h6` | Level 6 heading | |
| `head` | Document header | Ignored |
| `hr` | Horizontal line | |
| `i` | Italic | |
| `img` | Image | Supports the `src` and `alt` attributes. |
| `li` | List item | |
| `meta` | Meta-information | Ignored |
| `ol` | Ordered list | |
| `p` | Paragraph | |
| `pre` | Preformatted text| Works only with `code` |
| `s` | Strikethrough | Same as `del`. |
| `span` | Grouped elements | |
| `strong` | Strong | Same as `b`. |
| `table` | Table | |
| `tbody` | Table body | Does nothing. |
| `td` | Table data cell | Uses `align` from `th` |
| `tfoot` | Table footer | Does nothing. |
| `th` | Table header cell | Supports the `align` attribute |
| `thead` | Table header | Does nothing |
| `title` | Document title | Same as `h1` |
| `tr` | Table row | |
| `u` | Underlined | Uses HTML |
| `ul` | Unordered list | |


## License

html2md is licensed under [The MIT License (MIT)](https://opensource.org/licenses/MIT)
