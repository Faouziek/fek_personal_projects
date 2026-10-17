import argparse
import json
import os
from pathlib import Path
import sys

import httpx

from .corpus import download, extract_pages, write_jsonl
from .ollama import DEFAULT_LLM, Ollama


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description="PDF RAG local — Windows / Linux")
    parser.add_argument("--root", type=Path, default=Path.cwd(), help="Project directory (default: current directory)")
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("download", help="Download and verify corpus PDFs")
    commands.add_parser("extract", help="Extract pages into data/pages.jsonl")
    index = commands.add_parser("index", help="Extract, token-chunk, embed and index")
    index.add_argument("--model", default="intfloat/multilingual-e5-small", help="E5-compatible Sentence Transformers model")
    index.add_argument("--revision", default=None, help="Hugging Face revision; default E5 model is pinned")
    index.add_argument("--chunk-size", type=int, default=300)
    index.add_argument("--overlap", type=int, default=50)
    index.add_argument("--batch-size", type=int, default=16)
    for name in ("search", "ask"):
        sub = commands.add_parser(name)
        sub.add_argument("question")
        sub.add_argument("-k", type=int, default=5, choices=range(1, 11), metavar="1..10")
        if name == "ask":
            sub.add_argument("--llm", default=DEFAULT_LLM)
            sub.add_argument("--host", default=os.environ.get("OLLAMA_HOST", "http://127.0.0.1:11434"))
    pull = commands.add_parser("pull-model", help="Import the LLM into a running Ollama server")
    pull.add_argument("--llm", default=DEFAULT_LLM)
    pull.add_argument("--host", default=os.environ.get("OLLAMA_HOST", "http://127.0.0.1:11434"))
    gguf = commands.add_parser("import-gguf", help="Import a local GGUF via Ollama (offline alternative to pull)")
    gguf.add_argument("path", type=Path)
    gguf.add_argument("--llm", default=DEFAULT_LLM)
    gguf.add_argument("--host", default=os.environ.get("OLLAMA_HOST", "http://127.0.0.1:11434"))
    ev = commands.add_parser("evaluate", help="Measure retrieval on reference questions")
    ev.add_argument("--questions", type=Path, default=Path("evaluation/questions.jsonl"))
    ev.add_argument("-k", type=int, default=5)
    ev.add_argument("--output", type=Path, default=Path("artifacts/evaluation.json"))
    args = parser.parse_args(argv)
    root = args.root.resolve()
    os.environ.setdefault("HF_HOME", str(root / ".cache/huggingface"))
    os.environ.setdefault("HF_HUB_DISABLE_XET", "1")
    try:
        if args.command == "download":
            result = {"pdfs": [str(path) for path in download(root)]}
        elif args.command == "extract":
            pages = extract_pages(root)
            write_jsonl(root / "data/pages.jsonl", pages)
            result = {"pages": len(pages), "output": str(root / "data/pages.jsonl")}
        elif args.command == "index":
            from .index import DEFAULT_MODEL, DEFAULT_REVISION, build
            revision = args.revision or (DEFAULT_REVISION if args.model == DEFAULT_MODEL else "main")
            result = build(root, args.model, revision, args.chunk_size, args.overlap, args.batch_size)
        elif args.command == "import-gguf":
            path = args.path if args.path.is_absolute() else root / args.path
            result = Ollama(args.host, args.llm).import_gguf(path)
        elif args.command == "pull-model":
            result = Ollama(args.host, args.llm).pull()
        else:
            from .index import Retriever
            retriever = Retriever(root)
            if args.command == "search":
                result = retriever.search(args.question, args.k)
            elif args.command == "ask":
                result = Ollama(args.host, args.llm).answer(args.question, retriever.search(args.question, args.k))
            else:
                from .evaluate import evaluate, read_questions
                questions = args.questions if args.questions.is_absolute() else root / args.questions
                result = evaluate(retriever, read_questions(questions), args.k)
                output = args.output if args.output.is_absolute() else root / args.output
                output.parent.mkdir(parents=True, exist_ok=True)
                output.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding="utf-8")
                result = {key: value for key, value in result.items() if key != "results"}
                result["output"] = str(output)
        print(json.dumps(result, ensure_ascii=False, indent=2))
        return 0
    except (OSError, ValueError, RuntimeError, KeyError, httpx.HTTPError) as exc:
        print(f"Erreur : {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
