import json
import re
from pathlib import Path

import httpx
import numpy as np
import pymupdf
import pytest

from rag_python import corpus
from rag_python.cli import main
from rag_python.evaluate import evaluate, normalize, relevant, read_questions
from rag_python.index import Retriever, build, chunk_pages
from rag_python.ollama import Ollama


class WordTokenizer:
    def __call__(self, text, **kwargs):
        return {"offset_mapping": [match.span() for match in re.finditer(r"\S+", text)]}


class TestEncoder:
    __test__ = False
    tokenizer = WordTokenizer()
    max_seq_length = 512

    def encode(self, texts, **kwargs):
        vectors = []
        for text in texts:
            v = np.array([1 + text.lower().count("recursion"), 1 + text.lower().count("dictionary"), 0.1], dtype=np.float32)
            vectors.append(v / np.linalg.norm(v))
        return np.array(vectors)


@pytest.fixture
def sample_root(tmp_path):
    (tmp_path / "corpus").mkdir()
    (tmp_path / "data/pdfs").mkdir(parents=True)
    pdf = pymupdf.open()
    for text in ["recursion recursion recursion calls itself", "dictionary dictionary maps keys to values"]:
        page = pdf.new_page()
        page.insert_text((72, 72), text)
    path = tmp_path / "data/pdfs/sample.pdf"
    pdf.save(path)
    pdf.close()
    (tmp_path / "corpus/manifest.json").write_text(json.dumps({"version": 1, "documents": [{
        "id": "sample", "filename": "sample.pdf", "url": "https://example.org/sample.pdf",
        "sha256": corpus.sha256(path), "pages": 2}]}), encoding="utf-8")
    return tmp_path


def test_real_pdf_extract_and_checksum(sample_root):
    pages = corpus.extract_pages(sample_root)
    assert [p["page"] for p in pages] == [1, 2]
    assert "recursion" in pages[0]["text"]
    with (sample_root / "data/pdfs/sample.pdf").open("ab") as stream:
        stream.write(b"corrupted")
    with pytest.raises(ValueError, match="Checksum"):
        corpus.extract_pages(sample_root)


def test_token_chunks_preserve_overlap_pages_and_unicode():
    text = "un deux trois quatre cinq six sept huit neuf dix"
    pages = [{"document_id": "sample", "page": 1, "text": text},
             {"document_id": "sample", "page": 2, "text": "éthique 中文"}]
    chunks = chunk_pages(pages, WordTokenizer(), size=4, overlap=1)
    assert [c["text"] for c in chunks] == ["un deux trois quatre", "quatre cinq six sept", "sept huit neuf dix", "éthique 中文"]
    assert len({c["id"] for c in chunks}) == 4
    assert all(c["token_end"] - c["token_start"] <= 4 for c in chunks)
    with pytest.raises(ValueError):
        chunk_pages(pages, WordTokenizer(), 4, 4)


def test_faiss_build_reload_search_and_integrity(sample_root):
    encoder = TestEncoder()
    first = build(sample_root, encoder=encoder)
    old_generation = (sample_root / "artifacts/index/CURRENT").read_text()
    second = build(sample_root, encoder=encoder)
    assert first["chunks"] == second["chunks"] == 2
    assert old_generation != (sample_root / "artifacts/index/CURRENT").read_text()
    retriever = Retriever(sample_root, encoder=encoder)
    hits = retriever.search("recursion recursion", 100)
    assert len(hits) == 2
    assert hits[0]["page"] == 1
    assert hits[0]["score"] > hits[1]["score"]
    with pytest.raises(ValueError):
        retriever.search("", 5)
    generation = (sample_root / "artifacts/index/CURRENT").read_text()
    with (sample_root / "artifacts/index" / generation / "chunks.jsonl").open("ab") as stream:
        stream.write(b"{}\n")
    with pytest.raises(ValueError, match="integrity"):
        Retriever(sample_root, encoder=encoder)


def test_failed_build_preserves_previous_generation(sample_root):
    build(sample_root, encoder=TestEncoder())
    previous = (sample_root / "artifacts/index/CURRENT").read_text()
    with pytest.raises(ValueError, match="capacity"):
        build(sample_root, size=600, encoder=TestEncoder())
    assert (sample_root / "artifacts/index/CURRENT").read_text() == previous


def test_metrics_use_evidence_not_just_page():
    gold = {"document_id": "sample", "page": 1, "quote": "calls itself"}
    wrong = {"document_id": "sample", "page": 1, "text": "irrelevant text"}
    right = {"document_id": "sample", "page": 1, "text": "Recursion calls itself."}
    assert not relevant(wrong, gold)
    class Search:
        def search(self, question, k):
            return [wrong, right][:k]
    questions = [{"id": "q1", "question": "Recursion?", "answerable": True, "evidence": [gold]},
                 {"id": "q2", "question": "Weather?", "answerable": False, "evidence": []}]
    result = evaluate(Search(), questions, 2)
    assert result["hit_at_k"] == 1
    assert result["mrr_at_k"] == 0.5
    assert result["evidence_recall_at_k"] == 1
    assert result["unanswerable_count"] == 1
    assert evaluate(Search(), questions, 1)["hit_at_k"] == 0


def test_ollama_request_and_citation_validation(monkeypatch):
    bodies = []
    def post(url, json, timeout):
        bodies.append((url, json))
        return httpx.Response(200, request=httpx.Request("POST", url), json={"message": {"content": '{"answer": "Une fonction.", "citations": ["S1", "S99"], "insufficient_context": false}'}})
    monkeypatch.setattr(httpx, "post", post)
    response = Ollama().answer("Question?", [{"source": "sample.pdf", "page": 2, "text": "une fonction"}])
    assert response["invalid_citations"] == ["S99"]
    assert "[S1]" in response["answer"]
    assert "format" in bodies[0][1]
    assert "page PDF 2" in bodies[0][1]["messages"][1]["content"]
    assert bodies[0][1]["options"]["temperature"] == 0
    assert Ollama().answer("Question?", [])["sources"] == []


def test_ollama_connection_error_is_actionable(monkeypatch):
    def post(*args, **kwargs):
        raise httpx.ConnectError("refused")
    monkeypatch.setattr(httpx, "post", post)
    with pytest.raises(RuntimeError, match="ollama serve"):
        Ollama().pull()


def test_download_validates_before_publishing(sample_root, monkeypatch):
    existing = sample_root / "data/pdfs/sample.pdf"
    payload = existing.read_bytes()
    existing.unlink()
    def handler(request):
        return httpx.Response(200, content=payload)
    real_client = httpx.Client
    monkeypatch.setattr(httpx, "Client", lambda **kwargs: real_client(transport=httpx.MockTransport(handler), **kwargs))
    assert corpus.download(sample_root) == [existing]
    assert corpus.download(sample_root) == [existing]  # repeatable
    existing.unlink()
    payload = b"not a pdf"
    with pytest.raises(ValueError, match="Checksum"):
        corpus.download(sample_root)
    assert not existing.exists()
    assert not list(existing.parent.glob("*.part"))


def test_cli_reports_missing_index(tmp_path, capsys):
    assert main(["--root", str(tmp_path), "search", "test"]) == 1
    assert "rag index" in capsys.readouterr().err


def test_reference_dataset_schema():
    root = Path(__file__).resolve().parents[1]
    questions = read_questions(root / "evaluation/questions.jsonl")
    assert len(questions) == 23
    assert sum(q["answerable"] for q in questions) == 20
    for q in questions:
        assert q["reference_answer"]
        assert bool(q["evidence"]) == q["answerable"]
        assert all(1 <= e["page"] <= 15 and e["quote"].strip() for e in q["evidence"])


def test_import_gguf_upload_and_create(tmp_path, monkeypatch):
    path = tmp_path / "test.gguf"
    path.write_bytes(b"GGUFtest data")
    calls = []
    def post(url, **kwargs):
        calls.append((url, kwargs))
        if "/api/blobs/" in url:
            assert kwargs["content"].read() == b"GGUFtest data"
            return httpx.Response(201, request=httpx.Request("POST", url))
        return httpx.Response(200, request=httpx.Request("POST", url), json={"status": "success"})
    monkeypatch.setattr(httpx, "post", post)
    assert Ollama(model="test-model").import_gguf(path)["status"] == "success"
    assert calls[1][1]["json"]["model"] == "test-model"
    assert calls[1][1]["json"]["files"]["test.gguf"].startswith("sha256:")
    path.write_bytes(b"invalid header")
    with pytest.raises(ValueError, match="header"):
        Ollama().import_gguf(path)
