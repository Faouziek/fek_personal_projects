"""Download the official Qwen GGUF for a supported local Ollama import.

Run from the project root: uv run --frozen python scripts/download_gguf.py
"""
import hashlib
import os
from pathlib import Path
import tempfile

import httpx

URL = ("https://huggingface.co/Qwen/Qwen2.5-3B-Instruct-GGUF/resolve/"
       "7dabda4d13d513e3e842b20f0d435c732f172cbe/qwen2.5-3b-instruct-q4_k_m.gguf")
SHA256 = "626b4a6678b86442240e33df819e00132d3ba7dddfe1cdc4fbb18e0a9615c62d"


def digest(path):
    value = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def main():
    path = Path("artifacts/llm/qwen2.5-3b-instruct-q4_k_m.gguf")
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists():
        if digest(path) != SHA256:
            raise ValueError("Existing GGUF checksum mismatch; inspect before removing it")
        print(f"Already verified: {path}")
        return
    fd, temporary = tempfile.mkstemp(suffix=".part", dir=path.parent)
    try:
        h = hashlib.sha256()
        size = 0
        with os.fdopen(fd, "wb") as output, httpx.Client(follow_redirects=True, timeout=180) as client:
            with client.stream("GET", URL) as response:
                response.raise_for_status()
                for block in response.iter_bytes(1024 * 1024):
                    size += len(block)
                    if size > 3 * 1024**3:
                        raise ValueError("GGUF exceeds expected size limit")
                    output.write(block)
                    h.update(block)
        if h.hexdigest() != SHA256:
            raise ValueError("Downloaded GGUF checksum mismatch")
        os.replace(temporary, path)
    finally:
        Path(temporary).unlink(missing_ok=True)
    print(f"Verified official Qwen GGUF: {path}")


if __name__ == "__main__":
    main()
