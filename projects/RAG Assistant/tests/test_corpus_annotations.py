"""Optional real-corpus annotation verification; download is explicit, never implicit."""
from pathlib import Path

import pytest

from rag_python.corpus import extract_pages
from rag_python.evaluate import normalize, read_questions


def test_every_gold_quote_exists_on_its_pdf_page():
    root = Path(__file__).resolve().parents[1]
    if not (root / "data/pdfs/think-python-glossary.pdf").exists():
        pytest.skip("Run rag download to verify annotations against the actual PDF")
    pages = {(p["document_id"], p["page"]): p["text"] for p in extract_pages(root)}
    for question in read_questions(root / "evaluation/questions.jsonl"):
        for evidence in question["evidence"]:
            assert normalize(evidence["quote"]) in normalize(pages[evidence["document_id"], evidence["page"]]), question["id"]
