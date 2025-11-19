import pyhtml2md

html = """
<p>
	<strong>1. Title</strong></p>
<p>
	paragraph   with tabs	and    spaces</p>
"""

print("--- Original Output ---")
converter = pyhtml2md.Converter(html)
# Enable forceLeftTrim as user mentioned it was part of their partial solution
options = pyhtml2md.Options()
options.compressWhitespace = True
converter = pyhtml2md.Converter(html, options)
print(converter.convert())

expected = "**1. Title**\n\nparagraph with tabs and spaces\n"
print("\n--- Expected Output ---")
print(expected)
