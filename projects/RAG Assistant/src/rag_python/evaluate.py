"""Retrieval-only evaluation against independently annotated PDF evidence."""
import json
import unicodedata


def normalize(text: str) -> str:
    return " ".join(unicodedata.normalize("NFKC", text).casefold().split())


def relevant(hit: dict, evidence: dict) -> bool:
    return (hit["document_id"] == evidence["document_id"] and hit["page"] == evidence["page"]
            and normalize(evidence["quote"]) in normalize(hit["text"]))


def evaluate(retriever, questions: list[dict], k: int = 5) -> dict:
    if not questions or k < 1:
        raise ValueError("Evaluation requires questions and a positive k")
    rows = []
    for question in questions:
        hits = retriever.search(question["question"], k)
        gold = question["evidence"]
        row = {"id": question["id"], "question": question["question"],
               "answerable": question["answerable"], "hits": hits}
        if question["answerable"]:
            if not gold:
                raise ValueError("Answerable questions require gold evidence")
            ranks = [rank for rank, hit in enumerate(hits, 1) if any(relevant(hit, e) for e in gold)]
            row.update({"hit": bool(ranks), "reciprocal_rank": 1 / min(ranks) if ranks else 0,
                        "evidence_recall": sum(any(relevant(h, e) for h in hits) for e in gold) / len(gold)})
        rows.append(row)
    positives = [row for row in rows if row["answerable"]]
    if not positives:
        raise ValueError("No answerable questions; retrieval metrics would be undefined")
    return {"k": k, "answerable_count": len(positives), "unanswerable_count": len(rows) - len(positives),
            "hit_at_k": sum(row["hit"] for row in positives) / len(positives),
            "mrr_at_k": sum(row["reciprocal_rank"] for row in positives) / len(positives),
            "evidence_recall_at_k": sum(row["evidence_recall"] for row in positives) / len(positives),
            "note": "Negatives are recorded for manual abstention checks; no generation or answer correctness is scored.",
            "results": rows}


def read_questions(path):
    questions = [json.loads(line) for line in path.read_text(encoding="utf-8").splitlines() if line.strip()]
    if len({q["id"] for q in questions}) != len(questions):
        raise ValueError("Duplicate question IDs")
    return questions
