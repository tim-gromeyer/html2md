# html2md

## Table of Contents

- [What does it do](#what-does-it-do)
- [Usage Example](#usage-example)
- [Code Convention](#code-convention)
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


## Code Convention


The source code of the html2md header follows the 
[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).  


## License

html2md is licensed under [The MIT License (MIT)](https://opensource.org/licenses/MIT)
