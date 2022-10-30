## How does the test work?

Well, the program searches(in this dir) for files ending with `.md`.

1. It then converts the Markdown to HTML using [md4c](https://github.com/software-made-easy/MarkdownEdit_md4c).  
2. Afterwards it converts the HTML back to Markdown. 
3. The generated Markdown gets converted HTML
4. It compares the HTML generated from the original Markdown<br>and the HTML generated from the converted Markdown.
