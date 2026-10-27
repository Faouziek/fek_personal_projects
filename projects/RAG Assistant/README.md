# Local Python RAG — Windows and Linux

A PDF-based retrieval-augmented generation (RAG) project featuring text extraction with PyMuPDF, token-based chunking, **Sentence Transformers** embeddings, a **FAISS** index, answer generation with **Ollama**, and document/page citations.

## Requirements

- **Windows 10/11 x64** or a recent **Linux x86_64** distribution (glibc >= 2.28), with **Python 3.11 or 3.12**.
- [uv](https://docs.astral.sh/uv/getting-started/installation/) for reproducible dependency installation.
- [Ollama](https://ollama.com/download), installed using the native installer for your operating system.
- A suitable CPU; a GPU is optional for Ollama. At least **8 GB RAM** (16 GB recommended) and approximately **10 GB of free disk space**, depending on caches and installers.
- An internet connection for the initial downloads only. No API key is required.

## Installation

Run all commands below **from the repository root**, on the `preparation-projet` branch. Administrator privileges are not required for the Python project.

```sh
uv sync --frozen --extra dev
```

`uv.lock` pins dependency versions and package hashes for both operating systems and supported Python versions. PyTorch is installed from the official CPU package index, without CUDA dependencies. `uv run --frozen` automatically uses `.venv`, so there is no need to activate the environment in PowerShell or change its `ExecutionPolicy`.

Alternatively, without uv (dependencies are not locked), create and activate a virtual environment, then run:

```sh
python -m pip install torch --index-url https://download.pytorch.org/whl/cpu
python -m pip install -e ".[dev]"
```

On Linux, if Ollama is not installed system-wide, the optional helper below installs version **0.40.5** into a local directory and verifies its pinned official SHA-256 checksum. It requires `curl`, `tar`, `zstd`, and `sha256sum`:

```sh
bash scripts/install_ollama_linux.sh
```

In that case, replace `ollama` with `.cache/ollama/bin/ollama` in the startup commands. On Windows, use the official installer and reopen the terminal.

## End-to-End Usage

Start Ollama in **another terminal** if its application or service is not already running:

```sh
ollama serve
```

Then run the following commands from the repository root (the same commands work in PowerShell and Bash):

```sh
uv run --frozen rag download
uv run --frozen rag extract
uv run --frozen rag index
uv run --frozen rag pull-model
uv run --frozen rag search "What is recursion?" -k 5
uv run --frozen rag ask "What is recursion?" -k 5
uv run --frozen python -m pytest -q
```

`index` also performs extraction, so `extract` is provided separately for inspecting the extracted pages. The first indexing run downloads the E5 model weights (approximately 470 MB). `pull-model` downloads the LLM through Ollama's local API; the equivalent native command is `ollama pull qwen2.5:3b`.

Downloading, extraction, indexing, and search work without Ollama. Only LLM installation and answer generation require the Ollama service.

`ask` requests a constrained JSON response from Ollama, including the generated answer, source identifiers, and an `insufficient_context` flag. The output also includes retrieved excerpts, similarity scores, page numbers, and source labels such as `[S1]` and `[S2]`. Citations selected by the model are shown in the answer. Missing or invalid citations are flagged, although a syntactically valid citation does not guarantee that the claim is correct.

The prompt instructs the model to abstain when the documents do not provide sufficient information. Instructions found within PDFs are treated as document content, but resistance to prompt injection is not guaranteed.

To select a different Ollama server or model:

```sh
uv run --frozen rag pull-model --llm qwen2.5:7b --host http://127.0.0.1:11434
uv run --frozen rag ask "What is a tuple?" --llm qwen2.5:7b
```

`OLLAMA_HOST` is also supported (as a complete HTTP/HTTPS URL). Larger models require more memory. When running the CLI from another directory, use `rag --root PATH_TO_REPOSITORY ...`.

## How It Works and Project Files

```text
Verified PDF → pages with source metadata → 300-token chunks, 50-token overlap
             → E5: passage: prefix, normalized vectors → FAISS IndexFlatIP
Question     → E5: query: prefix → top K matches → Ollama → sourced answer
```

Chunking respects page boundaries and uses the model's actual tokenizer. Chunk sizes remain within E5's 512-token limit, accounting for prefixes and special tokens. FAISS performs exact inner-product search over normalized vectors, equivalent to cosine similarity. This straightforward approach is suitable for a small corpus and requires no model training.

- `corpus/manifest.json`: document sources, licenses, checksums, and page counts.
- `src/rag_python/`: CLI, downloading, extraction, indexing, and the Ollama client.
- `data/pdfs/`, `data/pages.jsonl`: downloaded documents and extracted text, excluded from Git.
- `artifacts/index/generation-*/`: FAISS index, JSONL chunks, and metadata.
- `artifacts/index/CURRENT`: active index generation, updated atomically after writing.
- `.cache/huggingface/`: cached embedding model weights, excluded from Git.

Previous index generations are retained to allow manual rollback. The revision of the embedding model actually loaded is recorded and reused during retrieval. Rebuild the index whenever the corpus, embedding model, or chunking configuration changes:

```sh
uv run --frozen rag index --chunk-size 250 --overlap 40 --batch-size 8
```

Alternative embedding models must follow the E5 `query:` / `passage:` prefix convention, and `--revision` can pin a specific model commit. Only load trusted local indexes: checksums can detect accidental corruption but do not provide cryptographic authenticity.

## Free Corpus Reference

The example corpus is the chapter-by-chapter glossary from *Think Python* by Allen B. Downey, downloaded from a pinned revision of the official repository. Questions can be asked in either English or French.
