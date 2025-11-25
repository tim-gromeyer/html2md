import pytest
import pyhtml2md

def test_escape_numbered_list_enabled_by_default():
    html = "4.<br />\nPlease implement as requested."
    expected = "4\\.  \nPlease implement as requested.\n"
    assert pyhtml2md.convert(html) == expected

def test_escape_numbered_list_disabled():
    html = "4.<br />\nPlease implement as requested."
    options = pyhtml2md.Options()
    options.escapeNumberedList = False
    
    # When disabled, it should be interpreted as a list item (or at least not escaped)
    # The original issue was that "4." became "1." because it was seen as a list.
    # So we expect "1. \nPlease..." or similar if it's interpreted as a list,
    # OR just "4. \n..." if it's not escaped but also not renumbered (depending on md4c/parser behavior).
    # However, based on the issue description: "The issue is, that this text is renumbered by Markdown and rendered as '1.\nPlease...'."
    # So if we disable escaping, we expect the output to contain "4.  \n" which might then be rendered as "1." by a viewer,
    # BUT html2md output itself is what we check.
    # If we don't escape, html2md outputs "4.  \n".
    
    expected = "4.  \nPlease implement as requested.\n"
    converter = pyhtml2md.Converter(html, options)
    assert converter.convert() == expected

def test_escape_numbered_list_with_other_content():
    html = "<p>1. Item</p>"
    # In a paragraph, it might be different.
    # But our fix is in ParseCharInTagContent which handles text content.
    # If it's "1. Item", it should be escaped to "1\. Item" if enabled.
    
    expected = "1\\. Item\n"
    assert pyhtml2md.convert(html) == expected

    options = pyhtml2md.Options()
    options.escapeNumberedList = False
    expected_disabled = "1. Item\n"
    converter = pyhtml2md.Converter(html, options)
    assert converter.convert() == expected_disabled
