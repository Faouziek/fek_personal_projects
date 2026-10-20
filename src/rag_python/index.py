"""Token chunks, normalized dense embeddings and an exact cosine FAISS index."""
import json
import os
from pathlib import Path
import tempfile

import faiss
import numpy as np

from .corpus import extract_pages, read_manifest, sha256, write_jsonl

DEFAULT_MODEL = "intfloat/multilingual-e5-small"
DEFAULT_REVISION = "614241f622f53c4eeff9890bdc4f31cfecc418b3"


def load_encoder(model: str, revision: str | None = None):
    from sentence_transformers import SentenceTransformer
    return SentenceTransformer(model, revision=revision, device="cpu", trust_remote_code=False)


def chunk_pages(pages: list[dict], tokenizer, size: int = 300, overlap: int = 50) -> list[dict]:
    if size < 1 or overlap < 0 or overlap >= size:
        raise ValueError("Require chunk size > overlap >= 0")
    chunks = []
    for page in pages:
        offsets = tokenizer(page["text"], add_special_tokens=False,
                            return_offsets_mapping=True, truncation=False)["offset_mapping"]
        for start in range(0, len(offsets), size - overlap):
            end = min(start + size, len(offsets))
            text = page["text"][offsets[start][0]:offsets[end - 1][1]].strip()
            if text:
                chunks.append({**page, "id": f"{page['document_id']}:p{page['page']}:t{start}",
                               "text": text, "token_start": start, "token_end": end})
            if end == len(offsets):
                break
    if not chunks:
        raise ValueError("No text chunks generated")
    return chunks


def encode(encoder, texts: list[str], kind: str, batch_size: int = 16) -> np.ndarray:
    # E5 requires these prefixes even for non-English input.
    values = encoder.encode([f"{kind}: {text}" for text in texts], batch_size=batch_size,
                            normalize_embeddings=True, convert_to_numpy=True,
                            show_progress_bar=False)
    values = np.ascontiguousarray(values, dtype=np.float32)
    if values.ndim != 2 or not np.isfinite(values).all():
        raise ValueError("Invalid embeddings")
    return values


def build(root: Path, model: str = DEFAULT_MODEL, revision: str | None = DEFAULT_REVISION,
          size: int = 300, overlap: int = 50, batch_size: int = 16, encoder=None) -> dict:
    if batch_size < 1:
        raise ValueError("Batch size must be positive")
    pages = extract_pages(root)
    encoder = encoder if encoder is not None else load_encoder(model, revision)
    # Reserve space for the E5 prefix and tokenizer special tokens.
    if size > encoder.max_seq_length - 8:
        raise ValueError(f"Chunk size exceeds model capacity ({encoder.max_seq_length - 8})")
    chunks = chunk_pages(pages, encoder.tokenizer, size, overlap)
    vectors = encode(encoder, [chunk["text"] for chunk in chunks], "passage", batch_size)
    index = faiss.IndexFlatIP(vectors.shape[1])
    index.add(vectors)
    output = root / "artifacts/index"
    output.mkdir(parents=True, exist_ok=True)
    # Never expose a half-written index: CURRENT is changed only after all writes.
    version = Path(tempfile.mkdtemp(prefix="generation-", dir=output))
    faiss.write_index(index, str(version / "vectors.faiss"))
    write_jsonl(version / "chunks.jsonl", chunks)
    resolved = getattr(encoder[0].auto_model.config, "_commit_hash", None) if hasattr(encoder, "__getitem__") else None
    metadata = {"schema": 1, "model": model, "revision": resolved or revision,
                "dimension": vectors.shape[1], "chunks": len(chunks), "pages": len(pages),
                "chunk_size": size, "overlap": overlap, "normalized": True,
                "corpus": read_manifest(root),
                "files": {name: sha256(version / name) for name in ("vectors.faiss", "chunks.jsonl")}}
    (version / "manifest.json").write_text(json.dumps(metadata, indent=2), encoding="utf-8")
    fd, pointer = tempfile.mkstemp(prefix=".current-", dir=output)
    try:
        with os.fdopen(fd, "w", encoding="utf-8") as stream:
            stream.write(version.name)
        os.replace(pointer, output / "CURRENT")
    finally:
        Path(pointer).unlink(missing_ok=True)
    return metadata


class Retriever:
    def __init__(self, root: Path, encoder=None):
        output = root / "artifacts/index"
        pointer = output / "CURRENT"
        if not pointer.exists():
            raise FileNotFoundError("No index; run `rag index` first")
        name = pointer.read_text(encoding="utf-8").strip()
        if not name.startswith("generation-") or Path(name).name != name or "/" in name or "\\" in name:
            raise ValueError("Invalid index generation")
        version = output / name
        self.metadata = json.loads((version / "manifest.json").read_text(encoding="utf-8"))
        if self.metadata.get("schema") != 1:
            raise ValueError("Unsupported index format; rebuild the index")
        for filename in ("vectors.faiss", "chunks.jsonl"):
            if sha256(version / filename) != self.metadata["files"][filename]:
                raise ValueError("Index integrity check failed; rebuild the index")
        self.chunks = [json.loads(line) for line in (version / "chunks.jsonl").read_text(encoding="utf-8").splitlines()]
        self.index = faiss.read_index(str(version / "vectors.faiss"))
        if self.index.ntotal != len(self.chunks) or self.index.d != self.metadata["dimension"]:
            raise ValueError("Index and metadata disagree")
        self.encoder = encoder if encoder is not None else load_encoder(self.metadata["model"], self.metadata["revision"])

    def search(self, question: str, k: int = 5) -> list[dict]:
        if not question.strip() or k < 1:
            raise ValueError("Question must be nonempty and k must be positive")
        query = encode(self.encoder, [question], "query")
        if query.shape[1] != self.index.d:
            raise ValueError("Embedding dimension mismatch; rebuild index")
        scores, positions = self.index.search(query, min(k, self.index.ntotal))
        return [{**self.chunks[int(position)], "score": float(score)}
                for score, position in zip(scores[0], positions[0]) if position >= 0]
