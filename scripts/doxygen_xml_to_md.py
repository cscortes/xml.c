#!/usr/bin/env python3
"""
Generate docs/xml_api.md from Doxygen XML output.
Reads build/api_docs/xml/xml_8h.xml (public API from the header) and writes
a single Markdown file. Run after Doxygen has been run (e.g. as part of the
api_docs CMake target).

Usage: doxygen_xml_to_md.py <xml_dir> <output_md_path>
Example: doxygen_xml_to_md.py build/api_docs/xml docs/xml_api.md
"""

import argparse
import re
import sys
import xml.etree.ElementTree as ET


def ns(tag):
    """Strip Doxygen namespace from tag name."""
    return tag.split("}")[-1] if "}" in tag else tag


def text_of(el, default=""):
    """Recursive text of an element; refs become their text."""
    if el is None:
        return default
    if el.text:
        out = [el.text]
    else:
        out = []
    for child in el:
        tag = ns(child.tag)
        if tag == "ref":
            out.append(child.text or "")
            if child.tail:
                out.append(child.tail)
        elif tag in ("para", "parameterdescription", "parameternamelist", "computeroutput"):
            out.append(text_of(child, ""))
            if child.tail:
                out.append(child.tail)
        else:
            out.append(text_of(child, ""))
            if child.tail:
                out.append(child.tail)
    return "".join(out).strip()


def type_to_str(type_el):
    """Flatten type element (may contain refs) to a string."""
    if type_el is None:
        return ""
    return "".join(type_el.itertext()).strip() or text_of(type_el, "")


def _process_para(para_el, lines):
    """Emit para content: full text if only text/refs, else leading text + parameterlist/simplesect."""
    if para_el is None:
        return
    direct = [ns(c.tag) for c in para_el]
    has_param_or_sect = "parameterlist" in direct or "simplesect" in direct
    if has_param_or_sect:
        # Leading text only (before first special child)
        lead = (para_el.text or "").strip()
        lead = re.sub(r"\s*\[PUBLIC API\]\s*", "", lead)
        if lead:
            lines.append(lead)
            lines.append("")
        for child in para_el:
            tag = ns(child.tag)
            if tag == "parameterlist":
                for item in child.findall(".//parameteritem"):
                    name_el = item.find(".//parametername")
                    desc_el = item.find(".//parameterdescription//para")
                    name = text_of(name_el, "") if name_el is not None else ""
                    desc = text_of(desc_el, "") if desc_el is not None else ""
                    if name:
                        lines.append(f"- **{name}** — {desc}")
                if lines and lines[-1] != "":
                    lines.append("")
            elif tag == "simplesect":
                kind = child.get("kind")
                sub_para = child.find("para")
                content = text_of(sub_para, "") if sub_para is not None else ""
                if content:
                    if kind == "warning":
                        lines.append(f"**Warning:** {content}")
                    elif kind == "return":
                        lines.append(f"**Returns:** {content}")
                    else:
                        lines.append(content)
                    lines.append("")
    else:
        # Para is only text and refs: use full recursive text
        full = text_of(para_el, "").strip()
        full = re.sub(r"\s*\[PUBLIC API\]\s*", "", full)
        if full:
            lines.append(full)
            lines.append("")


def format_detailed(dd_el):
    """Format detaileddescription as Markdown; skip [PUBLIC API] and empty paras."""
    if dd_el is None:
        return []
    lines = []
    for child in dd_el:
        tag = ns(child.tag)
        if tag == "para":
            _process_para(child, lines)
    return lines


def main():
    parser = argparse.ArgumentParser(description="Convert Doxygen XML (xml.h) to xml_api.md")
    parser.add_argument("xml_dir", help="Path to Doxygen xml output (e.g. build/api_docs/xml)")
    parser.add_argument("output_md", help="Output path (e.g. docs/xml_api.md)")
    args = parser.parse_args()

    xml_path = f"{args.xml_dir.rstrip('/')}/xml_8h.xml"
    try:
        tree = ET.parse(xml_path)
    except OSError as e:
        print(f"Error: could not read {xml_path}: {e}", file=sys.stderr)
        sys.exit(1)

    root = tree.getroot()
    # Handle optional namespace
    def find_all(parent, path):
        return parent.findall(path) or parent.findall(
            path.replace("}", "}" + root.tag.split("}")[0].strip("{"))
        )

    sectiondef = root.find(".//sectiondef[@kind='func']")
    if sectiondef is None:
        print("Error: no sectiondef kind='func' in xml_8h.xml", file=sys.stderr)
        sys.exit(1)

    out_lines = [
        "# xml.c API reference",
        "",
        "Generated from Doxygen comments in `src/xml.h`. Regenerate with:",
        "`cmake --build build --target api_docs`.",
        "",
        "---",
        "",
    ]

    for member in sectiondef.findall("memberdef"):
        if member.get("kind") != "function":
            continue
        name_el = member.find("name")
        type_el = member.find("type")
        args_el = member.find("argsstring")
        name = name_el.text or "" if name_el is not None else ""
        ret_type = type_to_str(type_el)
        args_str = (args_el.text or "") if args_el is not None else "()"
        decl = f"**{ret_type}** `{name}`{args_str}"
        out_lines.append(f"## {name}")
        out_lines.append("")
        out_lines.append(decl)
        out_lines.append("")
        dd = member.find("detaileddescription")
        for line in format_detailed(dd):
            out_lines.append(line)
        out_lines.append("")

    try:
        with open(args.output_md, "w", encoding="utf-8") as f:
            f.write("\n".join(out_lines))
            if out_lines and out_lines[-1] != "":
                f.write("\n")
    except OSError as e:
        print(f"Error: could not write {args.output_md}: {e}", file=sys.stderr)
        sys.exit(1)

    return 0


if __name__ == "__main__":
    sys.exit(main())
