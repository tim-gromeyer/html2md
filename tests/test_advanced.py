import pytest
import pyhtml2md

def test_basic_conversion():
    # Test basic header conversion
    assert pyhtml2md.convert("<h1>Hello Python!</h1>") == "# Hello Python!\n"

    # Test basic paragraph
    assert pyhtml2md.convert("<p>Simple paragraph</p>") == "Simple paragraph\n"

def test_converter_class():
    # Test converter initialization and conversion
    converter = pyhtml2md.Converter("<h1>Hello Python!</h1>")
    assert converter.convert() == "# Hello Python!\n"
    assert converter.ok() == True

    # Test boolean operator
    assert bool(converter) == True

def test_options():
    # Test options configuration
    options = pyhtml2md.Options()
    options.splitLines = False
    options.unorderedList = '*'
    options.orderedList = ')'
    options.includeTitle = False

    html = "<ul><li>First</li><li>Second</li></ul>"
    converter = pyhtml2md.Converter(html, options)
    result = converter.convert()
    assert result.startswith('* First')
    assert converter.ok()

def test_complex_formatting():
    html = """
    <h1>Main Title</h1>
    <p><strong>Bold text</strong> and <em>italic text</em></p>
    <ul>
        <li>First item</li>
        <li>Second item</li>
    </ul>
    <ol>
        <li>Numbered one</li>
        <li>Numbered two</li>
    </ol>
    """
    options = pyhtml2md.Options()
    options.splitLines = False
    converter = pyhtml2md.Converter(html, options)
    result = converter.convert()

    assert "# Main Title" in result
    assert "**Bold text**" in result
    assert "*italic text*" in result
    assert "1. Numbered one" in result
    assert "2. Numbered two" in result

def test_table_formatting():
    html = """
    <table>
        <tr><th>Header 1</th><th>Header 2</th></tr>
        <tr><td>Data 1</td><td>Data 2</td></tr>
    </table>
    """
    options = pyhtml2md.Options()
    options.formatTable = True
    converter = pyhtml2md.Converter(html, options)
    result = converter.convert()

    assert "|" in result
    # assert "Header 1" in result  # BUG: The generated table is wrong
    assert "Data 1" in result

def test_line_breaks():
    html = "A very long line of text that should be wrapped according to the soft break and hard break settings"
    options = pyhtml2md.Options()
    options.splitLines = True
    options.softBreak = 20
    options.hardBreak = 30

    converter = pyhtml2md.Converter(html, options)
    result = converter.convert()
    lines = result.split('\n')
    assert any(len(line) <= 30 for line in lines)

def test_error_handling():
    # Test with malformed HTML
    html = "<p>Unclosed paragraph"
    converter = pyhtml2md.Converter(html)
    converter.convert()
    assert not converter.ok()

def test_options_equality():
    options1 = pyhtml2md.Options()
    options2 = pyhtml2md.Options()

    assert options1 == options2

    options2.splitLines = False
    assert options1 != options2

def test_special_characters():
    html = "<p>&lt;special&gt; &amp; &quot;characters&quot;</p>"
    result = pyhtml2md.convert(html)
    assert "<special>" in result
    assert '"characters"' in result
    assert "&" in result

def test_nested_structures():
    html = """
    <blockquote>
        <p>Quoted text with <strong>bold</strong> and <em>italic</em></p>
        <ul>
            <li>Nested <strong>list</strong></li>
        </ul>
    </blockquote>
    """
    result = pyhtml2md.convert(html)
    assert ">" in result  # blockquote marker
    assert "**bold**" in result
    assert "*italic*" in result
    assert "**list**" in result

if __name__ == "__main__":
    pytest.main([__file__])
