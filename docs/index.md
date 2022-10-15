# html2md

## Table of Contents

- [What does it do](#autotoc_md2)
- [Use this library](#autotoc_md3)
- [Supported Tags](#autotoc_md4)
- [License](#autotoc_md5)


## What does it do

*html2md* is designed to convert HTML to Markdown. It is fast and lightweight but has no options yet.

Also the project is a good example for efficient abstraction of logical decision trees via strategy pattern.


## Use this library

First of all, **clone** the library: `git clone https://github.com/software-made-easy/html2md`.  

Then **add the files** `include/html2md.h` and `src/html2md.cpp` **to your build**.  

Afterwards follow the **example below**.

```cpp
#include <html2md.h>

//...

std::cout << html2md::Convert("<h1>foo</h1>"); // # foo
```


## Supported Tags

The following table lists the supported HTML tags:


| Tag          | Description        | Comment                                    |
| ------------ | ------------------ | ------------------------------------------ |
| `a`          | Anchor or link     | Supports the `href` and `name` attributes. |
| `b`          | Bold               |                                            |
| `blockquote` | Indented paragraph |                                            |
| `br`         | Line break         |                                            |
| `cite`       | Inline citation    | Same as `i`.                               |
| `code`       | Code               |                                            |
| `dd`         | Definition data    |                                            |
| `del`        | Strikethrough      |                                            |
| `dfn`        | Definition         | Same as `i`.                               |
| `div`        | Document division  |                                            |
| `em`         | Emphasized         | Same as `i`.                               |
| `h1`         | Level 1 heading    |                                            |
| `h2`         | Level 2 heading    |                                            |
| `h3`         | Level 3 heading    |                                            |
| `h4`         | Level 4 heading    |                                            |
| `h5`         | Level 5 heading    |                                            |
| `h6`         | Level 6 heading    |                                            |
| `head`       | Document header    | Ignored.                                   |
| `hr`         | Horizontal line    |                                            |
| `i`          | Italic             |                                            |
| `img`        | Image              | Supports the `src` and `alt` attributes.   |
| `li`         | List item          |                                            |
| `meta`       | Meta-information   | Ignored.                                   |
| `ol`         | Ordered list       | Don't use other lists in this list.        |
| `p`          | Paragraph          |                                            |
| `pre`        | Preformatted text  | Works only with `code`.                    |
| `s`          | Strikethrough      | Same as `del`.                             |
| `span`       | Grouped elements   |                                            |
| `strong`     | Strong             | Same as `b`.                               |
| `table`      | Table              |                                            |
| `td`         | Table data cell    | Uses `align` from `th`.                    |
| `th`         | Table header cell  | Supports the `align` attribute.            |
| `title`      | Document title     | Same as `h1`.                              |
| `tr`         | Table row          |                                            |
| `u`          | Underlined         | Uses HTML.                                 |
| `ul`         | Unordered list     |                                            |


## License

html2md is licensed under [The MIT License (MIT)](https://opensource.org/licenses/MIT)
