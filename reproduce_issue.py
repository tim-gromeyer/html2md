import pyhtml2md

html = """
4.<br />
Please implement as requested.
"""

print("--- Original Output ---")
converter = pyhtml2md.Converter(html)
print(converter.convert())

expected = "4\\.  \nPlease implement as requested.\n"
print("\n--- Expected Output ---")
print(expected)
