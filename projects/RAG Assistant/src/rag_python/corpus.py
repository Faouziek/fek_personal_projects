"""Verified PDF download and page-preserving text extraction."""
import hashlib
import json
import os
from pathlib import Path
import tempfile

import httpx
import pymupdf


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def read_manifest(root: Path) -> dict:
    manifest = json.loads((root / "corpus/manifest.json").read_text(encoding="utf-8"))
    if manifest.get("version") != 1 or not manifest.get("documents"):
        raise ValueError("Unsupported or empty corpus manifest")
    ids, names = set(), set()
    for doc in manifest["documents"]:
        name = doc["filename"]
        if not name or Path(name).name != name or "/" in name or "\\" in name:
            raise ValueError("PDF filename must be a plain filename")
        if doc["id"] in ids or name in names:
            raise ValueError("Duplicate corpus ID or filename")
        ids.add(doc["id"])
        names.add(name)
    return manifest


def download(root: Path) -> list[Path]:
    target = root / "data/pdfs"
    target.mkdir(parents=True, exist_ok=True)
    paths = []
    with httpx.Client(follow_redirects=True, timeout=120) as client:
        for doc in read_manifest(root)["documents"]:
            path = target / doc["filename"]
            if path.exists():
                if sha256(path) != doc["sha256"]:
                    raise ValueError(f"Checksum mismatch: {path}. Remove it explicitly to download again.")
                paths.append(path)
                continue
            fd, temporary = tempfile.mkstemp(dir=target, suffix=".part")
            try:
                total = 0
                with os.fdopen(fd, "wb") as output:
                    with client.stream("GET", doc["url"]) as response:
                        response.raise_for_status()
                        for block in response.iter_bytes():
                            total += len(block)
                            if total > 100 * 1024 * 1024:
                                raise ValueError("PDF exceeds the 100 MiB download limit")
                            output.write(block)
                temp_path = Path(temporary)
                if sha256(temp_path) != doc["sha256"]:
                    raise ValueError(f"Checksum mismatch for {doc['id']}")
                with temp_path.open("rb") as stream:
                    if not stream.read(1024).lstrip().startswith(b"%PDF-"):
                        raise ValueError("Downloaded file is not a PDF")
                os.replace(temp_path, path)
            finally:
                Path(temporary).unlink(missing_ok=True)
            paths.append(path)
    return paths


def extract_pages(root: Path) -> list[dict]:
    pages = []
    for doc in read_manifest(root)["documents"]:
        path = root / "data/pdfs" / doc["filename"]
        if not path.exists():
            raise FileNotFoundError(f"Missing {path}; run `rag download` first")
        if sha256(path) != doc["sha256"]:
            raise ValueError(f"Checksum mismatch: {path}")
        with pymupdf.open(path) as pdf:
            if len(pdf) != doc["pages"]:
                raise ValueError(f"Unexpected page count for {doc['id']}")
            for number, page in enumerate(pdf, 1):
                # Use physical PDF pages (one-based), not printed chapter numbers.
                text = " ".join(page.get_text("text", sort=True).split())
                if text:
                    pages.append({"document_id": doc["id"], "source": doc["filename"],
                                  "url": doc["url"], "page": number, "text": text})
    if not pages:
        raise ValueError("No extractable text: scanned PDFs need OCR before indexing")
    return pages


def write_jsonl(path: Path, rows: list[dict]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("".join(json.dumps(row, ensure_ascii=False) + "\n" for row in rows), encoding="utf-8")
