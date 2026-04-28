import re
from pathlib import Path
from markdown_pdf import MarkdownPdf, Section

ROOT = Path(__file__).parent

CSS = """
body { font-family: 'Segoe UI', Arial, sans-serif; color: #222; line-height: 1.5; }
h1 { color: #556B2F; border-bottom: 3px solid #556B2F; padding-bottom: 6px; }
h2 { color: #556B2F; border-bottom: 1px solid #cfd6b8; padding-bottom: 4px; margin-top: 24px; }
h3 { color: #3d4a1f; margin-top: 18px; }
h4 { color: #3d4a1f; }
code { background: #f1efe3; padding: 1px 4px; border-radius: 3px; font-size: 0.92em; }
pre { background: #f1efe3; padding: 10px; border-radius: 5px; overflow-x: auto; }
pre code { background: transparent; padding: 0; }
table { border-collapse: collapse; margin: 10px 0; width: 100%; }
th, td { border: 1px solid #bfbfbf; padding: 6px 10px; text-align: left; vertical-align: top; }
th { background: #e6e2c8; }
blockquote { border-left: 4px solid #556B2F; background: #f6f4e7; padding: 8px 12px; margin: 10px 0; color: #3d4a1f; }
a { color: #556B2F; text-decoration: none; }
a:hover { text-decoration: underline; }
hr { border: 0; border-top: 1px solid #cfd6b8; margin: 20px 0; }
"""

TOC_PATTERNS = [
    re.compile(r"## Table of contents\n\n(?:\d+\..*\n)+\n---\n", re.MULTILINE),
    re.compile(r"## Índice\n\n(?:\d+\..*\n)+\n---\n", re.MULTILINE),
]


def strip_manual_toc(md: str) -> str:
    for pat in TOC_PATTERNS:
        md = pat.sub("", md, count=1)
    return md


def build(src_name: str, out_name: str):
    src = ROOT / src_name
    out = ROOT / out_name
    md = src.read_text(encoding="utf-8")
    md = strip_manual_toc(md)

    pdf = MarkdownPdf(toc_level=2)
    pdf.add_section(Section(md, toc=True), user_css=CSS)
    pdf.meta["title"] = src.stem.replace("_", " ")
    pdf.meta["author"] = "BCGA Airsoft"
    pdf.save(out)
    print(f"wrote {out} ({out.stat().st_size} bytes)")


if __name__ == "__main__":
    build("MANUAL_EN.md", "MANUAL_EN.pdf")
    build("MANUAL_PT.md", "MANUAL_PT.pdf")
