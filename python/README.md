# pyhtml2md

pyhtml2md provides a way to use the html2md C++ library in Python. html2md is a fast and reliable library for converting HTML content into markdown.

- [Installation](#installation)
- [Basic usage](#basic-usage)
- [Advanced usage](#advanced-usage)
- [Supported Tags](#supported-tags)
- [License](#license)


## Installation

You can install using pip:

```bash
pip3 install pyhtml2md
```

### Manually

1.  Make sure you have a compiler with c++11 and CMake installed on you system
2. Clone html2md: `git clone https://github.com/tim-gromeyer/html2md --recurse-submodules --depth=1`
3. Build and install the python package: `pip3 install ./html2md/`

## Basic usage

Here is an example of how to use the pyhtml2md to convert HTML to markdown:

```python
import pyhtml2md

markdown = pyhtml2md.convert("<h1>Hello, world!</h1>")
print(markdown)
```

The `convert` function takes an HTML string as input and returns a markdown string.

## Advanced usage

pyhtml2md provides a `Options` class to customize the generation process.  
You can find all information on the c++ [documentation](https://tim-gromeyer.github.io/html2md/index.html)

Here is an example:

```python
import pyhtml2md

options = pyhtml2md.Options()
options.splitLines = False

converter = pyhtml2md.Converter("<h1>Hello Python!</h1>", options)
markdown = converter.convert()
print(markdown)
print(converter.ok())
```

## Supported Tags

pyhtml2md supports the following HTML tags:

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
| `ol`         | Ordered list       |                                            |
| `p`          | Paragraph          |                                            |
| `pre`        | Preformatted text  | Works only with `code`.                    |
| `s`          | Strikethrough      | Same as `del`.                             |
| `span`       | Grouped elements   |                                            |
| `strong`     | Strong             | Same as `b`.                               |
| `table`      | Table              |                                            |
| `tbody`      | Table body         | Does nothing.                              |
| `td`         | Table data cell    | Uses `align` from `th`.                    |
| `tfoot`      | Table footer       | Does nothing.                              |
| `th`         | Table header cell  | Supports the `align` attribute.            |
| `thead`      | Table header       | Does nothing.                              |
| `title`      | Document title     | Same as `h1`.                              |
| `tr`         | Table row          |                                            |
| `u`          | Underlined         | Uses HTML.                                 |
| `ul`         | Unordered list     |                                            |

## License

pyhtml2md is licensed under [The MIT License (MIT)](https://opensource.org/licenses/MIT)
