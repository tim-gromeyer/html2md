import pytest
import pyhtml2md

def test_default_html_symbol_conversions():
    """Test that default HTML symbol conversions work correctly"""

    assert '"' in pyhtml2md.convert("&quot;")
    assert "<" in pyhtml2md.convert("&lt;")
    assert ">" in pyhtml2md.convert("&gt;")
    assert "&" in pyhtml2md.convert("&amp;")
    assert "→" in pyhtml2md.convert("&rarr;")
    assert "→" in pyhtml2md.convert("&rarr;")    

def test_add_html_symbol_conversion():
    """Test adding new HTML symbol conversions"""
    converter = pyhtml2md.Converter("&copy; &reg; &custom;")
    
    # Before adding conversions
    result = converter.convert()
    assert "&copy;" in result
    assert "&reg;" in result
    assert "&custom;" in result
    
    # Add new conversions
    converter = pyhtml2md.Converter("&copy; &reg; &custom;")
    converter.add_html_symbol_conversion("&copy;", "©")
    converter.add_html_symbol_conversion("&reg;", "®")
    converter.add_html_symbol_conversion("&custom;", "CUSTOM")
    
    result = converter.convert()
    assert "©" in result
    assert "®" in result
    assert "CUSTOM" in result

def test_modify_html_symbol_conversion():
    """Test modifying existing HTML symbol conversions"""
    converter = pyhtml2md.Converter("&nbsp;")
    
    # Default conversion
    assert " " in converter.convert()
    
    # Modify the conversion
    converter = pyhtml2md.Converter("&nbsp;")
    converter.add_html_symbol_conversion("&nbsp;", "\t")
    assert "\t" in converter.convert()

def test_remove_html_symbol_conversion():
    """Test removing HTML symbol conversions"""
    converter = pyhtml2md.Converter("&amp; &lt;")
    
    # Remove &amp; conversion
    converter.remove_html_symbol_conversion("&amp;")
    result = converter.convert()
    assert "&amp;" in result  # Should remain unconverted
    assert "<" in result      # &lt; should still be converted

def test_clear_html_symbol_conversions():
    """Test clearing all HTML symbol conversions"""
    converter = pyhtml2md.Converter("&quot; &lt; &gt; &amp; &nbsp;")
    
    # Clear all conversions
    converter.clear_html_symbol_conversions()
    result = converter.convert()
    
    # All symbols should remain unconverted
    assert "&quot;" in result
    assert "&lt;" in result
    assert "&gt;" in result
    assert "&amp;" in result
    assert "&nbsp;" in result

def test_multiple_conversions_in_text():
    """Test that multiple conversions work in the same text"""
    html = """
    <p>
        &quot;Quoted text&quot; with &lt;tags&gt; and &amp; entities &nbsp; separated by &rarr; arrows
    </p>
    """
    options = pyhtml2md.Options()
    options.splitLines = False
    converter = pyhtml2md.Converter(html, options)
    
    # Add some custom conversions
    converter.add_html_symbol_conversion("&rarr;", "->")
    converter.add_html_symbol_conversion("&nbsp;", "\t")
    
    result = converter.convert()
    assert '"Quoted text"' in result
    assert "<tags>" in result
    assert "& entities" in result  # &amp; becomes &
    assert "\t separated by" in result
    assert "-> arrows" in result

def test_html_symbols_in_attributes():
    """Test that HTML symbols in attributes are converted"""
    html = '<a href="page.html?p1=1&amp;p2=2" title="&quot;Title&quot;">Link</a>'
    converter = pyhtml2md.Converter(html)
    
    result = converter.convert()
    assert '[Link](page.html?p1=1&p2=2 "' in result
    # assert '"Title")' in result # TODO: Fix this assertion

def test_special_case_conversions():
    """Test special cases like numeric and named entities"""
    converter = pyhtml2md.Converter("&#169; &#174; &#x1F600;")
    
    # Add numeric entity conversions
    converter.add_html_symbol_conversion("&#169;", "©")  # ©
    converter.add_html_symbol_conversion("&#174;", "®")  # ®
    converter.add_html_symbol_conversion("&#x1F600;", "😀")  # 😀
    
    result = converter.convert()
    assert "©" in result
    assert "®" in result
    assert "😀" in result

if __name__ == "__main__":
    pytest.main([__file__])